import socket
from tokenize import Single
import numpy as np
from jaxtyping import Int, Float
from loguru import logger
import anyio
import asyncio
from anyio import create_udp_socket, run, create_memory_object_stream
from anyio.abc import UDPSocket
from anyio.streams.memory import MemoryObjectReceiveStream, MemoryObjectSendStream
from anyio.to_thread import run_sync
from threading import Thread
import click
import cbor2 as cbor
from dataclasses import dataclass
from pydantic import BaseModel
import streamlit as st
from streamlit import config as st_config
from streamlit.web.bootstrap import run as st_run
from streamlit.runtime.scriptrunner.script_run_context import add_script_run_ctx, get_script_run_ctx
from streamlit import session_state
import pyarrow as pa
import pyarrow.parquet as pq
from fastparquet import write
from plotly.graph_objects import Scatter
from typing import Any, Dict, Tuple, List, TypedDict
from time import time
from typeguard import typechecked, check_type
from npy_append_array import NpyAppendArray
import npy_append_array

NDArray = np.ndarray

ArrayType = Int[NDArray, "... 2"]

DEFAULT_HOST = "0.0.0.0"
DEFAULT_PORT = 8080

BATCH_SIZE = 5_000


async def run_udp(host: str, port: int,
                  channel: MemoryObjectSendStream[NDArray]):
    logger.info(f"Listening on {host}:{port}")
    # https://stackoverflow.com/questions/64791558/create-parquet-files-from-stream-in-python-in-memory-efficient-manner
    # https://github.com/dask/fastparquet/issues/476
    schema = pa.schema([("red", pa.uint32())])
    acc = np.zeros((0, 2), dtype=np.uint32)

    async def sock_recv_gen(sock: UDPSocket):
        while True:
            b, addr = await sock.receive()
            yield b, addr

    try:
        async with await create_udp_socket(family=socket.AF_INET,
                                           local_port=port,
                                           local_host=host) as sock:
            async for b, addr in sock_recv_gen(sock):
                logger.info(f"len(data)={len(b)} from {addr}")
                data: any = cbor.loads(b)  # type: ignore
                strides = list(data[0])
                arr = np.array(data[1]).reshape((-1, strides[1]))
                await channel.send(arr)
                acc = np.vstack((acc, arr))
                count = acc.shape[0]
                if count >= BATCH_SIZE:

                    def save_as_np():
                        logger.info("write chunk to data.npy")
                        with NpyAppendArray("data.npy") as npaa:
                            npaa.append(acc)

                    await run_sync(save_as_np)
                    acc = np.zeros((0, 2), dtype=np.uint32)
    except KeyboardInterrupt:
        # write the remaining data
        logger.warning("KeyboardInterrupt. Try to write the remaining data.")
    except Exception as e:
        logger.exception(e)


# https://github.com/streamlit/streamlit/issues/1792


def st_main(channel: MemoryObjectReceiveStream[NDArray]):
    logger.info("Streamlit main")
    st.title("Pulse")
    red_chart = st.empty()
    ir_chart = st.empty()
    acc = np.zeros((0, 2))

    def sync_iter_channel():
        while True:
            try:
                data = channel.receive_nowait()
                yield data
            except anyio.WouldBlock:
                pass

    async def async_iter_channel():
        async for data in channel:
            yield data

    # for 400 samples per second
    # interval 2.5
    # for 800 samples per second
    # interval 1.25
    for data in sync_iter_channel():
        WINDOW_SIZE = 3000
        acc = np.vstack((acc, data))
        count = acc.shape[0]
        if count > WINDOW_SIZE:
            acc = acc[-WINDOW_SIZE:]
        red = acc[:, 0]
        ir = acc[:, 1]
        xs = np.arange(count) * 1.25

        red_fig = {
            "data": Scatter(x=xs, y=red, name="Red"),
        }
        ir_fig = {
            "data": Scatter(x=xs, y=ir, name="IR"),
        }
        red_chart.plotly_chart(red_fig)
        ir_chart.plotly_chart(ir_fig)


# https://discuss.streamlit.io/t/how-can-i-invoke-streamlit-from-within-python-code/6612/6
# https://github.com/streamlit/streamlit/issues/6290


@dataclass
class Params:
    host: str
    port: int

    def __hash__(self):
        return hash((self.host, self.port))


class AppState(TypedDict):
    thread: Thread
    receiver: MemoryObjectReceiveStream[NDArray]


# https://discuss.streamlit.io/t/how-to-have-global-variable-outside-of-session-state/54885/2
# https://docs.streamlit.io/library/api-reference/performance/st.cache_resource
# https://docs.streamlit.io/library/advanced-features/caching
@st.cache_resource
def init(params: Params) -> AppState:
    sender, receiver = create_memory_object_stream[NDArray]()
    t = Thread(target=run, args=(run_udp, params.host, params.port, sender))
    t.start()
    return {"thread": t, "receiver": receiver}


@click.command()
@click.option("--host", default=DEFAULT_HOST, help="Host address")
@click.option("--port", default=DEFAULT_PORT, help="Port number")
def main(host: str, port: int):
    params = Params(host=host, port=port)
    app_state = init(params)
    st_main(app_state["receiver"])


if __name__ == "__main__":
    main() # pylint: disable=no-value-for-parameter

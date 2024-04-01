import socket
from tokenize import Single
from altair import sample
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
import json
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
from typing import Any, Dict, Optional, Tuple, List, TypedDict
from time import time
from typeguard import typechecked, check_type
from npy_append_array import NpyAppendArray
from pathlib import Path
import npy_append_array

NDArray = np.ndarray

ArrayType = Int[NDArray, "... 2"]

DEFAULT_HOST = "0.0.0.0"
DEFAULT_PORT = 8080


class PulseData(TypedDict):
    sample_rate: int | float
    data: NDArray


async def run_udp(host: str, port: int,
                  channel: MemoryObjectSendStream[PulseData]):
    logger.info(f"Listening on {host}:{port}")
    # https://stackoverflow.com/questions/64791558/create-parquet-files-from-stream-in-python-in-memory-efficient-manner
    # https://github.com/dask/fastparquet/issues/476
    schema = pa.schema([("red", pa.uint32())])
    acc = np.zeros((0, 2), dtype=np.uint32)
    WINDOW_SIZE_SEC = 10
    # will be set after the first data arrives
    # as a reference value
    sr: Optional[int] = None
    # remove data.npy if it exists
    if Path("data.npy").exists():
        Path("data.npy").unlink()

    async def sock_recv_gen(sock: UDPSocket):
        while True:
            b, addr = await sock.receive()
            yield b, addr

    try:
        async with await create_udp_socket(family=socket.AF_INET,
                                           local_port=port,
                                           local_host=host) as sock:
            async for b, addr in sock_recv_gen(sock):
                data: any = cbor.loads(b)  # type: ignore
                header = list(data[0])
                strides = list(header[0])
                # I expect the sample rate to be consistent
                sample_rate = header[1]
                if sr is None:
                    sr = sample_rate
                assert sr == sample_rate, f"sample rate inconsistency: {sr} != {sample_rate}"
                batch_size = WINDOW_SIZE_SEC * sample_rate
                logger.info(
                    f"len(data)={len(b)} from {addr}; strides={strides}; sample_rate={sample_rate}"
                )
                arr = np.array(data[1]).reshape((-1, strides[1]))
                await channel.send({"sample_rate": sample_rate, "data": arr})
                acc = np.vstack((acc, arr))
                count = acc.shape[0]
                if count >= batch_size:

                    def save_as_np():
                        logger.info("write chunk to data.npy")
                        with NpyAppendArray("data.npy") as npaa:
                            npaa.append(acc)
                        with open("data.npy.json", "w") as f:
                            json.dump({"sample_rate": sr}, f)

                    await run_sync(save_as_np)
                    acc = np.zeros((0, 2), dtype=np.uint32)
    except KeyboardInterrupt:
        # write the remaining data
        logger.warning("KeyboardInterrupt. Try to write the remaining data.")
    except Exception as e:
        logger.exception(e)


# https://github.com/streamlit/streamlit/issues/1792


def st_main(channel: MemoryObjectReceiveStream[PulseData]):
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
        WINDOW_SIZE_SEC = 10
        sample_rate = data["sample_rate"]
        window_size_sample = WINDOW_SIZE_SEC * sample_rate
        sample_interval = 1 / sample_rate
        arr = data["data"]
        acc = np.vstack((acc, arr))
        count = acc.shape[0]
        if count > window_size_sample:
            acc = acc[-window_size_sample:]
        red = acc[:, 0]
        ir = acc[:, 1]
        xs = np.arange(len(red)) * sample_interval

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
    receiver: MemoryObjectReceiveStream[PulseData]


# https://discuss.streamlit.io/t/how-to-have-global-variable-outside-of-session-state/54885/2
# https://docs.streamlit.io/library/api-reference/performance/st.cache_resource
# https://docs.streamlit.io/library/advanced-features/caching
@st.cache_resource
def init(params: Params) -> AppState:
    sender, receiver = create_memory_object_stream[PulseData]()
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
    main()  # pylint: disable=no-value-for-parameter

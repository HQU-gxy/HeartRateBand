import socket
from idna import check_nfc
import numpy as np
from jaxtyping import Int, Float
from loguru import logger
import anyio
from anyio import create_udp_socket, run, create_memory_object_stream
from anyio.streams.memory import MemoryObjectReceiveStream, MemoryObjectSendStream
from anyio.to_thread import run_sync
from threading import Thread
import click
import cbor2 as cbor
import streamlit as st
from streamlit.runtime.scriptrunner.script_run_context import add_script_run_ctx, get_script_run_ctx
from plotly.graph_objects import Scatter
from typing import Tuple, List
from time import time
from typeguard import typechecked, check_type

NDArray = np.ndarray

ArrayType = Int[NDArray, "... 2"]

DEFAULT_HOST = "0.0.0.0"
DEFAULT_PORT = 8080


async def run_udp(host: str, port: int,
                  channel: MemoryObjectSendStream[NDArray]):
    logger.info(f"Listening on {host}:{port}")
    async with await create_udp_socket(family=socket.AF_INET,
                                       local_port=port,
                                       local_host=host) as sock:
        while True:
            b, addr = await sock.receive()
            logger.info(f"len(data)={len(b)} from {addr}")
            data: any = cbor.loads(b)  # type: ignore
            strides = list(data[0])
            arr = np.array(data[1]).reshape((-1, strides[1]))
            await channel.send(arr)


def st_main(channel: MemoryObjectReceiveStream[NDArray]):
    logger.info("Streamlit main")
    st.title("Pulse")
    arr = np.zeros((0, 2))

    def iter_channel():
        while True:
            try:
                data = channel.receive_nowait().reshape((-1, 2))
                yield data
            except anyio.WouldBlock:
                pass


@click.command()
@click.option("--host", default=DEFAULT_HOST, help="Host address")
@click.option("--port", default=DEFAULT_PORT, help="Port number")
def main(host: str, port: int):
    sender, receiver = create_memory_object_stream[NDArray]()
    ctx = get_script_run_ctx()
    t = Thread(target=lambda: st_main(receiver))
    add_script_run_ctx(t, ctx)
    t.start()
    anyio.run(run_udp, host, port, sender)
    t.join()


if __name__ == "__main__":
    main()

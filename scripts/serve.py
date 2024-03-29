import socket
import numpy as np
from jaxtyping import Int, Float
from loguru import logger
from anyio import create_udp_socket, run
import cbor2 as cbor
from typing import Tuple, List
from typeguard import typechecked, check_type

NDArray = np.ndarray

ArrayType = Int[NDArray, "... 2"]


async def main():
    async with await create_udp_socket(family=socket.AF_INET,
                                       local_port=8080,
                                       local_host="0.0.0.0") as sock:
        while True:
            b, addr = await sock.receive()
            data: any = cbor.loads(b)  # type: ignore
            strides = list(data[0])
            arr = np.array(data[1]).reshape((-1, strides[1]))
            logger.info(f"len(data)={len(b)} from {addr}")
            logger.info(f"arr={arr}")


if __name__ == "__main__":
    logger.info("Starting UDP server")
    run(main)

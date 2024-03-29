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
            data, addr = await sock.receive()
            arr = np.array(cbor.loads(data))
            logger.info(f"len(data)={len(data)} from {addr}")
            check_type("arr", arr, ArrayType)
            red = arr[:, 0]
            ir = arr[:, 1]


if __name__ == "__main__":
    logger.info("Starting UDP server")
    run(main)

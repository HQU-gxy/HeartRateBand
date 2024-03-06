import datetime
import time

import paho.mqtt.client as mqtt
from influxdb_client import Point, InfluxDBClient, WriteApi
from influxdb_client.client.write_api import SYNCHRONOUS, ASYNCHRONOUS
from dataclasses import dataclass
from loguru import logger
import cbor2 as cbor
from paho.mqtt.enums import CallbackAPIVersion

READING_TOPIC = "/puncher/reading/#"
READING_COEFF = 10000
READING_PER_KG = 20000


class DBClient():
    def __init__(self):
        self.bucket = "puncher"
        self.token = 'rz88d-IiGwca68haLYKcOtwTvVtWxxaYj3Cm-cwwT8TkP-z0BklHXFcZNz2gJAuGsrFE0Zo-BDUo8T4P0yPnFA=='
        self.org = 'org'
        self.client = InfluxDBClient(url='http://weihua-iot.cn:8086',
                                     token=self.token,
                                     org=self.org)
        self.write_api = self.client.write_api(write_options=SYNCHRONOUS)

    def write2db(self, record: Point):
        self.write_api.write(bucket=self.bucket, org=self.org, record=record)


db_client = DBClient()


def on_message(client: mqtt.Client, userdata: any, message: mqtt.MQTTMessage):
    if ("reading" in message.topic):
        res = cbor.loads(message.payload)
        if hasattr(res, "__len__"):
            arr: list[float] = res
            origin = map(lambda x: x * READING_COEFF, arr)
            kgs = map(lambda x: x / READING_PER_KG, origin)
            print(list(kgs))
            for kg in list(kgs):
                record = (
                    Point("kg")
                    .field("val", kg)
                )
                db_client.write2db(record)


def main():
    client = mqtt.Client(callback_api_version=CallbackAPIVersion.VERSION1)
    client.connect("weihua-iot.cn", 1883, 60)
    topic = READING_TOPIC
    logger.info("subscribing to {}".format(topic))
    client.subscribe(topic)
    client.on_message = on_message
    client.loop_forever()


if __name__ == "__main__":
    main()

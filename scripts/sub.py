import paho.mqtt.client as mqtt
from influxdb_client import Point, InfluxDBClient, WriteApi
from influxdb_client.client.write_api import SYNCHRONOUS, ASYNCHRONOUS
from dataclasses import dataclass
from loguru import logger
import cbor2 as cbor

READING_TOPIC = "/puncher/reading/#"
READING_COEFF = 10000
READING_PER_KG = 20000


def on_message(client: mqtt.Client, userdata: any, message: mqtt.MQTTMessage):
  if ("reading" in message.topic):
    res = cbor.loads(message.payload)
    if hasattr(res, "__len__"):
      arr: list[float] = res
      origin = map(lambda x: x * READING_COEFF, arr)
      kgs = map(lambda x: x / READING_PER_KG, origin)
      print(list(kgs))


def main():
  client = mqtt.Client()
  client.connect("weihua-iot.cn", 1883, 60)
  topic = READING_TOPIC
  logger.info("subscribing to {}".format(topic))
  client.subscribe(topic)
  client.on_message = on_message
  client.loop_forever()


if __name__ == "__main__":
  main()

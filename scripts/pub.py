import time
import cbor2 as cbor
import paho.mqtt.client as mqtt
from loguru import logger
import click
from enum import Enum, auto

COMMAND_TOPIC = "/puncher/command"


class Command(Enum):
  ONCE = 0x12,
  SUCCESSIVE = 0x13,
  STOP = 0x14,
  TARE = 0x20,
  BTN_DISABLE = 0x30,
  BTN_ENABLE = 0x31,


def create_spot_group():
  client = client = mqtt.Client()
  client.connect("weihua-iot.cn", 1883, 60)

  @click.group()
  def spot_group():
    pass

  @spot_group.command()
  def enable_btn():
    logger.info("enable button")
    client.publish(COMMAND_TOPIC, cbor.dumps(Command.BTN_ENABLE.value))

  @spot_group.command()
  def disable_btn():
    logger.info("disable button")
    client.publish(COMMAND_TOPIC, cbor.dumps(Command.BTN_DISABLE.value))

  @spot_group.command()
  def once():
    logger.info("once")
    client.publish(COMMAND_TOPIC, cbor.dumps(Command.BTN_DISABLE.value))
    client.publish(COMMAND_TOPIC, cbor.dumps(Command.ONCE.value))

  @spot_group.command()
  def successive():
    logger.info("successive")
    client.publish(COMMAND_TOPIC, cbor.dumps(Command.BTN_DISABLE.value))
    client.publish(COMMAND_TOPIC, cbor.dumps(Command.SUCCESSIVE.value))

  @spot_group.command()
  def stop():
    logger.info("stop")
    client.publish(COMMAND_TOPIC, cbor.dumps(Command.STOP.value))
  
  @spot_group.command()
  def tare():
    logger.info("tare")
    client.publish(COMMAND_TOPIC, cbor.dumps(Command.TARE.value))

  return spot_group


if __name__ == "__main__":
  create_spot_group()()

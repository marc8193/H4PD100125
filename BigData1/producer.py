#!/usr/bin/env python3

import csv
import json

from confluent_kafka import Producer

config = {
    "bootstrap.servers": "192.168.39.54:9092,192.168.39.55:9092",
}

producer = Producer(**config)

def delivery_report(err, msg):
    if err is not None:
        print(f"Message delivery failed: {err}")
    else:
        print(f"Message delivered to {msg.topic()} [{msg.partition()}]")

with open("giant.csv", "r") as f:
    reader = csv.DictReader(f, delimiter=';')    
    for index, row in enumerate(reader):
        message = json.dumps(row)
        while True:
            try:
                producer.produce("marc8193.giant.csv", key=str(index),
                                 value=message, callback=delivery_report)
                
                break

            except BufferError:
                producer.poll(0.1)
                        
producer.flush()

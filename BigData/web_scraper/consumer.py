#!/usr/bin/env python3

import csv
import sqlite3
import pathlib

csv_file = "computere.csv"

with open(csv_file, newline="") as f:
    reader = csv.reader(f)
    headers = next(reader)

    connection = sqlite3.connect("computere.db")
    cursor = connection.cursor()

    create_table_query = (
        "CREATE TABLE IF NOT EXISTS computere ("
        + ", ".join([f'"{header}" TEXT' for header in headers])
        + ")"
    )

    cursor.execute(create_table_query)

    insert_query = (
        f"INSERT INTO computere ("
        f"{', '.join([f'\"{col}\"' for col in headers])}) "
        f"VALUES ({', '.join(['?'] * len(headers))})"
    )

    for price, title in reader:
        euro_price = f"{round(int(price.split()[0]) * .13)} EUR"
        cursor.execute(insert_query, (euro_price, title))

    connection.commit()
    connection.close()

pathlib.Path(csv_file).unlink()

#!/usr/bin/env python3

import csv
from bs4 import BeautifulSoup

with open("computere.html", "r") as f:
    soup = BeautifulSoup(f, "html.parser")

    lis = soup.find_all("li", class_="group xs:after:border-none relative flex w-full flex-1 snap-start flex-col bg-white px-4 transition-all after:pointer-events-none after:absolute after:inset-0 after:z-1 after:outline-0 after:outline-white after:transition-all xs:pb-6 xs:pt-0 after:border-line pt-4 pb-2 after:border-b-2 product-card-with-hover hover:after:shadow-product-card md:hover:after:outline-8")

    with open("computere.csv", "w", newline="", encoding="utf-8") as csvfile:
        writer = csv.writer(csvfile)
        writer.writerow(["Price", "Laptop Title"])

        for li in lis:
            # Extract product title
            h2 = li.find("h2", class_="font-regular font-bold text-balance break-words text-lg leading-6 line-clamp-3 lg:line-clamp-2")
            title = h2.get_text(strip=True) if h2 else "N/A"

            # Extract product price
            price_span = li.find("span", class_="font-headline text-[2.875rem] leading-[2.875rem] inc-vat")
            price = price_span.get_text(strip=True) if price_span else "N/A"

            writer.writerow([price, title])

print("Data has been written to computere.csv")


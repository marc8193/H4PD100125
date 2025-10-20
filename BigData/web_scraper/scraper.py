#!/usr/bin/env python3

import csv

from bs4 import BeautifulSoup

with open("computere.html", "r") as f:
    soup = BeautifulSoup(f, "html.parser")

    lis = soup.find_all("li", class_="group xs:after:border-none relative flex w-full flex-1 snap-start flex-col bg-white px-4 transition-all after:pointer-events-none after:absolute after:inset-0 after:z-1 after:outline-0 after:outline-white after:transition-all xs:pb-6 xs:pt-0 after:border-line pt-4 pb-2 after:border-b-2 product-card-with-hover hover:after:shadow-product-card md:hover:after:outline-8")

    # Open CSV file for writing
    with open("computere.csv", "w", newline="", encoding="utf-8") as csvfile:
        writer = csv.writer(csvfile)

        # Loop through each <li> and extract <h2> text
        for li in lis:
            h2s = li.find_all("h2", class_="font-regular font-bold text-balance break-words text-lg leading-6 line-clamp-3 lg:line-clamp-2")
            for h2 in h2s:
                title = h2.get_text(strip=True)
                writer.writerow([title])

print("Data has been written to computere.csv")


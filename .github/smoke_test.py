"""Check that the WebAssembly game starts in a browser and reacts to clicks."""
import sys
import time

from playwright.sync_api import sync_playwright

URL = "http://localhost:8000/martiancafetuinematycoon.html"

# The tray, which shows the glasses picked up
TRAY = {"x": 8, "y": 75, "width": 112, "height": 50}
# Where the band plays in the other room
BAND = {"x": 530, "y": 110, "width": 130, "height": 140}

with sync_playwright() as p:
    browser = p.chromium.launch(channel="chrome")
    # The same size as the iframe on the website
    page = browser.new_page(viewport={"width": 952, "height": 602})
    errors = []
    page.on("pageerror", lambda e: errors.append(str(e)))
    page.on("console", lambda m: print("console:", m.text))
    page.goto(URL)
    time.sleep(15)
    page.screenshot(path="screenshot_1_start.png")
    empty_tray = page.screenshot(clip=TRAY)
    no_band = page.screenshot(clip=BAND)
    # Pick up the full glasses on the bar, which fill up at random
    for _ in range(20):
        for x in range(632, 777, 16):
            page.mouse.click(x + 7, 370)
        time.sleep(0.6)
    time.sleep(1)
    page.screenshot(path="screenshot_2_glasses.png")
    full_tray = page.screenshot(clip=TRAY)
    # Go to the room with the band
    page.mouse.click(32, 165)
    time.sleep(1.5)
    page.screenshot(path="screenshot_3_band.png")
    band = page.screenshot(clip=BAND)
    browser.close()

if errors:
    sys.exit(f"Page errors: {errors}")
if empty_tray == full_tray:
    sys.exit("Clicking the glasses on the bar did not put them on the tray")
if no_band == band:
    sys.exit("Clicking the arrow did not go to the room with the band")
print("OK")

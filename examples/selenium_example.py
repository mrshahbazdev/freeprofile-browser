#!/usr/bin/env python3
"""
Selenium WebDriver attach example for FreeProfile Browser.

1. Launch a profile from the dashboard with a remote debugging port, e.g. 9222.
2. Make sure ChromeDriver is on PATH and chromedriver version matches the
   Chromium/CEF version used by FreeProfile Browser.
3. Run this script to attach to the already-running browser instance and drive it.
"""

from selenium import webdriver
from selenium.webdriver.common.by import By

PORT = 9222

options = webdriver.ChromeOptions()
options.add_experimental_option("debuggerAddress", f"127.0.0.1:{PORT}")

driver = webdriver.Chrome(options=options)

try:
    print("Connected to FreeProfile Browser profile")
    driver.get("https://example.com")
    print("Page title:", driver.title)
    # element = driver.find_element(By.CSS_SELECTOR, "h1")
    # print("H1 text:", element.text)
finally:
    # Do not close the browser, only detach.
    pass

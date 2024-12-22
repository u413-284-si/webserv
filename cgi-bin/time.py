#!/usr/bin/env python3

import datetime

print("HTTP/1.1 200 OK\r\n", end="")
print("Content-Type: text/html\r\n", end="")
print("Connection: close\r\n", end="")
print("Cache-Control: no-cache\r\n", end="")
print("Pragma: no-cache\r\n\r\n", end="")
print("<html>")
print("<head>")
print(datetime.datetime.strftime(datetime.datetime.now(), "<h1>  %H:%M:%S </h1>"))

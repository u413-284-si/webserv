#!/usr/bin/env python3

import datetime

#print("HTTP/1.1 200 OK")
print("Content-Type: text/html\r\n\r\n")
print("<html>")
print("<head>")
print(datetime.datetime.strftime(datetime.datetime.now(), "<h1>  %H:%M:%S </h1>"))

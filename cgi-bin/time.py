#!/usr/bin/env python3

import datetime

print("Status: 200 Hobdy Dopdy\r\n", end="")
print("Content-Type: text/html\r\n", end="")
print("Connection: close\r\n", end="")
print("Cache-Control: no-cache\r\n", end="")
print("Pragma: no-cache\r\n\r\n", end="")
print("<html>")
print("<head>")
print("<title>Current time</title>")
print("</head>")
print("<body>")
print("<h1>The current time is: " + datetime.datetime.strftime(datetime.datetime.now(), "%H:%M:%S") + "</h1>")
print("</body>")
print("</html>")

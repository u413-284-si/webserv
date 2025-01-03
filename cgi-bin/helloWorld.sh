#!/bin/bash

# Output the Content-Type header
printf "Content-Type: text/html\r\n"
printf "\r\n"

# Output the HTML content
cat <<EOF
<html>
<head>
<title>Hello World - First CGI Program</title>
</head>
<body>
<h2>Hello World! This is my first CGI program</h2>
</body>
</html>
EOF

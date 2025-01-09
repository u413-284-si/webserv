#!/bin/bash

# Output the Content-Type header
printf "Status: 400 YOLO\r\n"
printf "BB: 4Life\r\n"
printf "\r\n"

# Output the HTML content
cat <<EOF
<html>
<head>
<title>Bad BOI</title>
</head>
<body>
<h2>Whatyou gonna do when they come for you</h2>
</body>
</html>
EOF

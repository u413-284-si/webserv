#!/bin/bash

# Output content type header
printf "Content-Type: text/html\r\n"
printf "\r\n"

# Function to URL-decode a string
urldecode() {
    local decoded="${1//+/ }"
    printf '%b' "${decoded//%/\\x}"
}

# Read input based on the request method
if [ "$REQUEST_METHOD" = "POST" ]; then
    # Read the POST data
    read -n "$CONTENT_LENGTH" POST_DATA
    # Extract 'text' from POST data
    INPUT_TEXT=$(echo "$POST_DATA" | sed -n 's/^.*text=\(.*\)$/\1/p')
elif [ "$REQUEST_METHOD" = "GET" ]; then
    # Extract 'text' from the query string
    INPUT_TEXT=$(echo "$QUERY_STRING" | sed -n 's/^.*text=\(.*\)$/\1/p')
else
    INPUT_TEXT=""
fi

# Decode the URL-encoded input
DECODED_TEXT=$(urldecode "$INPUT_TEXT")

# Convert the input text to uppercase
UPPERCASE_TEXT=$(echo "$DECODED_TEXT" | tr '[:lower:]' '[:upper:]')

# Generate the HTML response
cat <<EOF
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Converted Text</title>
</head>
<body>
    <h1>Converted to Uppercase</h1>
    <p>${UPPERCASE_TEXT}</p>
</body>
</html>
EOF

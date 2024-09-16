#!/bin/bash

# Set the content type for the response (CGI header)
echo "Content-Type: text/plain"
echo ""

# Read from stdin and convert to uppercase
cat | tr '[:lower:]' '[:upper:]'
#!/bin/bash

# Set the content type for the response (CGI header)
printf "Content-Type: text/plain\r\n"
printf "\r\n"

# Read from stdin and convert to uppercase
cat | tr '[:lower:]' '[:upper:]'
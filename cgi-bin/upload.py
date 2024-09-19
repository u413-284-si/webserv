#!/usr/bin/python3

import os
import sys
from urllib.parse import parse_qs

# Define the directory where the file will be saved
save_dir = os.getcwd() + '/tmp/'

# Ensure the directory exists
if not os.path.exists(save_dir):
    os.makedirs(save_dir)

# Retrieve the query string from the environment variable
query_string = os.environ.get('QUERY_STRING', '')

# Parse the query string to extract parameters
params = parse_qs(query_string)

# Get the filename from the query string, or use a default name if not provided
filename = params.get('filename', ['uploaded_file.txt'])[0]

# Create the full path to save the file
file_path = os.path.join(save_dir, filename)

# Set the content type for the response
print("Content-Type: text/html;charset=utf-8")
print()

try:
    # Read from stdin
    content = sys.stdin.read()

    # Write the content to the file
    with open(file_path, 'wb') as f:
        f.write(content.encode('utf-8'))

    # Print success message
    print("<html><body>")
    print("<h1>File successfully uploaded to {}</h1>".format(file_path))
    print("</body></html>")

except Exception as e:
    # Print error message if something goes wrong
    print("<html><body>")
    print("<h1>Error: {}</h1>".format(str(e)))
    print("</body></html>")


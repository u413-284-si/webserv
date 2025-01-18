#!/usr/bin/env python3

import os
import sys
from urllib.parse import parse_qs

# Define the base directory where the file will be saved
base_save_dir = os.path.abspath(os.path.join(os.getcwd(), '../html/uploads/'))

# List of allowed directories
allowed_directories = ['tmp', 'images', 'documents']

# Ensure the base directory exists
if not os.path.exists(base_save_dir):
    os.makedirs(base_save_dir)

# Retrieve the query string from the environment variable
query_string = os.environ.get('QUERY_STRING', '')

# Parse the query string to extract parameters
params = parse_qs(query_string)

# Get the filename from the query string, or use a default name if not provided
filename = params.get('filename', ['uploaded_file.txt'])[0]

# Get the directory from the query string, or set directory to empty if not provided
directory = params.get('directory', [''])[0]

# Sanitize directory input
if directory and directory in allowed_directories:
    directory_path = os.path.join(base_save_dir, directory)
else:
    directory_path = base_save_dir  # Use default if directory is invalid

# Create the full path to save the file
file_path = os.path.join(directory_path, filename)

# Ensure the directory exists
if not os.path.exists(directory_path):
    os.makedirs(directory_path)

# Set the content type for the response
print("Content-Type: text/plain;charset=utf-8\r\n", end="")
print("Location: " + file_path + "\r\n", end="")
print("Access-Control-Allow-Origin: *\r\n", end="")
print("\r\n", end="")

try:
    # Read from stdin
    content = sys.stdin.read()

    # Write the content to the file
    with open(file_path, 'wb') as f:
        f.write(content.encode('utf-8'))

    # Print success message
    print(f"File successfully uploaded to {file_path}")

except Exception as e:
    # Print error message if something goes wrong
    print(f"Error: {str(e)}")

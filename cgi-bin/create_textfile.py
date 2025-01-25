#!/usr/bin/env python3

import os
import sys
from urllib.parse import parse_qs

# Define the base directory where the file will be saved
base_save_dir = os.path.abspath(os.path.join(os.getcwd(), '../html/uploads/'))

# Ensure the base directory exists
os.makedirs(base_save_dir, exist_ok=True)

# List of allowed sub-directories
allowed_sub_directories = ['tmp', 'documents', 'notes']

# Set the Content-Type header for the HTTP response
print("Content-Type: text/html;charset=utf-8\r\n", end="")
# Enable CORS (Cross-Origin Resource Sharing)
print("Access-Control-Allow-Origin: *\r\n", end="")

try:
    # Get the content length from the environment
    content_length = int(os.environ.get("CONTENT_LENGTH", 0))
    if content_length == 0:
        raise ValueError("No form data received")

    # Read the form data from stdin
    form_data = sys.stdin.read(content_length)

    # Parse the form data
    form = parse_qs(form_data)

    # Extract form fields
    filename = form.get("filename", ["default.txt"])[0]
    content = form.get("content", [""])[0]
    directory = form.get("directory", [""])[0]

    # Validate and sanitize inputs
    if not filename:
        raise ValueError("Filename is required")

    # Sanitize filename
    filename = "".join(c for c in filename if c.isalnum() or c in (' ', '.', '_', '-')).strip()
    if not filename.endswith(".txt"):
        filename += ".txt"

    # Validate directory
    if directory in allowed_sub_directories:
        directory_path = os.path.join(base_save_dir, directory)
    else:
        directory_path = base_save_dir  # Use base directory if input is invalid

    # Ensure the target directory exists
    os.makedirs(directory_path, exist_ok=True)

    # Full path to save the file
    file_path = os.path.join(directory_path, filename)

    # Write the content to the text file
    with open(file_path, "w", encoding="utf-8") as f:
        f.write(content)

    # Add location header to the response
    print("Location: " + file_path + "\r\n", end="")
    print("\r\n", end="")
    # Print success message
    print(f'''<html>
                <head>
                    <title>File Created</title>
                </head>
                <body>
                    <h2>Success</h2>
                    <p>File '{filename}' successfully created in '{directory_path}'</p>
                </body>
            </html>''')

except Exception as e:
    # Add bad status error
    print("Status: 500 Internal Server Error\r\n", end="")
    print("\r\n", end="")
    # Print error message
    print(f'''<html>
                <head>
                    <title>Error</title>
                </head>
                <body>
                    <h2>Error: {str(e)}</h2>
                </body>
            </html>''')

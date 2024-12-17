# This module is for succesful POST requests

import requests
import os

def test_POST_simple():
    print("Request for /uploads/testfile.txt")
    # Body to send
    payload = "Beam me up, Scotty!"
    response = requests.post("http://localhost:8080/uploads/testfile.txt", data=payload)

    assert response.status_code == 201
    assert response.headers["location"] == "/uploads/testfile.txt"
    # Check if file exists
    assert os.path.isfile("/workspaces/webserv/html/uploads/testfile.txt")
    # Delete created file
    os.remove("/workspaces/webserv/html/uploads/testfile.txt")

# For encoding chunked
# sa https://requests.readthedocs.io/en/latest/user/advanced/#chunk-encoded-requests
def generate_chunks():
    yield "First chunk of data".encode("utf-8")
    yield "Second chunk of data".encode("utf-8")
    yield "Third chunk of data".encode("utf-8")

def test_POST_chunked_encoding():
    print("Chunked Request for /uploads/testfile_chunked.txt")
    response = requests.post("http://localhost:8080/uploads/testfile_chunked.txt", data=generate_chunks())

    assert response.status_code == 201
    assert response.headers["location"] == "/uploads/testfile_chunked.txt"
    # Check if file exists
    assert os.path.isfile("/workspaces/webserv/html/uploads/testfile_chunked.txt")
    # Delete created file
    os.remove("/workspaces/webserv/html/uploads/testfile_chunked.txt")

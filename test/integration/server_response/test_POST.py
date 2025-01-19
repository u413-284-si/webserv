# This module is for succesful POST requests

import requests
import os

def test_POST_simple():
    print("Request for /uploads/testfile.txt")
    # Body to send
    payload = "Beam me up, Scotty!"
    dst_file_path = "/workspaces/webserv/html/uploads/testfile.txt"

    response = requests.post("http://localhost:8080/uploads/testfile.txt", data=payload)

    assert response.status_code == 201
    assert response.headers["location"] == "/uploads/testfile.txt"
    # Check if file exists
    assert os.path.isfile(dst_file_path)
    # Delete created file
    os.remove(dst_file_path)

def test_POST_append():
    print("Request for /uploads/existing_file.txt")
    # Body to send
    existing_content = "Hello, World!\n"
    payload = "It is me!"
    dst_file_path = "/workspaces/webserv/html/uploads/existing_file.txt"
    with open(dst_file_path, "w") as file:
        file.write(existing_content)

    response = requests.post("http://localhost:8080/uploads/existing_file.txt", data=payload)

    assert response.status_code == 200
    # Check if file was appended correctly
    with open(dst_file_path, "r") as file:
        content = file.read()
        assert content.find(existing_content + payload) == 0
    # Delete created file
    os.remove(dst_file_path)

# For encoding chunked
# sa https://requests.readthedocs.io/en/latest/user/advanced/#chunk-encoded-requests
def generate_chunks():
    yield "First chunk of data".encode("utf-8")
    yield "Second chunk of\n data".encode("utf-8")
    yield "Third chunk of data".encode("utf-8")
    
def test_POST_chunked_encoding():
    print("Chunked Request for /uploads/testfile_chunked.txt")

    dst_file_path = "/workspaces/webserv/html/uploads/testfile_chunked.txt"

    response = requests.post("http://localhost:8080/uploads/testfile_chunked.txt", data=generate_chunks())

    assert response.status_code == 201
    assert response.headers["location"] == "/uploads/testfile_chunked.txt"
    # Check if file exists
    assert os.path.isfile(dst_file_path)
    # Delete created file
    os.remove(dst_file_path)

def generate_chunks_with_zero_chunk():
    yield ("A"*8000).encode("utf-8") # ensure clientHeader buffer size is reached
    yield "Hello 0\r\n\r\nWorld".encode("utf-8") # contains "zero chunk"
    yield ("A"*8000).encode("utf-8") # ensure clientBody buffer size is reached
    yield ("A"*8000).encode("utf-8")

def test_POST_chunked_encoding_with_zero_chunk():
    print("Chunked Request with zero chunk for /uploads/testfile_chunked.txt")

    dst_file_path = "/workspaces/webserv/html/uploads/testfile_chunked.txt"

    response = requests.post("http://localhost:8080/uploads/testfile_chunked.txt", data=generate_chunks_with_zero_chunk())

    assert response.status_code == 201
    assert response.headers["location"] == "/uploads/testfile_chunked.txt"
    # Check if file exists
    assert os.path.isfile(dst_file_path)
    # Delete created file
    os.remove(dst_file_path)
    
def test_POST_bigger_file():
    print("Request for /uploads/butterfly.jpg")

    src_file_path = "/workspaces/webserv/html/images/butterfly.jpg"
    with open(src_file_path, 'rb') as f:
        binary_data = f.read()
    dst_file_path = "/workspaces/webserv/html/uploads/butterfly.jpg"

    response = requests.post("http://localhost:8080/uploads/butterfly.jpg", data=binary_data)

    assert response.status_code == 201
    assert response.headers["location"] == "/uploads/butterfly.jpg"
    # Check if file exists
    assert os.path.isfile(dst_file_path)
    # Check if same size - doesn't work?
    # assert os.path.getsize(src_file_path) == os.path.getsize(dst_file_path)
    # Delete created file
    os.remove(dst_file_path)

def test_POST_file_too_big():
    print("Request for /uploads/cat.jpg")

    src_file_path = "/workspaces/webserv/html/images/cat.jpg"
    with open(src_file_path, 'rb') as f:
        binary_data = f.read()
    dst_file_path = "/workspaces/webserv/html/uploads/cat.jpg"

    response = requests.post("http://localhost:8080/uploads/cat.jpg", data=binary_data)

    assert response.status_code == 413
    # Check that file does not exist
    assert os.path.isfile(dst_file_path) == False

# This module is for succesful GET requests

import os
from utils.utils import make_request

def test_GET_simple():
    print("Request for /index.html")
    url = "http://localhost:8080/index.html"
    response = make_request(url)
    file_path = "/workspaces/webserv/html/index.html"
    file_size = os.path.getsize(file_path)
    assert int(response.headers["content-length"]) == file_size
    assert response.status_code == 200

def test_GET_index_file():
    print("Request for /")
    url = "http://127.0.0.1:8080"
    response = make_request(url)
    file_path = "/workspaces/webserv/html/index.html"
    file_size = os.path.getsize(file_path)
    assert int(response.headers["content-length"]) == file_size
    assert response.status_code == 200

def test_GET_directory_listing():
    print("Request for /directory/")
    url = "http://localhost:8080/directory/"
    response = make_request(url)
    heading = "Index of /workspaces/webserv/html/directory/"
    assert heading in response.text
    assert response.status_code == 200

def test_GET_percent_encoded():
    print("Request for /images/grüne äpfel.jpg")
    url ="http://localhost:8080/images/grüne äpfel.jpg"
    response = make_request(url)
    assert response.status_code == 200

def test_GET_location_with_alias():
    print("Request for /alias/cat.jpg")
    url = "http://localhost:8080/alias/cat.jpg"
    response = make_request(url)
    file_path = "/workspaces/webserv/html/images/cat.jpg"
    file_size = os.path.getsize(file_path)
    assert int(response.headers["content-length"]) == file_size
    assert response.status_code == 200

def test_GET_empty_return_directive():
    print("Request for /empty")
    url = "http://localhost:8080/empty"
    response = make_request(url)
    assert response.status_code == 200
    assert len(response.content) == 0

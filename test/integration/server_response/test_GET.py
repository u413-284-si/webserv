# This module is for succesful GET requests

import os
from utils.utils import make_request
import requests
from utils.utils import parse_http_response
import socket

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
    print("Request for /health")
    url = "http://localhost:8080/health"
    response = make_request(url)
    assert response.status_code == 200
    assert len(response.content) == 0

def test_GET_reuse_connection():
    print("Request for / and then /directory/")
    session = requests.Session()
    base_url = "http://localhost:8080"
    response1 = session.get(f"{base_url}/")
    assert response1.status_code == 200
    response2 = session.get(f"{base_url}/directory/")
    assert response2.status_code == 200
    session.close()

def test_GET_sent_partial_request():
    with socket.create_connection(("localhost", "8080")) as sock:
        partial_request = b"GET / HTTP/1.1\r\n"
        sock.sendall(partial_request)
        partial_request = b"Host: localhost\r\n\r\n"
        sock.sendall(partial_request)

        # Receive the response
        response = parse_http_response(sock)
        print(response["status_code"])
        assert response["status_code"] == 200

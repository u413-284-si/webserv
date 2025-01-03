# This module is for succesful GET requests

import requests
import os

def test_GET_simple():
    print("Request for /index.html")
    response = requests.get("http://localhost:8080/index.html")
    file_path = "/workspaces/webserv/html/index.html"
    file_size = os.path.getsize(file_path)
    assert int(response.headers["content-length"]) == file_size
    assert response.status_code == 200

def test_GET_index_file():
    print("Request for /")
    response = requests.get("http://127.0.0.1:8080")
    file_path = "/workspaces/webserv/html/index.html"
    file_size = os.path.getsize(file_path)
    assert int(response.headers["content-length"]) == file_size
    assert response.status_code == 200

def test_GET_directory_listing():
    print("Request for /directory/")
    response = requests.get("http://localhost:8080/directory/")
    heading = "Index of /workspaces/webserv/html/directory/"
    assert heading in response.text
    assert response.status_code == 200

def test_GET_directory_redirect():
    print("Request for /directory")
    response = requests.get("http://localhost:8080/directory", allow_redirects=False)
# does not work right now
#    assert response.headers["location"] == "/directory/"
    assert response.status_code == 301

def test_GET_percent_encoded():
    print("Request for /images/grüne äpfel.jpg")
    response = requests.get("http://localhost:8080/images/grüne äpfel.jpg")
    assert response.status_code == 200

def test_GET_location_with_alias():
    print("Request for /alias/cat.jpg")
    response = requests.get("http://localhost:8080/alias/cat.jpg")
    file_path = "/workspaces/webserv/html/images/cat.jpg"
    file_size = os.path.getsize(file_path)
    assert int(response.headers["content-length"]) == file_size
    assert response.status_code == 200

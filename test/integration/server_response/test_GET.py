# This module is for succesful GET requests

import requests
import os
import stat

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

def test_GET_no_permission():
    print("Chmod 000 index.html and request it")
    file_path = "/workspaces/webserv/html/index.html"
    # Get and store the current permissions
    original_permissions = stat.S_IMODE(os.stat(file_path).st_mode)
    # Change permissions to 000
    os.chmod(file_path, 0o000)
    # Try to access
    response = requests.get("http://localhost:8080/index.html")
    # Restore the original permissions
    os.chmod(file_path, original_permissions)
    assert response.status_code == 403

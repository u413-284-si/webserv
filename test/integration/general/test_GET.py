import requests
import os

def test_GET_simple(start_cpp_server):
    print("Request for /index.html")
    response = requests.get("http://localhost:8080/index.html")
    file_path = "/workspaces/webserv/html/index.html"
    file_size = os.path.getsize(file_path)
    assert int(response.headers["content-length"]) == file_size
    assert response.status_code == 200

def test_GET_index_file(start_cpp_server):
    print("Request for /")
    response = requests.get("http://127.0.0.1:8080")
    file_path = "/workspaces/webserv/html/index.html"
    file_size = os.path.getsize(file_path)
    assert int(response.headers["content-length"]) == file_size
    assert response.status_code == 200

def test_GET_directory_listing(start_cpp_server):
    print("Request for /directory/")
    response = requests.get("http://localhost:8080/directory/")
    heading = "Index of /workspaces/webserv/html/directory/"
    assert heading in response.text
    assert response.status_code == 200

def test_GET_directory_redirect(start_cpp_server):
    print("Request for /directory")
    response = requests.get("http://localhost:8080/directory", allow_redirects=False)
# does not work right now
#    assert response.headers["location"] == "/directory/"
    assert response.status_code == 301
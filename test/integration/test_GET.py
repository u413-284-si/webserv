import requests

def test_GET_simple(start_cpp_server):
    print("Request for /index.html")
    response = requests.get("http://localhost:8080/index.html")
    assert response.status_code == 200

def test_GET_index_file(start_cpp_server):
    print("Request for /")
    response = requests.get("http://127.0.0.1:8080")
    assert response.status_code == 200

def test_GET_directory_listing(start_cpp_server):
    print("Request for /directory/")
    response = requests.get("http://localhost:8080/directory/")
    assert response.status_code == 200

def test_GET_directory_redirect(start_cpp_server):
    print("Request for /directory")
    response = requests.get("http://localhost:8080/directory")
    assert response.status_code == 301

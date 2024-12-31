# This module is for succesful GET requests

import os

def test_GET_simple(make_request):
    print("Request for /index.html")
    url = "http://localhost:8080/index.html"
    response = make_request(url)
    file_path = "/workspaces/webserv/html/index.html"
    file_size = os.path.getsize(file_path)
    assert int(response.headers["content-length"]) == file_size
    assert response.status_code == 200

def test_GET_index_file(make_request):
    print("Request for /")
    url = "http://127.0.0.1:8080"
    response = make_request(url)
    file_path = "/workspaces/webserv/html/index.html"
    file_size = os.path.getsize(file_path)
    assert int(response.headers["content-length"]) == file_size
    assert response.status_code == 200

def test_GET_directory_listing(make_request):
    print("Request for /directory/")
    url = "http://localhost:8080/directory/"
    response = make_request(url)
    heading = "Index of /workspaces/webserv/html/directory/"
    assert heading in response.text
    assert response.status_code == 200

def test_GET_directory_redirect(make_request):
    print("Request for /directory")
    url = "http://localhost:8080/directory"
    response = make_request(url)
# does not work right now
#    assert response.headers["location"] == "/directory/"
    assert response.status_code == 200

def test_GET_percent_encoded(make_request):
    print("Request for /images/grüne äpfel.jpg")
    url ="http://localhost:8080/images/grüne äpfel.jpg"
    response = make_request(url)
    assert response.status_code == 200

def test_GET_location_with_alias(make_request):
    print("Request for /alias/cat.jpg")
    url = "http://localhost:8080/alias/cat.jpg"
    response = make_request(url)
    file_path = "/workspaces/webserv/html/images/cat.jpg"
    file_size = os.path.getsize(file_path)
    assert int(response.headers["content-length"]) == file_size
    assert response.status_code == 200

def test_GET_directory_forbidden(make_request):
    print("Request for /secret/")
    url = "http://localhost:8080/secret/"
    response = make_request(url)
    assert response.status_code == 403

def test_GET_simple_redirect(make_request):
    print("Request for /redirect")
    url = "http://localhost:8080/redirect"
    response = make_request(url)
    assert response.history[0].status_code == 301
    assert response.history[0].headers["Location"] == "/secret"
    assert response.status_code == 403

def test_GET_return_forbidden_error_page_returns_also(make_request):
    print("Request for /another")
    url = "http://localhost:8080/another"
    response = make_request(url)
    assert response.status_code == 403
    assert "Return Message" in response.text

def test_GET_missing_indices(make_request):
    print("Request for /missingIndex/")
    url = "http://localhost:8080/missingIndex/"
    response = make_request(url)
    assert response.status_code == 403

def test_max_recurision(make_request):
    print("Request for /recursion/")
    url = "http://localhost:8080/recursion/"
    response = make_request(url)
    assert response.status_code == 500

def test_character_device(make_request):
    print("Request for /tty")
    url = "http://localhost:8080/tty"
    response = make_request(url)
    assert response.status_code == 500

def test_GET_empty_return_directive(make_request):
    print("Request for /empty")
    url = "http://localhost:8080/empty"
    response = make_request(url)
    assert response.status_code == 200
    assert len(response.content) == 0

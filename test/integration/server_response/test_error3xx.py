# This module is for requests which result in status code 3xx

from utils.utils import make_request

def test_GET_directory_redirect():
    print("Request for /directory")
    url = "http://localhost:8080/directory"
    response = make_request(url, allow_redirects=False)
    assert response.headers["location"] == "/directory/"
    assert response.status_code == 301

def test_GET_simple_redirect():
    print("Request for /redirect")
    url = "http://localhost:8080/redirect"
    response = make_request(url, allow_redirects=False)
    assert response.status_code == 301
    assert response.headers["Location"] == "/secret"
    
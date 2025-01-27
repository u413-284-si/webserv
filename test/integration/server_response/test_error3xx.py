# This module is for requests which result in status code 3xx

from utils.utils import make_request

def test_3xx_directory_redirect():
    print("Request for /directory")
    url = "http://localhost:8080/directory"
    response = make_request(url, allow_redirects=False)
    assert response.headers["location"] == "/directory/"
    assert response.status_code == 301

def test_3xx_return_redirect():
    print("Request for /redirect")
    url = "http://localhost:8080/redirect"
    response = make_request(url, allow_redirects=False)
    assert response.status_code == 301
    assert response.headers["Location"] == "/secret"

def test_3xx_permanent_redirect():
    url = "http://localhost:8080/nevergonna"
    response = make_request(url, allow_redirects=False)
    assert response.status_code == 308
    assert response.headers["Location"] == "https://www.youtube.com/watch?v=dQw4w9WgXcQ"
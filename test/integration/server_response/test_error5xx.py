# This module is for requests which result in status code 5xx

from utils.utils import make_request

def test_5xx_max_recurision():
    print("Request for /recursion/")
    url = "http://localhost:8080/recursion/"
    response = make_request(url)
    assert response.status_code == 500
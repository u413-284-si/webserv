import requests
import time

# Regular tests using the shared server fixture
def test_server_running(start_cpp_server):
    # Send a request to the running server
    response = requests.get("http://127.0.0.1:8080",  timeout=(10, 30))
    assert response.status_code == 200

def test_server_another_feature(start_cpp_server):
    # Another test using the running server
    response = requests.get("http://localhost:8080/index.html", timeout=(10, 30))
    assert response.status_code == 200
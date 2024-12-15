# This module is for succesful POST requests

import requests
import os

# def test_POST_simple():
#     print("Request for /uploads/test01")
#     # Body to send
#     payload = "Beam me up, Scotty!"
#     response = requests.post("http://localhost:8080/uploads/test01.txt", data=payload)
#     assert response.status_code == 201
#     # Delete created file
#     os.remove("workspaces/webserv/uploads/test01")

# For encoding chunked
# sa https://requests.readthedocs.io/en/latest/user/advanced/#chunk-encoded-requests
# def generate_chunks():
#     yield 'First chunk of data'.encode("utf-8")
#     yield 'Second chunk of data'.encode("utf-8")
#     yield 'Third chunk of data'.encode("utf-8")

# def test_POST_chunked_encoding():
#     print("Chunked Request for /uploads/test01")
#     response = requests.post("http://localhost:8080/uploads/test01.txt", data=generate_chunks())
#     assert response.status_code == 201
#     # Delete created file
#     os.remove("workspaces/webserv/uploads/test01")

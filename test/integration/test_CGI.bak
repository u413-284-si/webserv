import requests

def test_CGI_simple(start_cpp_server):
    print("Request for /uploads/test01")
    response = requests.get("http://127.0.0.1:8080/cgi-bin/helloWorld.sh")
    print(response.text)
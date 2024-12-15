import requests

def test_no_CGI_defined(start_cpp_server):
    print("Try to access CGI without it defined")
    response = requests.get("http://127.0.0.1:8080/cgi-bin/time.py")
    assert response.status_code == 200

#def test_CGI_helloWorld(start_cpp_server):
#    print("Request for /cgi-bin/helloWorld.sh")
#    response = requests.get("http://127.0.0.1:8080/cgi-bin/helloWorld.sh")
#    assert response.status_code == 200

#def test_CGI_time(start_cpp_server):
#    print("Request for /cgi-bin/time.py")
#    response = requests.get("http://127.0.0.1:8081/cgi-bin/time.py")
#    assert response.status_code == 200

#def test_CGI_upload_file(start_cpp_server):
#    print("Upload file with /cgi-bin/upload.py")
#    # Query string parameters
#    query_params = {
#        "filename": "myfile.txt",
#        "directory": "documents"
#    }
#    # Define body
#    payload = "This is the content of the dudu."
#
#    response = requests.post("http://127.0.0.1:8081/cgi-bin/upload.py", params=query_params, data=payload)
#    assert response.status_code == 200

# def test_CGI_toUpper(start_cpp_server):
#     print("Change to upper case with /cgi-bin/upload.py")
#     # Define body
#     payload = (
#         "The old clock tower stood at the center of the village, its hands frozen at midnight for as long "
#         "as anyone could remember. Stories swirled among the villagers about why it had stopped-some claimed "
#           "it was cursed, while others whispered of forgotten rituals. At the base of the tower, ivy crept up the "
#           "stone walls, winding itself around the door that hadn't been opened in generations. One day, a stranger "
#           "arrived. She was dressed in black, with eyes that gleamed like stars. Without a word, she walked straight "
#           "to the door and knocked three times. The air seemed to grow still as if time itself had paused to watch. "
#           "When the door creaked open, the villagers held their breath. No one knew what lay beyond that threshold, "
#           "but they would soon find out."
#     )
#     response = requests.post("http://127.0.0.1:8080/cgi-bin/upperCase.sh", data=payload)
#     assert response.status_code == 200

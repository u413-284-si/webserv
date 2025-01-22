# This module is for succesful CGI requests

import os
import requests

def test_no_CGI_defined():
    print("Try to access CGI without it defined")
    response = requests.get("http://127.0.0.1:8080/cgi-bin/time.py")
    assert response.status_code == 200

def test_CGI_helloWorld():
   print("Request for /cgi-bin/helloWorld.sh")
   response = requests.get("http://127.0.0.1:8080/cgi-bin/helloWorld.sh")
   assert response.status_code == 200

def test_CGI_badBoi():
   print("Request for /cgi-bin/badBoi.sh")
   response = requests.get("http://127.0.0.1:8080/cgi-bin/badBoi.sh")
   assert response.status_code == 400
   assert response.headers["BB"] == "4Life"

def test_CGI_time():
   print("Request for /cgi-bin/time.py")
   response = requests.get("http://127.0.0.1:8081/cgi-bin/time.py")
   assert response.status_code == 200

def test_CGI_create_textfile():
   print("Upload file with /cgi-bin/create_textfile.py")

   form_data = {
      "filename": "myfile.txt",
      "content": "This is the content of the dudu.",
      "directory": "documents"
   }
   dst_file_path = "/workspaces/webserv/html/uploads/documents/myfile.txt"

   response = requests.post("http://127.0.0.1:8081/cgi-bin/create_textfile.py", data=form_data)
   assert response.status_code == 200
   assert response.headers["location"] == "/workspaces/webserv/html/uploads/documents/myfile.txt"
   # Check if file exists
   with open(dst_file_path, "r") as file:
    content = file.read()
    assert content == form_data["content"]
   # Delete created file
   os.remove(dst_file_path)

def test_CGI_toUpper():
    print("Change to upper case with /cgi-bin/upperCase.sh")
    # Define body
    payload = (
        "The old clock tower stood at the center of the village, its hands frozen at midnight for as long "
        "as anyone could remember. Stories swirled among the villagers about why it had stopped-some claimed "
          "it was cursed, while others whispered of forgotten rituals. At the base of the tower, ivy crept up the "
          "stone walls, winding itself around the door that hadn't been opened in generations. One day, a stranger "
          "arrived. She was dressed in black, with eyes that gleamed like stars. Without a word, she walked straight "
          "to the door and knocked three times. The air seemed to grow still as if time itself had paused to watch. "
          "When the door creaked open, the villagers held their breath. No one knew what lay beyond that threshold, "
          "but they would soon find out."
    )
    response = requests.post("http://127.0.0.1:8080/cgi-bin/upperCase.sh", data=payload)
    assert response.status_code == 200
    assert payload.upper() in response.text

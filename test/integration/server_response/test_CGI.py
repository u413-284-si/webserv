# This module is for succesful CGI requests

from utils.utils import make_request

def test_CGI_not_defined():
    print("Try to access CGI without it defined")
    url = "http://127.0.0.1:8080/cgi-bin/time.py"
    response = make_request(url)
    assert response.status_code == 200

def test_CGI_helloWorld():
   print("Request for /cgi-bin/helloWorld.sh")
   url = "http://127.0.0.1:8080/cgi-bin/helloWorld.sh"
   response = make_request(url)
   assert response.status_code == 200

def test_CGI_badBoi():
   print("Request for /cgi-bin/badBoi.sh")
   url = "http://127.0.0.1:8080/cgi-bin/badBoi.sh"
   response = make_request(url)
   # badBoi returns custom 400 status
   assert response.status_code == 400
   assert response.headers["BB"] == "4Life"

def test_CGI_time():
   print("Request for /cgi-bin/time.py")
   url = "http://127.0.0.1:8081/cgi-bin/time.py"
   response = make_request(url)
   assert response.status_code == 200

def test_CGI_create_textfile(test_file_cleanup):
   print("Upload file with /cgi-bin/create_textfile.py")

   form_data = {
      "filename": "myfile.txt",
      "content": "This is the content of the dudu.",
      "directory": "documents"
   }
   dst_file_path = "/workspaces/webserv/html/uploads/documents/myfile.txt"
   url = "http://127.0.0.1:8081/cgi-bin/create_textfile.py"

   test_file_cleanup.append(dst_file_path)

   response = make_request(url, method = "POST", data=form_data)
   assert response.status_code == 200
   assert response.headers["location"] == "/workspaces/webserv/html/uploads/documents/myfile.txt"
   with open(dst_file_path, "r") as file:
    content = file.read()
    assert content == form_data["content"]

def test_CGI_upperCase_GET():
    print("Change to upper case with /cgi-bin/upperCase.sh and GET")

    form_data = {
       "text": "please capitalize"
    }
    url = "http://127.0.0.1:8080/cgi-bin/upperCase.sh"
    response = make_request(url, params=form_data)
    assert response.status_code == 200
    assert form_data["text"].upper() in response.text

def test_CGI_upperCase_POST():
    print("Change to upper case with /cgi-bin/upperCase.sh and POST")
    long_string = (
        "The old clock tower stood at the center of the village, its hands frozen at midnight for as long "
        "as anyone could remember. Stories swirled among the villagers about why it had stopped-some claimed "
          "it was cursed, while others whispered of forgotten rituals. At the base of the tower, ivy crept up the "
          "stone walls, winding itself around the door that hadn't been opened in generations. One day, a stranger "
          "arrived. She was dressed in black, with eyes that gleamed like stars. Without a word, she walked straight "
          "to the door and knocked three times. The air seemed to grow still as if time itself had paused to watch. "
          "When the door creaked open, the villagers held their breath. No one knew what lay beyond that threshold, "
          "but they would soon find out."
    )
    form_data = {
       "text": long_string
    }
    url = "http://127.0.0.1:8080/cgi-bin/upperCase.sh"
    response = make_request(url, method = "POST", data=form_data)
    assert response.status_code == 200
    assert form_data["text"].upper() in response.text

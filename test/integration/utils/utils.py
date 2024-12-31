import subprocess
import os
import time
import requests
import pytest
from typing import Optional, Dict
import socket

def start_server(
    server_executable: str,
    config_file: str,
    with_coverage: bool,
    kcov_output_dir: str,
    kcov_excl_path: str
) -> subprocess.Popen:
    """
    Starts the server with or without coverage.

    Args:
        server_executable (str): The path to the server executable.
        config_file (str): The path to the configuration file.
        with_coverage (bool): Whether or not to run with code coverage.
        kcov_output_dir (str): The directory for the coverage output.
        kcov_excl_path (str): The path to the exclusions file for kcov.

    Returns:
        subprocess.Popen: The process object that represents the running server.
    """
    if with_coverage:
    # Create the kcov output directory if it doesn't exist
        os.makedirs(kcov_output_dir, exist_ok=True)
        server_process = subprocess.Popen(["kcov", kcov_excl_path, kcov_output_dir, server_executable, config_file])
        print("Running server with coverage (kcov)...")
    else:
        server_process = subprocess.Popen([server_executable, config_file])
        print("Running server without coverage...")
    return server_process

def stop_server(server_process: subprocess.Popen) -> None:
    """
    Terminates and waits for the server process to stop.

    Args:
        server_process (subprocess.Popen): The process object of the running server.

    Returns:
        None
    """
    server_process.terminate()
    server_process.wait()

def wait_for_startup() -> None:
    """
    Waits for the server to start by checking its health endpoint.

    If the server does not start within the timeout period, it stops the server
    and exits the test with an error.

    Returns:
        None
    """
    timeout = 15 # Timeout period in seconds
    start_time = time.time()
    # Keep checking the server health until the timeout is reached
    while time.time() - start_time < timeout:
        try:
            response = requests.get("http://localhost:8080/health")
            if response.status_code == 200:
                break # Server started successfully
        except requests.ConnectionError:
            pass # Server not reachable, continue retrying
        time.sleep(0.1)  # Small delay between retries
    else:
        # If the loop ends without breaking, raise an error
        stop_server()
        pytest.exit("Server did not start within the timeout period.")


def make_request(
    url: str,
    method: str = "GET",
    data: Optional[dict] = None,
    headers: Optional[dict] = None,
    timeout: int = 10,
    allow_redirects: bool = True
) -> requests.Response:
    """
    Make an HTTP request with the specified method, URL, and parameters.

    Args:
        url (str): The URL to request.
        method (str): The HTTP method to use ('GET', 'POST', 'DELETE').
        data (Optional[dict]): The payload for POST or DELETE requests, or a callable that generates chunks.
        headers (Optional[dict]): The headers to include in the request.
        timeout (int): The timeout in seconds for the request.
        allow_redirects (bool): Whether to follow redirects automatically.

    Returns:
        requests.Response: The response object from the request.

    Raises:
        pytest.fail: If the request fails.
    """
    try:
        # Handle GET request
        if method.upper() == "GET":
            response = requests.get(url, headers=headers, timeout=timeout, allow_redirects=allow_redirects)

        # Handle POST request
        elif method.upper() == "POST":
            if callable(data):
                response = requests.post(url, data=data(), headers=headers, timeout=timeout, allow_redirects=allow_redirects)
            else:
                response = requests.post(url, data=data, headers=headers, timeout=timeout, allow_redirects=allow_redirects)

        # Handle DELETE request
        elif method.upper() == "DELETE":
            if callable(data):
                response = requests.delete(url, data=data(), headers=headers, timeout=timeout, allow_redirects=allow_redirects)
            else:
                response = requests.delete(url, data=data, headers=headers, timeout=timeout, allow_redirects=allow_redirects)

        else:
            pytest.fail(f"Unsupported HTTP method: {method}")

        return response

    except requests.exceptions.RequestException as e:
        pytest.fail(f"Failed to make request to {url} with method {method}: {e}", pytrace=False)

def parse_http_response(sock: socket.socket) -> Dict[str, str]:
    """
    Parse the HTTP response from a socket object and return the status code, headers, and body.

    Args:
        sock (socket.socket): The socket object from which to receive the HTTP response.

    Returns:
        dict: A dictionary containing:
            - "status_code" (str): The HTTP status code.
            - "headers" (dict): The HTTP headers, as a dictionary of key-value pairs.
            - "body" (str): The body content of the HTTP response, if any.
    """
    # Receive the full response (4096 bytes at a time)
    response_data = sock.recv(4096)

    # Decode response into a string using UTF-8 encoding
    response_str = response_data.decode('utf-8')

    # Split the response string into lines (separated by \r\n)
    response_lines = response_str.split('\r\n')

    # Extract the status line (first line of the response)
    status_line = response_lines[0]
    status_code = int(status_line.split()[1])  # Extract the status code from the status line

    # Parse the headers from the remaining lines
    headers = {}
    for line in response_lines[1:]:
        if line.strip() == "":  # An empty line indicates the end of headers
            break
        key, value = line.split(":", 1)  # Split header line into key and value
        headers[key.strip()] = value.strip()  # Store header in dictionary, stripping whitespace

    # Extract the body, which comes after a double CRLF sequence (\r\n\r\n)
    body = response_str.split('\r\n\r\n', 1)[1] if '\r\n\r\n' in response_str else ""

    # Return the parsed response as a dictionary
    return {
        "status_code": status_code,
        "headers": headers,
        "body": body
    }

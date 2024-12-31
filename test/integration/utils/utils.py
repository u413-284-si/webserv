import subprocess
import os
import time
import requests
import pytest
from typing import Optional, Callable

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
    timeout: int = 10
) -> requests.Response:
    """
    Make an HTTP request with the specified method, URL, and parameters.

    Args:
        url (str): The URL to request.
        method (str): The HTTP method to use ('GET', 'POST', 'DELETE').
        data (Optional[dict]): The payload for POST or DELETE requests, or a callable that generates chunks.
        headers (Optional[dict]): The headers to include in the request.
        timeout (int): The timeout in seconds for the request.

    Returns:
        requests.Response: The response object from the request.

    Raises:
        pytest.fail: If the request fails.
    """
    try:
        # Handle GET request
        if method.upper() == "GET":
            response = requests.get(url, headers=headers, timeout=timeout)

        # Handle POST request
        elif method.upper() == "POST":
            if callable(data):
                response = requests.post(url, data=data(), headers=headers, timeout=timeout)
            else:
                response = requests.post(url, data=data, headers=headers, timeout=timeout)

        # Handle DELETE request
        elif method.upper() == "DELETE":
            if callable(data):
                response = requests.delete(url, data=data(), headers=headers, timeout=timeout)
            else:
                response = requests.delete(url, data=data, headers=headers, timeout=timeout)

        else:
            pytest.fail(f"Unsupported HTTP method: {method}")

        return response

    except requests.RequestException as e:
        pytest.fail(f"Failed to make request to {url} with method {method}: {e}")

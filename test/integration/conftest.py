import pytest
import os
import subprocess
import time
import requests
from typing import Callable, Optional

def pytest_addoption(parser):
    print("Registering custom pytest options")
    parser.addoption("--server-executable", action="store", default="/workspaces/webserv/webserv", help="Path to the server executable")
    parser.addoption("--config-file", action="store", default="/workspaces/webserv/config_files/valid_config.conf", help="Path to the server config file")
    parser.addoption("--with-coverage", action="store_true", help="Run server with kcov for coverage")
    parser.addoption("--kcov-output-dir", action="store", default="/workspaces/webserv/.vscode/coverage", help="Path to the kcov output directory")
    parser.addoption("--kcov-excl-path", action="store", default="--exclude-path=/usr/include,/usr/lib,/usr/local", help="Path to exclude from kcov coverage")

@pytest.fixture(scope="session")
def start_server() -> Callable[[str, str, bool, str, str], subprocess.Popen]:
    """
    Fixture to start a server either with or without coverage.

    Returns:
        Callable[[str, str, bool, str, str], subprocess.Popen]: A function that starts the server.
    """
    def start_server_function(
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
    return start_server_function

@pytest.fixture(scope="session")
def stop_server() -> Callable[[subprocess.Popen], None]:
    """
    Fixture to stop a running server.

    Returns:
        Callable[[subprocess.Popen], None]: A function that stops a running server.
    """
    def stop_server_function(server_process: subprocess.Popen) -> None:
        """
        Terminates and waits for the server process to stop.

        Args:
            server_process (subprocess.Popen): The process object of the running server.

        Returns:
            None
        """
        server_process.terminate()
        server_process.wait()
        print("Server stopped from test fixture start_cpp_server.")
    return stop_server_function

@pytest.fixture(scope="session")
def wait_for_startup()  -> Callable[[], None]:
    """
    Fixture to wait for the server to start.

    Returns:
        Callable[[], None]: A function that waits for the server to start.
    """
    def wait_for_startup_function() -> None:
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
    return wait_for_startup_function

@pytest.fixture
def make_request() -> Callable[[str, str, Optional[dict], Optional[dict], int], requests.Response]:
    """
    Fixture to make HTTP requests with error handling.

    Returns:
        Callable[[str, str, Optional[dict], Optional[dict], int], requests.Response]:
        A function to make HTTP requests with different methods.
    """
    def request_function(
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
            method (str): The HTTP method to use ('GET', 'POST', 'DELETE', etc.).
            data (Optional[dict]): The payload for POST or DELETE requests.
            headers (Optional[dict]): The headers to include in the request.
            timeout (int): The timeout in seconds for the request.

        Returns:
            requests.Response: The response object from the request.

        Raises:
            pytest.fail: If the request fails.
        """
        try:
            if method.upper() == "GET":
                response = requests.get(url, headers=headers, timeout=timeout)
            elif method.upper() == "POST":
                response = requests.post(url, json=data, headers=headers, timeout=timeout)
            elif method.upper() == "DELETE":
                response = requests.delete(url, json=data, headers=headers, timeout=timeout)
            else:
                pytest.fail(f"Unsupported HTTP method: {method}")
            return response
        except requests.RequestException as e:
            pytest.fail(f"Failed to make request to {url} with method {method}: {e}")

    return request_function

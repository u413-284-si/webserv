import pytest
import subprocess
import os
import stat
from pytest import FixtureRequest
from typing import List, Generator
from utils.utils import start_server, wait_for_startup, stop_server

# Fixture to start the server for normal tests
@pytest.fixture(scope="package", autouse=True)
def init_server_instance(request: FixtureRequest) -> Generator[subprocess.Popen, None, None]:
    """
    Fixture to initialize and start a C++ server instance before tests and stop it after tests in the package.

    This fixture starts the server with the specified executable and configuration,
    waits for the server to start, and provides the server process to the tests. After
    the tests are completed, it stops the server.

    Args:
        request (FixtureRequest): The pytest request object, used to access command-line options.

    Yields:
        subprocess.Popen: The process object representing the running server.

    """

    # Get the command-line options passed to pytest
    server_executable = request.config.getoption("--server-executable")
    config_file = request.config.getoption("--config-file")

    # Check if the user passed the --with-coverage option
    with_coverage = request.config.getoption("--with-coverage")

    kcov_output_dir = request.config.getoption("--kcov-output-dir")
    kcov_excl_path = request.config.getoption("--kcov-excl-path")

    server_process = start_server(server_executable, config_file, with_coverage, kcov_output_dir, kcov_excl_path)

    wait_for_startup(server_process)

    # Provide the server process to the tests
    yield server_process

    stop_server(server_process)

@pytest.fixture
def test_file_cleanup() -> Generator[List[str], None, None]:
    """Fixture to clean up test files after a test.

    Yields:
        List[str]: A list of file paths to be cleaned up after the test.
    """
    files_to_cleanup: List[str] = []

    yield files_to_cleanup

    # Restore permissions and clean up
    for file_path in files_to_cleanup:
        try:
            os.chmod(file_path, 0o666)  # Default to read/write for deletion
            os.remove(file_path)
        except FileNotFoundError:
            pass

@pytest.fixture
def temp_permission_change() -> Generator[None, None, None]:
    """
    Fixture to temporarily change permissions of a file or directory.

    Yields:
        None: Provides a context for the test to temporarily change file permissions.
    """
    original_permissions = {}

    def set_permissions(file_path: str, new_permissions: int) -> None:
        """
        Change the permissions of the given file and store the original permissions.

        Args:
            file_path (str): Path to the file or directory.
            new_permissions (int): New permissions to apply (e.g., 0o000).
        """
        if file_path not in original_permissions:
            # Store the original permissions
            original_permissions[file_path] = stat.S_IMODE(os.stat(file_path).st_mode)
        # Set new permissions
        os.chmod(file_path, new_permissions)

    yield set_permissions

    # Restore original permissions
    for file_path, permissions in original_permissions.items():
        try:
            os.chmod(file_path, permissions)
        except FileNotFoundError:
            pass

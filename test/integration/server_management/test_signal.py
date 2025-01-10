# This module is for signal handling

import signal
import os
import subprocess
from utils.utils import start_server, wait_for_startup

def test_signal_graceful_shutdown(request):

    server_executable = request.config.getoption("--server-executable")
    config_file = request.config.getoption("--config-file")

    # Check if the user passed the --with-coverage option
    with_coverage = request.config.getoption("--with-coverage")

    kcov_output_dir = request.config.getoption("--kcov-output-dir")
    kcov_excl_path = request.config.getoption("--kcov-excl-path")

    server_process = start_server(server_executable, config_file, with_coverage, kcov_output_dir, kcov_excl_path)

    wait_for_startup(server_process)

    if with_coverage:
        pid = int(subprocess.check_output(["pgrep", "webserv"]).decode().strip())
    else:
        pid = server_process.pid

    print("Sending SIGQUIT for graceful shutdown")
    os.kill(pid, signal.SIGQUIT)
    server_process.wait()
    exit_status = server_process.returncode
    assert exit_status == 0

# def test_signal_forced_shutdown(request):

#     server_executable = request.config.getoption("--server-executable")
#     config_file = config_file = request.config.getoption("--config-file")

#     # Check if the user passed the --with-coverage option
#     with_coverage = request.config.getoption("--with-coverage")

#     kcov_output_dir = request.config.getoption("--kcov-output-dir")
#     kcov_excl_path = request.config.getoption("--kcov-excl-path")

#     server_process = start_server(server_executable, config_file, with_coverage, kcov_output_dir, kcov_excl_path)

#     wait_for_startup()

#     if with_coverage:
#         pid = int(subprocess.check_output(["pgrep", "webserv"]).decode().strip())
#     else:
#         pid = server_process.pid

#     print("Sending SIGINT...")
#     os.kill(pid, signal.SIGINT)
#     server_process.wait()
#     exit_status = server_process.returncode
#     assert exit_status != 0

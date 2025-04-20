import subprocess
import os
import platform
import shutil
import time
import socket
import argparse

def parse_cmd_line_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--ci", nargs="?", const=60, type=int)
    cmd_line_arguments = parser.parse_args()
    return cmd_line_arguments

def setup_env(is_ci):
    subprocess.run(["python3", "build_test_foundry.py"], cwd="../..")
    if os.path.exists("tmp"):
        shutil.rmtree("tmp")
    if os.path.exists("logs"):
        shutil.rmtree("logs")

    os.makedirs("tmp")
    os.makedirs("logs")
    #os.path.join("logs", "log1.log")

    if (is_ci):
        os.environ["CLEANUP_TIME"] = str(is_ci)
    else: 
        os.environ["CLEANUP_TIME"] = "10"

def compile_and_run_test():
    if platform.system() == "Darwin":
        rpath_flag = "-Wl,-rpath,@executable_path/../../logger_foundry_lib/lib"
    elif platform.system() == "Linux":
        os.environ["LD_LIBRARY_PATH"] = "../../logger_foundry_lib/lib"
        rpath_flag = "-Wl,-rpath,'$ORIGIN/../../logger_foundry_lib/lib'"
    else:
        rpath_flag = ""

    subprocess.run(["clang++", "-o", "listener", "-I../../logger_foundry_lib/include",
                "-L../../logger_foundry_lib/lib",
                "-std=c++20",  
                rpath_flag,
                "-llogger_foundry",
                "-ldaemon_orchestrator_lib",
                "-lbuffer_parsing_lib",
                "-llog_writer_lib",
                "-linput_socket_lib",
                "logger_daemon.cpp"], check=True)

    listener_proc = subprocess.Popen(["./listener"])
    return listener_proc


def wait_for_unix_socket(socket_path, timeout=30):
    #print(f"Waiting on socket: {socket_path}")
    start = time.time()
    while not os.path.exists(socket_path):
        if time.time() - start > timeout:
            raise TimeoutError(f"Timeout - Waiting for socket: {socket_path}")
        time.sleep(0.01)
    print(f"Socket created: {socket_path}")

def wait_for_port(host, port, timeout=30):
    start = time.time()
    while time.time() - start < timeout:
        with socket.create_connection((host, port), timeout=timeout):
            return
        time.sleep(0.01)

def wait_for_unix_sockets(sockets):
    for socket in sockets:
        wait_for_unix_socket(socket)

def cleanup():
    os.remove("listener")

    shutil.rmtree("tmp")
    #shutil.rmtree("log")



def send_data(socket_path, message):
    with socket.socket(socket.AF_UNIX, socket.SOCK_STREAM) as client:
        client.connect(socket_path)
        client.sendall(message.encode("utf-8"))


def send_ipv6_message(host, port, message):
    with socket.socket(socket.AF_INET6, socket.SOCK_STREAM) as sock:
        sock.connect((host, port))
        sock.sendall(message.encode('utf-8'))

def send_ipv4_message(host, port, message):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
        sock.connect((host, port))
        sock.sendall(message.encode('utf-8'))

def validate_file_contents():
    expected_strings = ["HELLOOOO FROM SOCKET 1\n", "HELLOOOO FROM SOCKET 2\n", "HELLOOO FROM IPV4\n", "HELLOOO FROM IPV6\n"]

    with open("logs/log1.log", "r", encoding="utf-8") as f:
        contents = f.read()

    for expected in expected_strings:
        if expected not in contents:
            raise AssertionError(f"Missing log message: {expected}")
    
    print("Multi Unix, ipv4, ipv6 daemon log file contains all expected strings.")

def main():
    cmd_line_args = parse_cmd_line_args()

    setup_env(cmd_line_args.ci)
    listener_proc = compile_and_run_test()
    wait_for_unix_sockets(["tmp/sock1.sock", "tmp/sock2.sock"])
    wait_for_port('::1', 50051)
    send_data("tmp/sock1.sock", "HELLOOOO FROM SOCKET 1")
    send_data("tmp/sock2.sock", "HELLOOOO FROM SOCKET 2")
    send_ipv4_message('127.0.0.1', 50051, "HELLOOO FROM IPV4")
    send_ipv6_message('::1', 50051, "HELLOOO FROM IPV6")


    listener_proc.wait()

    validate_file_contents()

    cleanup()



main()
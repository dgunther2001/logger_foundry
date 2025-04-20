import subprocess
import os
import platform
import shutil
import time
import socket

def setup_env():
    subprocess.run(["python3", "build_test_foundry.py"], cwd="../..")
    if os.path.exists("tmp"):
        shutil.rmtree("tmp")
    if os.path.exists("logs"):
        shutil.rmtree("logs")

    os.makedirs("tmp")
    os.makedirs("logs")
    os.path.join("logs", "log1.log")

    os.environ["CLEANUP_TIME"] = "10"

def compile_and_run_test():
    if platform.system() == "Darwin":
        rpath_flag = "-Wl,-rpath,@executable_path/../../logger_foundry_lib/lib"
    elif platform.system() == "Linux":
        rpath_flag = "-Wl,-rpath,$ORIGIN/../../logger_foundry_lib/lib"
    else:
        rpath_flag = ""

    subprocess.run(["clang++", "-o", "listener", "-I../../logger_foundry_lib/include",
                "-L../../logger_foundry_lib/lib",
                "-std=c++20",  
                rpath_flag,
                "-llogger_foundry",
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



def main():
    setup_env()
    listener_proc = compile_and_run_test()
    wait_for_unix_sockets(["tmp/sock1.sock", "tmp/sock2.sock"])
    send_data("tmp/sock1.sock", "HELLOOOO FROM SOCKET 1")
    send_data("tmp/sock2.sock", "HELLOOOO FROM SOCKET 2")


    listener_proc.wait()


    cleanup()



main()
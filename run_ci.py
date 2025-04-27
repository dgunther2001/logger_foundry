import subprocess
import argparse
import sys

def parse_cmd_line_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--ci", nargs="?", const=60, type=int)
    parser.add_argument("--clean", nargs="?", type=str)
    cmd_line_arguments = parser.parse_args()
    return cmd_line_arguments


def main():
    #subprocess.run(["python3", "build_test_foundry.py"], cwd="ci_tests")
    cmd_line_args = parse_cmd_line_args()
    end_to_end_multi_socket_test = ["python3", "test_unix_and_web_socket.py"]
    if cmd_line_args.ci:
        end_to_end_multi_socket_test.extend(["--ci", str(cmd_line_args.ci)])
    if cmd_line_args.clean:
        end_to_end_multi_socket_test.append("--clean")

    test_1 = subprocess.Popen(end_to_end_multi_socket_test, cwd="ci_tests/end_to_end_tests/test_unix_and_web_socket")

    test_1.wait()


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("[!] SIGINT received — exiting CI cleanly.")
        sys.exit(0)

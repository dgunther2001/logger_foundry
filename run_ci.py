import subprocess
import argparse
import sys

def parse_cmd_line_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--ci", nargs="?", const=60, type=int)
    cmd_line_arguments = parser.parse_args()
    return cmd_line_arguments


def main():
    #subprocess.run(["python3", "build_test_foundry.py"], cwd="ci_tests")
    cmd_line_args = parse_cmd_line_args()
    if cmd_line_args.ci:
        test_1 = subprocess.Popen(["python3", "test_unix_and_web_socket.py", "--ci", str(cmd_line_args.ci)], cwd="ci_tests/end_to_end_tests/test_unix_and_web_socket")
    else:
        test_1 = subprocess.Popen(["python3", "test_unix_and_web_socket.py"], cwd="ci_tests/end_to_end_tests/test_unix_and_web_socket")

    test_1.wait()


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("[!] SIGINT received — exiting CI cleanly.")
        sys.exit(0)

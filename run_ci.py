import subprocess
import os


def main():
    subprocess.run(["python3", "build_test_foundry.py"], cwd="ci_tests")

main()
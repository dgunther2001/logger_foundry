import shutil
import subprocess
import platform
import sys

def dependency_exists(dependency):
    return shutil.which(dependency) is not None

def install_linux(dependencies):
    print("[+] Updating apt...")
    subprocess.run(["apt-get", "update"], check=True)
    print(f"[+] Installing: {' '.join(dependencies)}")
    subprocess.run(["apt-get", "install", "-y"] + dependencies, check=True)

def get_linux_dep_names():
    return ["build-essential", "cmake", "python3", "clang", "llvm"]

def install_mac_os(dependencies, missing_dependencies):
    # add check for homebrew here!!!
    print("[+] Updating Homebrew…")
    subprocess.run(["brew", "update"], check=True)
    for dep in missing_dependencies:
        package = dependencies.get(dep)
        if package:
            print(f"[+] brew install {package}")
            subprocess.run(["brew", "install", package], check=True)
        else:
            print(f"[-] No brew mapping for '{dep}', please install yourself and add to $PATH (Xcode CLI).")

def get_mac_os_dep_names():
    return {"cmake" : "cmake", "python3" : "python", "clang" : "llvm"}

def mac_os_check_brew_installation():
    return shutil.which("brew") is not None

def main():
    required_dependencies = ["cmake", "clang", "make", "python3"]
    missing_dependencies = [dep for dep in required_dependencies if not dependency_exists(dep)]

    if not missing_dependencies:
        print("[+] All dependencies installed for Logger Foundry.")
        return
    
    os_type = platform.system()

    #print(f"[+] Missing dependencies: {', '.join(missing_dependencies)}")
    print(f"[+] Detected OS: {os_type}")

    if os_type == "Linux":
        install_linux(get_linux_dep_names())
    elif os_type == "Darwin":
        if not mac_os_check_brew_installation():
            print("[+] Install Homebrew Package Manager to Proceed.")
            sys.exit(1)
        install_mac_os(get_mac_os_dep_names(), missing_dependencies)
    else:
        print(f"[-] Unsupported OS '{os_type}'.")
        sys.exit(1)

    print("[+] Dependency installation for Logger Foundry complete.")

main()
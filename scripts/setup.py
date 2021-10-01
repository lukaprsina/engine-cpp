#!/usr/bin/env python3

import os
import sys
import subprocess
import shlex
import json
from pathlib import Path


def run_command(command):
    process = subprocess.Popen(shlex.split(command), stdout=subprocess.PIPE)
    while True:
        line = process.stdout.readline()
        print(line.decode("ascii"), end="")
        if not line: break

def CheckDependencies():
    vendor_config = json.loads(config.read_text())
    submodules = vendor_config["submodules"]
    external = vendor_config["external"]
    submodule_missing = vendor_config["submodule_missing"]
    external_missing = vendor_config["external_missing"]    

    print("Found submodules:")
    for dependency in submodules:
        print(f"\t{dependency}")

    print("Found external:")
    for dependency in external:
        print(f"\t{dependency}")

    print("Submodule missing:")
    for dependency in submodule_missing:
        print(f"\t{dependency}")        

    print("External missing:")
    for dependency in external_missing:
        print(f"\t{dependency}")

    submodule_missing_len = len(submodule_missing)
    if submodule_missing_len == 0:
        print("All dependencies found.")
    else:
        print(f"{submodule_missing_len} {'submodule' if submodule_missing_len == 1 else 'submodules'}\
 which CAN'T be installed externally {'is' if submodule_missing_len == 1 else 'are'} missing.")
        while True:
            choice = input("Do you want to:\nD) Download all (default).\nI) Ignore all.\nC) Choose individually.\n>").upper()
            if choice in ['D', 'I', 'C']:
                break

        if choice == "D": 
            for dependency in submodule_missing:
                run_command(f"git submodule update --init engine/vendor/{dependency}")
            print ("Downloaded.")
        elif choice == "I": 
            print ("Ignoring all.")
        elif choice == "C": 
            for dependency in submodule_missing:
                print(f"\n{dependency}:")
                while True:
                    choice = input("D) Download (recommended).\nI) Ignore.\n>").upper()
                    if choice in ['D', 'I']:
                        break

                if choice == 'D':
                    run_command(f"git submodule update --init engine/vendor/{dependency}")   

    external_missing_len = len(external_missing)
    if external_missing_len == 0:
        print("All dependencies found.")
    else:
        print(f"{external_missing_len} {'external dependencies' if external_missing_len == 1 else 'external dependency'}\
 which CAN be installed externally {'is' if submodule_missing_len == 1 else 'are'} missing.")
        while True:
            choice = input("Do you want to:\nD) Download locally (as submodules).\nI) Ignore all.\nC) Choose individually (recommended).\nW) View every dependency website.\n>").upper()
            if choice in ['D', 'I', 'C', 'W']:
                break

        if choice == "D": 
            for dependency in external_missing:
                run_command(f"git submodule update --init engine/vendor/{dependency}")
            print ("Downloaded.")
        elif choice == "I": 
            print ("Ignoring all.")
        elif choice == "C": 
            for dependency in external_missing:
                print(f"\n{dependency}:")
                while True:
                    choice = input("Do you want to:\nD) Download locally (as submodule).\nI) Ignore.\nW) View dependency website (recommended).\n>").upper()
                    if choice in ['D', 'I', 'W']:
                        break

                if choice == 'D':
                    run_command(f"git submodule update --init engine/vendor/{dependency}")

        elif choice == 'W':
            pass



        


scripts_directory = Path(os.path.dirname(os.path.realpath(__file__)))

if (not scripts_directory.exists()):
    raise FileNotFoundError("Scripts directory doesn't exist")

base_directory = scripts_directory.parent
os.chdir(base_directory)

config = base_directory / "engine/vendor/config.json"

if (not config.exists()):
    print("Please run CMake Configure from your IDE or terminal and run this script again.")
    sys.exit()
else:
    CheckDependencies()

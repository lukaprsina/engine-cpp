#!/usr/bin/env python3

import os
import sys
import json
from pathlib import Path


def ReadConfig():
    vendor_config = json.loads(config.read_text())
    submodules = vendor_config["submodules"]
    external = vendor_config["external"]
    not_found = vendor_config["not_found"]

    print("Found submodules:")
    for dependency in submodules:
        print(f"\t{dependency}")

    print("Found external:")
    for dependency in external:
        print(f"\t{dependency}")

    print("Not found:")
    for dependency in not_found:
        print(f"\t{dependency}")


scripts_directory = Path(os.path.dirname(os.path.realpath(__file__)))

if (not scripts_directory.exists()):
    raise FileNotFoundError("Scripts directory doesn't exist")

base_directory = scripts_directory.parent
os.chdir(base_directory)

config = base_directory / "engine/vendor/config.json"

if (not config.exists()):
    sys.exit()
else:
    ReadConfig()

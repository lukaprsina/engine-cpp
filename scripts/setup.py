#!/usr/bin/env python3

import os
import sys
import json
from pathlib import Path


def ReadJSON():
    json_data = json.loads(config.read_text())
    print(json_data["test"])


scripts_directory = Path(os.path.dirname(os.path.realpath(__file__)))

if (not scripts_directory.exists()):
    raise FileNotFoundError("Scripts directory doesn't exist")

base_directory = scripts_directory.parent
os.chdir(base_directory)

config = base_directory / "engine/vendor/config.json"

if (config.exists()):
    ReadJSON()

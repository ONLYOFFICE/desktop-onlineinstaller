#!/usr/bin/env python

import os
import sys
__dir__name__ = os.path.dirname(os.path.abspath(__file__))
sys.path.append(__dir__name__ + '/scripts')
sys.path.append(__dir__name__ + '/../build_tools/scripts')
import config
import base
import qmake
import deploy_onlineinst
import argparse

base.check_python()

parser = argparse.ArgumentParser(description="options")
args = parser.parse_args()

# parse configuration
config.parse()

base_dir = base.get_script_dir(__file__)
base.set_env("BUILD_PLATFORM", config.option("platform"))
base.check_build_version(base_dir)

# update
# if ("1" == config.option("update")):
#   repositories = {}
#   repositories["desktop-onlineinstaller"] = [False, False]
#   base.update_repositories(repositories)

if "windows" == base.host_platform():
  base.set_env("DESKTOP_URL_UPDATES_MAIN_CHANNEL", "https://download.onlyoffice.com/install/desktop/editors/windows/onlyoffice/appcast.json")
  base.set_env("DESKTOP_URL_UPDATES_DEV_CHANNEL", "https://download.onlyoffice.com/install/desktop/editors/windows/onlyoffice/appcastdev.json")

  # build
  qmake.make(config.option("platform"), "OnlineInstaller.pro")

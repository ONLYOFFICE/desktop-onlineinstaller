#!/usr/bin/env python

import config
import base
import platform

def make():
  base_dir = base.get_script_dir(__file__) + "/../out"
  git_dir = base.get_script_dir() + "/../.."
  branding = config.branding()

  platforms = config.option("platform").split()
  for native_platform in platforms:
    if not native_platform in config.platforms:
      continue

    root_dir = base_dir + ("/" + native_platform + "/" + branding + "/OnlineInstaller")
    if (base.is_dir(root_dir)):
      base.delete_dir(root_dir)
    base.create_dir(root_dir)

    # check platform on xp
    isWindowsXP = False if (-1 == native_platform.find("_xp")) else True
    platform = native_platform[0:-3] if isWindowsXP else native_platform

    apps_postfix = "build" + base.qt_dst_postfix()
    if ("" != config.option("branding")):
      apps_postfix += ("/" + config.option("branding"))
    apps_postfix += "/"
    apps_postfix += platform
    if isWindowsXP:
      apps_postfix += "/xp"
      base.copy_file(git_dir + "/desktop-onlineinstaller/" + apps_postfix + "/online-installer.exe", root_dir + "/online-installer.exe")

  return

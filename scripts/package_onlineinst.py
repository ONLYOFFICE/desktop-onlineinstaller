#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import re
import package_utils as utils
import package_common as common
import package_branding as branding

def make():
  utils.log_h1("DESKTOP ONLINE INSTALLER")
  if utils.is_windows():
    make_windows()
  else:
    utils.log("Unsupported host OS")
  return

def s3_upload(files, dst):
  if not files: return False
  ret = True
  for f in files:
    key = dst + utils.get_basename(f) if dst.endswith("/") else dst
    upload = utils.s3_upload(f, "s3://" + branding.s3_bucket + "/" + key)
    if upload:
      utils.log("URL: " + branding.s3_base_url + "/" + key)
    ret &= upload
  return ret

#
# Windows
#

def make_windows():
  global package_name, package_version, arch, xp, suffix
  utils.set_cwd("desktop-onlineinstaller\\scripts")

  package_name = branding.desktop_package_name
  package_version = common.version + "." + common.build
  arch = {
    "windows_x64":    "x64",
    "windows_x64_xp": "x64",
    "windows_x86":    "x86",
    "windows_x86_xp": "x86"
  }[common.platform]
  xp = common.platform.endswith("_xp")
  suffix = arch + ("-xp" if xp else "")

  if common.clean:
    utils.log_h2("desktop online installer clean")
    utils.delete_files("*.exe")
    utils.delete_files("*.tmp")

  make_prepare()
  make_online()

  utils.set_cwd(common.workspace_dir)
  return

def make_prepare():
  args = [
    "-Version", package_version,
    "-Arch", arch
  ]
  if xp:
    args += ["-Target", "xp"]
  if common.sign:
    args += ["-Sign"]

  utils.log_h2("desktop online installer prepare")
  ret = utils.ps1("make.ps1", args, verbose=True)
  utils.set_summary("desktop online installer prepare", ret)
  return

def make_online():
  if not common.platform in ["windows_x86_xp"]:
    return
  online_file = "%s-%s-%s.exe" % ("OnlineInstaller", package_version, suffix)
  ret = utils.is_file(online_file)
  utils.set_summary("desktop online installer build", ret)

  if common.deploy and ret:
    utils.log_h2("desktop online installer deploy")
    ret = s3_upload([online_file], "desktop/win/online/")
    utils.set_summary("desktop online installer deploy", ret)
  return

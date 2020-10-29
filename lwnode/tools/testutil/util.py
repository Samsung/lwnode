#
# Copyright (c) 2020-present Samsung Electronics Co., Ltd
#
#  This library is free software; you can redistribute it and/or
#  modify it under the terms of the GNU Lesser General Public
#  License as published by the Free Software Foundation; either
#  version 2 of the License, or (at your option) any later version.
#
#  This library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  Lesser General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public
#  License along with this library; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
#  USA

import subprocess
import shutil
import os
import path
import json


def exec_cmd(cmd, cwd=path.ROOT_PATH):
  try:
    print(' '.join(cmd))
    process = subprocess.Popen(cmd, stdout=subprocess.PIPE,
                               stderr=subprocess.PIPE, cwd=cwd)

    output = process.communicate()[0]
    exitcode = process.returncode

    if exitcode:
      raise Exception('Non-zero exit value.')

    print(output)
    return output, exitcode

  except Exception as e:
    print('[Failed - %s] %s' % (cmd, str(e)))


def exec_shell(cmd, cwd=path.ROOT_PATH):
  print(' '.join(cmd))
  exitcode = subprocess.call(' '.join(cmd), shell=True, cwd=cwd)

  if exitcode != 0:
    print('[Failed - %s] %s' % (exitcode, cmd,))
    exit(1)


def rmdir(path):
  if os.path.exists(path):
    shutil.rmtree(path)


def mkdir(directory):
  if os.path.exists(directory):
    return

  os.makedirs(directory)


def force_copy(root_src_dir, root_dst_dir):
  for src_dir, dirs, files in os.walk(root_src_dir):
    dst_dir = src_dir.replace(root_src_dir, root_dst_dir, 1)
    if not os.path.exists(dst_dir):
      os.makedirs(dst_dir)
    for file_ in files:
      src_file = os.path.join(src_dir, file_)
      dst_file = os.path.join(dst_dir, file_)
      if os.path.exists(dst_file):
        try:
          os.remove(dst_file)
        except PermissionError as exc:
          os.chmod(dst_file, stat.S_IWUSR)
          os.remove(dst_file)

      shutil.copy(src_file, dst_dir)


def copy(src, dst):
  if not os.path.exists(src):
    return

  if os.path.isdir(src):
    if os.path.exists(dst):
      shutil.rmtree(dst)

    shutil.copytree(src, dst, symlinks=False, ignore=None)
  else:
    basedir = os.path.dirname(dst)
    if not os.path.exists(basedir):
      mkdir(basedir)

    shutil.copy(src, dst)


def move(src, dst):
  shutil.move(src, dst)


def read_json_file(filename):
  with open(filename, 'r') as json_file:
    return json.load(json_file)


# get file name from path
def get_filename(path):
  filepath = path.split('/')
  filename = filepath[-1]
  if not filename:
    filename = filepath[-2]
  return filename

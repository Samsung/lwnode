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

import util
import path
import log
import os

from jinja2 import Environment, FileSystemLoader

join = os.path.join


class TPKProject():
  def __init__(self, options):
    self.app_path = options['app_path']
    self.app_name = options.setdefault('app_name', util.get_filename(self.app_path))
    self.job_path = path.OUT_PATH
    self.tpk_project_path = join(self.job_path, self.app_name)
    self.node_path = options.setdefault('node', join(path.OUT_PATH, 'Release', 'node'))

  def create(self):
    log.info('Make Project: %s' % self.app_name)
    util.copy(path.TPK_TEMPLATE_PATH, self.tpk_project_path)

  def package(self):
    # copy app folder
    util.copy(join(self.app_path, 'res'), join(self.tpk_project_path, 'res'))
    util.copy(join(self.app_path, 'tizen-manifest.xml'), self.tpk_project_path)

    # copy node
    util.copy(self.node_path, join(self.tpk_project_path, 'bin', 'node'))

    # signing native file
    util.exec_cmd(['signing.sh', self.tpk_project_path], join(path.TOOLS_PATH, 'testutil'))

    # zip
    zip_file = ('%s.tpk' % self.app_name)
    util.exec_shell(['zip', zip_file, '*', '-r'],
                    self.tpk_project_path)
    output_path = join(path.OUT_PATH, zip_file)
    util.move(join(self.tpk_project_path, zip_file), output_path)

    # package tpk
    util.exec_shell(['tizen', 'package', '-t', 'tpk', '--', '%s' % output_path], path.OUT_PATH)
    return output_path

  def run(self):
    self.create()
    return self.package()

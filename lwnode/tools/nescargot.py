#!/usr/bin/env python

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

import argparse
import os
from testutil import util, path, log
from testutil.builder import TPKProject
from testutil.connection import DTV

def parse_options():
  parser = argparse.ArgumentParser()
  parser.add_argument('--all',
            default=False, action='store_true',
            help='package, install and start application')
  parser.add_argument('--package',
            default=False, action='store_true',
            help='package application')
  parser.add_argument('--install',
            default=False, action='store_true',
            help='install application on target')
  parser.add_argument('--start',
            default=False, action='store_true',
            help='start application on target')
  return parser.parse_args()

def main():
  options = parse_options()

  pwd = os.getcwd()
  package_json_path = os.path.join(pwd, 'package.json')
  if not os.path.exists(package_json_path):
    raise Exception('Cannot find package.json')
  package_json = util.read_json_file(package_json_path)
  if not 'nescargot' in package_json:
    package_json['nescargot'] = {}

  config = {
    'app_path': pwd,
    'app_name': package_json['name'],
    'ip': package_json['nescargot'].setdefault('target-ip', None),
    'native': package_json['nescargot'].setdefault('native', 'no'),
    'node': package_json['nescargot'].setdefault('node', 
      os.path.join(path.ROOT_PATH, 'out', 'Release', 'node'))
  }

  if not config['ip']:
    config['ip'] = os.environ.get('NESCARGOT_CONFIG_TARGET_IP')

  log.info('Configuration')
  log.info('app_name: %s' % config['app_name'])
  log.info('target_ip: %s' % config['ip'])
  log.info('node: %s' % config['node'])

  if options.all:
    options.package = True
    options.install = True
    options.start = True

  app_output = None
  if options.package:
    project = TPKProject(config)
    project.create()
    app_output = project.package()

  dtv = DTV(config)
  try:
    if options.install:
      if not config['ip']:
        raise Exception('Cannot know your target ip')
      dtv.install_app(app_output)

    if options.start:
      if not config['ip']:
        raise Exception('Cannot know your target ip')
      dtv.start_app()
  finally:
    dtv.close()


if __name__ == '__main__':
  main()

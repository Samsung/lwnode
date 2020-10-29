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

import socket
import paramiko
from scp import SCPClient, SCPException
import util
import path
import log
import os

class TimeoutException(Exception):
  pass

class SSH():
  def __init__(self, device_info):
    self.username = 'root'
    self.password = None #TODO
    self.ip = device_info['ip']
    self.timeout = 60
    self.prompt = '#'

    # to avoid getting password.
    self.ssh = paramiko.client.SSHClient()
    if not self.password:
      self.ssh.load_system_host_keys()
    self.ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())

  def open(self):
    self.ssh.connect(hostname=self.ip, username=self.username,
                    password=self.password, look_for_keys=not bool(self.password))

    log.info('Connect %s' % self.ip)
    self.chan = self.ssh.invoke_shell()
    self.read_until(self.prompt)

  def close(self):
    log.info('Disconnect %s' % self.ip)
    self.ssh.close()

  def exec_command(self, cmd):
    self.chan.settimeout(self.timeout)
    try:
      self.send(cmd)
      data = self.read_until(self.prompt)
    except socket.timeout:
      raise TimeoutException

    return data

  def send(self, cmd):
    self.chan.send(cmd + '\n')

  def read_until(self, expected, verbos=True):
    temp = ''
    while expected not in temp:
      char = self.chan.recv(1)
      temp += char
      if verbos:
        log.log(char)

    temp = temp.split('\r\n')
    if verbos:
      log.log('\n')

    try:
      temp.pop()
      return temp[-1]
    except IndexError:
      log.error('Index Error from SSH')
      pass

  def copy_file(self, localpath, remotepath='/tmp'):
    log.info('Copy file: %s -> %s' % (localpath, remotepath))
    try:
      with SCPClient(self.ssh.get_transport()) as scp:
        scp.put(localpath, remotepath, preserve_times=True)
    except SCPException:
      raise SCPException.message

class DTV():
  def __init__(self, options):
    self._ssh = SSH(options)
    self._is_connect = False
    self.app_path = options['app_path']
    self.app_name = options['app_name']

  def open(self):
    if not self._is_connect:
      self._ssh.open()
      self._is_connect = True

  def close(self):
    if self._is_connect:
      self._ssh.close()
      self._is_connect = False

  def install_app(self, file_path):
    if not file_path:
      file_path = os.path.join(path.OUT_PATH, '%s.tpk' % self.app_name)
      if not os.path.exists(file_path):
        file_path = os.path.join(path.OUT_PATH, '%s.wgt' % self.app_name)
        if not os.path.exists(file_path):
          raise Exception("Cannot fine app file")

    self.open()
    self._ssh.copy_file(file_path)
    file_name, file_extension = os.path.splitext(file_path)
    app_type = file_extension[1:]

    log.info('Install App: %s' % self.app_name)
    self._ssh.exec_command('cd /tmp')
    self._ssh.exec_command('pkgcmd -i -t %s -p %s.%s'
      % (app_type, self.app_name, app_type)) # install app

  def start_app(self):
    self.open()
    self._ssh.exec_command('app_launcher -s %s' % self.app_name)


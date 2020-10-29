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

from __future__ import print_function

CVIOLET = '\33[35m'
CEND      = '\33[0m'
CBOLD     = '\33[1m'
CYELLOW = '\33[33m'
CGREEN  = '\33[32m'
CRED    = '\33[31m'
CBLUEBG   = '\33[44m'

def info(msg):
  print(CGREEN + msg + CEND)

def warn(msg):
  print(CYELLOW + msg + CEND)

def error(msg):
  print(CRED + msg + CEND)

def log(msg):
  print(msg, end='')

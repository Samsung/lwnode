#!/bin/bash

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

if [ -z $BASH_ENV ]; then
  echo "* Guard: BASH_ENV is empty. This script is intended to run on CircleCI."
  exit 1
fi

function install_jdk() {
  wget https://download.java.net/java/GA/jdk12/GPL/openjdk-12_linux-x64_bin.tar.gz
  tar xzf openjdk-12_linux-x64_bin.tar.gz
  JDK_PATH=$PWD/jdk-12

  sudo update-alternatives --install "/usr/bin/java" "java" "$JDK_PATH/bin/java" 1
  sudo update-alternatives --install "/usr/bin/javac" "javac" "$JDK_PATH/bin/javac" 1

  sudo update-alternatives --config java
  sudo update-alternatives --config javac

  java -version
}

sudo apt-get install -y -q npm zip
wget http://download.tizen.org/sdk/Installer/tizen-studio_3.7/web-cli_Tizen_Studio_3.7_ubuntu-64.bin
chmod +x web-cli_Tizen_Studio_3.7_ubuntu-64.bin; ./web-cli_Tizen_Studio_3.7_ubuntu-64.bin --accept-license ~/tizen-studio
echo 'export PATH=~/tizen-studio/package-manager:~/tizen-studio/tools/ide/bin/:$PATH' >> $BASH_ENV; source $BASH_ENV
tizen security-profiles add -n tizen_author -a ./tools/certificates/tizen_author.p12 -p tizenauthor -d ./tools/certificates/tizen-distributor-partner-manufacturer-signer.p12 -dp tizenpkcs12passfordsigner
sed -i 's|/home/circleci/project/tools/certificates/tizen_author.pwd|tizenauthor|g' ~/tizen-studio-data/profile/profiles.xml
sed -i 's|/home/circleci/project/tools/certificates/tizen-distributor-partner-manufacturer-signer.pwd|tizenpkcs12passfordsigner|g' ~/tizen-studio-data/profile/profiles.xml

cat ~/tizen-studio-data/profile/profiles.xml

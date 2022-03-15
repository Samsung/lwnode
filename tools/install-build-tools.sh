#!/bin/bash

# Copyright 2020-present Samsung Electronics Co., Ltd.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

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

function install_node() {
  curl -sL https://deb.nodesource.com/setup_14.x | sudo -E bash -
  sudo apt install nodejs
  node -v
  npm -version
}

install_node

sudo apt-get install -y -q zip rsync
wget http://download.tizen.org/sdk/Installer/tizen-studio_3.7/web-cli_Tizen_Studio_3.7_ubuntu-64.bin
chmod +x web-cli_Tizen_Studio_3.7_ubuntu-64.bin; ./web-cli_Tizen_Studio_3.7_ubuntu-64.bin --accept-license ~/tizen-studio
echo 'export PATH=~/tizen-studio/package-manager:~/tizen-studio/tools/ide/bin/:$PATH' >> $BASH_ENV; source $BASH_ENV
tizen security-profiles add -n tizen_author -a ./tools/certificates/tizen_author.p12 -p tizenauthor -d ./tools/certificates/tizen-distributor-partner-manufacturer-signer.p12 -dp tizenpkcs12passfordsigner
sed -i 's|/home/circleci/project/tools/certificates/tizen_author.pwd|tizenauthor|g' ~/tizen-studio-data/profile/profiles.xml
sed -i 's|/home/circleci/project/tools/certificates/tizen-distributor-partner-manufacturer-signer.pwd|tizenpkcs12passfordsigner|g' ~/tizen-studio-data/profile/profiles.xml

cat ~/tizen-studio-data/profile/profiles.xml

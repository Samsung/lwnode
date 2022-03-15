#!/bin/bash

# How to release
# 0. Precondition: clone source and destination repos
# git clone git@github.sec.samsung.net:lws/sqlite3-service.git sqlite3-service-clean
# git clone -b release git@github.sec.samsung.net:lws/sqlite3-service.git sqlite3-service-release
# git git clone git@github.sec.samsung.net:VD-20-B2B-NodeSqlite3/node-sqlite3.git
#
# 1. Run this script
# cd sqlite3-service-clean
# ./tools/release.sh [ node-sqlite3 | sqlite3-service-release ]

ROOT=`pwd`

repo=$1

if [ "$repo" == "node-sqlite3" ] || [ "$repo" == "sqlite3-service-release" ]; then
  echo "Syncing with: $repo"
else
  echo "Usage: $0 [ node-sqlite3 | sqlite3-service-release ]"
  exit 0
fi

if [ "$repo" == "node-sqlite3" ] && [ ! -d ../node-sqlite3 ]; then
  # git clone git@github.sec.samsung.net:VD-20-B2B-NodeSqlite3/node-sqlite3.git
  echo "Error: $repo not exist"
  exit 0
elif [ "$repo" == "sqlite3-service-release" ] && [ ! -d ../sqlite3-service-release ]; then
  # git clone -b release git@github.sec.samsung.net:lws/sqlite3-service.git sqlite3-service-release
  echo "Error: $repo not exist"
  exit 0
fi

if [ ! $(node -v | grep -q "12.20.0") ]; then
  source ~/.nvm/nvm.sh
  nvm use 12.20.0
  if [ $? != 0 ]; then
    echo "Error: Need nvm and node 12.20.0 (nvm install 12.20.0)"
    exit 0
  fi
fi

clean_repo() {
  git clean -xfd
  git submodule foreach --recursive git clean -xfd
}

echo "Found: $repo"

# Clean repos
clean_repo
pushd ../$repo
clean_repo
popd

# Install packages
git submodule update --init
npm install
npm run build:all_noenv
npm run docs
rm -rf ./node_modules
npm install --production
./tools/removeNPMAbsolutePaths/bin/removeNPMAbsolutePaths .

# sync source repo to $repo
rsync -avmL --delete --delete-excluded --filter="merge tools/sqlite3-filter.txt" . ../$repo

pushd ../$repo
cp .circleci/config_release.yml .circleci/config.yml

hash=`git log | head -1 | cut -f2 -d' ' | cut -c 1-7`
today=`date +%y%m%d`
msg="Node_SQLite3_Release_$today""_$hash"

echo "======================================="
echo git commit -m "$msg"
echo "======================================="

git add -A
git commit -m "$msg"
popd

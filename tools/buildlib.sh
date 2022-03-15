#!/bin/bash

set -e

if ! command -v npx &> /dev/null; then
    echo "npm should be installed"
    exit 1
fi

cd frontend

rm -rf dist

npx parcel build lib/libsqlite.js --global SQLite3 \
  --no-source-maps \
  --out-dir=dist \
  --out-file=libsqlite.js

cd -

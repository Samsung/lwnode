# Node-Sqlite3 Service
Node-Sqlite3 is a pure JavaScript interface to libsqlite3, which allows a Web App to connect and
utilize a sqlite3 database.

## Supported Platforms
* Ubuntu 18.04, 16.04
* Tizen 4.0 and above

## Update submodule
```sh
git submodule update --init --recursive
```

## How to Run Tests
### Prerequisite
Set up a node environment on a Ubuntu machine. Then run:
```sh
npm install
npm run build:all
```

### Run
```sh
npm run test
```

### Analyze line coverage
```sh
npm run coverage
```

## How to Compile: Ubuntu
### Prerequisite
Set up a node environment on a Ubuntu machine. Then run:
```sh
npm install
npm run build:all
```

### Run
The following environment variables can be set:
* DB_STORE: it is a directory where libsqlite3 DBs are to be read and written.
* DEBUG: (optional) it displays packets sent and received from the backend.
```sh
export DB_STORE='/path/to/DB/store'
// export DEBUG="rpc:*,sql:*,api:*"
npm run start
```

```sh
export DB_STORE='/path/to/DB/store'
cp sample/script/test/support/*.db $DB_STORE  // copy sample dbs used by testcases
google-chrome sample/index.html
```

## How to Compile: Tizen
### Prerequisite
Set up a node environment on a Ubuntu machine. Then run:
```sh
npm install
npm run build:all
```

### Build a wgt package
```sh
npm run build
./tools/buildwgt.sh
```
### Build rpms
```sh
gbs -c ./packaging/gbs.conf build -A armv7l --incremental --include-all --define 'build_native true'
```

### Install the .wgt and .rpm
Copy ``.wgt`` and ``.rpm`` to a target device. Then, run:
```sh
rpm -Uvh node-sqlite3-ep-1.0.0-1.armv7l.rpm --force --nodeps
pkgcmd -i -t wgt -p sqlite3.wgt
```

### Run the Web App
The node service starts automatically when it is installed.
Only the Web app needs to be started manually.
The Web app runs a set of libsqlite3 testcases.

```sh
app_launcher -s OwOD84MFBg.sqlite3
```

To print node-sqlite3 service log, run:
```sh
dlogutil NODE_CONSOLE
```

## Maintainers
A list of maintainers can be found in [MAINTAINERS.md](MAINTAINERS.md).

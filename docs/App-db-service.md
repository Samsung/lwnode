# DB Service and Client App Guide

In this guide, we will show you how to write a `hello world` database service app. To do so, two apps are required, one for running a database service on a server, and a client app, which accesses to the database service. A server is usually a Tizen device, such as a Tizen TV, and a client is typically a Tizen app. The database service runs on top of a Web server, which in turn runs on `lwnode` in our framework.

## 1. Run DB service on a server

### 1.1. Build and install a DB service app
[Build lwnode rpms](Build.md) and
install `lwnode-sqlite3.armv7l.rpm` on your device. The rpms can also be downloaded from [tizen.org](https://download.tizen.org/snapshots/tizen/unified/latest/repos/standard/packages/armv7l/).
```sh
$ rpm -Uvh lwnode-sqlite3.armv7l.rpm
```

### 1.2. Run the DB service app
Run the DB service app on your device.
```sh
$ lwnode /usr/apps/lwnode/apps/sqlite3/script
```

We use `lwnode` to run a Web server, which in turn, runs a DB server. When successful, a message below is printed.
```sh
[xx:xx:xx][INFO] listening port: 8140
```


## 2. Run a client app
### 2.1. Download `libsqlite.js`
Download `libsqlite.js` from `http://<your device ip>:8140/api/download/libsqlite.js`. A client app will use this library to communicate with the DB service.
```sh
$ wget http://<your device ip>:8140/api/download/libsqlite.js
```

### 2.2. Add `libsqlite.js` library to your client Web app
Add the library to your Tizen Web app. This library provides Database API required to commnunite with the DB service.

```html
<!-- index.html -->
<script src="./libsqlite.js"></script>
```

### 2.3. Run DB operations on the client
To perform DB operations in your Web app, use the `sqlite3` object that is defined in `global`.

The following example shows you how to create a table.

```js
// main.js
const DB_HOSTNAME = '127.0.0.1';
const DB_PORT = 8140;
const DB_PATH = '/';
const url = `ws://${DB_HOSTNAME}:${DB_PORT}${DB_PATH}`;

window.onload = async function () {
  sqlite3.configure(url);

  // use the sqlite3 object to access to Database service
  let db = await new sqlite3.Database(':memory:', sqlite3.OPEN_READWRITE | sqlite3.OPEN_CREATE);

  const q = `CREATE TABLE user (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        name text,
        email text UNIQUE,
        password text,
        CONSTRAINT email_unique UNIQUE (email)
        );`;
  try {
    let result = await db.run(q);
    console.log('success', result);
  } catch (err) {
    console.log('fail');
    throw err;
  }
};
```

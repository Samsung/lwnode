# Node-Sqlite3 Service
Node-Sqlite3 is a pure JavaScript interface to libsqlite3, which allows a Web App to connect and
utilize a sqlite3 database.


## How to setup and run sqlite3 server app

### 1. Install sqlite3 app
Install `lwnode-sqlite3-x.x.x.armv7l.rpm` on your device.
```sh
rpm -Uvh lwnode-sqlite3-1.0.0-1.armv7l.rpm
```

### 2. Run sqlite3 server app
Run sqlite3 server app on your device.
```sh
lwnode /usr/apps/lwnode/apps/sqlite3/script
```
Then you will see the message below.
```
[xx:xx:xx][INFO] listening port: 8140
```


## How to Run sqlite3 app
### 1. Download `libsqlite.js` and config app
First, load `libsqlite.js` on your web app.
You can get this file from `http://<your device ip>:8140/api/download/libsqlite.js`.

### 2. load `libsqlite.js`
Add the script below to your web app.
```html
<!-- index.html -->
<script src="./libsqlite.js"></script>
```

### 3. configure sqlite3
To use sqlite3 on your web app, first complete the configuration.
```js
// main.js
const DB_HOSTNAME = '127.0.0.1';
const DB_PORT = 8140;
const DB_PATH = '/';
const url = `ws://${DB_HOSTNAME}:${DB_PORT}${DB_PATH}`;

window.onload = function () {
  sqlite3.configure(url);
};
```

### 4. Use sqlite3 global variable
To use sqlite3 db service, take `sqlite3` variable that is defined in `global`.
The following is an example of creating a table.

```js
// main.js
let db = await new sqlite3.Database(':memory:', sqlite3.OPEN_READWRITE | sqlite3.OPEN_CREATE);

const q = `CREATE TABLE user (
      id INTEGER PRIMARY KEY AUTOINCREMENT,
      name text,
      email text UNIQUE,
      password text,
      CONSTRAINT email_unique UNIQUE (email)
      );`;
try {
  await db.run(q);
} catch (err) {
  throw err;
}
```

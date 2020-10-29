## How to use nescargot util

### Install python module
```bash
pip install paramiko
pip install scp
```

If you fail to install scp because of `pyOpenSSL`, try this command `sudo python -m easy_install --upgrade pyOpenSSL`.

### Set up
#### Add ssh-id in your target
To login ssh of TV, copy your ssh-id with the command below
```bash
ssh-copy-id root@address
```
#### Add PATH
Add nescargot/tools path
You should add `<project_path>/nescargot/tools` in your linux PATH.

#### Add package.json in your application
You should add `nescargot` propery in package.json.
For example:
```json
{
    "name": "hello-world-node",
    "nescargot": {
        "target-ip": "0.0.0.0"
    },
    "scripts": {
        "build": "nescargot.py --all",
        "start": "nescargot.py --start"
    }
}
```

You can also add config by writting environment value.
For example:
```
export NESCARGOT_CONFIG_TARGET_IP="0.0.0.0"
```

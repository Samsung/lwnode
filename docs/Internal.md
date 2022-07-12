# Useful information for internal collaborators
This guide contains useful information for internal collaborators of `lwnode`.

## How to release `lwnode` code for Tizen

1. Checkout clean `lwnode` repos from both github and Tizen gerrit.

```sh
git clone git@github.com:Samsung/lwnode.git lwnode
git clone git://git.tizen.org/platform/framework/web/lwnode lwnode-tizen
cd lwnode-tizen
git checkout tizen
```

2. Sync `lwnode-tizen` with `lwnode`.
```sh
cd lwnode
./tools/release.sh lwnode-tizen
// A new commit is created in the `lwnode-tizen` repo
```

3. Apply the new commit to Tizen gerrit.

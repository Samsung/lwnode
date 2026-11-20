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

## String boundary checks

The UTF-8 to Latin-1 fast path rejects malformed and truncated sequences.
Returning `false` lets the loader use its UTF-16 conversion path instead.
If UTF-16 conversion fails, the reloadable source loader returns no source.
CommonJS then reads the file through its regular `fs.readFileSync()` path.

`String::WriteOneByte()` copies at most the requested number of characters.
For UTF-16 storage, it writes the low byte of each code unit directly.
It adds a null terminator only when the options allow it and space remains.

Shebang handling requires at least two source characters before checking `#!`.

#!/bin/bash

ND_ROOT=$1
LWNODE_ROOT=$2

if [ -z "$ND_ROOT" ] || [ -z "$LWNODE_ROOT" ]; then
  echo "Usage: $0 <ND_ROOT> <LWNODE_ROOT> [--execute]"
  exit 1
fi

if [ ! -d "$ND_ROOT" ]; then
  echo "Error: $ND_ROOT is not a directory."
  exit 1
fi

if [ ! -d "$LWNODE_ROOT" ]; then
  echo "Error: $LWNODE_ROOT is not a directory."
  exit 1
fi

MSG_PORT_ROOT=${LWNODE_ROOT}/deps/message-port

dry_run=true
if [ "$3" == "--execute" ]; then
  dry_run=false
  shift
fi

dirs_and_targets=(
  "${ND_ROOT}/deps/message-port ${MSG_PORT_ROOT}"
)

files_and_targets=(
  # ${MSG_PORT_ROOT}/nd/
  "${ND_ROOT}/src/es-helper.cc ${MSG_PORT_ROOT}/nd/"
  "${ND_ROOT}/src/es-helper.h ${MSG_PORT_ROOT}/nd/"
  "${ND_ROOT}/src/es.cc ${MSG_PORT_ROOT}/nd/"
  "${ND_ROOT}/src/es.h ${MSG_PORT_ROOT}/nd/"
  "${ND_ROOT}/src/gc-util.cc ${MSG_PORT_ROOT}/nd/"
  "${ND_ROOT}/src/gc-util.h ${MSG_PORT_ROOT}/nd/"
  "${ND_ROOT}/src/nd-debug.cc ${MSG_PORT_ROOT}/nd/"
  "${ND_ROOT}/src/nd-debug.h ${MSG_PORT_ROOT}/nd/"
  "${ND_ROOT}/src/nd-logger.cc ${MSG_PORT_ROOT}/nd/"
  "${ND_ROOT}/src/nd-logger.h ${MSG_PORT_ROOT}/nd/"
  "${ND_ROOT}/src/utils/sf-vector.h ${MSG_PORT_ROOT}/nd/utils/"
  "${ND_ROOT}/src/utils/optional.h ${MSG_PORT_ROOT}/nd/utils/"

  # ${LWNODE_ROOT}/src/lwnode/
  "${ND_ROOT}/src/nd-mod-base.h ${LWNODE_ROOT}/src/lwnode/"
  "${ND_ROOT}/src/nd-mod-base.cc ${LWNODE_ROOT}/src/lwnode/"
  "${ND_ROOT}/src/nd-mod-message-port.cc ${LWNODE_ROOT}/src/lwnode/"
  "${ND_ROOT}/src/nd-vm-message-channel.cc ${LWNODE_ROOT}/src/lwnode/"

  # ${LWNODE_ROOT}/include/lwnode/
  "${ND_ROOT}/deps/message-port/message-port.h ${LWNODE_ROOT}/include/lwnode/"
  "${ND_ROOT}/src/nd-vm-message-channel.h ${LWNODE_ROOT}/include/lwnode/"
  "${ND_ROOT}/src/uv-loop-holder.h ${LWNODE_ROOT}/include/lwnode/"
)

# Create hard links
for entry in "${dirs_and_targets[@]}"; do
  dir=$(echo $entry | awk '{print $1}')
  target=$(echo $entry | awk '{print $2}')
  if [ "$dry_run" = true ]; then
    echo "[DRY RUN] create a hard link : $dir -> $target"
  else
    if ! [ -d "$target" ]; then
      cp -alv "$dir" "$target"
    fi
  fi
done

for entry in "${files_and_targets[@]}"; do
  file=$(echo $entry | awk '{print $1}')
  target=$(echo $entry | awk '{print $2}')
  if [ "$dry_run" = true ]; then
    echo "[DRY RUN] create a hard link : $file -> $target"
  else
    mkdir -p "$target"
    ln -vf "$file" "$target"
  fi
done

if [ "$dry_run" = true ]; then
  echo "To execute, run this script with the --execute."
fi

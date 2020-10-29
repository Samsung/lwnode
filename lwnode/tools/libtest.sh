#!/bin/bash

# Copyright (c) 2020-present Samsung Electronics Co., Ltd
#
#  This library is free software; you can redistribute it and/or
#  modify it under the terms of the GNU Lesser General Public
#  License as published by the Free Software Foundation; either
#  version 2 of the License, or (at your option) any later version.
#
#  This library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  Lesser General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public
#  License along with this library; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
#  USA

# set -e

remote() {
  if [ ! -z $TARGET ]; then
    ssh -t root@$TARGET $@
  else
    sdb root on
    sdb wait-for-device
    sdb shell $@
  fi
}

install_pkg() {
  local PKG_FPATH=$1
  local PKG_FNAME=$2
  local PKG_ID=$3

  if [[ -z $PKG_FPATH || -z $PKG_FNAME ]]; then
    echo -e "Usage: ${FUNCNAME[0]} <PKG_FPATH> <PKG_FNAME> [PKG_ID]"
    return
  fi

  sdb wait-for-device
  if [ ! -z $PKG_ID ]; then
    echo -e "* uninstall $PKG_ID"
    tizen uninstall -p $PKG_ID
  fi
  echo -e "* install $PKG_FNAME $PKG_ID"
  tizen install -n $PKG_FNAME -- $PKG_FPATH/
}

show_pkg_log() {
  local PKG_NAME=$1
  local PATTERN="$PKG_NAME\|NODE_ESCARGOT\|EscargotDeviceAPI"
  local COMMAND="dlogutil -c; dlogutil | grep '$PATTERN'"
  remote $COMMAND
}

setup_connection() {
  local TARGET=$1
  local HOST_IP=$2
  echo -e "Setup tv connection (target:${TARGET}, host:${HOST_IP})\n"
  ssh -t root@$TARGET "buxton2ctl set-int32 system db/sdk/develop/mode 1"
  ssh -t root@$TARGET "buxton2ctl set-string system db/sdk/develop/ip ${HOST_IP}"
  sdb connect $TARGET
}

run_pkg() {
  local PKG_NAME=$1

  if [[ -z $PKG_NAME ]]; then
      echo -e "Usage: ${FUNCNAME[0]} <PKG_NAME>"
      return
  fi

  remote "app_launcher -k $PKG_NAME; app_launcher -s $PKG_NAME;"
}

trace_memory() {
  local PATTERN=$1
  local N_LABEL=18
  local N_PSS=8
  local N_SWAP=15
  local POS_PSS=$N_LABEL+$N_PSS
  local POS_SWAP=$N_LABEL+$N_SWAP
  local GREPPED="vd_memps -x 1 | grep '$PATTERN\|COMMAND' | grep -v 'grep\|dlog*'"
  local TIME_FT="%m/%d-%H:%M:%S"
  local NPSS_TH=500 # threshold (KB)
  local COMMAND="
    while true; do
      [[ -z \$PSS ]] && { START=\$(date +$TIME_FT); XMEM=-1; NMEM=999999; SQ=0; };
      RESULT=\$($GREPPED);
      TOKENS=(\${RESULT// / });
      PSS="\${TOKENS[$POS_PSS]}";
      SWAP="\${TOKENS[$POS_SWAP]}";
      VPSS=\$(echo \$PSS | sed "s/,//g");
      VSWAP=\$(echo \$SWAP | sed "s/,//g");
      VSUM=\$((VPSS+VSWAP));
      [[ ! -z \$VSUM && \$VSUM -gt \$XMEM ]] && XMEM=\$VSUM;
      [[ ! -z \$VSUM && \$VSUM -lt \$NMEM && \$VSUM -gt $NPSS_TH ]] && NMEM=\$VSUM;
      if [ ! -z \$VSUM ]; then
        let "SQ++";
        echo -e \"\${RESULT}\" | cut -c 0-190;
        date +\"#\$SQ $TIME_FT (\$START, MAX/CUR/MIN \${TOKENS[$N_PSS]}+\${TOKENS[$N_SWAP]}: \$XMEM KB/ \$VSUM KB/ \$NMEM KB)\";\
        echo '';
      fi;
      sleep 1;
    done;"

  echo -e "* grep $PATTERN\n"
  remote $COMMAND
}

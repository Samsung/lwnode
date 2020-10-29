#!/bin/bash

# Replace Tizen CLI with a fix for packaging a hybrid app combined with WEB UI (main) + SERVICE (sub) + Native (sub)
# This fix will be published in Tizen CLI 2.5.23
# Table. Combinations: https://developer.tizen.org/development/training/web-application/application-development-process?#multi

TIZEN_STUDIO_PATH=~/tizen-studio
TIZEN_STUDIO_LIBNCLI_PATH=$TIZEN_STUDIO_PATH/tools/ide/lib-ncli
SCRIPT_PATH="$( cd "$(dirname "$0")" >/dev/null 2>&1; pwd )"

NEW_FILE=$SCRIPT_PATH/tizen-new-cli.jar
OLD_FILE=$TIZEN_STUDIO_LIBNCLI_PATH/tizen-new-cli.jar

cmp -s $NEW_FILE $OLD_FILE || cp -v $NEW_FILE $OLD_FILE

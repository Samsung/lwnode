#!/bin/bash
SINGING_PATH=$1
TIZEN_MAJOR_VERSION=5

if [ -z $SINGING_PATH ]; then
    exit 1
fi

curl -o ./kuep_net_signer.sh http://10.40.68.214/kuep_net_signer.sh && chmod +x ./kuep_net_signer.sh
find $SINGING_PATH -name 'node' -exec ./kuep_net_signer.sh -s -tizen_major_ver $TIZEN_MAJOR_VERSION {} \;
find $SINGING_PATH -name '*.so' -exec ./kuep_net_signer.sh -s -tizen_major_ver $TIZEN_MAJOR_VERSION {} \;
find $SINGING_PATH -name '*.node' -exec ./kuep_net_signer.sh -s -tizen_major_ver $TIZEN_MAJOR_VERSION {} \;
rm kuep_net_signer.sh

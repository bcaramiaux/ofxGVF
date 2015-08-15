#!/bin/sh


MaxSDK="max6-sdk"

if [ -d "$MaxSDK" ]; then
	rm -rf $MaxSDK
fi
echo "Cloning max sdk"
git clone --depth 1 https://github.com/Cycling74/max6-sdk.git



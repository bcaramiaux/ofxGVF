#!/bin/sh


MaxSDK="max6-sdk"

echo "Cloning max sdk"

if [ -d "$MaxSDK" ]; then
	rm -rf $MaxSDK
fi

git clone --depth 1 https://github.com/Cycling74/max6-sdk.git



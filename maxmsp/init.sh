#!/bin/sh


MaxSDK="max-sdk"
if [ -d "$MaxSDK" ]; then
	rm -rf $MaxSDK
fi
echo "Cloning MaxMSP SDK from github.com/Cycling74"
git clone --depth 1 https://github.com/Cycling74/max-sdk.git

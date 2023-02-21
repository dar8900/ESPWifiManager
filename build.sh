#!/bin/bash

source /root/.platformio/penv/bin/activate
RET=`pio run -e esp8266`
# echo "$RET"
string='SUCCESS'
if [[ "$RET" == *"$string"* ]]; then
	echo "OK"
	RET=0
else
	RET=1
fi

if [ "$RET" -eq 1 ]; then
	echo "KO"
	exit 1
fi

RET=`pio run -e esp32`
# echo "$RET"
string='SUCCESS'
if [[ "$RET" == *"$string"* ]]; then
	echo "OK"
	RET=0
else
	RET=1
fi

if [ "$RET" -eq 1 ]; then
	echo "KO"
	exit 1
fi


exit 0
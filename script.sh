#!/bin/bash
chmod 444 "$1";

if [ "$(wc -l < "$1")" -ge 3 ]; 
then
        echo "Safe";
        exit 0;
fi

if [ "$(wc -w < "$1")" -le 1000 ]; 
then
        echo "Safe" ;
        exit 0;
fi

if [ "$(wc -m < "$1")" -le 2000 ]; 
then
        echo "Safe";
        exit 0;
fi


if grep -qE "(dangerous|malicious|risky|attack|malware|corrupted)" "$1";
    then
        echo "$1"
        chmod 000 "$1"
        exit 1;
    fi

if grep -qE "['\x80' - '\xFF']" "$1";
    then
        echo "$1"
        chmod 000 "$1"
        exit 1;
    fi

echo "Safe";
chmod 000 "$1";
exit 0;

#!/bin/bash
chmod 777 "$1";

# shellcheck disable=SC2000
echo "Numar caractere : $(echo "$1" | wc -m) ";
echo "Numar linii : $(echo "$1" | wc -l) ";
echo "Numar cuvinte : $(echo "$1" | wc -w) ";


if grep -qE "(dangerous|malicious|risky)" "$1";
    then
        mv "$1" "$2";
        exit 1;
    fi

if grep -qE "['\x80' - '\xFF']" "$1";
    then
        mv "$1" "$2";
        exit 1;
    fi

exit 0;

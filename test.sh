#!/bin/sh

make &> /dev/null

GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

r () {
    echo "$1"
    ./simm -e "$1" > tmp.s
    gcc -e main  -o tmp0 tmp.s
    ./tmp0
    ret=$?
    if [ $ret -eq $2 ]; then
        echo "  ${GREEN}OK${NC}: $2"
    else
        echo "  ${RED}FAIL${NC}: $ret"
        exit 1
    fi
    rm -rf tmp0 tmp.s
}

r "1 + 2" 3
r "1 + 2 * 3" 7
r "42 / 2 + 1" 22

#!/bin/sh

rm -rf tmp
mkdir tmp

cp -f ../src/conf/* tmp/
cp -f ../src/sectun tmp/
cp -f start_server.sh tmp/
cp -f client-* tmp/
cp -f start_client* tmp/

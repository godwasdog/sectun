#!/bin/sh

rm -rf server
mkdir server

cp -f ../src/conf/server* server/
cp -f ../src/sectun server/
cp -f start_sectun_server* server/


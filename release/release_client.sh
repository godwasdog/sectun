#!/bin/sh

rm -rf client
mkdir client

cp -f ../src/conf/client* client/
cp -f ../src/sectun client/
cp -f start_sectun_client* client/


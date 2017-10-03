#!/bin/sh

rm -rf sectun
mkdir sectun
scp -r root@192.168.2.1:/root/sectun/test/tmp/* sectun/

rm -rf server
mkdir server
cp sectun/server* server/
cp sectun/sectun server/
cp sectun/start_server.sh server/


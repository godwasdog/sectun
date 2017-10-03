#!/bin/sh

rm -rf sectun
mkdir sectun
scp -r root@192.168.56.101:/root/sectun/* sectun/

rm -rf client
mkdir client
cp sectun/client* client/
cp sectun/sectun client/
cp sectun/start_client* client/


#!/bin/sh

## start server here
./sectun -c server-config.json -s start  > /var/log/sectun-server.log  2>&1 &



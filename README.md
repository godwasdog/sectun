# sectun
secret tunnel VPN based on kcp

cd $(code_dir)

./autogen.sh

./configure

make

cd test
./release.sh # generate test release  code + scripts

login to test server,run
./pull_server.sh   would pull server code for testing

login to client, run
./pull_client.sh   would pull client code for testing

cd release
./release.sh # generate release code + scripts

push to github

cd /mnt/DEV/GitRoot/sectun-github.git

git push



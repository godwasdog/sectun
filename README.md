# sectun
secret tunnel VPN based on kcp

cd $(code_dir)

./autogen.sh

./configure

make

cd test
./release.sh # generate test release  code + scripts

cd release
./release.sh # generate release code + scripts

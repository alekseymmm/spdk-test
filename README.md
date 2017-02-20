# spdk-test
Test intel SPDK library

run comand tests
./spdk-test -M 50 -w randrw -c 0xfff -q 32 -s 4096 -t 300 -r 'trtype:RDMA adrfam:IPv4 traddr:10.30.0.11 trsvcid:4420 subnqn:nqn.2016-06.io.spdk:cnode11_1'
./spdk-test -w randrread -c 0xfff -q 32 -s 4096 -t 300 -r 'trtype:RDMA adrfam:IPv4 traddr:10.30.0.11 trsvcid:4420 subnqn:nqn.2016-06.io.spdk:cnode11_1'
./spdk-test -w randrwrite -c 0xfff -q 32 -s 4096 -t 300 -r 'trtype:RDMA adrfam:IPv4 traddr:10.30.0.11 trsvcid:4420 subnqn:nqn.2016-06.io.spdk:cnode11_1'

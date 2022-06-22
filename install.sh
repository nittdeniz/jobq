#!/bin/bash
sudo mkdir -p /etc/jobq
sudo chown -R root:jobq /etc/jobq

mkdir -p build
cd build
make
cd ..

sudo cp jobq_exec.sh /etc/jobq/.jobq_exec
sudo cp jobq_server_start.sh /usr/local/sbin/jobq_server_start
sudo cp build/jobq_server /usr/local/sbin/jobq_server
sudo cp build/jobq_server_stop /usr/local/sbin/jobq_server_stop
sudo cp build/jobq_status /usr/local/sbin/jobq_status
sudo cp build/jobq_stop /usr/local/sbin/jobq_stop
sudo cp build/jobq_submit /usr/local/sbin/jobq_submit

sudo chown -R root:jobq  /usr/local/sbin/jobq_server
sudo chown -R root:jobq  /usr/local/sbin/jobq_server_stop
sudo chown -R root:jobq  /usr/local/sbin/jobq_status
sudo chown -R root:jobq  /usr/local/sbin/jobq_stop
sudo chown -R root:jobq  /usr/local/sbin/jobq_submit

sudo setfacl -Rm g:jobq:rxw /etc/jobq
sudo setfacl -Rm g:jobq:rxw /usr/local/sbin
sudo setfacl -Rx u:jobq /etc/jobq
sudo setfacl -Rx u:jobq /usr/local/sbin

sudo setfacl -m g:jobq:x /usr/local/sbin/jobq_submit
sudo setfacl -m g:jobq:x /usr/local/sbin/jobq_server
sudo setfacl -m g:jobq:x /usr/local/sbin/jobq_server_stop
sudo setfacl -m g:jobq:x /usr/local/sbin/jobq_server_start
sudo setfacl -m g:jobq:x /usr/local/sbin/jobq_status
sudo setfacl -m g:jobq:x /usr/local/sbin/jobq_stop

sudo setfacl -x g:jobq /usr/local/sbin/jobq_submit
sudo setfacl -x g:jobq /usr/local/sbin/jobq_server
sudo setfacl -x g:jobq /usr/local/sbin/jobq_server_stop
sudo setfacl -x g:jobq /usr/local/sbin/jobq_server_start
sudo setfacl -x g:jobq /usr/local/sbin/jobq_status
sudo setfacl -x g:jobq /usr/local/sbin/jobq_stop
sudo setfacl -x g:jobq /usr/local/sbin/jobq_submit
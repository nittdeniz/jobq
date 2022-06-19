#!/bin/bash
sudo mkdir -p /etc/jobq
sudo setfacl -m u:$USER:rw /etc/jobq
sudo chown -R $USER /etc/jobq
cp jobq_exec.sh /etc/jobq/jobq_exec.sh
mkdir -p build
cd build
git pull
make
cp jobq_server /usr/local/sbin/jobq_server
cp jobq_server_start /usr/local/sbin/jobq_server_start
cp jobq_server_stop /usr/local/sbin/jobq_server_stop
cp jobq_status /usr/local/sbin/jobq_status
cp jobq_stop /usr/local/sbin/jobq_stop
cp jobq_submit /usr/local/sbin/jobq_submit
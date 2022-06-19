#!/bin/bash
sudo mkdir -p /etc/jobq
sudo setfacl -m u:$USER:rw /etc/jobq
sudo chown -R $USER /etc/jobq
cp jobq_exec.sh /etc/jobq/jobq_exec.sh
mkdir -p build
cd build
make
sudo cp jobq_server /usr/local/sbin/jobq_server
sudo cp jobq_server_start /usr/local/sbin/jobq_server_start
sudo cp jobq_server_stop /usr/local/sbin/jobq_server_stop
sudo cp jobq_status /usr/local/sbin/jobq_status
sudo cp jobq_stop /usr/local/sbin/jobq_stop
sudo cp jobq_submit /usr/local/sbin/jobq_submit
#!/bin/bash
sudo mkdir -p /etc/jobq
sudo setfacl -m u:$USER:rxw /etc/jobq
sudo chown -R $USER /etc/jobq
cp jobq_exec.sh /etc/jobq/jobq_exec.sh
sudo cp jobq_server_start.sh /usr/local/sbin/jobq_server_start
mkdir -p build
cd build
make
sudo cp jobq_server /usr/local/sbin/jobq_server
sudo cp jobq_server_stop /usr/local/sbin/jobq_server_stop
sudo cp jobq_status /usr/local/sbin/jobq_status
sudo cp jobq_stop /usr/local/sbin/jobq_stop
sudo cp jobq_submit /usr/local/sbin/jobq_submit
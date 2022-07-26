#!/bin/bash

chown root jobq
chown root jobq_server
chgrp root jobq
chgrp root jobq_server
chmod 744 jobq_server
chmod 755 jobq
mv jobq /usr/local/sbin/
mv jobq_server /opt/jobq/
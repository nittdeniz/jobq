#!/bin/bash

if ! pgrep -x jobq_server > /dev/null; then
  /etc/jobq/job_server &> /dev/null 2>&1 &
  if pgrep -x jobq_server > /dev/null; then
    echo "Server started."
  else
    echo "Server could not be started."
  fi
else
  echo "Server already running."
fi

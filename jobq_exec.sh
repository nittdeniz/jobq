#!/bin/bash
cd $1
$5 1> $2 2> $3 &
echo $! > $4 2>&1
#!/bin/bash
cd $1
$4 1> $2 2> $3 &
echo $! > $4 2>&1
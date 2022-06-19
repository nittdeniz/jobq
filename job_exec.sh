#!/bin/bash
$4 1> $1 2> $2 &
echo $! > $3 2>&1
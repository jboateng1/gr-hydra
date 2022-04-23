#!/bin/bash
echo ==================================================
echo              Creating container$1
echo ==================================================
echo
#echo HERE IT IS: \|_\|
docker run -itd --name vr$1 ubuntu:20.04 bash
#echo arguments: \'$1\' \'$2\' \'$3\'
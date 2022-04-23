#!/usr/bin/python
import docker
import sys
client = docker.from_env()

print("================================")
print(  "Creating Docker Container" + sys.argv[1]   )
print("================================")
print("")
client.containers.run(image="ubuntu:20.04", command=["bash"], name=sys.argv[1], stdin_open=True, tty=True, detach=True)

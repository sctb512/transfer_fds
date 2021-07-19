# transfer_fds
transfer fds with unix socket

## how to run?
1. run server
```shell
./fserver
```
2. run daemon
```shell
./daemon
```
now you can send message to server.
3. run client
```shell
./fclient
```
this time, server will send fds to client, message from daemon will be received by client. at the same time, you can run another daemon to connect with client(new server).

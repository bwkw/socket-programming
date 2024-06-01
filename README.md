## What is
Socket programming using C

## How to set up
### all
Build all server

```
$(~/socket-programming) make all
```

### udp
1. Build udp server

```
$(~/socket-programming) make udp-server
```

2. Build udp client

```
$(~/socket-programming) make udp-client
```

### tcp
1. Build tcp server

```
$(~/socket-programming) make tcp-server
```

2. Build tcp semaphore server

```
$(~/socket-programming) make tcp-semaphore-server
```

3. Build tcp client

```
$(~/socket-programming) make tcp-client
```

## How to use
### udp
1. Run udp server

```
$(~/socket-programming) ./udp/server.out -a (IP address) -p (port)
```
※ Optional arguments need not be specified. In that case, IP address is `127.0.0.1`, port is `12345`

2. Run udp client

```
$(~/socket-programming) ./udp/client.out -a (IP address) -p (port)
```

### tcp
1. Run tcp server

```
$(~/socket-programming) ./tcp/server.out -a (IP address) -p (port)
```
※ Optional arguments need not be specified. In that case, IP address is `127.0.0.1`, port is `12345`

2. Run tcp client

```
$(~/socket-programming) ./tcp/client.out -a (IP address) -p (port)
```

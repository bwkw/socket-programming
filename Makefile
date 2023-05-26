udp-server:
	gcc -o udp/server.out udp/server.c validation/validate.c
udp-client:
	gcc -o udp/client.out udp/client.c validation/validate.c
tcp-server:
	gcc -o tcp/server.out tcp/server.c validation/validate.c
tcp-semaphore-server:
	gcc -o tcp/semaphore-server.out tcp/semaphore-server.c validation/validate.c
tcp-client:
	gcc -o tcp/client.out tcp/client.c validation/validate.c

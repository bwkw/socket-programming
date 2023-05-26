udp-server:
	gcc -o udp/server.out udp/server.c validation/validate.c
udp-client:
	gcc -o udp/client.out udp/client.c validation/validate.c

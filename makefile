default: build

build: server.c
	gcc server.c -o server

client:
	dig @127.0.0.1 -p 8989 www.google.com

clean:
	rm server

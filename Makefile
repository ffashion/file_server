all: server.c client.c 
	gcc server.c -o server 
	gcc client.c -o client
clean:
	rm -rf client server 
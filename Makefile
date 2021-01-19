all: server.c client.c 
	gcc -g server.c -o server
	gcc -g client.c -o client
debug:
	gcc -DDEBUG_SERVER server.c -o server
	gcc -DDEBUG_CLIENT client.c -o client
clean:
	rm -rf client server 
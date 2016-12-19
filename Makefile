all:
	gcc -Wall Server.c -o Server
	gcc -Wall DataClient.c -o DataClient
	gcc -Wall HeartbeatClient.c -o HeartbeatClient
	gcc -Wall HeartbleedClient.c -o HeartbleedClient


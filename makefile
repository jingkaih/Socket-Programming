all: compileclient compileaws compileserverA compileserverB compileserverC

compileclient: client.cpp
	g++ -o client client.cpp

compileaws: AWS.cpp
	g++ -o server_aws AWS.cpp

compileserverA: serverA.cpp
	g++ -o server_a serverA.cpp

compileserverB: serverB.cpp
	g++ -o server_b serverB.cpp

compileserverC: serverC.cpp
	g++ -o server_c serverC.cpp

aws: ./server_aws
	./server_aws

serverA: ./server_a
	./server_a

serverB: ./server_b
	./server_b

serverC: ./server_c
	./server_c
clean:
	rm client server_aws server_a server_b server_c *.o
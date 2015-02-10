all: kMeans

clean:
	rm *.o a.out

# tcpSock: tcpsock.o client.o server.o
# 	g++ -g -o server tcpsock.o server.o
# 	g++ -g -o client tcpsock.o client.o

# client.o:  client.cpp
# 	g++ -c client.cpp

# server.o:  server.cpp
# 	g++ -c server.cpp
	
# tcpsock.o: tcpsock.cpp
# 	g++ -c tcpsock.cpp

kMeans: kMeans.o
	gcc kMeans.o -o -fopenmp kMeans

kMeans.o: kMeans.c
	gcc -O2 -Wall -pedantic -pthread -fopenmp kMeans.c

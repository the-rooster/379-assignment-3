all: client server

client: client.o tands.o
	g++ -O -o client client.o tands.o

server: server.o tands.o
	g++ -O -pthread -o server server.o tands.o

server.o: server.cc
	g++ -O -c server.cc -o server.o

client.o: client.cc
	g++ -O -c client.cc -o client.o

tands.o: tands.cc
	g++ -O -c tands.cc -o tands.o

clean:
	rm server client *.o
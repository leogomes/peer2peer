all:    p2p
	cp p2p ./cl1
	cp p2p ./cl2
	cp p2p ./cl3
	cp p2p ./cl4
	cp p2p ./cl5
	cp p2p ./cl6
	cp p2p ./cl7
	cp p2p ./cl8

mysock.o: mysock.c
	g++ -Wall -c mysock.c

Timer.o: Timer.c
	g++ -Wall -c Timer.c

Estruturas.o: Estruturas.c
	g++ -Wall -c Estruturas.c

Eventos.o: Eventos.c
	g++ -Wall -c Eventos.c

Peer.o: Peer.c
	g++ -Wall -c Peer.c

Principal.o: Principal.c
	g++ -Wall -c Principal.c

p2p: Principal.o Eventos.o mysock.o Peer.o Estruturas.o Timer.o
	g++ -Wall -o p2p Principal.o Eventos.o mysock.o Peer.o Estruturas.o Timer.o  -lpthread

clean:
	rm -f *.o
	rm p2p ./cl1/p2p ./cl2/p2p ./cl3/p2p ./cl4/p2p ./cl5/p2p ./cl6/p2p ./cl7/p2p ./cl8/p2p

install: prepare all

uninstall: clean
	rm -R ./cl1 ./cl2 ./cl3 ./cl4

prepare:
	mkdir ./cl1 ./cl2 ./cl3 ./cl4
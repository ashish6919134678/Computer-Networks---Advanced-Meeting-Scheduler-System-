CXXFLAGS=-Wall -std=c++11 -pedantic

all: serverA serverB serverM client

common.o: common.cpp
	g++ $(CXXFLAGS) common.cpp -c -o common.o

backend.o: backend.cpp
	g++ $(CXXFLAGS) backend.cpp -c -o backend.o

serverA: common.o serverA.cpp backend.o
	g++ $(CXXFLAGS) common.o backend.o serverA.cpp -o serverA

serverB: common.o serverB.cpp backend.o
	g++ $(CXXFLAGS) common.o backend.o serverB.cpp -o serverB

serverM: common.o serverM.cpp
	g++ $(CXXFLAGS) common.o serverM.cpp -o serverM

client: common.o client.cpp
	g++ $(CXXFLAGS) common.o client.cpp -o client

clean:
	rm -rf *.o serverA serverB serverM client
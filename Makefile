
all: xaiPacker main

xaiPacker: xaiPacker.cpp
	g++ -o xaiPacker xaiPacker.cpp -lboost_system -lboost_filesystem -std=c++11 -g

main:
	g++ -o xai main.cpp -lboost_system -lboost_filesystem -std=c++11 -g

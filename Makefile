
all: xai unxai

xai: xaiPacker.cpp
	g++ -o xai xaiPacker.cpp -lboost_system -lboost_filesystem -std=c++11 -g

unxai: main.cpp
	g++ -o unxai main.cpp -lboost_system -lboost_filesystem -std=c++11 -g

clean:
	rm unxai xai

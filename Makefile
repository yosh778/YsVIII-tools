
all: xai unxai

xai: xai.cpp
	g++ -o xai xai.cpp -lboost_system -lboost_filesystem -std=c++11 #-g

unxai: unxai.cpp
	g++ -o unxai unxai.cpp -lboost_system -lboost_filesystem -std=c++11 #-g

clean:
	rm unxai xai

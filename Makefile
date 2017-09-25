
all: xai unxai dat undat plt unplt

xai: xai.cpp
	g++ -o xai xai.cpp -lboost_system -lboost_filesystem -std=c++11 #-g

unxai: unxai.cpp
	g++ -o unxai unxai.cpp -lboost_system -lboost_filesystem -std=c++11 #-g

dat: dat.cpp
	g++ -o dat dat.cpp -lboost_system -lboost_filesystem -std=c++11 #-g

undat: undat.cpp
	g++ -o undat undat.cpp -lboost_system -lboost_filesystem -std=c++11 #-g

plt: plt.cpp
	g++ -o plt plt.cpp -std=c++11 #-g

unplt: unplt.cpp
	g++ -o unplt unplt.cpp -std=c++11 #-g

clean:
	rm unxai xai
	rm undat dat
	rm unplt plt


INC=
LIB=
LSUFFIX=

CCFLAGS=-std=c++11 #-g

ifeq ($(OS),Windows_NT)
	CCFLAGS += -D WIN32 -static -static-libgcc -static-libstdc++
	INC=C:\Program Files\boost\include\boost-1_65_1
	LIB=C:\Program Files\boost\lib
	LSUFFIX=-mgw63-mt-1_65_1

	ifeq ($(PROCESSOR_ARCHITEW6432),AMD64)
		CCFLAGS += -D AMD64
	else
		ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
			CCFLAGS += -D AMD64
		endif
		ifeq ($(PROCESSOR_ARCHITECTURE),x86)
			CCFLAGS += -D IA32
		endif
	endif
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Linux)
		CCFLAGS += -D LINUX
	endif
	ifeq ($(UNAME_S),Darwin)
		CCFLAGS += -D OSX
	endif
	UNAME_P := $(shell uname -p)
	ifeq ($(UNAME_P),x86_64)
		CCFLAGS += -D AMD64
	endif
	ifneq ($(filter %86,$(UNAME_P)),)
		CCFLAGS += -D IA32
	endif
	ifneq ($(filter arm%,$(UNAME_P)),)
		CCFLAGS += -D ARM
	endif
endif

IFLAGS=-I"${INC}"
LFLAGS=-L"${LIB}" -lboost_system${LSUFFIX} -lboost_filesystem${LSUFFIX}

all: xai unxai dat undat plt unplt

xai: xai.cpp
	g++ -o xai xai.cpp ${IFLAGS} ${CCFLAGS} ${LFLAGS}

unxai: unxai.cpp
	g++ -o unxai unxai.cpp ${IFLAGS} ${CCFLAGS} ${LFLAGS}

dat: dat.cpp
	g++ -o dat dat.cpp ${IFLAGS} ${CCFLAGS} ${LFLAGS}

undat: undat.cpp
	g++ -o undat undat.cpp ${IFLAGS} ${CCFLAGS} ${LFLAGS}

plt: plt.cpp
	g++ -o plt plt.cpp ${CCFLAGS}

unplt: unplt.cpp
	g++ -o unplt unplt.cpp ${CCFLAGS}

clean:
	rm unxai xai
	rm undat dat
	rm unplt plt

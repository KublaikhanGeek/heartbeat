#CC=gcc -std=c99
#CPP=g++ -std=c++0x
CPP=mipsel-openwrt-linux-g++
AR=mipsel-openwrt-linux-ar
CC=mipsel-openwrt-linux-gcc

all:
	$(CPP) -Wall -W -std=c++0x -c  mxcppapi.cpp
	$(AR) crs libapi.a mxcppapi.o
	rm -f mxcppapi.o
	$(CC) -Wall -W -std=c99 -L/usr/local/mip-tool/lib -lm -c   bfupdateapi.c  mxlib.c main.c
	$(AR) crs libmxsw.a *.o
	$(CC) -Wall -W -o mxupdate mxjson.c -L. -lapi -lmxsw -L/usr/local/mip-tool/lib -lstdc++ -lc -lm


clean:
	rm -rf *.o
	rm -rf *.a
	rm -rf mxupdate

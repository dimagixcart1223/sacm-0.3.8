all:
	gcc-3.4 -O2 -c -w sqlite/*.c
	gcc-3.4 -O2 -c -w -Iamx/ -DLINUX -DSAMPSRV -DAMX_NODYNALOAD amx/*.c
	g++-3.4 -O2 -c -w -Iamx/ -DLINUX -DSAMPSRV -DAMX_NODYNALOAD *.cpp
	g++-3.4 -O2 -c -w -Iamx/ -DLINUX -DSAMPSRV -DAMX_NODYNALOAD ../raknet/*.cpp
	gcc-3.4 -O2 -c -w ../raknet/zlib/*.c
	g++-3.4 -O2 -w -Iamx/ -ldl -lpthread -lm -o mpsvr -DLINUX -DSAMPSRV -DAMX_NODYNALOAD *.o

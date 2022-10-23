.PHONY: all clean

READ_LENGTH=100
MX_READ_LIST_SZ=16853510
HASH_TABLE_SZ=314606869

all: chl unchl

chl: chl.cpp
	g++ -DREAD_LENGTH=$(READ_LENGTH) -DMX_READ_LIST_SZ=$(MX_READ_LIST_SZ) -DHASH_TABLE_SZ=$(HASH_TABLE_SZ) -DUSE_FS_SIM -std=c++17 -fdiagnostics-color=always -O3 -mcmodel=medium -ochl chl.cpp

unchl: unchl.cpp
	g++ -DREAD_LENGTH=$(READ_LENGTH) -DMX_READ_LIST_SZ=$(MX_READ_LIST_SZ) -DHASH_TABLE_SZ=$(HASH_TABLE_SZ) -std=c++17 -fdiagnostics-color=always -O3 -mcmodel=medium -ounchl unchl.cpp

clean:
	rm -rf chl unchl
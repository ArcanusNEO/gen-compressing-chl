.PHONY: all clean

READ_LENGTH=150

all: chl unchl

chl: chl.cpp
	g++ -DREAD_LENGTH=$(READ_LENGTH) -std=c++17 -fdiagnostics-color=always -O3 -mcmodel=medium -ochl chl.cpp

unchl: unchl.cpp
	g++ -DREAD_LENGTH=$(READ_LENGTH) -std=c++17 -fdiagnostics-color=always -O3 -mcmodel=medium -ounchl unchl.cpp

clean:
	rm -rf chl unchl
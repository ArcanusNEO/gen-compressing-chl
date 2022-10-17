.PHONY: all clean chl unchl

READ_LENGTH=150

all: chl unchl

chl:
	g++ -DREAD_LENGTH=$(READ_LENGTH) -std=c++17 -fdiagnostics-color=always -O3 -mcmodel=medium -ochl chl.cpp

unchl:
	g++ -DREAD_LENGTH=$(READ_LENGTH) -std=c++17 -fdiagnostics-color=always -O3 -mcmodel=medium -ounchl unchl.cpp

clean:
	rm -rf chl unchl
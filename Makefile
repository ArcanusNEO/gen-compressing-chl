.PHONY: all clean chl unchl

all: chl unchl

chl:
	g++ -std=c++17 -fdiagnostics-color=always -O3 -mcmodel=medium -ochl chl.cpp

unchl:
	g++ -std=c++17 -fdiagnostics-color=always -O3 -mcmodel=medium -ounchl unchl.cpp

clean:
	rm -rf chl unchl
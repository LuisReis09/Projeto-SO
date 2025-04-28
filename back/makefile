all: update clean install programa clean_terminal run

update:
	sudo apt update
	sudo apt install build-essential

programa: server.cpp
	g++ server.cpp -o programa `pkg-config --cflags --libs opencv4` -lpthread

run: programa
	./programa

install:
	sudo apt install -y libopencv-dev

clean:
	rm -f programa

clean_terminal:
	clear

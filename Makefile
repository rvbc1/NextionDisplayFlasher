all: compile run

compile:
	g++ -std=c++17 -o Nextion-UART-Flasher -Wall *.cpp *.c

run:
	./Nextion-UART-Flasher.exe Aquael.tft COM4

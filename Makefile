all: compile

compile:
	g++ -o STM32-UART-Flasher -Wall *.cpp *.c

run:
	./main

# Makefile for Writing Make Files Example
 
# *****************************************************
# Variables to control Makefile operation
 
CC = g++

main: main.o main.h 
	$(CC) -o main main.o

main.o: main.cpp VM.h VM.o
	$(CC) -c main.cpp

VM.o: VM.cpp VM.h main.h
	$(CC) -c VM.cpp VM.h

clean:
	rm *.o main
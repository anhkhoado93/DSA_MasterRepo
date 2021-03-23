# DSA_MasterRepo

## Installation
This project use Makefile, g++, valgrind 
```Shell
sudo apt-get install Makefile
sudo apt-get install g++
sudo apt-get install valgrind 
sudo apt-get update


touch mem.log
alias memcheck=valgrind --leak-check=full --show-leak-kind=all --log-file=mem.log
```
## Run
To compile and run project
```
make 
./main [path to testcase]
```

## Debug 
VSC C/C++ extension is used for line debugging.
```
memcheck ./main [path to testcase]
```

### Makefile for tasks

.PHONY: all clean

### Control variables
CPP = g++
FLAGS = -std=c++23 -ggdb3 -I include/ -Wall -Werror

### Targets
BINS=KeyValueServer KeyValueClient

all: $(BINS)

KeyValueServer: KeyValueServer.o
	$(CPP) $(FLAGS) -o KeyValueServer KeyValueServer.o

KeyValueClient: KeyValueClient.o
	$(CPP) $(FLAGS) -o KeyValueClient KeyValueClient.o

KeyValueServer.o: src/key_value_server.cpp
	$(CPP) $(FLAGS) -c src/key_value_server.cpp -o KeyValueServer.o

KeyValueClient.o: src/key_value_client.cpp
	$(CPP) $(FLAGS) -c src/key_value_client.cpp -o KeyValueClient.o


clean:
	rm -f *.o $(BINS)

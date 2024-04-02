Code has Makefile included which will create both server and client executables so need to run **make** for binary generation

# Design

1. A key_value_store.h abstract class is being used to represent a key-value lookup table. This can be derived to write lookup tables with different storage engines, using run-time polymorphism
2. Currently concurrent_hash_map.h is used as a lookup table, which uses a vector of lists. These data structures use memory_pool.h as their allocator, which is thread-safe
3. A custom read_write_lock.h is written to use multiple reader threads or a single writer thread in the critical section. Corresponding RAII wrapper is also written for proper unlocking
4. network_socket.h maintains all client-server network code for TCP socket

# Memory profiling

valgrind and its tools are used for leak check and memory profiling

1. For leak checking: `valgrind --leak-check=full --log-file="server_leak.txt" ./KeyValueServer 5005 5`. Log file will have the output
2. For profiling: `valgrind --tool=massif --massif-out-file="server_memstat.txt" --stacks=yes ./KeyValueServer 5005 5`, output visualization: `ms_print server_memstat.txt | less`. You can visualize the txt file graph by pasting txt file output on this site: [Plotter](https://boutglay.com/massifjs/)

## For running server

`./KeyValueServer <server_port> <num_buckets (optional, needs to be prime, default 19)>`

## For running client

`./KeyValueClient <server_port> <api_type (GET/PUT/POST/DELETE)> <key_val_params (min 1 required)`
server_port: should be same as server for connection
POST(insert) and PUT(update): requires 2 key_val_params for key and value
Rest need only 1 argument

## Instructions to run

1. Navigate to `build/`. 
2. Run `./KNN_server <port_number>` to run the server, and `./KNN_client <num_servers> <server_port_1> <server_port_2> ...` to run the client.  

**Input format to client:**
```
k l
x1 x2 x3 ... xl
```
**Here, k is the number of nearest neighbours to find, l is the number of coordinates in the query point and x1 to xl are the coordinates of the point.**

---

## Assumptions
1. Port of server will be passed as the 2nd command line argument to the server.
2. Number of servers to use will be passed as the 2nd command line argument to the client, and port number of each server as 3rd, 4th, ... command line args.
3. Invalid command line args will not be passed to the server and/or client.
4. The dataset that I've included as an example is of 2D KNN. However, any arbitrary dimension KNN is possible with this code, as it is general enough.
5. Client is responsible for sending portion of dataset to be computed on to the server, and server is responsible for performing the necessary computation.
6. Client asynchronously issues calls to each of the servers, and server asynchronously responds to each client request.
7. Finally, client is responsible for aggregating those results and getting the correct K nearest neighbours.
8. The client and server do not crash in between, and client always sends request only to a server that is alive.

---
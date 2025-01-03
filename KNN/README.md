## Instructions to run

Run `cmake .` in the folder with CMakeLists.txt, and then run `make` in the same folder as the MakeFile will be created there.  

Now, you can run `./Q2_server <port_number>` to run the server, and `./Q2_client <num_servers> <server_port_1> <server_port_2> ...` to run the client.  

**Input format to client:**
```
k l
x1 x2 x3 ... xl
```
**Here, k is the number of nearest neighbours to find, l is the number of coordinates in the query point and x1 to xl are the coordinates of the point.**

---

## Instructions to check the program

Simply use a singular server and singular client, and compare the output obtained from this case with the cases with multiple servers and/or clients.

---

## Assumptions
1. Port of server will be passed as the 2nd command line argument to the server.
2. Number of servers to use will be passed as the 2nd command line argument to the client, and port number of each server as 3rd, 4th, ... command line args.
3. Invalid command line args will not be passed to the server and/or client.
4. The dataset that I've included as an example is of 2D KNN. However, any arbitrary dimension KNN is possible with this code, as it is general enough.
5. Dataset will be preloaded from `dataset.csv` file in the `dataset` folder.
6. Client is responsible for sending portion of dataset to be computed on to the server, and server is responsible for performing the necessary computation.
7. Client asynchronously issues calls to each of the servers, and server asynchronously responds to each client request.
8. Finally, client is responsible for aggregating those results and getting the correct K nearest neighbours.
9. The client and server do not crash in between, and client always sends request only to a server that is alive.

---
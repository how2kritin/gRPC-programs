# Distributed Programming using gRPC

This repository contains a subset of problems from Assignment-4 of the Distributed Systems course, taken in Monsoon'24 semester at IIIT Hyderabad. These algorithms have been implemented in a distributed setting using gRPC, in C++ and Python.

Note that comments to indicate what each block of code does have been provided in the source code for each of these programs.
1. [K Nearest Neighbours](#k-nearest-neighbours)
2. [MyUber](#myuber)

---

## Pre-Requisites
1. protobuf
2. gRPC for python and C++ (you can refer to [this handy guide](https://grpc.io/blog/installation/))
3. redis
4. python
5. A package manager for python like `pip` or `conda`.
6. Python libraries mentioned in `requirements.txt`.
7. CMake

Once you have all of these installed, create a directory named `build` in the root directory, and enter the directory using `cd build`. Now, run `cmake ..`, followed by `make` to generate the executables.

---

## K Nearest Neighbours

### Instructions to run
1. Follow the instructions mentioned in [this README](KNN/README.md).

### Source Code: 
* [Client](KNN/client/client.cpp)
* [Server](KNN/server/server.cpp)
* [Protofile](KNN/protofiles/q2.proto)

### Input Format
* The first entry in the first line is `K`, which specifies the number of nearest neighbours to consider.
* The second entry in the first line is 'l', which specifies the dimension of the provided point. Note that this has to be the same as the dimensions of each datapoint present in the dataset.
* The second line contains `l` space separated floating point numbers which represent the point for which we are supposed to find `K` nearest neighbours.

### Output Format
* `K` points, representing the top K nearest neighbours in ascending order of distance.

For details regarding the implementation and performance analysis, please refer to [the report](MyUber/Report.pdf).

---

## MyUber
Created a distributed, asynchronous ride-sharing platform using gRPC, with support for rider and driver clients and servers to facilitate communication between the clients.  

### Salient Features
* SSL/TLS Authentication for both the client and the server (Mutual TLS).
* Interceptors for authorization, logging and other miscellaneous data.
* Timeout and rejection handling for drivers and riders.
* Client-side load balancing across servers.

### Instructions to run
1. Create a CA certificate (you can follow [this guide](https://arminreiter.com/2022/01/create-your-own-certificate-authority-ca-using-openssl/)) and name it `ca.crt`. Put the `ca.crt` as well as its key named `ca.key` in the [certificate folder](MyUber/certificate).
2. Follow the instructions mentioned in [this README](MyUber/README.md).

### Source Code: 
* [Client](MyUber/client) 
* [Server](MyUber/server) 
* [Protofile](MyUber/protofiles/q3.proto)
* [Certificate Creation Script](MyUber/utils/certificate_creation_script.sh)

For details regarding the implementation, please refer to [the report](MyUber/Report.pdf).

---

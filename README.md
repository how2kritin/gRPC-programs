# Distributed Programming using gRPC

This repository contains a subset of problems from Assignment-4 of the Distributed Systems course, taken in Monsoon'24 semester at IIIT Hyderabad. These algorithms have been implemented in a distributed setting using gRPC, in C++ and Python.

Note that comments containing reference material and implementation ideas, as well as comments to indicate what each block of code does, have been provided in the source code for each of these programs.
1. [Prefix Sum](#prefix-sum)
2. [Matrix Inversion](#matrix-inversion)
3. [Matrix Chain Multiplication](#matrix-chain-multiplication)

---

## Pre-requisites

```mpic++```
Kindly ensure that `mpich` and `build-essential` packages are installed, if on a Linux distribution.

---

## Pre-Requisites


---

## K Nearest Neighbours
Please find the source code for the client at [client.cpp](KNN/client/client.cpp).

For `N=1000000` randomly generated floating point numbers.

### Instructions to run
Run `./Q2_server <port_number>` to run the server, and `./Q2_client <num_servers> <server_port_1> <server_port_2> ...` to run the client.

### Input Format
* The first line contains `N`, i.e., the number of elements there are.
* The second line contains `N` space-separated floating point numbers.

### Output Format
* A single line containing `N` space-separated floating point numbers, where the `i`th element is the prefix sum of the first `i` elements.

For details regarding the implementation and performance analysis, please refer to [the report](MyUber/Report.pdf).

---

## MyUber
Created a ride-sharing distributed platform using gRPC, with support for rider and driver clients and servers to facilitate communication between the clients.

### Instructions to run
1. Create a CA certificate (you can follow [this guide](https://arminreiter.com/2022/01/create-your-own-certificate-authority-ca-using-openssl/)) and name it `ca.crt`. Put the `ca.crt` as well as its key named `ca.key` in the [certificate folder](MyUber/certificate).
2. Follow the instructions mentioned at [README.md](MyUber/README.md)

### Source Code: 
* [Client](MyUber/client) 
* [Server](MyUber/server) 
* [Protofile](MyUber/protofiles/q3.proto)
* [Certificate Creation Script](MyUber/utils/certificate_creation_script.sh)

For details regarding the implementation, please refer to [the report](MyUber/Report.pdf).

---

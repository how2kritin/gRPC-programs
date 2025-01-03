## Instructions to run

1. Open a terminal session in `/MyUber/server`, and run `python -m grpc_tools.protoc -I../protofiles/ --python_out=. --pyi_out=. --grpc_python_out=. q3.proto`.
2. To run the server, run `python server.py <server_port>` in the same directory.
3. Navigate to `build/` and run `./MyUber_client <client_role> <load_balancing_policy> <num_servers> <server_port_1> <server_port_2> ...` to run the client.

**_client\_role_ MUST NECESSARILY be one of _rider_ or _driver_, and _load\_balancing\_policy_ MUST NECESSARILY be one of _pick_first_ or _round_robin_.**

---

## Assumptions
1. Both; rider and driver are clients. The server is what facilitates the requests between the clients.
2. There can be multiple servers, and each client can send a request only to a single server at any given time.
3. Assuming that clients and servers don't just crash randomly.
4. Each client knows of all the servers, and chooses a server to send a request to, while performing load balancing. (client-side load balancing)
5. As C++ doesn't support custom load balancing yet, I am unfortunately unable to do implement custom load balancing. (see, bottom of: https://grpc.io/docs/guides/custom-load-balancing/).
6. As load balancing is to be done, all the servers will share a common database to keep track of riders and drivers. They will not store the state of riders and drivers themselves.
7. Assuming that a request gets cancelled if it gets rejected/ignored (timed out) by 3 drivers.
8. CA certificate exists at the start, and each client/server creates its own SSL certificate on demand (based on its UUID), and gets it signed with the `ca.crt`. All these certificates are stored in the `certificate` folder.
9. I am using Redis for my database storage (in-memory). It has pub/sub events which I am making use of.
10. The server which I am using will be asynchronous. The client itself however, is synchronous.
11. LogAndAuthInterceptor is responsible for checking certificate expiry date as well, apart from acting as both; a logging and an authorization interceptor.
12. All calls are logged, including those which were denied access.
13. Accounted for race conditions while logging as well. Logs are stored in `.log` file in the `server` folder.
14. A rider can request multiple rides (say, for different people) but a driver can only take one ride at a time.

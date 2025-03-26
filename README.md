# udp_server_client_test
A simple UDP server-client communication test framework with logging and signal handling in C++.

# Prerequisites

### 📌 Requirements
- C++ compiler (such as `g++`)
- CMake (if using a CMake-based project)

---

# How to build

```
mkdir build
cd build
cmake ..
cmake --build .

```

# How to run the server
- Add port number

```
cd build
./udp_server 8080

```

# How to run the client
- Add port number and IP address of the server 

```
cd build
./udp_client 8080 192.168.1.71

```

# Example output
- server:
```
================================================================================
Received 5 bytes from 192.168.1.76:34384: Ping!
================================================================================
Received 5 bytes from 192.168.1.76:34384: Ping!
================================================================================
Received 5 bytes from 192.168.1.76:34384: Ping!
================================================================================
Received 5 bytes from 192.168.1.76:34384: Ping!
================================================================================
Received 5 bytes from 192.168.1.76:34384: Ping!
```

- client:
```
================================================================================
Received 5 bytes from 192.168.1.71:8080: Pong!
================================================================================
Received 5 bytes from 192.168.1.71:8080: Pong!
================================================================================
Received 5 bytes from 192.168.1.71:8080: Pong!
================================================================================
Received 5 bytes from 192.168.1.71:8080: Pong!
================================================================================
Received 5 bytes from 192.168.1.71:8080: Pong!
```

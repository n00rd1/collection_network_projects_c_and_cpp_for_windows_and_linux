# Networking Projects in C for Linux and Windows

This repository contains a collection of projects written in the C programming language, focused on network operations for both Linux and Windows operating systems. These projects represent my work and contributions in networking under these platforms.

## Linux Projects in C
This section includes Linux projects written in the C programming language, with a focus on networking operations.

- [eserv](https://github.com/n00rd1/test_c_cpp_projects_on_VS2022_by_N00rd1/tree/master/Linux_project_on_c/eserv): A collection of Linux projects, including an implementation of a UDP speed test tool for measuring the performance of UDP traffic between two endpoints over a network.

- [ping_utility](https://github.com/n00rd1/test_c_cpp_projects_on_VS2022_by_N00rd1/tree/master/Linux_project_on_c/ping_utility): A C implementation of the `ping` utility, based on BusyBox's `ping` utility (specifically the version from 1999).

- [pingtest](https://github.com/n00rd1/test_c_cpp_projects_on_VS2022_by_N00rd1/tree/master/Linux_project_on_c/pingtest): A C implementation of a network latency test utility, similar to the `ping` utility but designed for more specific testing requirements.


## Windows Projects in C
This section includes Windows projects written in the C programming language, focusing on networking functions.

- [Win_Old_TFTP_project.c](https://github.com/n00rd1/test_c_cpp_projects_on_VS2022_by_N00rd1/blob/master/Win_TFTP_project_on_c/Win_Old_TFTP_project.c): A TFTP server implementation with UDP packets for the Windows operating system.

- [Win_TFTP_project_on_c.c](https://github.com/n00rd1/test_c_cpp_projects_on_VS2022_by_N00rd1/blob/master/Win_TFTP_project_on_c/Win_TFTP_project_on_c.c): A partial revision of the previous TFTP server implementation.


## [eserv](https://github.com/n00rd1/test_c_cpp_projects_on_VS2022_by_N00rd1/tree/master/Linux_project_on_c/eserv)
The "eserv" repository contains Linux projects written in the C programming language. One of the projects is an implementation of a UDP speed test tool that measures the performance of UDP traffic between two endpoints over a network.

To compile and run the software in this repository, ensure that GCC is installed in your Linux environment. Use the provided Makefile to compile the software. Run the command `make release` in the terminal to clean any previously compiled binaries and then compile the `main.c` file into a new binary named "eserv".

The main executable, "eserv", runs a UDP speed test between two hardcoded IP addresses: "192.168.1.1" (server) and "192.168.1.2" (client). The test is repeated 100 times, with each test sending 1500 packets of size 1500 bytes from the client to the server.

# Documentation of the Code

The repository mainly consists of a single C file, "main.c", and a Makefile. The "main.c" file includes the definitions for two structures `T_SpeedTest` and `T_packet`, and two functions `udp_speedtest()` and `main()`.

## Structures

`T_SpeedTest`: This structure holds parameters necessary for the speed test, such as the server IP, packet count, MTU data, client count, client IP, and statistics about sent, dropped, and received packets.

`T_packet`: This structure represents a single packet to be sent and received. It consists of a counter, a block number, and a buffer for the packet data.

This project can be a great starting point for those who want to dive into network programming with C on a Linux platform. It provides a practical example of how to use sockets, UDP, and the `poll()` function in a real-world scenario.

## [ping_utility](https://github.com/n00rd1/test_c_cpp_projects_on_VS2022_by_N00rd1/tree/master/Linux_project_on_c/ping_utility)
The "ping_utility" repository contains a C implementation of the `ping` utility, based on BusyBox's `ping` utility (specifically the version from 1999). The repository includes the following source files:

- `ping.c`: Contains the main code for the `ping` utility, including raw socket handling and various validations for IPv4, MAC, hostname, etc.
- `libtype.h`: A header file with various validation functions for different data types such as unsigned integers, boolean values, strings, MAC addresses, IPv4 addresses, hostnames, ports, time values, and WPA keys.

To build the project, use the `make all` command, which compiles the source code into a shared library `ping.so`. Please note that the project depends on BusyBox 1.36.0, which must be downloaded separately.

To run the `ping` utility, load the `ping.so` library into your application. Keep in mind that the utility requires raw socket capability, which may require special permissions on certain systems.

## [pingtest](https://github.com/n00rd1/test_c_cpp_projects_on_VS2022_by_N00rd1/tree/master/Linux_project_on_c/pingtest)
The "pingtest" repository includes a C implementation of a network latency test utility. This utility is similar to the `ping` utility but designed for more specific testing requirements. The repository contains the following source file:

- `pingtest.c`: Contains the main code for the `pingtest` utility, including raw socket handling and ICMP echo request and reply functionality. This utility sends a sequence of ICMP echo requests and measures the response times to calculate network latency.

To build the project, use the `make release` command, which compiles the source code into an executable `pingtest`.

To run the `pingtest` utility, execute it from the command line with the following arguments: `<bind_ip> <remote_ip> <number_icmp> <number_block> <mtu> [<timeout>]`. Each argument represents:

- `bind_ip`: The local IP address to bind to.
- `remote_ip`: The remote IP address to send ICMP echo requests to.
- `number_icmp`: The number of ICMP messages to send in one packet.
- `number_block`: The total number of packets to send.
- `mtu`: The size of the data in one message.
- `timeout` (optional): The timeout for waiting for ICMP echo replies, in seconds. The default timeout is 1 second.

The utility outputs a report that includes the number of ICMP echo requests sent, received, and missed, along with the total time and the bitrate.

## [Win_Old_TFTP_project.c](https://github.com/n00rd1/test_c_cpp_projects_on_VS2022_by_N00rd1/blob/master/Win_TFTP_project_on_c/Win_Old_TFTP_project.c)
This project implements the Trivial File Transfer Protocol (TFTP) for the Windows operating system. TFTP is a simple file transfer protocol often used for booting or loading files onto embedded systems.

The repository contains a TFTP server implementation with UDP packets for the Windows operating system.

## Compilation

To compile the `Win_Old_TFTP_project.c` file, make sure to include the necessary headers and libraries, such as `winsock2.h` and `windows.h`. Additionally, link against the appropriate libraries when building your project.

Here's an example compilation command using `gcc`:

```bash
gcc Win_Old_TFTP_project.c -o Win_Old_TFTP_project.exe -lws2_32
```
*note: Please note that you may need to adjust the compilation command based on your specific setup and requirements.*

## [Win_TFTP_project_on_c.c](https://github.com/n00rd1/test_c_cpp_projects_on_VS2022_by_N00rd1/blob/master/Win_TFTP_project_on_c/Win_TFTP_project_on_c.c)
This server is designed to transfer files over the TFTP protocol within a local network. It is implemented in the C programming language and utilizes Winsock and WinAPI.

### Compilation

To compile the `Win_TFTP_project_on_c.c` file, include the necessary headers and libraries such as `winsock2.h` and `windows.h`. Additionally, link against the appropriate libraries when building your project.

### Usage

1. Modify the following variables in the code to fit your requirements:
   - `DIR_OF_FILE`: The default folder for TFTP files.
   - `CLIENT_DEF_IP4`: The default IP address of the TFTP client.
   - `SERVER_DEF_IP4`: The IP address of the TFTP server.
   - `DEF_TIMEOUT`: The timeout value for receiving requests.

2. Run the program.

3. Follow the interactive menu to start the TFTP server, print the configuration, change the configuration, or exit the program.

Please note that this code is a simplified example and may require modifications or additional functionality to meet your specific needs.

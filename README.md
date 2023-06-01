# Networking Projects in C for Linux and Windows

This repository contains a collection of projects written in the C programming language, focused on network operations for both Linux and Windows operating systems. These projects represent my work and contributions in networking under these platforms.

## Linux Projects in C
(Insert a description of each project, its purpose, and the technologies used. Unfortunately, without access to the code, this is difficult to automatically generate)

[eserv](https://github.com/n00rd1/test_c_cpp_projects_on_VS2022_by_N00rd1/tree/master/Linux_project_on_c/eserv)
This repository, named "eserv", is a collection of Linux projects written in the C programming language. The main focus of this repository is the implementation of a UDP speed test tool. This tool, written in C, measures the performance of UDP traffic between two endpoints over a network. It is especially useful for performance testing, network troubleshooting, and for learning about the UDP protocol and socket programming in Linux.

[ping_utility](https://github.com/n00rd1/test_c_cpp_projects_on_VS2022_by_N00rd1/tree/master/Linux_project_on_c/ping_utility)
This repository contains a C implementation of the `ping` utility, based on BusyBox's `ping` utility, specifically version from 1999.

[pingtest](https://github.com/n00rd1/test_c_cpp_projects_on_VS2022_by_N00rd1/tree/master/Linux_project_on_c/pingtest)
This repository contains a C implementation of a network latency test utility, similar to the `ping` utility, but designed for more specific testing requirements.

## Windows Projects in C
These projects implement networking functions in the C programming language for the Windows operating system, notably using Winsock for network operations.

[Win_Old_TFTP_project.c](https://github.com/n00rd1/test_c_cpp_projects_on_VS2022_by_N00rd1/blob/master/Win_TFTP_project_on_c/Win_Old_TFTP_project.c)
This repository contains a TFTP server implementation with UDP packets for the Windows operating system.

[Win_TFTP_project_on_c.c](https://github.com/n00rd1/test_c_cpp_projects_on_VS2022_by_N00rd1/blob/master/Win_TFTP_project_on_c/Win_TFTP_project_on_c.c)
This code is a partial revision of the previous code (which was provided earlier).

## [eserv](https://github.com/n00rd1/test_c_cpp_projects_on_VS2022_by_N00rd1/tree/master/Linux_project_on_c/eserv)
To compile and run the software in this repository, you need to have GCC installed in your Linux environment. Once you have ensured that GCC is installed, you can compile the software using the provided Makefile. Just type make release in the terminal to clean any previously compiled binaries and then compile the main.c file into a new binary named "eserv".

The main executable, "eserv", accepts no command line arguments. It runs a UDP speed test between two hardcoded IP addresses: "192.168.1.1" (server) and "192.168.1.2" (client). The test is repeated 100 times, with each test sending 1500 packets of size 1500 bytes from the client to the server.

# Documentation of the Code

The repository mainly consists of a single C file, "main.c", and a Makefile. The "main.c" file includes the definitions for two structures `T_SpeedTest` and `T_packet`, and two functions `udp_speedtest()` and `main()`.

## Structures

`T_SpeedTest`: This structure holds parameters necessary for the speed test, such as the server IP, packet count, MTU data, client count, client IP, and statistics about sent, dropped, and received packets.

`T_packet`: This structure represents a single packet to be sent and received. It consists of a counter, a block number, and a buffer for the packet data.

## Functions

`udp_speedtest()`: This function performs a UDP speed test between two endpoints. It creates a UDP socket, sets it to non-blocking mode, and sends packets from the client to the server. It also uses the `poll()` function to receive packets and collects statistics about sent, received, and dropped packets. This function returns an error code if any step in the process fails.

`main()`: This is the entry point of the program. It initializes a `T_SpeedTest` structure with hardcoded values and then repeatedly calls the `udp_speedtest()` function 100 times. It prints the number of received and dropped packets after each test.

This project can be a great starting point for those who want to dive into network programming with C on a Linux platform. It provides a practical example of how to use sockets, UDP, and the `poll()` function in a real-world scenario.

##[ping_utility](https://github.com/n00rd1/test_c_cpp_projects_on_VS2022_by_N00rd1/tree/master/Linux_project_on_c/ping_utility)
The project includes:
- A `Makefile` to build the `ping` utility.
- `ping.c`, the primary source file.
- `libtype.h`, a header file with various validation functions.

### Usage

To build the project, use the `make all` command, which compiles the source code into a shared library `ping.so`.

#### Source Code Overview

- `ping.c`: Contains main code for the `ping` utility, including raw socket handling and various validations for IPv4, MAC, hostname, etc.
- `libtype.h`: Includes validation functions for different data types such as unsigned integers, boolean values, strings, MAC addresses, IPv4 addresses, hostnames, ports, time values, and WPA keys.

This project depends on BusyBox 1.36.0, which must be downloaded separately.

### Building

Use the `make all` command to build the project. This will produce a shared library `ping.so`.

### Running

Load the `ping.so` library into your application to use it. Note that the utility requires raw socket capability, which may need special permissions on some systems.

## [pingtest](https://github.com/n00rd1/test_c_cpp_projects_on_VS2022_by_N00rd1/tree/master/Linux_project_on_c/pingtest))
The project includes:
- A `Makefile` to build the `pingtest` utility.
- `pingtest.c`, the primary source file.

### Usage

To build the project, use the `make release` command, which compiles the source code into an executable `pingtest`.

#### Source Code Overview

- `pingtest.c`: Contains main code for the `pingtest` utility, including raw socket handling and ICMP echo request and reply functionality. This utility sends a sequence of ICMP echo requests and measures the response times to calculate network latency.

The project does not depend on any specific libraries.

### Building

Use the `make release` command to build the project. This will produce an executable `pingtest`.

### Running

Run the `pingtest` utility from the command line with the following arguments: `<bind_ip> <remote_ip> <number_icmp> <number_block> <mtu> [<timeout>]`. Here's a brief explanation of each argument:

- `bind_ip`: The local IP address to bind to.
- `remote_ip`: The remote IP address to send ICMP echo requests to.
- `number_icmp`: The number of ICMP messages to send in one packet.
- `number_block`: The total number of packets to send.
- `mtu`: The size of the data in one message.
- `timeout` (optional): The timeout for waiting for ICMP echo replies, in seconds. Default is 1 second.

The utility outputs a report including the number of ICMP echo requests sent, received, and missed, along with the total time and the bitrate.

## Win_Old_TFTP_project.c

This project implements the Trivial File Transfer Protocol (TFTP) for the Windows operating system. TFTP is a simple file transfer protocol often used for booting or loading files onto embedded systems.

This repository contains a TFTP server implementation with UDP packets for the Windows operating system.

## Compilation

To compile the `Win_Old_TFTP_project.c` file, make sure to include the necessary headers and libraries, such as `winsock2.h` and `windows.h`. Additionally, link against the appropriate libraries when building your project.

### Example Compilation Command

```bash
gcc Win_Old_TFTP_project.c -o Win_Old_TFTP_project.exe -lws2_32
```
*note: Please note that you may need to adjust the compilation command based on your specific setup and requirements.*

## WinAPI Documentation

The WinAPI Documentation is the official documentation for the WinAPI (Windows Application Programming Interface) provided by Microsoft. It contains detailed information, reference materials, and examples for various functions, structures, and concepts related to Windows programming.

You can access the WinAPI Documentation at the following link: [WinAPI Documentation](https://docs.microsoft.com/en-us/windows/win32/api/)
Please refer to the official WinAPI Documentation for comprehensive and up-to-date information on Windows programming using the WinAPI.


## Win_TFTP_project_on_c.c

This server is designed to transfer files over the TFTP protocol within a local network. It is implemented in the C programming language and utilizes Winsock and WinAPI.


## Compilation

To compile the `Win_TFTP_project_on_c.c` file, include the necessary headers and libraries such as `winsock2.h` and `windows.h`. Additionally, link against the appropriate libraries when building your project.


## Usage

1. Modify the following variables in the code to fit your requirements:
   - `DIR_OF_FILE`: The default folder for TFTP files.
   - `CLIENT_DEF_IP4`: The default IP address of the TFTP client.
   - `SERVER_DEF_IP4`: The IP address of the TFTP server.
   - `DEF_TIMEOUT`: The timeout value for receiving requests.

2. Run the program.

3. Follow the interactive menu to start the TFTP server, print the configuration, change the configuration, or exit the program.

Please note that this code is a simplified example and may require modifications or additional functionality to meet your specific needs.

## Documentation

- [WinAPI Documentation](https://docs.microsoft.com/en-us/windows/win32/api/): Official documentation for the WinAPI.

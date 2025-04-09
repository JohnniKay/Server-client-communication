# Server-Client Communication

## Description
This project demonstrates a client-server communication system using TCP/IP. The server manages multiple clients, allowing them to communicate with each other in real-time. It includes both the server and client-side code and can be compiled and run in Visual Studio.

Note: This project was created during my college years as part of a learning experience, so some issues may arise while setting it up. These challenges have been noted, and steps to resolve them are included in the documentation. This was my first time working on this type of system, and it provided valuable insight into real-time communication protocols and multi-client management.

## Technologies Used:
- C++
- Winsock2
- Socket Programming
- Visual Studio ~ [Download Visual Studios Here](https://visualstudio.microsoft.com/downloads)

## Features:
- Client registration and message sending
- Server manages multiple connected clients
- Logs all server-client communication to a file

## Walkthrough to Set Up and Run the Project

This project was originally created in college, and there are a few steps you need to follow to get it working correctly. Please follow these steps carefully to avoid issues.

### 1. **Clone the Repository**

First, clone the repository to your local machine:

bash
git clone https://github.com/JohnniKay/Server-client-communication.git

Navigate into the project directory:

bash
Copy
cd Server-client-communication

### 2. **Install Necessary Software**
Make sure you have Visual Studio installed on your machine. You can download it from here.

During installation, ensure you select the C++ development tools and any necessary SDKs for building and running C++ projects.

### 3. **Open the Solution File**
Once the repository is cloned, open the solution file:

Copy
Server-Client-Communication.sln
This file is located in the root directory of the project and should automatically open in Visual Studio.

### 4. **Building the Project**
In Visual Studio, once the solution is open, follow these steps to build the project:

Go to the Build menu at the top.

Select Build Solution (or press Ctrl+Shift+B).

This will compile the project and prepare it to run.

### 5. **Handling Known Issues**
There are a couple of known issues due to the environment this project was originally developed in (college lab setup). Please follow these steps to fix them:

**Issue 1:** Debugging the Initial Build
If you encounter an issue with missing libraries or unresolved external symbols, follow these steps:

In Solution Explorer, right-click on the project and select Properties.

Go to VC++ Directories -> Include Directories and add the necessary paths to your library files (e.g., C:\path\to\libraries).

**Issue 2:** Socket Connection Issues
If the server and client fail to connect:

Ensure that your firewall is not blocking the connection on the specified port.

Double-check that both the client and server are using the correct IP addresses (127.0.0.1 for local testing) and the correct port number (31337).
You may need to adjust the hardcoded values in the source code if necessary.
The port number can be found in Server.cpp on line 26.

### 6. **Running the Project**
To run the project, follow these steps:

  **1. Multiple Clients:**
  The application is designed to allow up to three clients to connect simultaneously. You will need to open a separate instance of the solution (.sln) for each client you wish to connect.

  **2. Client Registration:**
  For each client, follow these steps:

  Connect to the server.

  Enter your chosen name.

  Type $register to register the client.

  This registration process must be completed for the first client before you can add the second, and similarly for the third client.

  **3. Messaging Between Clients:**
  Once all three clients are connected, they can send messages to each other in real-time. Communication between the clients can be tested by typing and sending messages back and forth.


**Additional Information:**
If you encounter any issues during installation or need assistance with setup, feel free to contact me at **info@johnnikay.dev.**

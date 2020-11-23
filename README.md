# DSA
Homework for DSA

Requirements: 
  make
  gcc compiler (with PTHREAD support)
  
Build:
  make all 
  make client (Compile only client application)
  make server (Compile only server application)
  make clean (Clean build directory)
  
 Applications are installed inside build directory build/client/ build/server/ under name client and server respectively.
 
 Usage of client application
 
 ./client ip port 
 eg: ./client 127.0.0.1 1337  
 
 Usage of server application 
 ./server port MODE(THREAD || PROC)
 ./server 1337 THREAD
 
 
 MODE: 
 Server application can run in two modes 
 1. THREAD: Each new client that connnects to application will be handled in different thread.
 2. PROC: Each new client that connects to application will be handled in new proccess.
 

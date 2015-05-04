<< EE 122 Spring 2015 Project #2 Report >>

** Names and Emails **

Sun Shuo 23602997 ssfei81@berkeley.edu
Gabriel Tan 23845679 tanjunxian@berkeley.edu

** Platform **

Mac
￼￼
** Compilation instructions **

run "make" in command line

** Run-time instructions (command-line arguments, etc.) **

(remember to run senders last so that all packets sent are received by the router)

1) run multiple destinations/receivers (up to 2)
./destination PORTNUMBER

e.g. ./destination 6000
6000 is the port number of the destination

2) run router
./router MODE PORTNUMBER
MODE: 0 = 1 sender 1 receiver 
      1 = 2 senders 2 receivers, single queue
      2 = 2 senders 2 receivers, priority

e.g. ./router 0 5000
Running Mode 0: 1 sender 1 receiver
5000 is the port number of the router

3) run multiple sources/senders (up to 2)
./source ROUTERADDR ROUTERPORT SERVICEDELAY SENDERID DESTADDR DESTPORT PACKETCOUNT
ROUTERADDR: The IP address of the router
ROUTERPORT: The port number of the router
SERVICEDELAY: The service delay in ms/packet
SENDERID: The ID of this sender. This can be either 1 or 2. ID of receiver will be the same as sender ID 
DESTADDR: The IP address of the receiver
DESTPORT: The port number of the receiver
PACKETCOUNT: Number of packets to be sent

e.g. ./source 127.0.0.1 5000 10 1 127.0.0.1 6000 500
IP address of router is 127.0.0.1
Port number of router is 5000
Service Delay is 10ms/pkt
Sender ID is 1
IP address of destination/receiver is 127.0.0.1
Port number of destination is 6000
To send 500 packets
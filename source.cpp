#include <iostream>
#include <cstring>
#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/time.h>
#include <cstdlib>
#include <netinet/in.h>
#include <netdb.h>
#include <random>
#include "packet.h"
using namespace std;

void error(char *msg) 
{
    cout<<"Error: "<<msg<<endl;
    exit(1);
}

void errorMsg(char *msg)
{
    cout<<endl;
    cout<<"Invalid input(s)..."<<endl;
    cout<<"Correct format:"<<endl;
    cout<<"<router addr> The IP address of the router."<<endl;
    cout<<"<router port> The port number of the router."<<endl;
    cout<<"<service_delay> The service delay in ms/packet."<<endl;
    cout<<"<sender_ID> The ID of this sender. This can be either 1 or 2."<<endl;
    cout<<"<destination addr> The IP address of the receiver."<<endl;
    cout<<"<destination port> The port number of the receiver."<<endl;
    cout<<"<packet_count> How many packets to be sent."<<endl;
    cout<<endl;
    exit(1);
}

int main(int argc, char *argv[]) 
{
    //socket stuff
    int sockfd, portno, destportno;
    socklen_t len;
    struct sockaddr_in svrAddr, cliAddr;
    struct hostent *server;
    float delay;
    int senderID, receiverID, packetCount;

    cout<<"************************************"<<endl;
    cout<<"*** Welcome to EE 122 Project #2 ***"<<endl;
    cout<<"***     Shuo Sun and Gabriel     ***"<<endl;
    cout<<"***        Source Program        ***"<<endl;
    cout<<"************************************"<<endl; 
    cout<<"(ctrl + c to exit)"<<endl;

    //read from command line
    if (argc != 8) errorMsg("Invalid input format"); 

    server = gethostbyname(argv[1]);
    if(server==NULL) error("Incorrect server address.");
    cout<<"Router IP address: "<<argv[1]<<endl;

    portno = atoi(argv[2]);
    if(portno > 65535 || portno < 0) error("Incorrect port number.");
    cout<<"Router port: "<<atoi(argv[2])<<endl;

    delay = atof(argv[3]);
    if(delay < 0) error("Delay must be >= 0");
    cout<<"R = "<<delay<<" ms/packet."<<endl;

    senderID = atoi(argv[4]);
    if(senderID != 1 && senderID != 2) error("Server ID must be 1 or 2.");
    cout<<"Sender ID: "<<senderID<<endl;

    receiverID = senderID;
    cout<<"Sending packets to receiver "<<receiverID<<"..."<<endl;

    cout<<"Destination IP address: "<<argv[5]<<endl;

    destportno = atoi(argv[6]);
    if(destportno > 65535 || destportno < 0) error("Incorrect port number.");
    cout<<"Destination port: "<<destportno<<endl;

    packetCount = atoi(argv[7]);
    if(packetCount <= 0) error("Packet count must be greater than 0.");
    cout<<"Packet count: "<<packetCount<<endl<<endl;

    //initialize program
    cout<<"Starting source program..."<<endl;
    cout<<"Creating socket...";
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) error("Unable to create socket.");
    cout<<"Done"<<endl;

    //initialize poisson distribution generator
    std::default_random_engine generator;
    std::poisson_distribution<int> distribution(delay);

    cout<<"Binding socket to port...";
    bzero((char *) &cliAddr, sizeof(cliAddr));
    cliAddr.sin_family = AF_INET;
    cliAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    cliAddr.sin_port = htons(0);
    if(::bind(sockfd, (struct sockaddr *)&cliAddr, sizeof(cliAddr)) < 0) error("Unable to bind socket to port.");
    cout<<"Done"<<endl;

    //clear buffer
    bzero((char *) &svrAddr, sizeof(svrAddr));
    //set server address
    svrAddr.sin_family = AF_INET;
    svrAddr.sin_port = htons(portno);
    bcopy((char *)server->h_addr,
          (char *)&svrAddr.sin_addr.s_addr,
          server->h_length);

    len = sizeof(svrAddr);

    //start sending packets
    for (int x = 1; x <= packetCount; x++)
    {
        //construct a packet
        packet p;
        p.sequenceNumber = x;
        p.source = senderID;
        p.destination = receiverID;
        strcpy(p.destinationIP, argv[5]);
        p.destinationPort = destportno;
        gettimeofday(&p.tv, NULL);

        unsigned long long msSinceEpoch =
                (unsigned long long)(p.tv.tv_sec) * 1000 +
                    (unsigned long long)(p.tv.tv_usec) / 1000;

        if(sendto(sockfd, &p, MAX_PACKET_SIZE, 0, (struct sockaddr *)&svrAddr, len)==-1) error("Unable to send packet.");
        cout<<"Sending packet "<<x<<endl;
        //delay 
        usleep((int) distribution(generator) * 1000);
    }
    return 0;
}

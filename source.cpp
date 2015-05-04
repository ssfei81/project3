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
#include <pthread.h>
#include <cstdio>
#include <ctime>
#include "packet.h"
using namespace std;

#define WINDOW_SIZE 8
int window_start = 1, window_end = WINDOW_SIZE, resend = 0;
bool receiverStart = false;

int mode = 1; // 0: Step 1, 1: Step 2
struct timeval startTime;
int sockfd;
char buf[MAX_PACKET_SIZE]; 
struct sockaddr_in svrAddr, cliAddr;
socklen_t clilen;

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

void* receiver(void *ptr)
{
    gettimeofday(&startTime, NULL);
    unsigned long long msSinceEpoch =
       (unsigned long long)(startTime.tv_sec) * 1000 +
          (unsigned long long)(startTime.tv_usec) / 1000;

while(1){
    //timeout for recvfrom
    struct timeval to;
    
    to.tv_sec = 2;
    to.tv_usec = 0;
    
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&to,sizeof(struct timeval));
    
    clilen = sizeof(struct sockaddr);
    int n = recvfrom(sockfd, buf, MAX_PACKET_SIZE, 0,(struct sockaddr *)&cliAddr, &clilen);
    ackpacket *pkt = (ackpacket *)&buf;
    
    struct timeval tv;
    
    float timeout, a_n, d_n, b;
    b = 0.5; //algorithm parameter
    gettimeofday(&tv, NULL);
    unsigned long long msSinceEpoch =
    (unsigned long long)(tv.tv_sec) * 1000 +
    (unsigned long long)(tv.tv_usec) / 1000;
    
    unsigned long long msSinceEpoch2 =
    (unsigned long long)(pkt->tv.tv_sec) * 1000 +
    (unsigned long long)(pkt->tv.tv_usec) / 1000;
    
    float delay = msSinceEpoch - msSinceEpoch2;
    
    //if delay < time out
    //cout<<delay<<" "<<timeout<<endl;
    if(delay < timeout) 
    {
        if(pkt->sequenceNumber == window_start) {window_start++; window_end++;}
        else if (pkt->sequenceNumber > window_start) resend = 1;
        //increase window size
        if (mode == 1) window_end++; 
    }
    else
    {
        resend = 1;
        // half window size
        if (mode == 1) window_end = window_start + int((window_end - window_start) / 2);
    }

    
    if (!receiverStart)
        {
        a_n = float(delay);
        d_n = float(delay);
        receiverStart = true;
        }
    else
        {
        a_n = (1-b) * a_n + b * (float)delay;
        d_n = (1-b) * d_n + b * (float(delay) - a_n);
        }
    
    timeout = a_n + 4 * d_n;
    cout<<"timeout "<<timeout<<endl;
    cout<<"delay "<<delay<<endl;
    }
}

int main(int argc, char *argv[]) 
{
    //project 3 thread var
    //
    pthread_t receiver_thread;

    //socket stuff
    int portno, destportno;
    socklen_t len;
    struct sockaddr_in svrAddr, cliAddr;
    struct hostent *server;
    float delay;
    int senderID, receiverID, packetCount;

    std::clock_t start;
    double duration;
    bool sendToggle = true;

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

    //project 3
    //new thread for receiving ACKs
    if(senderID==2){
    pthread_create(&receiver_thread,NULL, receiver, NULL);
    } 
    //start clock for mode 1 source 1 
    start = std::clock();

    //start sending packets
    for (int x = 1; x <= packetCount; x++)
    {
       if(senderID == 2){
           while(x > window_end){
        //busy wait
            if(resend) {x = window_start; resend = 0;}
        }
        }
        //construct a packet
        if (sendToggle) 
        {
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
        if (mode == 1 && senderID == 1)
        {
            if (!sendToggle)
            {
                sleep(5);
                sendToggle = true;
                start = std::clock();
            }
            duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;

            if (duration > 0.1) sendToggle = false;
        }
        // cout<<duration<<endl;
        cout<<"start "<<window_start<<endl;
        cout<<"windowsize "<<window_end - window_start<<endl;
        
        
    }
    return 0;
}

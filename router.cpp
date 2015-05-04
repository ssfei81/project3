#include "queue.h"
#include "packet.h"
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
using namespace std;

#define BUFFER_LIMIT 64
#define MAX_BUFFER_SIZE 200

//initialize the queues
queue<packet *> p_queue1(BUFFER_LIMIT); 
queue<packet *> p_queue2(BUFFER_LIMIT);

int sockfd;
int mode;
int sumOfQ1, sumOfQ2;


typedef struct thdata{
    char* ipAddr;
    int destinationportno;
} sender_thdata;

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
    cout<<"<mode> 0 = 1 sender 1 receiver | 1 = 2 senders 2 receivers, single queue | 2 = 2 senders 2 receivers, priority."<<endl;
    cout<<"<router port> The port number of the router."<<endl;
    cout<<endl;
    exit(1);
}

void* sender(void *ptr) 
{
    //socket stuff
    socklen_t len1, len2;
    struct sockaddr_in svrAddr1, svrAddr2;
    struct hostent *server1, *server2;
    bool dest1saved = false;  
    bool dest2saved = false;
    packet p;
    int prioritycount = 0;
    int prioritycap = 2;
        
    while(1) 
    {
        if (mode == 0 || mode == 1)
        {
            p = *p_queue1.pop();
        } 
        else if (mode == 2)
        {
            while (p_queue1.size() == 0 && p_queue2.size() == 0) {}
            if ((p_queue2.size() > 0 && prioritycount > prioritycap) || p_queue1.size() == 0) 
            {
                p = *p_queue2.pop();
                prioritycount = 0;
            }
            else 
            {
                p = *p_queue1.pop();
                prioritycount += 1;
            }
        }
        if (p.destination == 1)
        {
            if (dest1saved)
            {
                if(sendto(sockfd, &p, MAX_PACKET_SIZE, 0, (struct sockaddr *)&svrAddr1, len1)==-1) error("Unable to send packet.");
                cout<<"Sending packet "<<endl;
                //service delay default: 10 ms
                usleep(10000);
            }
            else
            {
                server1 = gethostbyname(p.destinationIP);
                //set server address
                bzero((char *) &svrAddr1, sizeof(svrAddr1));

                svrAddr1.sin_family = AF_INET;
                svrAddr1.sin_port = htons(p.destinationPort);
                bcopy((char *)server1->h_addr,
                  (char *)&svrAddr1.sin_addr.s_addr,
                  server1->h_length);
                len1 = sizeof(svrAddr1);
                dest1saved = true;
            }
            
        } 
        else if (p.destination == 2)
        {
            if (dest2saved)
            {
                if(sendto(sockfd, &p, MAX_PACKET_SIZE, 0, (struct sockaddr *)&svrAddr2, len2)==-1) error("Unable to send packet.");
                cout<<"Sending packet "<<endl;
                //service delay default: 10 ms
                usleep(10000);
            }
            else 
            {
                server2 = gethostbyname(p.destinationIP);
                //set server address
                bzero((char *) &svrAddr2, sizeof(svrAddr2));

                svrAddr2.sin_family = AF_INET;
                svrAddr2.sin_port = htons(p.destinationPort);
                bcopy((char *)server2->h_addr,
                  (char *)&svrAddr2.sin_addr.s_addr,
                  server2->h_length);
                len2 = sizeof(svrAddr2);
                dest2saved = true;
            }

        }
    }
    pthread_exit(0);
}


int main(int argc, char *argv[]) 
{
    //thread arguments
    sender_thdata sender_arg;

    //pthreads
    pthread_t sender_thread, receiver_thread;
    
    //socket stuff
    char *buf;
    int newsockfd, portno;
    struct sockaddr_in svrAddr, cliAddr;
    socklen_t clilen;
    struct hostent *server;
    sumOfQ1 = 0;
    sumOfQ2 = 0;

    cout<<"************************************"<<endl;
    cout<<"*** Welcome to EE 122 Project #2 ***"<<endl;
    cout<<"***     Shuo Sun and Gabriel     ***"<<endl;
    cout<<"***        Router Program        ***"<<endl;
    cout<<"************************************"<<endl; 
    cout<<"(ctrl + c to exit)"<<endl<<endl;

    //read from command line
    if (argc != 3) 
    {
        errorMsg("Invalid input format"); 
    }

    mode = atoi(argv[1]);
    if (mode != 0 && mode !=1 && mode != 2) error("Mode must be 0, 1 or 2.");
    cout<<"Mode "<<mode<<" ";
    if (mode==0) cout<<"(1 sender 1 receiver)"<<endl;
    else if (mode==1) 
    {
        cout<<"(2 senders 2 receivers, single queue)"<<endl;
    }
    else if (mode==2) 
    {
        //cut buffers into half
        p_queue1.setQueueSize(BUFFER_LIMIT/2);
        p_queue2.setQueueSize(BUFFER_LIMIT/2);
        cout<<"(2 senders 2 receivers, priority)"<<endl;
    }

    portno = atoi(argv[2]);
    if(portno > 65535 || portno < 0) error("Port number must be between 0 - 65535");
    cout<<"Port number: "<<portno<<endl;

    //initialize program
    cout<<"Starting router..."<<endl;
    cout<<"Creating listening socket...";
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) error("Unable to create socket.");
    cout<<"Done"<<endl;

    cout<<"Binding socket to port...";
    bzero((char *) &svrAddr, sizeof(svrAddr));
    svrAddr.sin_family = AF_INET;
    svrAddr.sin_addr.s_addr = INADDR_ANY;
    svrAddr.sin_port = htons(portno);
    if(::bind(sockfd, (struct sockaddr *)&svrAddr, sizeof(svrAddr)) < 0) error("Unable to bind socket to port.");
    cout<<"Done"<<endl;

    cout<<"Initializing sender...";
    pthread_create (&sender_thread, NULL, sender, &sender_arg);
    cout<<"Done"<<endl;

    cout<<"Initializing receiver...Done"<<endl;
    cout<<"Initializing buffer...";
    buf = new char[MAX_PACKET_SIZE];
    cout<<"Done"<<endl;

    cout<<"Listening on port "<<portno<<"..."<<endl;

    while(1) 
    {
        int n = recvfrom(sockfd, buf, MAX_PACKET_SIZE, 0, (struct sockaddr *)&cliAddr, &clilen);
        packet *pkt = (packet *) malloc(sizeof(packet));
        memcpy(pkt, buf, MAX_PACKET_SIZE);

        cout<<"Packet "<<pkt->sequenceNumber<<" received from source "<<pkt->source<<", ";
        sumOfQ1 += p_queue1.size();
        sumOfQ2 += p_queue2.size();
        cout<<sumOfQ1<<" "<<sumOfQ2<<endl;
        if(mode == 0 || mode == 1) 
        {
            int n = p_queue1.add(pkt);
            if(!n) 
            {
                cout<<"Queue full, packet dropped."<<endl;
            }
            else cout<<"Packet added to queue."<<endl;
        }
        else if(mode == 2) 
        {
            int n;
            if(pkt->destination == 1) 
            {
                n = p_queue1.add(pkt);
                if (n) cout<<"Packet added to queue 1."<<endl;
                else cout <<"Queue full, packet dropped."<<endl;
            }
            else if(pkt->destination == 2) 
            {
                n = p_queue2.add(pkt);
                if (n) cout<<"Packet added to queue 2."<<endl;
                else cout << "Queue full, packet dropped."<<endl;
            }
        }
    }
    return 0;
}

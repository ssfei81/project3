#include <iostream>
#include <cstring>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include "packet.h"
using namespace std;

void error(char *msg) 
{
    perror(msg);
    exit(1);
}

void errorMsg(char *msg)
{
    cout<<endl;
    cout<<"Invalid input(s)..."<<endl;
    cout<<"Correct format:"<<endl;
    cout<<"<portno> port number."<<endl;
    cout<<endl;
    exit(1);
}

int main(int argc, char *argv[]) 
{
    
    //socket stuff
    int sockfd, portno;
    socklen_t clilen;
    struct sockaddr_in svrAddr, cliAddr;
    char *buf;

    cout<<"************************************"<<endl;
    cout<<"*** Welcome to EE 122 Project #2 ***"<<endl;
    cout<<"***     Shuo Sun and Gabriel     ***"<<endl;
    cout<<"***        Router Program        ***"<<endl;
    cout<<"************************************"<<endl; 
    cout<<"(ctrl + c to exit)"<<endl;

    //read from command line
    if (argc != 2) errorMsg("Invalid input format"); 

    portno = atoi(argv[1]);
    if(portno > 65535 || portno < 0) error("Port number must be between 0 - 65535");
    cout<<"Port number: "<<portno<<endl;

    //initialize program
    cout<<"Starting client..."<<endl;
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

    cout<<"Initializing receiver...Done"<<endl;
    cout<<"Initializing buffer...";
    buf = new char[MAX_PACKET_SIZE];
    int count = 1;

    struct timeval tv;
    int delaySum = 0;
    while(1) 
    {
        cout<<"Receiving Packet "<< count <<endl;
        int n = recvfrom(sockfd, buf, MAX_PACKET_SIZE, 0, (struct sockaddr *)&cliAddr, &clilen);
        count += 1;
        //cal delay
        gettimeofday(&tv, NULL);
        unsigned long long msSinceEpoch =
                (unsigned long long)(tv.tv_sec) * 1000 +
                    (unsigned long long)(tv.tv_usec) / 1000;

        packet *p = (packet *)buf;
        unsigned long long msSinceEpoch2 =
                (unsigned long long)(p->tv.tv_sec) * 1000 +
                    (unsigned long long)(p->tv.tv_usec) / 1000;

        unsigned long long delay = msSinceEpoch - msSinceEpoch2;
        //cout<<delay<<endl;
        delaySum += delay;
        cout<<delaySum<<endl;
    }
    return 0;
}

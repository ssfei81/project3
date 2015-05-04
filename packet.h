//define the struct of a packet
#include <sys/time.h>
#define MAX_PACKET_SIZE 128

typedef struct apacket{
int sequenceNumber;
int source;
int destination;
char sourceIP[20];
char destinationIP[20];
int sourcePort;
int destinationPort;
struct timeval tv;
char padding[MAX_PACKET_SIZE - 64 - sizeof(struct timeval)];
} packet;

typedef struct akpacket{
int sequenceNumber;
struct timeval tv;
char padding[MAX_PACKET_SIZE - 4 - sizeof(struct timeval)];
} ackpacket;

//define the struct of a packet
#include <sys/time.h>
#define MAX_PACKET_SIZE 128

typedef struct apacket{
int sequenceNumber;
int source;
int destination;
char destinationIP[20];
int destinationPort;
struct timeval tv;
char padding[MAX_PACKET_SIZE - 36 - sizeof(struct timeval)];
} packet;

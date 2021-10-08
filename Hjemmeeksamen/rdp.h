#ifndef rdp_h
#define rdp_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <arpa/inet.h>

#define BUFFER_S 1016
#define PACKET_S 16
#define SERVER_ID 222
#define CONNECT 0x01
#define CLOSE 0x02
#define DATA 0x04
#define ACK 0x08
#define ACCEPT_REQ 0x10
#define DENIED_REQ 0x20

//dynamisk allokert struktur som skal representere vaare pakker
struct packet {
    unsigned char flag;
    unsigned char pktseq;
    unsigned char ackseq;
    unsigned char unassigned;
    int senderid;
    int recvid;
    int metadata;
    char* payload;
};

//dynamisk allokert struktur som skal representere vaare rdp-forbindelser
struct rdp {
    int server_id;
    int client_id; 
    struct sockaddr_in src_ad;
    unsigned long packet_count;
    unsigned long accept_count;
    unsigned char last_seq;
    struct packet* last_sendt_to_c;
    unsigned char ready;
    int file_read_count;
};

extern struct rdp** connections;
extern int size_rdp_list;
//Pakketeller for aa sette sekvensnummeret til hver enkelt datapakke
extern int packet_t;

void check_error(int res, char* message);

void check_malloc(void* res);

struct packet* accept_connect_p(int destid);

struct packet* denied_connect_p(int destid);

struct rdp* rdp_accept(struct packet* p, int filed, struct sockaddr_in* sourcead, int N, int teller);

void close_rdp(int client);

void rdp_send(int filed, struct sockaddr_in* dest, struct packet* p);

int rpd_select(int filed, fd_set* read_f, fd_set* copy, long sec, long usec);

struct packet* rdp_read(int filed, struct sockaddr* src_ad, socklen_t* ad_len, int sec, int usec);

void free_packet(struct packet* p);

void free_connections();

#endif
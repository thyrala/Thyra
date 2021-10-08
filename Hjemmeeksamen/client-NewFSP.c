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
#include <time.h>
#include "send_packet.h"
#include "rdp.h"

/*
Parametre: int senderid
Returnerer: struct packet* p

Mottar sender-id'en til en klient, og lager en forbindelsesforesporselspakke paa samme pakkeformat som er oppgitt
i oppgaven. Deretter returnerer funksjonen denne pakken

Denne funksjonen allokerer bytes til en pakke.
*/
struct packet* connect_p(int senderid) {
    struct packet* p = malloc(sizeof(struct packet));
    check_malloc(p);
    
    p->flag = CONNECT;
    p->pktseq = 0x00;
    p->ackseq = 0x00;
    p->unassigned = 0x00;
    p->senderid = senderid;
    p->recvid = 0x00;
    p->metadata = PACKET_S;

    return p;
}

/*
Parametre: int senderid
Returnerer: struct packet* p

Mottar sender-id'en til en klient, og lager en forbindelsesavslutningspakke paa samme pakkeformat som er oppgitt
i oppgaven. Deretter returnerer funksjonen denne pakken.

Denne funksjonen allokerer bytes til en pakke
*/
struct packet* connect_close_p(int senderid) {
    struct packet* p = malloc(sizeof(struct packet));
    check_malloc(p);
    
    p->flag = CLOSE;
    p->pktseq = 0x00;
    p->ackseq = 0x00;
    p->unassigned = 0x00;
    p->senderid = senderid;
    p->recvid = SERVER_ID;
    p->metadata = PACKET_S;

    return p;
}

/*
Parametre: int senderid, int mottatt_pk
Returnerer: struct packet* p

Mottar sender-id'en til en klient og den mottatte pakkens sekvensnummer, og lager en forbindelsesavslutningspakke 
paa samme pakkeformat som er oppgitt i oppgaven. Deretter returnerer funksjonen denne pakken.

Denne funksjonen allokerer bytes til en pakke
*/
struct packet* ACK_p(int senderid, int mottatt_pk) {
    struct packet* p = malloc(sizeof(struct packet));
    check_malloc(p);
    
    p->flag = ACK;
    p->pktseq = 0x00;
    p->ackseq = mottatt_pk;
    p->unassigned = 0x00;
    p->senderid = senderid;
    p->recvid = SERVER_ID;
    p->metadata = PACKET_S;
    return p;
}

//komandolinjeargumenter: IP-adresse, UDP-portnummeret, tapssannsynligheten
int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("To few arguments in client-NewFSP.c\n");
        return EXIT_FAILURE;
    }
    char *ip = argv[1];
    //atoi() tar inn en argv[i] og returnerer en int
    int PORT = atoi(argv[2]);
    float loss = atof(argv[3]);
    set_loss_probability(loss);

    //Oppretter en file discriptor til vaar socket
    int filed = socket(AF_INET, SOCK_DGRAM, 0);
    check_error(filed, "socket");

    struct sockaddr_in dest, sourcead;
    socklen_t ad_len;
    struct in_addr ip_ad;

    ad_len = sizeof(struct sockaddr_in);
    
    dest.sin_family = AF_INET;
    //Setter gitt port til network order (Big endian)
    dest.sin_port = htons(PORT);

    int write_count = inet_pton(AF_INET, ip, &ip_ad);
    dest.sin_addr = ip_ad;
    check_error(write_count, "inet_pton");
    
    if (!write_count) {
        printf("Invalid IP-adress: %s\n", ip);
        return EXIT_FAILURE;
    }
    int id = 0;

    //Setter id-en til en klient til aa vaere tilfeldig
    srand(time(NULL));
    id = rand() % 222;

    //Prover aa opprette rdp-kobling til server
    struct packet *connection = connect_p(id);

    rdp_send(filed, &dest, connection);
    
    struct packet* answer = rdp_read(filed, (struct sockaddr*)&sourcead, &ad_len, 1, 0);
    if (answer == NULL) {
        free(connection);
        printf("NO ANSWER\n");
        return EXIT_SUCCESS;
    }

    if (answer->flag == ACCEPT_REQ && answer->recvid == id) {
        //Setter <tilfedig tall> til id-en til klienten
        char str[] = "kernel-file-%d.txt";
        char filename[25];

        int number = id;
        sprintf(filename, str, number);
        //Aapner filen for aa skrive til
        FILE* file = fopen(filename, "w");
        struct packet* last_p = connection;
        struct packet* last_p_r = answer;
        unsigned char pkt = 0;
        int count = 0;

        while(1) {
            struct packet* read_count = rdp_read(filed, (struct sockaddr*)&sourcead, &ad_len, 0, 100000);
            
            if (read_count != NULL && read_count->flag == DATA) {
                if (read_count->metadata > 16) {
                    if (pkt == read_count->pktseq && count != 0) {
                        //kast pakke
                        printf("ALLEREDE MOTTAT: KASTER %d\n", pkt);
                        rdp_send(filed, &dest, last_p);
                        free(read_count->payload);
                        free(read_count);
                    } else {
                        printf("\nVi har mottatt en pakke med payload!\n");
                        struct packet* ack = ACK_p(id, read_count->pktseq);

                        rdp_send(filed, &dest, ack);
                        free_packet(last_p_r);
                        free_packet(last_p);
                        last_p_r = read_count;
                        pkt = last_p_r->pktseq;
                        last_p = ack;
                        fwrite(read_count->payload, sizeof(char), read_count->metadata - PACKET_S, file);
                        count++;
                    }
                } else {
                    struct packet* close_con = connect_close_p(id);
                    rdp_send(filed, &dest, close_con);

                    free(close_con);
                    free(read_count);
                    free_packet(last_p);
                    free_packet(last_p_r);
                    close(filed);
                    fclose(file);
                    return EXIT_SUCCESS;
                } 
            } 
        }
    } else {
        free(connection);
        free(answer);
        return EXIT_SUCCESS;
    }

}
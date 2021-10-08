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
#include "send_packet.h"
#include "rdp.h"


//Definerer de globale verdiene
struct rdp** connections;
int size_rdp_list = 0;
int packet_t = 0;


/*
Parametre: int res, char* message
Returnerer: 

Mottar et resultat av en funksjon hvor -1 tilsvarer en error og navnet paa funksjonen som er brukt. Kaller paa perror hvis en feil oppstaar.
*/
void check_error(int res, char* message) {
    if (res == -1) {
        perror(message);

        exit(EXIT_FAILURE);
    }
}

/*
Parametre: char* message
Returnerer: 

Mottar et resultat av en malloc og kaller paa perror hvis en feil oppstaar.
*/
void check_malloc(void* res) {
    if (!res) {
        perror("malloc");

        exit(EXIT_FAILURE);
    }
}

/*
Parametre: int destid
Returnerer: struct packet* p

Mottar destinasjons-id til klient og lager deretter en pakke paa pakkeformatet oppgitt i oppgaven. Til slutt returnerer funksjonen denne pakken.

Denne funksjonen allokerer bytes til en pakke
*/
struct packet* accept_connect_p(int destid) {
    struct packet* p = malloc(sizeof(struct packet));
    check_malloc(p);

    p->flag = ACCEPT_REQ;
    p->pktseq = 0x00;
    p->ackseq = 0x00;
    p->unassigned = 0x00;
    p->senderid = SERVER_ID;
    p->recvid = destid;
    p->metadata = PACKET_S;
    return p;
}

/*
Parametre: int destid
Returnerer: struct packet* p

Mottar destinasjons-id til klient og lager deretter en pakke paa pakkeformatet oppgitt i oppgaven. Til slutt returnerer funksjonen denne pakken.

Denne funksjonen allokerer bytes til en pakke
*/
struct packet* denied_connect_p(int destid) {
    struct packet* p = malloc(sizeof(struct packet));
    check_malloc(p);

    p->flag = DENIED_REQ;
    p->pktseq = 0x00;
    p->ackseq = 0x00;
    p->unassigned = 0x00;
    p->senderid = SERVER_ID;
    p->recvid = destid;
    p->metadata = PACKET_S;
    return p;
}

/*
Parametre: struct packet* p, int filed, struct sockaddr_in* sourcead, int N, int teller
Returnerer: struct rdp* r

Mottar en forbindelsesforesporsel og oppretter en rdp-forbindelse dersom klienten (eller en annen klient med samme id) allerede finnes i "lista" med rdp-forbindelser.
Funksjonen sender en aksepterings- eller avisningspakke til klienten som sendte pakken. Til slutt returnerer funkjsonen rdp-forbindelsen, om den er NULL eller ikke.

Denne funksjonen allokerer bytes til en rdp-forbindelse
*/
struct rdp* rdp_accept(struct packet* p, int filed, struct sockaddr_in* sourcead, int N, int teller) {
    struct rdp* r = malloc(sizeof(struct rdp));
    check_malloc(r);

    int s_id = SERVER_ID;
    int c_id = p->senderid;
    r->server_id = s_id;
    r->client_id = c_id;
    r->src_ad = *sourcead;
    r->packet_count = (unsigned long)0;
    r->accept_count = (unsigned long)0;
    r->last_seq = 0;
    r->ready = 0;
    r->file_read_count = 0;
    
    int o = 0;
    for (int i = 0 ; i < size_rdp_list ; i++) {
        if (connections[i] != NULL && connections[i]->client_id == c_id) {
            o++;
        }
    }
    // SJEKK N OGSaa
    if (o != 0 || teller >= N) {
        printf("\nNOT CONNECTED %d %d\n", p->senderid, SERVER_ID);
        struct packet* denied = denied_connect_p(p->senderid);
        rdp_send(filed, sourcead, denied);
        free(r);
        free(denied);
        return NULL;
    }

    printf("\nCONNECTED %d %d\n", p->senderid, SERVER_ID);
    struct packet* accepted = accept_connect_p(p->senderid);
    rdp_send(filed, sourcead, accepted);
    //Setter at den siste pakken som er sendt til denne rdp-fobindelsen sin klient er aksepteringspakken
    r->last_sendt_to_c = accepted;

    return r;
}

/*
Parametre: int client
Returnerer:

Mottar en klient-id til klienten som vil lukke rdp-forbindelsen. Metoden frigjor minnet til den siste pakken som er sendt til den gitte klienten, og klienten 
selv. Deretter setter den klienten lik NULL, og legger til baade denne NULL-pointeren og resten av de tilkoblede klientene i en midlertidig erstatning av connections.
Deretter kopierer metoden alt fra copy over i connections, og frigjor minnet til midlertidige dobbelpekeren. Da er rdp-forbindelsen til den gitte klienten lukket.
*/
void close_rdp(int client) {
    struct rdp** copy;
    copy = malloc((sizeof(struct rdp))*size_rdp_list);
    check_malloc(copy);
    

    for (int i = 0 ; i < size_rdp_list ; i++) {
        if (connections[i] != NULL && client == connections[i]->client_id) {
            free_packet(connections[i]->last_sendt_to_c);
            free(connections[i]);
            connections[i] = NULL;
        }
        copy[i] = connections[i];
    }
    memcpy(connections, copy, size_rdp_list);
    free(copy);
}

/*
Parametre: int filed, struct sockaddr_in* dest, struct packet* p
Returnerer:

Denne funksjonen fungerer som sendto/send_packet, men er spesifikt laget for aa sende pakker over en rdp-forbindelse. Den mottar en file discriptor, 
destinasjonsaddresse og pakken som skal sendes. Den oppretter et buffer fra den gitte pakken, og sender deretter buffere til destinasjonsaddressen ved bruk av
send_packet(). Den printer deretter ut informasjonen til pakken som akkurat ble sendt til terminalen. 
*/
void rdp_send(int filed, struct sockaddr_in* dest, struct packet* p) {
    char buffer[BUFFER_S];
    //Nuller ut bufferet slik at vi ikke tar med soppel
    memset(buffer, 0, BUFFER_S);

    buffer[0] = p->flag;
    buffer[1] = p->pktseq; 
    buffer[2] = p->ackseq; 
    buffer[3] = p->unassigned;
    //Endrer int'ene slik at de er paa standard format for internettet
    *(int*)&buffer[4] = htonl(p->senderid); 
    *(int*)&buffer[4+4] = htonl(p->recvid); 
    *(int*)&buffer[4+8] = htonl(p->metadata); 

    for (int i = 16 ; i < p->metadata ; i++) {
        if (p->metadata > 16) {
            buffer[i] = p->payload[i-16];
        } 
    }
    int write_count;
    write_count = send_packet(filed, buffer, p->metadata, 0, (struct sockaddr*)dest, sizeof(struct sockaddr_in));
    check_error(write_count, "send_packet");
    
    printf("\nSendte %d bytes:\n", write_count);
    printf("FLAGG: %d\n", p->flag);
    printf("SEKVENSNUMMER: %d\n", p->pktseq);
    printf("ACKNUMMER: %d\n", p->ackseq);
    printf("UNASSIGNED: %d\n", p->unassigned);
    printf("SENDERID: %d\n", p->senderid);
    printf("MOTTAKERID: %d\n", p->recvid);
    printf("METADATA: %d\n", p->metadata);
}

/*
Parametre: int filed, fd_set* read_f, fd_set* copy, long sec, long usec
Returnerer: int select() (returverdi fra funksjonskallet select)

Denne funksjonen mottar en file discriptor, to pekere til fd_set, og tidintervallet oppgit i sekunder, ogsaa mikrosekunder. Det funksjonen gjor er aa sette opp
et tidsintervall ut ifra parametrene, legger til vaar file discriptor til settene og kjorer select().
*/
int rpd_select(int filed, fd_set* read_f, fd_set* copy, long sec, long usec) {
    struct timeval timeout;

    timeout.tv_sec = sec;
    timeout.tv_usec = usec;

    FD_ZERO(read_f);
    FD_SET(filed, read_f);
    FD_SET(filed, copy);

    return select(filed+1, read_f, 0, 0, &timeout);
}

/*
Parametre: int filed, struct sockaddr* src_ad, socklen_t* ad_len, int sec, int usec
Returnerer: struct packet* p

Denne funksjonen mottar en pakke og leser til et buffer. Deretter opprette og returnerer den en pakke ut ifra dataen lest inn paa bufferet. Select brukes 
slik at det er mulig aa la funksjonen vente i 100 millisekunder for den gaar videre. Dermed maa vi ikke vente helt til recvfrom mottar en pakke, men maks 100ms.
Dersom recvfrom ikke mottar en pakke innen det skal funksjonen returnere NULL.
*/
struct packet* rdp_read(int filed, struct sockaddr* src_ad, socklen_t* ad_len, int sec, int usec) {
    int read_count = 0;
    char buffer[BUFFER_S];
    memset(buffer, 0, BUFFER_S);
    fd_set read_fdset, write_fdset;

    /*
    Select gjor det mulig aa avbryte recv/recvfrom sin ventetid etter et visst tidsintervall.
    */
    int recv = rpd_select(filed, &read_fdset, &write_fdset, sec, usec);

    while (1) {
        if (recv == -1) {
            perror("select");
            return NULL;
        } else if (recv == 0) {
            return NULL;
        } else {
            if (FD_ISSET(filed, &read_fdset)) {
                read_count = recvfrom(filed, buffer, BUFFER_S, 0, src_ad, ad_len);
                check_error(read_count, "recvfrom");
                buffer[read_count] = 0;

                struct packet* p = malloc(sizeof(struct packet));
                check_malloc(p);

                p->flag = buffer[0];
                p->pktseq = buffer[1];
                p->ackseq = buffer[2];
                p->unassigned = buffer[3];
                //Endrer buffer fra char til int, og setter til network order
                p->senderid = ntohl(*(int*)&buffer[4]);
                p->recvid = ntohl(*(int*)&buffer[4+4]);
                p->metadata = ntohl(*(int*)&buffer[4+8]);
                if (p->metadata > 16) {
                    p->payload = malloc(p->metadata-16);
                    check_malloc(p->payload);
                }

                for (int i = 16 ; i < BUFFER_S ; i++) {
                    if (p->metadata > 16 && buffer[i] != 0) {
                        p->payload[i-16] = buffer[i];
                    } 
                }

                printf("\nMottok %d bytes:\n", read_count);
                printf("FLAGG: %d\n", p->flag);
                printf("SEKVENSNUMMER: %d\n", p->pktseq);
                printf("ACKNUMMER: %d\n", p->ackseq);
                printf("UNASSIGNED: %d\n", p->unassigned);
                printf("SENDERID: %d\n", p->senderid);
                printf("MOTTAKERID: %d\n", p->recvid);
                printf("METADATA: %d\n", p->metadata);
                return p;
            }
        }
    }
    
}

/*
Parametre: struct packet* p
Returnerer:

Denne funksjonen er laget for aa frigjore minnet til en pakke. Den sjekker derfor om pakken inneholder noe data i payload, og frigjor hvis nodvendig. Deretter
frigjor den bytes'ene til selve pakken.
*/
void free_packet(struct packet* p) {
    if (p->metadata > 16) {
        free(p->payload);
    }
    free(p);
}

/*
Parametre: struct packet* p
Returnerer:

Denne funksjonen er laget for aa frigjore minnet til den glodale dobbeltpekeren til rdp-forbindelsene (connections). Denne lista kan inneholde NULL-pekere, og derfor
sjekker metoden om en rdp-forbindelse er lik NULL. Dermed kaller den paa free_packet() for aa frigjore den siste pakken som ble sendt til hver enkelt klient, og videre
rdp-forbindelsen selv. Helt tilslutt frigjor den selve lista.
*/
void free_connections() {
    for (int i = 0 ; i < size_rdp_list ; i++) {
        if (connections[i] != NULL) {
            free_packet(connections[i]->last_sendt_to_c);
            free(connections[i]);
        }
    }
    free(connections);
}


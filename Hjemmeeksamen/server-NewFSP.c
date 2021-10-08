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

/*
Parametre: int destid, char* data, int data_s
Returnerer: struct packet* p

Mottar destinasjons-id til klient, dataen den skal sende over og storrelsen paa dataen, og lager deretter en pakke paa
pakkeformatet oppgitt i oppgaven. Til slutt returnerer funksjonen denne pakken.

Denne funksjonen allokerer bytes til en pakke, og dens payload.
*/
struct packet* data_p(int destid, char* data, int data_s) {
    struct packet* p = malloc(sizeof(struct packet));
    check_malloc(p);
    p->payload = malloc(data_s);
    check_malloc(p->payload);
    
    p->flag = DATA;
    p->pktseq = packet_t;
    p->ackseq = 0x00;
    p->unassigned = 0x00;
    p->senderid = SERVER_ID;
    p->recvid = destid;
    p->metadata = PACKET_S + data_s;
    memcpy(p->payload, data, data_s);
    packet_t++;

    return p;
}

/*
Parametre: int destid
Returnerer: struct packet* p

Mottar destinasjons-id til klient, og lager deretter en tom pakke pakke paa
pakkeformatet oppgitt i oppgaven. Til slutt returnerer funksjonen denne pakken.

Denne funksjonen allokerer bytes til en pakke.
*/
struct packet* empty(int destid) {
    struct packet* p = malloc(sizeof(struct packet));
    check_malloc(p);
    
    p->flag = DATA;
    p->pktseq = packet_t;
    p->ackseq = 0x00;
    p->unassigned = 0x00;
    p->senderid = SERVER_ID;
    p->recvid = destid;
    p->metadata = PACKET_S;

    packet_t++;

    return p;
}

/*
Parametre: int filed, FILE* file
Returnerer:

Denne metoden er laget for aa sende data til alle de tilkoblede klientene. Dette gjores ved aa 
iterere gjennom den globale pekeren til rdp-structer, for aa deretter sende den neste pakke den
skal motta. Metoden sjekker om hver enkelt klient er klar for aa motta en pakke (altsaa at serveren
har mottatt en ack-pakke for den forrige pakken som var sendt). Hvis den siste pakken som ble
sendt til klienten var under 1000 bytes betyr det at klienten har mottatt hele filen, derfor sender
metoden en tom pakke i dette tilfellet. For aa opprette de riktige pakkene, saa kaller metoden paa data_p()
og empty().
*/
void send_data(int filed, FILE* file) {
    for (int i = 0 ; i < size_rdp_list ; i++) {
        if (connections[i] != NULL && connections[i]->ready == 0) {
            char filebuff[1000];
            if (connections[i]->packet_count == connections[i]->accept_count) {
                if (connections[i]->file_read_count > 0 && connections[i]->file_read_count < 1000) {
                    struct packet* p = empty(connections[i]->client_id);
                    rdp_send(filed, &connections[i]->src_ad, p);

                    //Passer paa aa frigjore minnet for jeg overskriver det, slik at bytes'ene ikke blir tapt
                    free_packet(connections[i]->last_sendt_to_c);
                    connections[i]->last_sendt_to_c = p;
                    connections[i]->last_seq = connections[i]->last_sendt_to_c->pktseq;
                    (connections[i]->packet_count)++;
                } else {
                    //Finner ut hvor i fila fread() skal starte aa lese fra ut ifra hvor mange pakker klienten har mottatt
                    int where = (connections[i]->accept_count) * (BUFFER_S-16);
                    fseek(file, where, SEEK_SET);
                    connections[i]->file_read_count = fread(filebuff, sizeof(char), (BUFFER_S-16), file);
                    struct packet* p = data_p(connections[i]->client_id, filebuff, connections[i]->file_read_count);
                    rdp_send(filed, &connections[i]->src_ad, p);

                    free_packet(connections[i]->last_sendt_to_c);
                    connections[i]->last_sendt_to_c = p;
                    connections[i]->last_seq = connections[i]->last_sendt_to_c->pktseq;
                    (connections[i]->packet_count)++;
                }
            }
        } else {
            if (connections[i] != NULL) {
                printf("IKKE KLAR FOR PAKKE: %d\n", connections[i]->client_id);
            }
        }
    }
}

//komandolinjeargumenter: UDP-portnummeret, tapssannsynligheten
int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("To few arguments in server-NewFSP.c\n");
        return EXIT_FAILURE;
    }
    //atoi() tar inn en argv[i] og returnerer en int
    int PORT = atoi(argv[1]);
    int N = atoi(argv[2]);
    float loss = atof(argv[3]);
    set_loss_probability(loss);
    struct sockaddr_in my_ad, sourcead;
    socklen_t ad_len;

    //file discriptor
    int filed = socket(AF_INET, SOCK_DGRAM, 0);
    check_error(filed, "socket");

    ad_len = sizeof(struct sockaddr_in);

    int read_count;

    my_ad.sin_family = AF_INET;
    my_ad.sin_port = htons(PORT);
    my_ad.sin_addr.s_addr = INADDR_ANY;

    read_count = bind(filed, (struct sockaddr*)&my_ad, sizeof(struct sockaddr_in));
    check_error(read_count, "bind");
    connections = malloc(sizeof(struct rdp)*size_rdp_list);
    check_malloc(connections);

    FILE* file = fopen("test.txt", "r");
    if (!file) {
        perror("fopen");
    }
    int i = 0;
    int clients = 0;

    while(1) {
        //Avslutter naar serveren har sendt filen til N klienter
        if (i >= N) {
            free_connections();
            close(filed);
            fclose(file);
            return EXIT_SUCCESS;
        }
        struct packet* p_1 = rdp_read(filed, (struct sockaddr*)&sourcead, &ad_len, 0, 100000);

        //Dersom serveren ikke mottar enn pakke innen 100 millisekunder har det skjedd noe galt, og da skal serveren sende den siste pakken
        //den har sendt paa nytt
        if (p_1 == NULL) {
            for (int j = 0; j < size_rdp_list ; j++) {
                //Sjekker hvilken klient det gjelder
                if (connections[j] != NULL && connections[j]->packet_count != connections[j]->accept_count) {
                    rdp_send(filed, &connections[j]->src_ad, connections[j]->last_sendt_to_c);
                }
            }
        } else {
            //Sjekker om en ny forbindelsesforesporsel er kommet
            if (p_1->flag == CONNECT) {
                struct rdp *uno = rdp_accept(p_1, filed, &sourcead, N, clients);
                if (uno != NULL) {
                    struct rdp** add_one = malloc(sizeof(struct rdp)*(size_rdp_list+1));
                    check_malloc(add_one);

                    memcpy(add_one, connections, sizeof(struct rdp)*(size_rdp_list));
                    add_one[size_rdp_list] = uno;
                    free(connections);

                    connections = add_one;
                    clients++;
                    size_rdp_list++;
                } 
            } else if (p_1->flag == ACK) {
                for (int j = 0 ; j < size_rdp_list ; j++) {
                    if (connections[j] != NULL && connections[j]->client_id == p_1->senderid) {
                        if (p_1->ackseq == connections[j]->last_seq) {
                            connections[j]->ready = 0;
                            (connections[j]->accept_count)++;
                        } else {
                            connections[j]->ready = 1;
                            printf("IT'S NOT THE RIGHT ACK\n");
                            rdp_send(filed, &connections[j]->src_ad, connections[j]->last_sendt_to_c);
                        }
                    }
                }
            } else if (p_1->flag == CLOSE) {
                for (int j = 0 ; j < size_rdp_list ; j++) {
                    if (connections[j] != NULL && connections[j]->client_id == p_1->senderid) {
                        printf("\nDISCONNECTED %d %d\n", connections[j]->client_id, SERVER_ID);
                        close_rdp(connections[j]->client_id);
                        i++;
                    }
                }
            }
            free_packet(p_1);
        }
        if (size_rdp_list > 0) {
            //Sender datapakker til alle de tilkoblede klientene
            send_data(filed, file);
        }
    }
}

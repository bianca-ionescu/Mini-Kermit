#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "lib.h"
#include "kermit.h"

#define HOST "127.0.0.1"
#define PORT 10001

void prelucrare_mesaj_primit (Mini_Kermit* m) {
    msg* mesaj = kermit_to_msg(m);
    send_message(mesaj);
    free(mesaj);
    free(m);
}

msg* receive_msg () {
    int c = 3;
    msg* r;
    while (c > 0) {
        r = receive_message_timeout(5000);
        if (r != NULL) {
            unsigned short check = crc16_ccitt(r->payload, r->len - 3);
            printf("crc calculat %d\n", check);
            unsigned short check_recv;
            memcpy (&check_recv, r->payload + r->len - 3, 2);
            printf("crc primit %d\n", check_recv);

            if (check == check_recv) {
                printf("ACK mesaj primit\n");
                Mini_Kermit* m = aloc('Y', (r->payload[2] + 1) % 64, 0, (unsigned char*) "");
                prelucrare_mesaj_primit(m);
                break;
            } else {
                printf("NAK mesaj primit\n");
                Mini_Kermit* m = aloc('N', (r->payload[2] + 1) % 64, 0, (unsigned char*) "");
                c = 3;
                prelucrare_mesaj_primit(m);
            }
            
        } else c--;
    }
    return r;
}

int main(int argc, char** argv) {
    msg* r;

    init(HOST, PORT);
    int fd; 
    do {
        r = receive_msg();
        if (r != NULL){
            if (r->payload[3] == 'F') { //creare fisier
                char nume[10];
                memcpy(nume, r->payload + 4, r->len - 7); //3 sus 4 jos
                char new_name[20];
                strcpy(new_name, "recv_");
                strncat(new_name, nume, 9);
                fd = open(new_name, O_CREAT | O_WRONLY, 0777);
                if(fd < 0) {
                    printf("Eroare creare fiser\n");
                    break;
                }
            } else if (r->payload[3] == 'D') { //scrie in fisier curent
                write(fd, r->payload + 4, r->len - 7);
            } else if (r->payload[3] == 'Z') { //inchidere fisier curent
                close(fd);
            }
            // free(r);
        } else {
            printf("-----TIMEOUT-----\n");
            break;
        }

    } while (r->payload[3] != 'B');
    
	return 0;
}

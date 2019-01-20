#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "lib.h"
#include "kermit.h"

#define HOST "127.0.0.1"
#define PORT 10000

unsigned char seq = 0;

void send_msg (msg* m) {
    int c = 3;
    send_message(m);
    while (c > 0) {
        msg* r = receive_message_timeout(5000);
        if (r != NULL) {
            if (r->payload[2] == (seq + 1) % 64) {
                if (r->payload[3] == 'Y') {
                    seq = (seq + 2) % 64;
                    printf("ACK mesaj trimis\n");
                    break;
                } else if (r->payload[3] == 'N') {
                    c = 3;
                    seq = (seq + 2) % 64;
                    printf("NAK mesaj trimis\n");
                    m->payload[2] = seq;
                    unsigned short check = crc16_ccitt(m->payload, m->len - 3);
                    memcpy(&m->payload[m->len - 3], &check, 2);
                    send_message(m);
                } 
            }
        } else {
            c--;
            send_message(m);
        }
        free(r);
    }
}

void prelucrare_mesaj_trimis (Mini_Kermit* m) {
    msg* mesaj = kermit_to_msg(m);
    send_msg(mesaj);
    free(mesaj);
    free(m);
}

int main(int argc, char** argv) {
    unsigned char buf[250];

    init(HOST, PORT);
    Mini_Kermit* m = aloc('S', 0, 0, (unsigned char*) "");
    prelucrare_mesaj_trimis(m);

    int fd;
    for(int i = 1; i < argc; i++) {
        fd = open(argv[i], O_RDONLY);
        printf("Nume fisier: %s\n", argv[i]);
        if(fd < 0) {
            printf("Nu s-a putut deschide fisierul nr. %d\n", i);
            continue;
        }
        
        m = aloc ('F', seq, strlen(argv[i]), (unsigned char*) argv[i]);
        prelucrare_mesaj_trimis(m);
        
        struct stat fs; // dim fisier
        fstat(fd, &fs);
        int dim = fs.st_size;
        printf("Dimensiune fisier: %d\n", dim);
        
        while(dim > 0) {
            int readAux = read(fd, buf, 250);
            
            m = aloc ('D', seq, readAux, buf);
            prelucrare_mesaj_trimis(m);
            
            dim -= readAux;
            printf("New dim: %d\n", dim); 
        }
        printf("Trimitere EOF fisier curent\n");
        m = aloc ('Z', seq, 0, (unsigned char*) "");
        prelucrare_mesaj_trimis(m);

    }
    printf("Trimitere pachet incheiere transmisie\n");
    m = aloc ('B', seq, 0, (unsigned char*) "");
    prelucrare_mesaj_trimis(m);

    return 0;
}
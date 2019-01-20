#ifndef KERMIT
#define KERMIT

typedef struct {
	unsigned char SOH, LEN, SEQ, TYPE;
	unsigned char* DATA;
	unsigned short CHECK;
	unsigned char MARK;
} __attribute__((__packed__)) Mini_Kermit;

typedef struct {
	unsigned char MAXL, TIME, NPAD, PADC, EOL;
	unsigned char QCTL, QBIN, CHKT, REPT, CAPA, R;
} __attribute__((__packed__)) Data_Init;

Mini_Kermit* aloc(unsigned char type, unsigned char seq, unsigned char size, unsigned char* data);
msg* kermit_to_msg (Mini_Kermit* m);
void print_package(Mini_Kermit* m);
void print_data(Mini_Kermit* m);

/* Implementare functii */

Mini_Kermit* aloc(unsigned char type, unsigned char seq, unsigned char size, unsigned char* data) { //size -> dim camp data; data -> text efectiv
	Mini_Kermit* m = calloc (1, sizeof(Mini_Kermit));
	if (m == NULL){
		printf("Eroare alocare m la pachet de tip %d\n", type);
		return NULL;
	}

	m->SEQ = seq;
	m->SOH = 0x01;
	m->TYPE = type;

	if (type == 'S'){
		m->LEN = 16;
		Data_Init* data_init = (Data_Init*) calloc(1, sizeof(Data_Init));
		data_init->MAXL = 250;
		data_init->TIME = 5;
		data_init->EOL = 0x0D;
		m->DATA = (unsigned char*) data_init;
	} else if (type == 'F' || type == 'D') {
		m->LEN = (unsigned char) 5 + size;
		free(m->DATA);
		m->DATA = (unsigned char*) malloc(size);
		memcpy(m->DATA, data, size);
	} else {
		m->LEN = 5;
		free(m->DATA);
		m->DATA = NULL;
	}
	print_package(m);
	return m;
}

msg* kermit_to_msg (Mini_Kermit* m) {
	msg* mesaj = malloc (sizeof(msg));
	memcpy(mesaj->payload, m, 4);
	memcpy(mesaj->payload + 4, m->DATA, m->LEN - 5); //de la val 4 incolo
	unsigned short check = crc16_ccitt(mesaj->payload, m->LEN - 1);
	memcpy(mesaj->payload + m->LEN - 1, &check, 2);
	memcpy(mesaj->payload + m->LEN + 1, &(m->MARK), 1);
	mesaj->len = m->LEN + 2;
	return mesaj;
}

/* Printare informatii */

void print_package(Mini_Kermit* m) {

	printf("+------+------+------+------+-------+------+\n");
	printf("| SOH  | LEN  | SEQ  | TYPE | CHECK | MARK |\n");
	printf("+------+------+------+------+-------+------+\n");
	printf("| 0x%02X | 0x%02X | 0x%02X | 0x%02X | 0x%02X  | 0x%02X |\n",
			m->SOH, m->LEN, m->SEQ, m->TYPE, m->CHECK, m->MARK);
	printf("+------+------+------+------+-------+------+\n");
	printf("\n");
	if(m->TYPE == 'S') print_data(m);
	else {
		for (int i = 0; i < m->LEN - 5; i++)
			printf("%02X ", m->DATA[i]);
		printf("\n");
	}
}

void print_data(Mini_Kermit* m) {
	printf("+------+------+------+------+------+------+------+------+------+------+------+\n");
	printf("| MAXL | TIME | NPAD | PADC | EOL  | QCTL | QBIN | CHKT | REPT | CAPA |   R  |\n");
	printf("+------+------+------+------+------+------+------+------+------+------+------+\n");
	printf("| 0x%02X | 0x%02X | 0x%02X | 0x%02X | 0x%02X | 0x%02X | 0x%02X | 0x%02X | 0x%02X | 0x%02X | 0x%02X |\n",
			((Data_Init*) m->DATA)->MAXL, ((Data_Init*) m->DATA)->TIME, ((Data_Init*) m->DATA)->NPAD, 
			((Data_Init*) m->DATA)->PADC, ((Data_Init*) m->DATA)->EOL, ((Data_Init*) m->DATA)->QCTL, 
			((Data_Init*) m->DATA)->QBIN, ((Data_Init*) m->DATA)->CHKT, ((Data_Init*) m->DATA)->REPT, 
			((Data_Init*) m->DATA)->CAPA, ((Data_Init*) m->DATA)->R);
	printf("+------+------+------+------+------+------+------+------+------+------+------+\n");
}

#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "mytypes.h"

//int scacchieraPrinc[NUM_RIGHE][NUM_COLONNE];

//int* riga = (int*) malloc(NUM_COLONNE * sizeof(int));

int** scacchieraPrinc;

int numero_umani = 1;

int casellevuote = NUM_RIGHE * NUM_COLONNE;

int numeronere, numerobianche = 0;

int turno_globale = 0;

// #define GET_ELEM(x) if (x == VUOTO) return *; if (x == NERO) return N; if (x == BIANCO) return B;

typedef struct _mossa {
	int my;
	int mx;
} mossa;

typedef struct t_nodo
{
   int** status;
   mossa **vettoremosse;
   //mossa vettoremosse[AI_MAX_DEPTH];
   int nmosse;
   float value;
   struct t_nodo **figli;
   //int* ciao[4];
   int nfigli;
} nodo;

void sel_elemento(int x, char* thechar) {
	//printf("%d\n", x);
	if (x == VUOTO) *thechar = '*';
	if (x == NERO ) *thechar = 'N';
	if (x == BIANCO) *thechar = 'B';
	//printf("%c\n", *thechar);
	//else printf("ELEMNTO NON VALIDO. Esco.\n");
	//exit(0);

}

int avversa(int x) {
	return ((x == NERO) ? BIANCO : NERO);
}


int crop(int x, int min, int max ) {
	if (x < min) {
		//printf("Primo caso\n");
		return min;
	}
	else if (x > max) {
		//printf("Secondo caso\n");
		return max;
	}
	else {
		//printf("Terzo caso\n");
		return x;
	}
}


void scacchiera_init() {
	int i,j;

	for (i=0; i<NUM_RIGHE; i++) {
		for (j=0; j<NUM_COLONNE; j++) {
			scacchieraPrinc[i][j] = VUOTO;
		}

	}

}

void scacchiera_print() {
	int i,j,k;
	char mychar = '\0';

	printf("\n");

	printf("\t");
	for (i=0; i < (NUM_COLONNE * 2) +1; i++) {
		printf("-");
	}
	printf("\n");

	for (i=0; i<NUM_RIGHE; i++) {
		printf("\t");
		for (j=0; j < NUM_COLONNE; j++) {
			sel_elemento(scacchieraPrinc[i][j], &mychar);
			printf("|%c", mychar);
		}
		
		printf("|\n");
		printf("\t");

		for (k=0; k < (NUM_COLONNE * 2) + 1; k++) {
			printf("-");
		}
		printf("\n");

	}
	printf("\n");

}

int write_game_to_file() {
	FILE *file;
	int i, j = 0;
	char mychar='\0';
	
	file = fopen("output.txt", "w");
	if (!file ) {
		printf("%s non è riuscita ad aprire il file in scrittura.", __func__);
		return -1;
	}
	// Dovrebbe scrivere 8 righe, ogni riga di 18 caratteri:
	// 8 x elemento + spazio + \r + \n 
	for (i=0; i< NUM_RIGHE; i++) {
		for (j=0; j< NUM_COLONNE; j++) {
			sel_elemento(scacchieraPrinc[i][j], &mychar);
			fprintf(file, "%c ", mychar);
		}
		fprintf(file, "%s","\r\n");
	}

	fclose(file);

	return 1;

}

int read_game_from_file() {

	FILE *file;
	long int filelength;
	int i = 0;
	int j = 0;
	//int eof = 0;
	char *contenuto;
	char * pcont;
	int provvmat[NUM_RIGHE][NUM_COLONNE];

	file = fopen("input.txt", "r");
	if (!file) {
		printf("Errore nella lettura!\n");
		return -1;
	}
	fseek(file, 0L, SEEK_END);
	filelength = ftell(file);
	rewind(file);

	contenuto = calloc(filelength +1, sizeof(char));

	if (contenuto == NULL) {
		printf("Errore nell'allocare memoria.\n");
		return -1;
	}

	fread(contenuto, filelength, 1, file);
	pcont = contenuto;

	while (*pcont) {
		char mychar = *pcont;
		printf("Carattere: %c\n",mychar); 
		switch (mychar) {
			case 'n':
			case 'N': {
						  printf("Almeno sono qui. i: %d e j: %d\n",i,j);
						  provvmat[i][j] = NERO;
						  j++;
						  break;
					  }
			case 'b':
			case 'B': { 
						  provvmat[i][j] = BIANCO;
						  j++;
						  break;
					  }
			case '*': {
						  provvmat[i][j] = VUOTO;
						  j++;
						  //printf("%c", mychar);
						  break;
					  }
			case '\r':
					  break;
			case '\n': {
						   printf("Qui almeno entro.");
						   if (j == NUM_COLONNE ) {
							   i++;
							   j = 0;
							   printf("Ho resettato. j: %d e i: %d.\n",j,i);
						   }
						   else {
							   printf("Numero oggetti per riga: %d.\n", j);
							   printf("LETTURA - Numero di oggetti per riga errato!\n");
							   return -1;
						   }
						   break;
					   }
			case ' ':
			case '\t':
					   break;
			default: {
						 printf("LETTURA - Carattere NON VALIDO!\n");
						 return -1;
					 }

		}

		if (j > NUM_COLONNE) {
			printf("Numero oggetti per riga: %d\n", j);
			printf("LETTURA - Numero di oggetti per riga errato!\n");
			return -1;
		}

		if (i > NUM_RIGHE) {
			printf("LETTURA - Numero di colonne errato!\n");
			return -1;

		}

		pcont++;

	}

	fclose(file);

	// Se arrivo qui, dovrebbe essere andato tutto bene, quindi copio
	numeronere = numerobianche = 0;
	//char mychar;

	for (i=0; i<NUM_RIGHE; i++) {
		for (j=0; j<NUM_COLONNE; j++) {
			scacchieraPrinc[i][j] = provvmat[i][j];
			//mychar = sel_elemento(scacchieraPrinc[i][j]);
			if (scacchieraPrinc[i][j] == NERO) numeronere++;
			else if (scacchieraPrinc[i][j] == BIANCO) numerobianche++;
		}
		
	}

	printf("Lettura andata a buon fine.\n");

	return 1;

	/*
	do {
		i = fscanf(file,"%s", &line); }
	while (i != ' ' && i != '\n' && i != 'r');

	i = 0;
	j = 0;
	eof = 0;

	while (!eof) {

		switch (line) { 
			case 'n':
			case 'N': { provvmat[i][j] = NERO; break; }
			case 'b':
			case 'B': { provvmat[i][j] = BIANCO; break; }
			case '*': { provvmat[i][j] = VUOTO; break; }

			default: 

		}
	}
	*/


}

int check_upperleft(int** matrix, int posy, int posx, int pedina, int sostflag) {
	// Devo cercare andando nella direzione in alto a sinistra.
	// Se trovo un vuoto, ritorno non legale.
	// Se trovo un'avversa, va bene, continuo a cercare E LA MODIFICO.
	// Se trovo una del colore che cerco, ok, ritorno ok.

	// Se sostflag è 0, non cambio le pedine sulla scacchiera.
	// Se sotflag è 1, cambio le pedine sulla scacchiera E aggiorno i contatori GLOBALI.
	// Se sostflag è 2, cambio le pedine sulla scacchiera MA NON aggiorno i contatori GLOBALI. 

	//int i;
	int ret = 0;
	int posyiniz = posy;
	int posxiniz = posx;
	
	//printf("Sono in check_upperleft.\n");

	while ( (posy >= 0) && (posx >0) ) {
		if (matrix[posy][posx] == VUOTO) {
			ret = 0;
			break;
		}
		
		if (matrix[posy][posx] == pedina ) {
			ret = 1;
			break;
		}

		/*
		if (matrix[posy][posx] == avversa(pedina)) {
			if (pedina == BIANCO) {  numerobianche++; numeronere--; } else { numeronere++; numerobianche--;}
			matrix[posy][posx] = pedina;
		}
		*/

		posx--;
		posy--;
	}

	if (ret == 0) {
		return 0;
	}
	else if (ret == 1) {
		int numeroprese = 0;
		while ( (posy != posyiniz ) && (posx != posxiniz) ) {
			if ( matrix[posyiniz][posxiniz] == avversa(pedina) ) {
				if (sostflag > 0) {
					if (sostflag == 1) {
						if (pedina == BIANCO) {numerobianche++, numeronere--;} else {numeronere++; numerobianche--;}
					}
					matrix[posyiniz][posxiniz] = pedina;
				}
				numeroprese++;
			}
			posxiniz--;
			posyiniz--;
		}
		//printf("%s: ritorna %d.\n",__func__, numeroprese);
		return numeroprese;
	}

	//printf("%s: ritorna %d\n",__func__, ret);
	//return ret;

}

int check_upper(int** matrix, int posy, int posx, int pedina, int sostflag) {
	// Devo cercare andando nella direzione in alto.
	// Se trovo un vuoto, ritorno non legale.
	// Se trovo un'avversa, va bene, continuo a cercare.
	// Se trovo una del colore che cerco, ok, ritorno ok.

	//int i;
	int ret = 0;
	int posyiniz = posy;
	int posxiniz = posx;
	while ( (posy >= 0) && (posx >0) ) {
		if (matrix[posy][posx] == VUOTO) {
			ret = 0;
			break;
		}
		
		if (matrix[posy][posx] == pedina ) {
			ret = 1;
			break;
		}

		/*
		if (matrix[posy][posx] == avversa(pedina)) {
			if (pedina == BIANCO) {  numerobianche++; numeronere--; } else { numeronere++; numerobianche--;}
			matrix[posy][posx] = pedina;
		}
		*/

		posy--;
	}

	if (ret == 0) {
		return 0;
	}
	else if (ret == 1 ) {
		int numeroprese = 0;
		while (posyiniz != posy) {
			if (matrix[posyiniz][posxiniz] == avversa(pedina)) {
				if (sostflag > 0) {
					if (sostflag == 1) {
						if (pedina == BIANCO) {  numerobianche++; numeronere--; } else { numeronere++; numerobianche--;}
					}
					matrix[posyiniz][posxiniz] = pedina;
				}
				numeroprese++;
			}
			posyiniz--;
		}
		//printf("%s: ritorna %d.\n",__func__, numeroprese);
		return numeroprese;
	}

	//printf("%s: ritorna %d\n",__func__, ret);
	//return ret;

}

int check_upperright(int** matrix, int posy, int posx, int pedina, int sostflag) {
	// Devo cercare andando nella direzione in alto a destra.
	// Se trovo un vuoto, ritorno non legale.
	// Se trovo un'avversa, va bene, continuo a cercare.
	// Se trovo una del colore che cerco, ok, ritorno ok.

	//int i;
	int ret = 0;
	int posyiniz = posy;
	int posxiniz = posx;
	while ( (posy >= 0) && (posx < NUM_COLONNE) ) {
		if (matrix[posy][posx] == VUOTO) {
			ret = 0;
			break;
		}
		
		if (matrix[posy][posx] == pedina ) {
			ret = 1;
			break;
		}

		/*
		if (matrix[posy][posx] == avversa(pedina)) {
			if (pedina == BIANCO) {  numerobianche++; numeronere--; } else { numeronere++; numerobianche--;}
			matrix[posy][posx] = pedina;
		}
		*/

		posy--;
		posx++;
	}

	if (ret == 0) {
		//ret = 0;
		return 0;
	}
	else if (ret == 1 ) {
		int numeroprese = 0;
		while ( (posyiniz != posy) && (posxiniz != posx) ) {
			if (matrix[posyiniz][posxiniz] == avversa(pedina)) {
				if (sostflag > 0) {
					if (sostflag == 1) {
						if (pedina == BIANCO) {  numerobianche++; numeronere--; } else { numeronere++; numerobianche--;}
					}
					matrix[posyiniz][posxiniz] = pedina;
				}
				numeroprese++;
			}
			posyiniz--;
			posxiniz++;
		}
		//printf("%s: ritorna %d.\n",__func__, numeroprese);
		return numeroprese;
	}

	//printf("%s: ritorna %d\n",__func__, ret);
	//return ret;

}

int check_right(int** matrix, int posy, int posx, int pedina, int sostflag) {
	// Devo cercare andando nella direzione a destra.
	// Se trovo un vuoto, ritorno non legale.
	// Se trovo un'avversa, va bene, continuo a cercare.
	// Se trovo una del colore che cerco, ok, ritorno ok.

	//int i;
	int ret = 0;
	int posyiniz = posy;
	int posxiniz = posx;
	while ( (posy >= 0) && (posx < NUM_COLONNE) ) {
		if (matrix[posy][posx] == VUOTO) {
			ret = 0;
			break;
		}
		
		if (matrix[posy][posx] == pedina ) {
			ret = 1;
			break;
		}

		/*
		if (matrix[posy][posx] == avversa(pedina)) {
			if (pedina == BIANCO) {  numerobianche++; numeronere--; } else { numeronere++; numerobianche--;}
			matrix[posy][posx] = pedina;
		}
		*/

		posx++;
	}

	if (ret == 0) {
		//ret = 0;
		return 0;
	}
	else if (ret == 1 ) {
		int numeroprese = 0;
		while (posxiniz != posx) {
			if (matrix[posyiniz][posxiniz] == avversa(pedina)) {
				if (sostflag > 0) {
					if (sostflag == 1) {
						if (pedina == BIANCO) {  numerobianche++; numeronere--; } else { numeronere++; numerobianche--;}
					}
					matrix[posyiniz][posxiniz] = pedina;
				}
				numeroprese++;
			}
			posxiniz++;
		}
		//printf("%s: ritorna %d.\n",__func__, numeroprese);
		return numeroprese;
	}

	//printf("%s: ritorna %d\n",__func__, ret);
	//return ret;

}

int check_lowerright(int** matrix, int posy, int posx, int pedina, int sostflag) {
	// Devo cercare andando nella direzione in basso a destra.
	// Se trovo un vuoto, ritorno non legale.
	// Se trovo un'avversa, va bene, continuo a cercare.
	// Se trovo una del colore che cerco, ok, ritorno ok.

	//int i;
	int ret = 0;
	int posyiniz = posy;
	int posxiniz = posx;
	while ( (posy < NUM_RIGHE) && (posx < NUM_COLONNE) ) {
		if (matrix[posy][posx] == VUOTO) {
			ret = 0;
			break;
		}
		
		if (matrix[posy][posx] == pedina ) {
			ret = 1;
			break;
		}

		/*
		if (matrix[posy][posx] == avversa(pedina)) {
			if (pedina == BIANCO) {  numerobianche++; numeronere--; } else { numeronere++; numerobianche--;}
			matrix[posy][posx] = pedina;
		}
		*/

		posy++;
		posx++;
	}

	if (ret == 0) {
		//ret = 0;
		return 0;
	}
	else if (ret == 1 ) {
		int numeroprese = 0;
		while ( (posxiniz != posx) && (posyiniz != posy) ) {
			if (matrix[posyiniz][posxiniz] == avversa(pedina)) {
				if (sostflag > 0) {
					if (sostflag == 1) {
						if (pedina == BIANCO) {  numerobianche++; numeronere--; } else { numeronere++; numerobianche--;}
					}
					matrix[posyiniz][posxiniz] = pedina;
				}
				numeroprese++;
			}
			posxiniz++;
			posyiniz++;
		}
		//printf("%s: ritorna %d.\n",__func__, numeroprese);
		return numeroprese;
	}

	//printf("%s: ritorna %d\n",__func__, ret);
	//return ret;

}

int check_lower(int** matrix, int posy, int posx, int pedina, int sostflag) {
	// Devo cercare andando nella direzione in basso.
	// Se trovo un vuoto, ritorno non legale.
	// Se trovo un'avversa, va bene, continuo a cercare.
	// Se trovo una del colore che cerco, ok, ritorno ok.

	//int i;
	int ret = 0;
	int posxiniz = posx;
	int posyiniz = posy;
	while ( (posy < NUM_RIGHE) && (posx < NUM_COLONNE) ) {
		if (matrix[posy][posx] == VUOTO) {
			ret = 0;
			break;
		}
		
		if (matrix[posy][posx] == pedina ) {
			ret = 1;
			break;
		}

		/*
		if (matrix[posy][posx] == avversa(pedina)) {
			if (pedina == BIANCO) {  numerobianche++; numeronere--; } else { numeronere++; numerobianche--;}
			matrix[posy][posx] = pedina;
		}
		*/

		posy++;
	}

	if (ret == 0) {
		//ret = 0;
		return 0;
	}
	else if (ret == 1) {
		int numeroprese = 0;
		while (posyiniz != posy) {
			if (matrix[posyiniz][posxiniz] == avversa(pedina)) {
				if (sostflag > 0) {
					if (sostflag == 1) {
						if (pedina == BIANCO) {  numerobianche++; numeronere--; } else { numeronere++; numerobianche--;}
					}
					matrix[posyiniz][posxiniz] = pedina;
				}
				numeroprese++;
			}
			posyiniz++;
		}
		//printf("%s: ritorna %d.\n",__func__, numeroprese);
		return numeroprese;
	}

	//printf("%s: ritorna %d\n",__func__, ret);
	//return ret;

}

int check_lowerleft(int** matrix, int posy, int posx, int pedina, int sostflag) {
	// Devo cercare andando nella direzione in basso a sinistra.
	// Se trovo un vuoto, ritorno non legale.
	// Se trovo un'avversa, va bene, continuo a cercare.
	// Se trovo una del colore che cerco, ok, ritorno ok.

	//int i;
	int ret = 0;
	int posyiniz = posy;
	int posxiniz = posx;
	while ( (posy < NUM_RIGHE) && (posx >= 0) ) {
		if (matrix[posy][posx] == VUOTO) {
			ret = 0;
			break;
		}
		
		if (matrix[posy][posx] == pedina ) {
			ret = 1;
			break;
		}

		/*
		if (matrix[posy][posx] == avversa(pedina)) {
			if (pedina == BIANCO) {  numerobianche++; numeronere--; } else { numeronere++; numerobianche--;}
			matrix[posy][posx] = pedina;
		}
		*/

		posx--;
		posy++;
	}

	if (ret == 0) {
		//ret = 0;
		return 0;
	}
	else if (ret == 1 ) {
		int numeroprese = 0;
		while ( (posxiniz != posx) && (posyiniz != posy) ) {
			if (matrix[posyiniz][posxiniz] == avversa(pedina)) {
				if (sostflag > 0) {
					if (sostflag == 1) {
						if (pedina == BIANCO) {  numerobianche++; numeronere--; } else { numeronere++; numerobianche--;}
					}
					matrix[posyiniz][posxiniz] = pedina;
				}
				numeroprese++;
			}
			posxiniz--;
			posyiniz++;
		}
		//printf("%s: ritorna %d.\n",__func__, numeroprese);
		return numeroprese;
	}

	//printf("%s: ritorna %d\n",__func__, ret);
	//return ret;
}

int check_left(int** matrix, int posy, int posx, int pedina, int sostflag) {
	// Devo cercare andando nella direzione a sinistra.
	// Se trovo un vuoto, ritorno non legale.
	// Se trovo un'avversa, va bene, continuo a cercare.
	// Se trovo una del colore che cerco, ok, ritorno ok.

	//int i;
	int ret = 0;
	int posyiniz = posy;
	int posxiniz = posx;
	while ( (posy < NUM_RIGHE) && (posx >= 0) ) {
		if (matrix[posy][posx] == VUOTO) {
			ret = 0;
			break;
		}
		
		if (matrix[posy][posx] == pedina ) {
			ret = 1;
			break;
		}

		/*
		if (matrix[posy][posx] == avversa(pedina)) {
			if (pedina == BIANCO) {  numerobianche++; numeronere--; } else { numeronere++; numerobianche--;}
			matrix[posy][posx] = pedina;
		}
		*/

		posx--;
	}

	if (ret == 0) {
		//ret = 0;
		return 0;
	}
	else if (ret == 1 ) {
		int numeroprese = 0;
		while (posxiniz != posx) {
			if (matrix[posyiniz][posxiniz] == avversa(pedina)) {
				if (sostflag > 0) {
					if (sostflag == 1) {
						if (pedina == BIANCO) {  numerobianche++; numeronere--; } else { numeronere++; numerobianche--;}
					}
					matrix[posyiniz][posxiniz] = pedina;
				}
				numeroprese++;
			}
			posxiniz--;
		}
		//printf("%s: ritorna %d.\n",__func__, numeroprese);
		return numeroprese;
	}

	//printf("%s: ritorna %d\n",__func__, ret);
	//return ret;

}

int check_mossa_ed_esegui(int posy, int posx, int pedina) {

	// Controlla che la mossa in ingresso sia valida e, nel caso, la esegue, sostituendo le pedine necessarie.

	int ret = -1;

	if (scacchieraPrinc[posy][posx] != VUOTO ) {
		// Mossa non valida
		return -1;
	}
	else {

		if ( scacchieraPrinc[crop(posy -1, 0, NUM_RIGHE -1)][crop(posx-1, 0, NUM_COLONNE -1)] == avversa(pedina) ) {
			if (check_upperleft(scacchieraPrinc, crop(posy -1, 0, NUM_RIGHE -1), crop(posx-1, 0, NUM_COLONNE -1), pedina, 1)) ret = 1;
		}

		if ( scacchieraPrinc[crop(posy -1, 0, NUM_RIGHE -1)][crop(posx, 0, NUM_COLONNE -1)] == avversa(pedina) ) {
			if (check_upper(scacchieraPrinc, crop(posy -1, 0, NUM_RIGHE -1), crop(posx, 0, NUM_COLONNE -1), pedina, 1)) ret = 1;
		}

		if ( scacchieraPrinc[crop(posy -1, 0, NUM_RIGHE -1)][crop(posx +1, 0, NUM_COLONNE -1)] == avversa(pedina) ) {
			if ( check_upperright(scacchieraPrinc, crop(posy -1, 0, NUM_RIGHE-1), crop(posx+1, 0, NUM_COLONNE-1),  pedina, 1) ) ret = 1;
		}

		if ( scacchieraPrinc[crop(posy , 0, NUM_RIGHE -1)][crop(posx+1, 0, NUM_COLONNE -1)] == avversa(pedina) ) {
			if ( check_right(scacchieraPrinc, crop(posy , 0, NUM_RIGHE- 1), crop(posx+1, 0, NUM_COLONNE -1), pedina, 1)  ) ret = 1;
		}

		if ( scacchieraPrinc[crop(posy , 0, NUM_RIGHE -1)][crop(posx-1, 0, NUM_COLONNE -1)] == avversa(pedina) ) {
			if ( check_left(scacchieraPrinc, crop(posy , 0, NUM_RIGHE -1), crop(posx-1, 0, NUM_COLONNE -1), pedina, 1) ) ret = 1;
		}

		if (scacchieraPrinc[crop(posy +1, 0, NUM_RIGHE -1)][crop(posx+1, 0, NUM_COLONNE -1)] == avversa(pedina) ) {
			if ( check_lowerright(scacchieraPrinc, crop(posy +1, 0, NUM_RIGHE -1), crop(posx+1, 0, NUM_COLONNE -1), pedina, 1) ) ret = 1;
		}

		if (scacchieraPrinc[crop(posy +1, 0, NUM_RIGHE -1)][crop(posx, 0, NUM_COLONNE -1)] == avversa(pedina) ) {
			if ( check_lower(scacchieraPrinc, crop(posy +1, 0, NUM_RIGHE -1), crop(posx, 0, NUM_COLONNE -1), pedina, 1) ) ret = 1;
		}

		if ( scacchieraPrinc[crop(posy +1, 0, NUM_RIGHE -1)][crop(posx-1, 0, NUM_COLONNE -1)] == avversa(pedina) ) {
			if ( check_lowerleft(scacchieraPrinc, crop(posy +1, 0, NUM_RIGHE -1), crop(posx-1, 0, NUM_COLONNE -1), pedina, 1) ) ret = 1;
		}

	}

	return ret;

}

/* TODO convertire a funzioni su matrice ache i check */

int check_mossa_ed_esegui_mat(int** matrix, int posy, int posx, int pedina) {

	// Controlla che la mossa in ingresso sia valida e, nel caso, la esegue, sostituendo le pedine necessarie.

	int ret = -1;

	if (matrix[posy][posx] != VUOTO ) {
		// Mossa non valida
		return -1;
	}
	else {

		if ( matrix[crop(posy -1, 0, NUM_RIGHE -1)][crop(posx-1, 0, NUM_COLONNE -1)] == avversa(pedina) ) {
			if (check_upperleft(matrix, crop(posy -1, 0, NUM_RIGHE -1), crop(posx-1, 0, NUM_COLONNE -1), pedina, 1)) ret = 1;
		}

		if ( matrix[crop(posy -1, 0, NUM_RIGHE -1)][crop(posx, 0, NUM_COLONNE -1)] == avversa(pedina) ) {
			if (check_upper(matrix, crop(posy -1, 0, NUM_RIGHE -1), crop(posx, 0, NUM_COLONNE -1), pedina, 1)) ret = 1;
		}

		if ( matrix[crop(posy -1, 0, NUM_RIGHE -1)][crop(posx +1, 0, NUM_COLONNE -1)] == avversa(pedina) ) {
			if ( check_upperright(matrix, crop(posy -1, 0, NUM_RIGHE-1), crop(posx+1, 0, NUM_COLONNE-1),  pedina, 1) ) ret = 1;
		}

		if ( matrix[crop(posy , 0, NUM_RIGHE -1)][crop(posx+1, 0, NUM_COLONNE -1)] == avversa(pedina) ) {
			if ( check_right(matrix, crop(posy , 0, NUM_RIGHE- 1), crop(posx+1, 0, NUM_COLONNE -1), pedina, 1)  ) ret = 1;
		}

		if ( matrix[crop(posy , 0, NUM_RIGHE -1)][crop(posx-1, 0, NUM_COLONNE -1)] == avversa(pedina) ) {
			if ( check_left(matrix, crop(posy , 0, NUM_RIGHE -1), crop(posx-1, 0, NUM_COLONNE -1), pedina, 1) ) ret = 1;
		}

		if ( matrix[crop(posy +1, 0, NUM_RIGHE -1)][crop(posx+1, 0, NUM_COLONNE -1)] == avversa(pedina) ) {
			if ( check_lowerright(matrix, crop(posy +1, 0, NUM_RIGHE -1), crop(posx+1, 0, NUM_COLONNE -1), pedina, 1) ) ret = 1;
		}

		if ( matrix[crop(posy +1, 0, NUM_RIGHE -1)][crop(posx, 0, NUM_COLONNE -1)] == avversa(pedina) ) {
			if ( check_lower(matrix,  crop(posy +1, 0, NUM_RIGHE -1), crop(posx, 0, NUM_COLONNE -1), pedina, 1) ) ret = 1;
		}

		if ( matrix[crop(posy +1, 0, NUM_RIGHE -1)][crop(posx-1, 0, NUM_COLONNE -1)] == avversa(pedina) ) {
			if ( check_lowerleft(matrix, crop(posy +1, 0, NUM_RIGHE -1), crop(posx-1, 0, NUM_COLONNE -1), pedina, 1) ) ret = 1;
		}

	}

	return ret;

}



int check_mossa_ed_esegui_mattomat(int** matrixsrc, int** matrixdst, int posy, int posx, int pedina) {

	if (matrixdst == NULL ) {
		printf("%s: matrice DST passata NULLA!\n", __func__);
		exit(-1);
	}

	//printf("Valore interno: %d.\n",matrixsrc[3][3]);
	//printf("Valore py: %d.\n", posy);
	// Controlla che la mossa in ingresso sia valida e, nel caso, la esegue, sostituendo le pedine necessarie.
	int ret = -1;
	if (matrixsrc[posy][posx] != VUOTO ) {
		// Mossa non valida
		return -1;
	}
	else {

	//printf("So long so good\n");

		int i,j;
		/*
		// Copio la matrice in dst, sulla quale poi farò i miei cambiamenti
		matrixdst = (int **) malloc (sizeof(int*) * NUM_RIGHE);
		if (matrixdst == NULL) {
			printf("MALLOC fallita in %s! Esco...\n", __func__);
			exit(-1);
			//SDL_Quit();
			//return -1;
		}
		for (i=0; i< NUM_RIGHE; i++) {
			matrixdst[i] = (int *) malloc (sizeof(int) * NUM_COLONNE );
			if (matrixdst[i] == NULL ) {
				printf("MALLOC fallita in %s! Esco...\n",__func__);
				exit(-1);
				//SDL_Quit();
				//return -1;
			}
		}
		*/

		//printf("So long so good\n");
		for (i=0; i< NUM_RIGHE; i++) {
			for (j=0; j < NUM_COLONNE; j++) {
				matrixdst[i][j] = matrixsrc[i][j];
			}
		}
		//printf("Fin qui ok\n");

		// Faccio i check ed eseguo
		if ( matrixdst[crop(posy -1, 0, NUM_RIGHE -1)][crop(posx-1, 0, NUM_COLONNE -1)] == avversa(pedina) ) {
			if (check_upperleft(matrixdst, crop(posy -1, 0, NUM_RIGHE -1), crop(posx-1, 0, NUM_COLONNE -1), pedina, 2)) ret = 1;
		}

		if ( matrixdst[crop(posy -1, 0, NUM_RIGHE -1)][crop(posx, 0, NUM_COLONNE -1)] == avversa(pedina) ) {
			if (check_upper(matrixdst, crop(posy -1, 0, NUM_RIGHE -1), crop(posx, 0, NUM_COLONNE -1), pedina, 2)) ret = 1;
		}

		if ( matrixdst[crop(posy -1, 0, NUM_RIGHE -1)][crop(posx +1, 0, NUM_COLONNE -1)] == avversa(pedina) ) {
			if ( check_upperright(matrixdst, crop(posy -1, 0, NUM_RIGHE-1), crop(posx+1, 0, NUM_COLONNE-1),  pedina, 2) ) ret = 1;
		}

		if ( matrixdst[crop(posy , 0, NUM_RIGHE -1)][crop(posx+1, 0, NUM_COLONNE -1)] == avversa(pedina) ) {
			if ( check_right(matrixdst, crop(posy , 0, NUM_RIGHE- 1), crop(posx+1, 0, NUM_COLONNE -1), pedina, 2) ) ret = 1;
		}

		if ( matrixdst[crop(posy , 0, NUM_RIGHE -1)][crop(posx-1, 0, NUM_COLONNE -1)] == avversa(pedina) ) {
			if ( check_left(matrixdst, crop(posy , 0, NUM_RIGHE -1), crop(posx-1, 0, NUM_COLONNE -1), pedina, 2) ) ret = 1;
		}

		if ( matrixdst[crop(posy +1, 0, NUM_RIGHE -1)][crop(posx+1, 0, NUM_COLONNE -1)] == avversa(pedina) ) {
			if ( check_lowerright(matrixdst, crop(posy +1, 0, NUM_RIGHE -1), crop(posx+1, 0, NUM_COLONNE -1), pedina, 2) ) ret = 1;
		}

		if ( matrixdst[crop(posy +1, 0, NUM_RIGHE -1)][crop(posx, 0, NUM_COLONNE -1)] == avversa(pedina) ) {
			if ( check_lower(matrixdst, crop(posy +1, 0, NUM_RIGHE -1), crop(posx, 0, NUM_COLONNE -1), pedina, 2) ) ret = 1;
		}

		if ( matrixdst[crop(posy +1, 0, NUM_RIGHE -1)][crop(posx-1, 0, NUM_COLONNE -1)] == avversa(pedina) ) {
			if ( check_lowerleft(matrixdst, crop(posy +1, 0, NUM_RIGHE -1), crop(posx-1, 0, NUM_COLONNE -1), pedina, 2) ) ret = 1;
		}

		//printf("Fine di MATTOMAT\n");

	}

	//printf("Valore mattomat: %d.\n", matrixdst[3][3]);
	//printf("Pointer matrixdst: %d.\n", matrixdst);
	return ret;

}



int check_mossa_mat(int **matrix, int posy, int posx, int pedina ) {
	
	if (matrix[posy][posx] != VUOTO ) {
		// Mossa non valida
		return -1;
	}
	else {
		if ( 
			 (
			 ( matrix[crop(posy -1, 0, NUM_RIGHE -1)][crop(posx-1, 0, NUM_COLONNE -1)] == avversa(pedina) &&
			 	check_upperleft(matrix, crop(posy -1, 0, NUM_RIGHE), crop(posx-1, 0, NUM_COLONNE), pedina, 0) ) 
			 ||
			 ( matrix[crop(posy -1, 0, NUM_RIGHE -1)][crop(posx, 0, NUM_COLONNE -1)] == avversa(pedina) &&
			   check_upper(matrix, crop(posy -1, 0, NUM_RIGHE -1), crop(posx, 0, NUM_COLONNE-1), pedina, 0) )
			 || 
			 ( matrix[crop(posy -1, 0, NUM_RIGHE-1)][crop(posx +1, 0, NUM_COLONNE-1)] == avversa(pedina) &&
			   check_upperright(matrix, crop(posy -1, 0, NUM_RIGHE-1), crop(posx+1, 0, NUM_COLONNE-1),  pedina, 0) )
			 ||
			 ( matrix[crop(posy , 0, NUM_RIGHE-1)][crop(posx+1, 0, NUM_COLONNE-1)] == avversa(pedina) &&
			   check_right(matrix, crop(posy , 0, NUM_RIGHE-1), crop(posx+1, 0, NUM_COLONNE-1), pedina, 0) )
			 || 
			 ( matrix[crop(posy , 0, NUM_RIGHE-1)][crop(posx-1, 0, NUM_COLONNE-1)] == avversa(pedina) &&
			   check_left(matrix, crop(posy , 0, NUM_RIGHE-1), crop(posx-1, 0, NUM_COLONNE-1), pedina, 0) )
			 ||
			 ( matrix[crop(posy +1, 0, NUM_RIGHE-1)][crop(posx+1, 0, NUM_COLONNE-1)] == avversa(pedina) &&
			   check_lowerright(matrix, crop(posy +1, 0, NUM_RIGHE-1), crop(posx+1, 0, NUM_COLONNE-1), pedina, 0) )
			 ||
			 ( matrix[crop(posy +1, 0, NUM_RIGHE-1)][crop(posx, 0, NUM_COLONNE-1)] == avversa(pedina) &&
			   check_lower(matrix, crop(posy +1, 0, NUM_RIGHE-1), crop(posx, 0, NUM_COLONNE-1), pedina, 0) )
			 ||
			 ( matrix[crop(posy +1, 0, NUM_RIGHE-1)][crop(posx-1, 0, NUM_COLONNE-1)] == avversa(pedina) &&
			   check_lowerleft(matrix, crop(posy +1, 0, NUM_RIGHE-1), crop(posx-1, 0, NUM_COLONNE-1), pedina, 0) )
			 )
			 == 1
			 //|| 1
			  ) {
				  // La mossa è legale
				  return 1;

			  }
		else return 0;
	}

}

int check_mossa(int posy, int posx, int pedina ) {
	
	if (scacchieraPrinc[posy][posx] != VUOTO ) {
		// Mossa non valida
		return -1;
	}
	else {
		if ( 
			 (
			 ( scacchieraPrinc[crop(posy -1, 0, NUM_RIGHE -1)][crop(posx-1, 0, NUM_COLONNE -1)] == avversa(pedina) &&
			 	check_upperleft(scacchieraPrinc, crop(posy -1, 0, NUM_RIGHE -1), crop(posx-1, 0, NUM_COLONNE -1), pedina, 0) ) 
			 ||
			 ( scacchieraPrinc[crop(posy -1, 0, NUM_RIGHE -1)][crop(posx, 0, NUM_COLONNE -1)] == avversa(pedina) &&
			   check_upper(scacchieraPrinc, crop(posy -1, 0, NUM_RIGHE -1), crop(posx, 0, NUM_COLONNE -1), pedina, 0) )
			 || 
			 ( scacchieraPrinc[crop(posy -1, 0, NUM_RIGHE -1)][crop(posx +1, 0, NUM_COLONNE -1)] == avversa(pedina) &&
			   check_upperright(scacchieraPrinc, crop(posy -1, 0, NUM_RIGHE -1), crop(posx+1, 0, NUM_COLONNE -1),  pedina, 0) )
			 ||
			 ( scacchieraPrinc[crop(posy , 0, NUM_RIGHE -1)][crop(posx+1, 0, NUM_COLONNE -1)] == avversa(pedina) &&
			   check_right(scacchieraPrinc, crop(posy , 0, NUM_RIGHE -1), crop(posx+1, 0, NUM_COLONNE -1), pedina, 0) )
			 || 
			 ( scacchieraPrinc[crop(posy , 0, NUM_RIGHE -1)][crop(posx-1, 0, NUM_COLONNE -1)] == avversa(pedina) &&
			   check_left(scacchieraPrinc, crop(posy , 0, NUM_RIGHE -1), crop(posx-1, 0, NUM_COLONNE -1), pedina, 0) )
			 ||
			 ( scacchieraPrinc[crop(posy +1, 0, NUM_RIGHE -1)][crop(posx+1, 0, NUM_COLONNE -1)] == avversa(pedina) &&
			   check_lowerright(scacchieraPrinc, crop(posy +1, 0, NUM_RIGHE -1), crop(posx+1, 0, NUM_COLONNE -1), pedina, 0) )
			 ||
			 ( scacchieraPrinc[crop(posy +1, 0, NUM_RIGHE -1)][crop(posx, 0, NUM_COLONNE -1)] == avversa(pedina) &&
			   check_lower(scacchieraPrinc, crop(posy +1, 0, NUM_RIGHE -1), crop(posx, 0, NUM_COLONNE -1), pedina, 0) )
			 ||
			 ( scacchieraPrinc[crop(posy +1, 0, NUM_RIGHE -1)][crop(posx-1, 0, NUM_COLONNE -1)] == avversa(pedina) &&
			   check_lowerleft(scacchieraPrinc, crop(posy +1, 0, NUM_RIGHE -1), crop(posx-1, 0, NUM_COLONNE -1), pedina, 0) )
			 )
			 == 1
			 //|| 1
			  ) {
				  // La mossa è legale
				  return 1;

			  }
		else return 0;
	}

}

int check_mossa_e_conta(int posy, int posx, int pedina ) {
	
	//int temp = 0;
	int prese = 0;
	int ret = 0;

	if (scacchieraPrinc[posy][posx] != VUOTO ) {
		// Mossa non valida
		return -1;
	}
	else {
		if ( scacchieraPrinc[crop(posy -1, 0, NUM_RIGHE -1)][crop(posx-1, 0, NUM_COLONNE -1)] == avversa(pedina)) {
			prese += check_upperleft(scacchieraPrinc, crop(posy -1, 0, NUM_RIGHE -1), crop(posx-1, 0, NUM_COLONNE -1), pedina, 0);
			ret = 1;
		}
		if ( scacchieraPrinc[crop(posy -1, 0, NUM_RIGHE -1)][crop(posx, 0, NUM_COLONNE -1)] == avversa(pedina)) {
			prese += check_upper(scacchieraPrinc, crop(posy -1, 0, NUM_RIGHE -1), crop(posx, 0, NUM_COLONNE -1), pedina, 0);
			ret = 1;
		}
		if  ( scacchieraPrinc[crop(posy -1, 0, NUM_RIGHE -1)][crop(posx +1, 0, NUM_COLONNE -1)] == avversa(pedina)) {
			prese += check_upperright(scacchieraPrinc, crop(posy -1, 0, NUM_RIGHE -1), crop(posx+1, 0, NUM_COLONNE -1),  pedina, 0);
			ret = 1;
		}
		if ( scacchieraPrinc[crop(posy , 0, NUM_RIGHE -1)][crop(posx+1, 0, NUM_COLONNE -1)] == avversa(pedina)) {
			prese += check_right(scacchieraPrinc, crop(posy , 0, NUM_RIGHE -1), crop(posx+1, 0, NUM_COLONNE -1), pedina, 0);
			ret = 1;
		}
		if ( scacchieraPrinc[crop(posy , 0, NUM_RIGHE -1)][crop(posx-1, 0, NUM_COLONNE -1)] == avversa(pedina)) {
			prese += check_left(scacchieraPrinc, crop(posy , 0, NUM_RIGHE -1), crop(posx-1, 0, NUM_COLONNE -1), pedina, 0);
			ret = 1;
		}
		if ( scacchieraPrinc[crop(posy +1, 0, NUM_RIGHE -1)][crop(posx+1, 0, NUM_COLONNE -1)] == avversa(pedina)) {
			prese += check_lowerright(scacchieraPrinc, crop(posy +1, 0, NUM_RIGHE -1), crop(posx+1, 0, NUM_COLONNE -1), pedina, 0);
			ret = 1;
		}
		if ( scacchieraPrinc[crop(posy +1, 0, NUM_RIGHE -1)][crop(posx, 0, NUM_COLONNE -1)] == avversa(pedina)) {
			prese += check_lower(scacchieraPrinc, crop(posy +1, 0, NUM_RIGHE -1), crop(posx, 0, NUM_COLONNE -1), pedina, 0);
			ret = 1;
		}
		if ( scacchieraPrinc[crop(posy +1, 0, NUM_RIGHE -1)][crop(posx-1, 0, NUM_COLONNE -1)] == avversa(pedina)) {
			prese += check_lowerleft(scacchieraPrinc, crop(posy +1, 0, NUM_RIGHE -1), crop(posx-1, 0, NUM_COLONNE -1), pedina, 0);
			ret = 1;
		}

		if (ret == 1) return prese; else return 0;

	}

}

float valuta_matrice(int** matrix, int righe, int colonne, int pedina) {
	int i,j;
	float val = 0.0;
	int count = 0;

	if (pedina > 2) {
		printf("Valore di pedina scorretto! valuta_matrice esce...\n");
		exit(-1);
	}

	if (pedina == NERO) printf("Valuto pedina NERA\n");
	else if (pedina == BIANCO) printf("Valuto pedina BIANCA\n");

	for (i=0; i< righe; i++ ) {
		for (j= 0; j < colonne; j++) {
			//printf("Valore corrente %d %d: %d.\n",i,j,matrix[i][j]);
			if ( matrix[i][j] > 2) {
				printf("Valore nella matrice scorretto: %d! valuta_matrice esce...\n", matrix[i][j]);
				exit(-1);
			}
			if ( matrix[i][j] == pedina ) {
				if ( (i == 0 && j ==0) ||
						(i == 0 && j == (colonne -1) ) || 
						(i == (righe -1) && j == 0  ) ||
						(i == (righe -1) && j == (colonne -1) )
						) val = val + 10.0;
				count++;
			}
			else if ( matrix[i][j] == VUOTO) {
				//printf("Controllo\n");
				if ( check_mossa_mat(matrix, i, j, pedina) == 1) {
					//printf("Ok mossa.\n");
					val = val + 1.0;
				}
			}
		}
	}
	
	//printf("Pedine trovate: %d.\n", count);
	//printf("Valore in perc: %f.\n", ((float )count) / 100);
	val = val + ( ((float) count) / 100);
	printf("valuta_matrice ritorna %f.\n", val);
	return val;
}

/* Ritorna 1 se la partita è finita */
int controlla_fine(int** matrix, int pedina) {
	int okmossa = 0;
	//printf("Qui\n");

	if (numerobianche == 0 || numeronere == 0 || casellevuote < 1) {
		okmossa = 0;
	}
	else {
		int j = 0;
		int i = 0;
		for (i=0; i < NUM_RIGHE && okmossa == 0; i++) {
			for (j=0; j< NUM_COLONNE && okmossa == 0; j++) {
				//printf("Roba\n");
				if (check_mossa_mat(matrix, i, j, pedina) == 1) {
					//printf("Beccata\n");
					okmossa = 1;
					break;
				}
			}
		}
	}

	return (1 - okmossa);
}


nodo* minmax_ab(nodo *input_nodo, int myturno, int depth, float alpha, float beta ) {

	nodo *bestnodo = NULL;
	float bestnodo_valore;
	int i,j;

	//printf("Sono dentro %s\n",__func__);
	printf("Le mosse dentro sono %d\n", input_nodo->nmosse);
	if (input_nodo->nmosse > 0) {
		printf("Mossa precedente: Y %d e X %d.\n", input_nodo->vettoremosse[input_nodo->nmosse -1]->my, input_nodo->vettoremosse[input_nodo->nmosse -1]->mx);
	}
	//printf("Un certo valore è: %d\n", input_nodo->status[3][3]);

	if ( controlla_fine(input_nodo->status, (myturno % 2) +1 ) == 1 || depth == 0 ) {
		printf("Sono in ENDCASE\n");
		input_nodo->value = valuta_matrice(input_nodo->status, NUM_RIGHE, NUM_COLONNE, (myturno % 2) +1 );

		return input_nodo;
	}

	if (myturno == turno_globale ) {
		// Giocatore MAX
		printf("Sono in Giocatore MAX\n");
		bestnodo_valore = -640.0;

		input_nodo->figli = malloc( sizeof(nodo*) * MAX_FIGLI );

		for (i=0; i < NUM_RIGHE; i++) {
			for (j=0; j < NUM_COLONNE; j++) {
				if ( check_mossa_mat(input_nodo->status, i, j, myturno) == 1) {
					printf("Mossa valida: y %d e x %d.\n",i,j);
					// La mossa è valida.
					//Aggiungo la mossa alla lista delle mosse, se posso.
					if (input_nodo->nfigli +1 > MAX_FIGLI) {
						printf("Troppe mosse. Esco..");
						exit(-1);
					}
					else {
						int thisfiglio = ++input_nodo->nfigli;
						input_nodo->figli[thisfiglio -1] = malloc( sizeof(nodo) );
						input_nodo->figli[thisfiglio -1]->nfigli = 0;
						
						/*
						// Se il padre ha già un vettore mosse, punto lì, altrimenti alloco
						if (input_nodo->vettoremosse == NULL) input_nodo->figli[thisfiglio -1]->vettoremosse = malloc( sizeof(mossa*) * AI_MAX_DEPTH);
						else input_nodo->figli[thisfiglio -1]->vettoremosse = input_nodo->vettoremosse;
						*/

						// Aggiungo la mossa
						int thismossa = input_nodo->figli[thisfiglio -1]->nmosse = input_nodo->nmosse +1;
						input_nodo->figli[thisfiglio -1]->vettoremosse = malloc( sizeof(mossa*) * thismossa );

						// Copio le precedenti mosse
						int k;
						for (k=0; k < thismossa-1; k++) {
							input_nodo->figli[thisfiglio -1]->vettoremosse[k] = malloc(sizeof(mossa));
							input_nodo->figli[thisfiglio -1]->vettoremosse[k]->my = input_nodo->vettoremosse[k]->my;
							input_nodo->figli[thisfiglio -1]->vettoremosse[k]->mx = input_nodo->vettoremosse[k]->mx;
						}
						// Aggiungo questa mossa
						input_nodo->figli[thisfiglio -1]->vettoremosse[thismossa - 1] = malloc( sizeof(mossa) );
						input_nodo->figli[thisfiglio -1]->vettoremosse[thismossa - 1]->my = i;
						input_nodo->figli[thisfiglio -1]->vettoremosse[thismossa - 1]->mx = j;
						
						// Creo la nuova situazione

						//printf("Primo BP %d \n",i);
						// Valuto
						nodo *tempnodo;

						input_nodo->figli[thisfiglio -1]->status = (int **) malloc(sizeof(int*) * NUM_RIGHE);
						for (k=0; k< NUM_RIGHE; k++) {
							input_nodo->figli[thisfiglio -1]->status[k] = (int *) malloc (sizeof(int) * NUM_COLONNE );
						}

						if ( check_mossa_ed_esegui_mattomat(input_nodo->status, input_nodo->figli[thisfiglio -1]->status, i, j, myturno) == 1) {
							printf("Prima della ricorsione - giocatore MAX. Mossa Y: %d e X: %d.\n",i,j);
							//printf("Prima della ricorsione - valore status: %d.\n",input_nodo->figli[thisfiglio -1]->status[i][j]);
							input_nodo->figli[thisfiglio -1]->status[i][j] = myturno;
							tempnodo = minmax_ab(input_nodo->figli[thisfiglio-1], ((myturno % 2) +1), depth -1, alpha, beta);
							
							if (tempnodo->value > bestnodo_valore) {
								printf("Ho un nuovo BEST - MAX\n");
								bestnodo_valore = tempnodo->value;
								bestnodo = tempnodo;
								alpha = bestnodo_valore;
							}
							//free(tempnodo);
						}

						if (beta > alpha) {
							printf("BETA cut in MAX\n");
							return bestnodo;
						}
					}
				}
			}
		}

		return bestnodo;
	}

	else {
		// Giocatore MIN
		printf("Sono in giocatore MIN\n");
		bestnodo_valore = 640.0;

		input_nodo->figli = malloc( sizeof(nodo*) * MAX_FIGLI );
		input_nodo->nfigli = 0;

		for (i=0; i < NUM_RIGHE; i++) {
			for (j=0; j < NUM_COLONNE; j++) {
				if ( check_mossa_mat(input_nodo->status, i, j, myturno) == 1) {
					// La mossa è valida.
					//Aggiungo la mossa alla lista delle mosse, se posso.
					if (input_nodo->nfigli +1 > MAX_FIGLI) {
						printf("Troppe mosse. Esco..");
						exit(-1);
					}
					else {
						int thisfiglio = ++input_nodo->nfigli;
						input_nodo->figli[thisfiglio -1] = malloc( sizeof(nodo) );
						input_nodo->figli[thisfiglio -1]->nfigli = 0;

						/*
						if (input_nodo->vettoremosse == NULL) input_nodo->figli[thisfiglio -1]->vettoremosse = malloc( sizeof(mossa*) * AI_MAX_DEPTH);
						else input_nodo->figli[thisfiglio -1]->vettoremosse = input_nodo->vettoremosse;
						*/

						// Aggiungo la mossa
						int thismossa = input_nodo->figli[thisfiglio -1]->nmosse = input_nodo->nmosse +1;
						input_nodo->figli[thisfiglio -1]->vettoremosse = malloc( sizeof(mossa*) * thismossa );

						// Copio le precedenti mosse
						int k;
						for (k=0; k < thismossa-1; k++) {
							input_nodo->figli[thisfiglio -1]->vettoremosse[k] = malloc(sizeof(mossa));
							input_nodo->figli[thisfiglio -1]->vettoremosse[k]->my = input_nodo->vettoremosse[k]->my;
							input_nodo->figli[thisfiglio -1]->vettoremosse[k]->mx = input_nodo->vettoremosse[k]->mx;
						}
						// Aggiungo questa mossa
						input_nodo->figli[thisfiglio -1]->vettoremosse[thismossa - 1] = malloc( sizeof(mossa) );
						input_nodo->figli[thisfiglio -1]->vettoremosse[thismossa - 1]->my = i;
						input_nodo->figli[thisfiglio -1]->vettoremosse[thismossa - 1]->mx = j;

						// Creo la nuova situazione

						// Valuto

						nodo *tempnodo;
						input_nodo->figli[thisfiglio -1]->status = (int **) malloc(sizeof(int*) * NUM_RIGHE);
						for (k=0; k< NUM_RIGHE; k++) {
							input_nodo->figli[thisfiglio -1]->status[k] = (int *) malloc (sizeof(int) * NUM_COLONNE );
						}
						if ( check_mossa_ed_esegui_mattomat(input_nodo->status, input_nodo->figli[thisfiglio -1]->status, i, j, myturno) == 1) {
							input_nodo->figli[thisfiglio -1]->status[i][j] = myturno;
							printf("Prima della ricorsione - giocatore MIN. Mossa Y: %d e X: %d.\n",i,j);
							tempnodo = minmax_ab(input_nodo->figli[thisfiglio-1], ((myturno % 2) +1), depth -1, alpha, beta);
							
							if (tempnodo->value < bestnodo_valore) {
								printf("Nuovo BEST - MIN\n");
								bestnodo_valore = tempnodo->value;
								bestnodo = tempnodo;
								beta = bestnodo_valore;
 
								printf("Totale mosse finora: %d.\n",bestnodo->nmosse );
								//printf("Mossa precedente: Y %d e X %d.\n", bestnodo->vettoremosse[0].my, bestnodo->vettoremosse[0].mx);
							}
						}

						if (alpha > beta) {
							printf("ALPHA cut in MIN\n");
							return bestnodo;
						}
					}
				}
			}
		}

		return bestnodo;

	}

	/*
	//nodo *mynodo = malloc (sizeof(nodo));
	mynodo->value = 10.0;
	mynodo->nmosse = 1;
	mynodo->vettoremosse[mynodo->nmosse -1].my = 5;
	mynodo->vettoremosse[mynodo->nmosse -1].mx = 7;
	*/

	//return (struct nodo*) mynodo;

}


void ai_mossa_random(int pedina, FILE * logfile) {
	// La funzione genera una mossa LEGALE per la pedina selezionata in modo casuale.

	printf("Tentativo AI RANDOM\n");

	int py = rand() % NUM_RIGHE;
	int px = rand() % NUM_COLONNE;

	while ( check_mossa(py, px, pedina) != 1 ) {
		printf("Tentativo AI RANDOM\n");
		py = rand() % NUM_RIGHE;
		px = rand() % NUM_COLONNE;
	}

	check_mossa_ed_esegui(py, px, pedina);

	scacchieraPrinc[py][px] = pedina;
	if (pedina == NERO) numeronere++; else numerobianche++;
	if (logfile != NULL) {
		//printf("Ok sono qui\n");
		if (pedina == NERO) {
			//fprintf(logfile, "MOSSA\n");
			fprintf(logfile, "MOSSA: Nero, Riga: %d, Colonna: %d.\n\n",py+1, px+1);
		}
		else if (pedina == BIANCO) fprintf(logfile, "MOSSA: Bianco, Riga: %d, Colonna: %d.\n\n",py+1, px+1);
	}
	fflush(logfile);

}

void ai_mossa_depth(int pedina, FILE * logfile) {
	// La funzione controlla tutte le mosse legali, ed esegue quella che massimizza il numero di pedine catturate.
	int py;
	int px;
	int maxpx = 0;
	int maxpy = 0;
	int max = 0;

	for ( py= 0; py< NUM_RIGHE; py++) {
		for ( px = 0; px < NUM_COLONNE; px++) {
			int num = check_mossa_e_conta(py, px, pedina);
			printf("Mossa Y: %d X: %d fornisce %d pedine.\n", py, px, num);
			if (num > max ) {
				max = num;
				maxpx = px;
				maxpy = py;
			}
		}
	}
	
	if (max == 0) {
		ai_mossa_random(pedina, logfile);
	}
	else {
		check_mossa_ed_esegui(maxpy, maxpx, pedina);

		scacchieraPrinc[maxpy][maxpx] = pedina;
		if (pedina == NERO) numeronere++; else numerobianche++;
		if (logfile != NULL) {
			//printf("Ok sono qui\n");
			if (pedina == NERO) {
				//fprintf(logfile, "MOSSA\n");
				fprintf(logfile, "MOSSA: Nero, Riga: %d, Colonna: %d.\n\n",py+1, px+1);
			}
			else if (pedina == BIANCO) fprintf(logfile, "MOSSA: Bianco, Riga: %d, Colonna: %d.\n\n",py+1, px+1);
		}
		
		fflush(logfile);
	}


}

int read_move(int pedina, FILE * logfile) {

	int movex, movey;

	printf("Inserisci la RIGA per la mossa [Alto = 1]: ");
	scanf("\n%d", &movey);
	movey--;

	while (movey >= NUM_RIGHE || movey < 0 ) {
		printf("Inserisci la RIGA per la mossa [Alto = 1]: ");
		scanf("\n%d", &movey);
		movey--;
	}

	printf("Inserisci la COLONNA per la mossa [Sinistra = 1]: ");
	scanf("\n%d", &movex);
	movex--;

	while (movex >= NUM_COLONNE || movex < 0 ) {
		printf("Inserisci la COLONNA per la mossa [Sinistra = 1]: ");
		scanf("\n%d", &movex);
		movex--;
	}

	int ret = check_mossa_ed_esegui(movey, movex, pedina);

	if (ret == -1) {
		// Mossa non valida
		return -1;
	}
	else {
		scacchieraPrinc[movey][movex] = pedina;
		if (pedina == NERO) numeronere++; else numerobianche++;
		if (logfile != NULL) {
			printf("Ok sono qui\n");
			if (pedina == NERO) {
				//fprintf(logfile, "MOSSA\n");
				fprintf(logfile, "MOSSA: Nero, Riga: %d, Colonna: %d.\n\n",movey+1, movex+1);
			}
			else if (pedina == BIANCO) fprintf(logfile, "MOSSA: Bianco, Riga: %d, Colonna: %d.\n\n",movey+1, movex+1);
		}
		return 1;
	}

	fflush(logfile);

}

void game_init(int logflag) {

	srand(time(0));
	
	scacchiera_init();

	int startX = (NUM_RIGHE / 2) -1;
	int startY = (NUM_COLONNE / 2) -1;

	scacchieraPrinc[startY][startX] = BIANCO;
	scacchieraPrinc[startY][startX + 1] = NERO;
	scacchieraPrinc[startY + 1][startX] = NERO;
	scacchieraPrinc[startY + 1][startX + 1] = BIANCO;

	numerobianche = 2;
	numeronere = 2;

	printf("Valore iniziale: %d.\n", scacchieraPrinc[3][3]);

	//valuta_matrice(scacchieraPrinc, 8, 8, NERO);


	nodo *newnodo = malloc (sizeof(nodo));
	newnodo->status = scacchieraPrinc;
	newnodo->vettoremosse = NULL;
	newnodo->nfigli = 0;
	newnodo->nmosse = 0;

	//printf("I figli fuori sono: %d\n", newnodo->nfigli);
	printf("Parto.\n");
	turno_globale = 1;
	nodo *mynodo = minmax_ab(newnodo, 1, 2, 0.0, 0.0);

	int i;
	printf("Numero di mosse: %d.\n", mynodo->nmosse);
	for (i=0; i < mynodo->nmosse; i++ ) {
		printf("Mossa %d: Y: %d e X: %d.\n",i, mynodo->vettoremosse[i]->my, mynodo->vettoremosse[i]->mx);
	}

	exit(-1);

	/*
	int **mymatrix;
	int i;
	mymatrix = (int **) malloc (sizeof(int*) * NUM_RIGHE);
	for (i=0; i< NUM_RIGHE; i++) {
		mymatrix[i] = (int *) malloc (sizeof(int) * NUM_COLONNE );
	}
	check_mossa_ed_esegui_mattomat(scacchieraPrinc, mymatrix, 2, 3, 1);
	//printf("Valore del pointer fuori: %d.\n", mymatrix);
	mymatrix[2][3] = NERO;
	printf("Valore nella nuova matrice: %d.\n", mymatrix[3][3]);
	*/

	FILE *historyfile = NULL; 

	if (logflag == 1) historyfile = fopen("othello_history.txt","w");
	if (!historyfile && logflag) {
		printf("Impossibile aprire il file per scrivere le mosse. Esco...\n");
		exit(-1);
	}
	//if (logflag==1) fprintf(historyfile, "Elenco mosse:\n");

	int okmove = 1;
	int fine = 0;

	int okw = 0;
	okw = write_game_to_file();
	okw = read_game_from_file();

	scacchiera_print();

	//if (okw != -1) printf("Game written!\n"); else printf("Game not written!!\n");

	// Partita tra due umani.
	if (numero_umani == 2) {

		if (logflag == 1) fprintf(historyfile, "Inizia partita tra 2 GIOCATORI UMANI.\n");

		fine = controlla_fine(scacchieraPrinc, NERO);
		printf("Ok\n");

		while (fine != 1) {

			// Il giocatore 1 è sempre il nero. Maledetti nazi.
			printf("Muove il NERO.\n");
			okmove = read_move(NERO, historyfile);

			while (okmove != 1) {
				printf("Mossa NON consentita!\n");
				okmove = read_move(NERO, historyfile);
			}

			casellevuote--;

			scacchiera_print();
			printf("Numero nere: %d\n", numeronere);
			printf("Numero bianche: %d\n", numerobianche);

			fine = controlla_fine(scacchieraPrinc, BIANCO);

			if (fine == 1) break;

			turno_globale = (turno_globale % 2) +1; 
			// Ora muove il bianco.
			printf("Muove il BIANCO.\n");
			okmove = read_move(BIANCO, historyfile);

			while (okmove != 1) {
				printf("Mossa NON consentita!\n");
				okmove = read_move(BIANCO, historyfile);
			}

			casellevuote--;

			scacchiera_print();

			fine = controlla_fine(scacchieraPrinc, NERO);

			//compute_move();

			//scacchiera_print();

			turno_globale = (turno_globale % 2) +1;

		}

		printf("Partita finita!\n");
		// Controlla chi ha vinto
		if (numerobianche > numeronere ) {
			printf("Il BIANCO ha VINTO!\n");
			if (logflag == 1) fprintf(historyfile, "Il BIANCO ha VINTO!\n");
		}
		else if (numeronere > numerobianche ) {
			printf("Il NERO ha VINTO!\n");
			if (logflag == 1) fprintf(historyfile, "Il NERO ha VINTO!\n");
		}
		else {
			printf("Incredibile, PAREGGIO!\n");
			if (logflag == 1) fprintf(historyfile, "Incredibile, PAREGGIO!\n");
		}

		if (historyfile != NULL) {
			fclose(historyfile);
		}

	}
	// Caso di partita tra UMANO e giocatore IA
	else if (numero_umani == 1) {

		if (logflag == 1) fprintf(historyfile, "Inizia partita tra giocatore UMANO (NERO) e giocatore CPU (BIANCO).\n");

		fine = controlla_fine(scacchieraPrinc, NERO);

		while (fine != 1) {
			
			// Il giocatore 1 è sempre il nero E è sempre UMANO. Maledetti nazi.
			printf("Muove il NERO.\n");
			okmove = read_move(NERO, historyfile);

			while (okmove != 1) {
				printf("Mossa NON consentita!\n");
				okmove = read_move(NERO, historyfile);
			}

			casellevuote--;

			scacchiera_print();
			printf("Numero nere: %d\n", numeronere);
			printf("Numero bianche: %d\n", numerobianche);

			fine = controlla_fine(scacchieraPrinc, BIANCO);

			if (fine == 1) break;

			turno_globale = (turno_globale % 2) +1;
			// Poi, muove il computer.
			printf("Muove il BIANCO.\n");

			//ai_mossa_random(BIANCO, historyfile);
			ai_mossa_depth(BIANCO, historyfile);

			casellevuote--;

			scacchiera_print();

			printf("Numero nere: %d\n", numeronere);
			printf("Numero bianche: %d\n", numerobianche);

			fine = controlla_fine(scacchieraPrinc, NERO);

			turno_globale = (turno_globale % 2) +1;

		}

		printf("Partita finita!\n");
		// Controlla chi ha vinto
		if (numerobianche > numeronere ) {
			printf("Il BIANCO ha VINTO!\n");
			if (logflag == 1) fprintf(historyfile, "Il BIANCO ha VINTO!\n");
		}
		else if (numeronere > numerobianche ) {
			printf("Il NERO ha VINTO!\n");
			if (logflag == 1) fprintf(historyfile, "Il NERO ha VINTO!\n");
		}
		else {
			printf("Incredibile, PAREGGIO!\n");
			if (logflag == 1) fprintf(historyfile, "Incredibile, PAREGGIO!\n");
		}

		if (historyfile != NULL) {
			fclose(historyfile);
		}

	}

}

void configura() {

	printf("Qui configuro.\n");

}

void print_bio() {

	printf("La mia scheda.\n");

	printf("%d\n", crop(10, 0, NUM_COLONNE));
	printf("%d\n", crop(-1, 0, NUM_RIGHE) );
	printf("%d\n", crop(4, 0, NUM_RIGHE));
}

void print_regole() {

	printf("Ecco le regole.\n");

}

void applicazione_init() {

	char input;
	int exit = 0;

	while (exit != 1) {
		//printf("Benvenuti in LMOthello!\n");
		printf("\n");
		printf("Menu principale:\n");
		printf("C-> Configurazione: Modifica i settaggi.\n");
		printf("B-> Bio: Scheda dell'autore.\n");
		printf("R-> Regole: Visualizza le regole di gioco.\n");
		printf("P-> Partita: Inizia una partita SENZA LOG DELLE MOSSE.\n");
		printf("L-> Partita con LOG: Inizia una partita CON LOG DELLE MOSSE.\n");
		printf("Q-> Quit: Esce.\n");
		printf("Selezione: ");
		//gets(&input);
		scanf("\n%c", &input);
		printf("\n");

		switch(input) {
			case 'c':
			case 'C': configura();
				break;

			case 'b':
			case 'B': print_bio();
				break;

			case 'r':
			case 'R': print_regole();
				break;

			case 'p':
			case 'P': game_init(0);
				break;

			case 'l':
			case 'L': game_init(1);
				break;

			case 'q':
			case 'Q': printf("Esco...\n");
				exit = 1;
				break;

			default: printf("Carattere non valido!\n");
					 break;
		
		}

	}

}


int main(int argc, char** argv) {

	int i;

	if (argc > 0) {
		for (i=0; i< argc; i++) {
			printf("Argomento %d: %s",i, argv[i]);
		}
	}
	
	scacchieraPrinc = (int **) malloc (sizeof(int*) * NUM_RIGHE);
	for (i=0; i< NUM_RIGHE; i++) {
		scacchieraPrinc[i] = (int *) malloc (sizeof(int) * NUM_COLONNE );

	}

	printf("Benvenuti in LMOthello!\n");

	/*
	nodo *newnodo = NULL;
	newnodo = (nodo*) malloc (sizeof(nodo));
	newnodo->nmosse = 6;
	newnodo->status = scacchieraPrinc;
	newnodo->figli = (nodo*) malloc (sizeof(nodo));

	//newnodo = (nodo *) minmax_ab(scacchieraPrinc, 1, 2, 1.0, 2.0);
	printf("Mosse: %d\n", newnodo->nmosse);
	printf("Size padre: %d\n", sizeof(newnodo));
	printf("Size figlio: %d\n", sizeof(newnodo->figli));
	//printf("Posx: %d\n", newnodo->vettoremosse[newnodo->nmosse -1].mx);
	//printf("Valore: %d\n", newnodo->status[3][3]);
	*/

	applicazione_init();

	//scacchiera_init();

	//game_init();

	//scacchiera_print();

	return 0;

}

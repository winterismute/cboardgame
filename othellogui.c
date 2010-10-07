#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "GCGraLib.h"
#include "mytypes.h"

// Variabili globali:

// matrice usata come scacchiera principale
int **scacchieraPrinc;

// valore che rappresenta numero di righe == numero di colonne. Può venire variato dalla finestra di menu
int nrighecolonne = 8;

int casellevuote = 64;

// contatore pedine nere e bianche in gioco
int numeronere, numerobianche = 0;

// turno è un intero progressivo che indica il numero di turni di gioco. L'espressione "(turno % 2) +1" permette di avere come risultato NERO o BIANCO, definiti rispettivamente come 1 e 2 
int turno;

// numero partite di default nel caso di test_mode. Viene variato da linea di comando.
int numero_partite_test = 5;

// le 11 righe che contengono le regole del gioco
char regole1[80] = "Si muove alternativamente (inizia il nero) appoggiando una nuova pedina";
char regole2[80] = "in una casella vuota in modo da imprigionare, tra la pedina che si sta"; 
char regole3[80] = "giocando e quelle del proprio colore gia' presenti sulla scacchiera,";
char regole4[80] = "una o piu' pedine avversarie.";
char regole5[80] = "Le pedine imprigionate diventano di proprieta' di chi ha mosso.";
char regole6[80] = "E' possibile incastrare le pedine in orizzontale, in verticale e in";
char regole7[80] = "diagonale. Ad ogni mossa si possono girare pedine in piu' direzioni.";
char regole8[80] = "Sono ammesse solo le mosse con le quali si gira almeno una pedina,";
char regole9[80] = "se non e' possibile farlo si salta il turno. Quando nessuno dei";
char regole10[80]= "giocatori ha la possibilita' di muovere si contano le pedine";
char regole11[80]= "e si assegna la vittoria a chi ne ha il maggior numero.";

// superficie che conterrà l'immagine per la scheda personale
SDL_Surface *bioimage = NULL;

// righe della descrizione personale
char bio1[80] = "I primi 70 caratteri";
char bio2[80] = "Altri 70 caratteri";
char bio3[80] = "Usa l'apostrofo al posto dell'accento";


// dichiarazione di strutture:

/* mossa serve come contenitore delle coordinate X e Y di una mossa */  
typedef struct _mossa {
	int my;
	int mx;
} mossa;

/* la struttura nodo viene usata per creare l'albero delle mosse durante minmax. Ogni elemento della struttura è un nodo, che contiene:
   - status: puntatore alla matrice della situazione a cui si è.
   - vettoremosse: vettore di strutture di tipo mossa. Memorizza le mosse fatte fino ad arrivare a quel nodo.
   - nmosse: numero di mosse nel vettore.
   - value: eventuale valore valutato dalla funzione di valutazione parziale di minmax.
   - figli: lista di puntatori ai figli.
   - nfigli: numero di figli.
	*/

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

/* la struttura menuentry viene usata per facilitare la gestione delle scritte "cliccabili sullo schermo. Ogni tipo entry è una di queste scritte, che contiene:
   - La stringa entry.
   - x e y, interi che determinano la posizione dell'angolo in alto a sinistra del rettangolo che include la scritta.
   - w e h, rispettivamente ampiezza e altezza del rettangolo sopra citato.
   Di conseguenza, per verificare se la scritta è stata cliccata, basta controllare se la posx cliccata è contenuta tra entry.x e entry.x + w, e ugualmente per la posy cliccata.
   */
typedef struct
{
   char * entry;
   int x;
   int y;
   int w;
   int h;
} menuentry;

// queste due variabili contengono ampiezza e altezza della finesta di gioco. Eventualmente, modificate dalla finestra di settings
int winwidth = 800;
int winheight = 600;

// superfici che memorizzano l'immagine di sfondo e lo schermo che verrà disegnato sulla finestra corrente di gioco.
SDL_Surface *skybg;
SDL_Surface *screen;

/* Modalità di gioco:
-1 = Human
0 = CPU Random
1 = Simple AI -> Maggiore numero di prese
2 = AI depth 2
3 = Ai depth 3
...
7 = Ai depth 7
*/

int playerneromode = -1;
int playerbiancomode = -1;

int ismenu = 1;
int isgame = 0;
//int issettings = 0;

//int iachoice = 0;

// sugg_depth memorizza il livello di profondità che verrà usato dall'algoritmo minmax nel caso si richieda un suggerimento. Modificabile dalla finestra di configurazione, di default è 2.
int sugg_depth = 2;

// variabili per descriminare se si userà o meno la potatura alpha/beta (1= no, 2= sì). Sì di default.
int minmax_nero_scelta = 2;
int minmax_bianco_scelta = 2;

// la variabile test_mode è 1 se e solo se viene richiesta un partita di tipo test_mode
int test_mode = 0;
// testfile è un puntatore al file che conterrà i log delle partite in test_mode
FILE * testfile = NULL;

// lunga fila di menuentry inizializzate.
menuentry primomenu_settings = {"Settings", 0,0,0,0};
menuentry primomenu_scheda = {"Scheda autore", 0,0,0,0};
menuentry primomenu_regole = {"Regole", 0,0,0,0};
menuentry primomenu_game = {"Inizia partita", 0,0,0,0};
menuentry primomenu_esci = {"Esci", 0,0,0,0};

menuentry settings_player_nero_mode = {"Nero Umano", 0,0,0,0};

menuentry settings_player_nero_ia_minmax = {"MinMax", 0,0,0,0};
menuentry settings_player_nero_ia_minmax_ab = {"MinMax con potatura Alpha/Beta", 0,0,0,0};

menuentry settings_player_bianco_mode = {"Bianco Umano", 0,0,0,0};

menuentry settings_player_bianco_ia_minmax = {"MinMax", 0,0,0,0};
menuentry settings_player_bianco_ia_minmax_ab = {"MinMax con potatura Alpha/Beta", 0,0,0,0};

menuentry settings_test_yes = {"Attiva", 0, 0, 0, 0};
menuentry settings_test_no = {"Non attiva", 0, 0, 0, 0};

menuentry settings_lowres = {"800x600", 0, 0, 0, 0};
menuentry settings_hires = {"1024x768", 0, 0, 0, 0};

menuentry settings_nelementi = {"8 Elementi", 0, 0, 0, 0};

menuentry partita_save = {"(S)alva schema", 0,0,0,0};
menuentry partita_load = {"(L)oad schema", 0,0,0,0};
menuentry partita_info = {"(I)nfo regole", 0,0,0,0};
menuentry partita_history = {"Log (H)istory", 0,0,0,0};
menuentry partita_restart = {"(R)eset partita", 0,0,0,0};
menuentry partita_esci = {"(ESC)i", 0,0,0,0};

menuentry partita_suggerimento = {"Suggerimento[P]", 0, 0, 0, 0};


/* Funzioni proprie del gioco */

void sel_elemento(int x, char* thechar) {
	//printf("%d\n", x);
	if (x == VUOTO) *thechar = '*';
	if (x == NERO ) *thechar = 'N';
	if (x == BIANCO) *thechar = 'B';
	//printf("%c\n", *thechar);
	//else printf("ELEMNTO NON VALIDO. Esco.\n");
	//exit(0);

}

// ritorna NERO se input è BIANCO, e vicecersa
int avversa(int x) {
	return ((x == NERO) ? BIANCO : NERO);
}

// ritorna x se min < x max. x > max, ritorna max. Se x < min, ritorna min. 
int crop(int x, int min, int max ) {
	if (x <= min) return min;
	else if (x >= max) return max;
	else return x;
}

// si occupa di salvare lo schema di gioco su file.
int write_game_to_file() {
	FILE *file;
	int i, j = 0;
	char mychar='\0';
	
	file = fopen("othello_scheme.txt", "w");
	if (!file ) {
		printf("%s non è riuscita ad aprire il file in scrittura.", __func__);
		return -1;
	}
	// Dovrebbe scrivere 8 righe, ogni riga di 18 caratteri:
	// 8 x elemento + spazio + \r + \n 
	for (i=0; i< nrighecolonne; i++) {
		for (j=0; j< nrighecolonne; j++) {
			sel_elemento(scacchieraPrinc[i][j], &mychar);
			fprintf(file, "%c ", mychar);
		}
		fprintf(file, "%s","\r\n");
	}

	fclose(file);

	return 1;

}

// si occupa di leggere uno schema di gioco da un file e di importarlo.
int read_game_from_file() {

	FILE *file;
	long int filelength;
	int i = 0;
	int j = 0;
	char *contenuto;
	char * pcont;
	int provvmat[nrighecolonne][nrighecolonne];

	file = fopen("othello_scheme.txt", "r");
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
						  break;
					  }
			case '\r':
					  break;
			case '\n': {
						   if (j == nrighecolonne ) {
							   i++;
							   j = 0;
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

		if (j > nrighecolonne) {
			printf("Numero oggetti per riga: %d\n", j);
			printf("LETTURA - Numero di oggetti per riga errato!\n");
			return -1;
		}

		if (i > nrighecolonne) {
			printf("LETTURA - Numero di colonne errato!\n");
			return -1;

		}

		pcont++;

	}

	fclose(file);

	// Se arrivo qui, dovrebbe essere andato tutto bene, quindi copio
	numeronere = numerobianche = 0;

	for (i=0; i<nrighecolonne; i++) {
		for (j=0; j<nrighecolonne; j++) {
			scacchieraPrinc[i][j] = provvmat[i][j];
			if (scacchieraPrinc[i][j] == NERO) numeronere++;
			else if (scacchieraPrinc[i][j] == BIANCO) numerobianche++;
		}
		
	}

	printf("Lettura andata a buon fine.\n");

	return 1;

}

/* Funzioni di controllo */

/* inizio funzioni usate per controllare se in una data direzione la mossa è legale. Funzionano tutte allo stesso modo:
   si parte dalla posizione in input e a seconda della funzione ci si muove in una direzione finchè non trovo una casella vuota o una pedina del
   mio colore. Nel primo caso, la mossa è non valida. Nel secondo caso, la mossa è valida. Finchè trovo pedine avverse, continuo a muovermi nella
   direzione. "sostflag" discrimina se dovrò cambiare o meno le pedine (avverse) della matrice e/o aggiornare le variabili globali usate come
   contatori */

int check_upperleft(int **matrix, int posy, int posx, int pedina, int sostflag) {
	// Devo cercare andando nella direzione in alto a sinistra.
	// Se trovo un vuoto, ritorno non legale.
	// Se trovo un'avversa, va bene, continuo a cercare ED eventualmente modifico.
	// Se trovo una del colore che cerco, ok, ritorno ok.

	// Se sostflag è 0, non cambio le pedine sulla scacchiera.
	// Se sotflag è 1, cambio le pedine sulla scacchiera E aggiorno i contatori GLOBALI.
	// Se sostflag è 2, cambio le pedine sulla scacchiera MA NON aggiorno i contatori GLOBALI. 

	//int i;
	int ret = 0;
	int posyiniz = posy;
	int posxiniz = posx;
	int numeroprese = 0;
	
	while ( (posy >= 0) && (posx >= 0) ) {
		if (matrix[posy][posx] == VUOTO) {
			ret = 0;
			break;
		}
		
		if (matrix[posy][posx] == pedina ) {
			ret = 1;
			break;
		}

		posx--;
		posy--;
	}

	if (ret == 0) {
		numeroprese = 0;
	}
	else if (ret == 1) {
		while ( (posy != posyiniz ) && (posx != posxiniz) ) {
			if ( matrix[posyiniz][posxiniz] == avversa(pedina) ) {
				if (sostflag > 0) {
					if (sostflag == 1) {
						if (pedina == BIANCO) {numerobianche++; numeronere--;} else {numeronere++; numerobianche--;}
					}
					matrix[posyiniz][posxiniz] = pedina;
				}
				numeroprese++;
			}
			posxiniz--;
			posyiniz--;
		}
	}

	return numeroprese;

}

int check_upper(int** matrix, int posy, int posx, int pedina, int sostflag) {
	// Devo cercare andando nella direzione in alto.
	// Se trovo un vuoto, ritorno non legale.
	// Se trovo un'avversa, va bene, continuo a cercare.
	// Se trovo una del colore che cerco, ok, ritorno ok.

	int ret = 0;
	int posyiniz = posy;
	int posxiniz = posx;
	int numeroprese = 0;

	while ( (posy >= 0) && (posx >= 0) ) {
		if (matrix[posy][posx] == VUOTO) {
			ret = 0;
			//printf("Sono VUOTO\n");
			break;
		}
		
		if (matrix[posy][posx] == pedina ) {
			ret = 1;
			//printf("Sono PEDINA\n");
			break;
		}

		posy--;
	}

	if (ret == 0) {
		//printf("Vediamo\n");
		numeroprese = 0;
	}
	else if (ret == 1 ) {
		//printf("Ok sono qui\n");
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
	}

	//printf("%s: ritorna %d\n",__func__, numeroprese);
	//return ret;
	return numeroprese;

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
	int numeroprese = 0;

	while ( (posy >= 0) && (posx < nrighecolonne) ) {
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
		numeroprese = 0;
	}
	else if (ret == 1 ) {
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
	}

	//printf("%s: ritorna %d\n",__func__, numeroprese);
	//return ret;
	return numeroprese;

}

int check_right(int** matrix, int posy, int posx, int pedina, int sostflag) {
	// Devo cercare andando nella direzione a destra.
	// Se trovo un vuoto, ritorno non legale.
	// Se trovo un'avversa, va bene, continuo a cercare.
	// Se trovo una del colore che cerco, ok, ritorno ok.

	int ret = 0;
	int posyiniz = posy;
	int posxiniz = posx;
	int numeroprese = 0;

	while ( (posy >= 0) && (posx < nrighecolonne) ) {
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
		numeroprese = 0;
	}
	else if (ret == 1 ) {
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
	}

	//printf("%s: ritorna %d\n",__func__, ret);
	//return ret;
	return numeroprese;

}

int check_lowerright(int** matrix, int posy, int posx, int pedina, int sostflag) {
	// Devo cercare andando nella direzione in basso a destra.
	// Se trovo un vuoto, ritorno non legale.
	// Se trovo un'avversa, va bene, continuo a cercare.
	// Se trovo una del colore che cerco, ok, ritorno ok.

	int ret = 0;
	int posyiniz = posy;
	int posxiniz = posx;
	int numeroprese = 0;

	while ( (posy < nrighecolonne) && (posx < nrighecolonne) ) {
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
		numeroprese = 0;
	}
	else if (ret == 1 ) {
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
	}

	//printf("%s: ritorna %d\n",__func__, ret);
	//return ret;
	return numeroprese;

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
	int numeroprese = 0;

	while ( (posy < nrighecolonne) && (posx < nrighecolonne) ) {
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
		numeroprese = 0;
	}
	else if (ret == 1) {
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
	}

	//printf("%s: ritorna %d\n",__func__, ret);
	//return ret;
	return numeroprese;

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
	int numeroprese = 0;

	while ( (posy < nrighecolonne) && (posx >= 0) ) {
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
		numeroprese = 0;
	}
	else if (ret == 1 ) {
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
	}

	//printf("%s: ritorna %d\n",__func__, ret);
	//return ret;
	return numeroprese;
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
	int numeroprese = 0;

	while ( (posy < nrighecolonne) && (posx >= 0) ) {
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
		numeroprese = 0;
	}
	else if (ret == 1 ) {
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
	}

	//printf("%s: ritorna %d\n",__func__, ret);
	//return ret;
	return numeroprese;

}


/* check_mossa_mat prende in input una matrice "matrix" rappresentante la nostra situazione, una "posy" e una "posx", e una "pedina", e ritorna 1 se la mossa "posy","posx" è legale per il colore rappresentato da "pedina", nella nostra situazione. Altrimenti, viene ritornato -1. */

int check_mossa_mat(int **matrix, int posy, int posx, int pedina ) {
	
	if (matrix[posy][posx] != VUOTO ) {
		// Mossa non valida
		return -1;
	}
	else {
		if ( 
			 (
			 ( matrix[crop(posy -1, 0, nrighecolonne -1)][crop(posx-1, 0, nrighecolonne -1)] == avversa(pedina) &&
			 	check_upperleft(matrix, crop(posy -1, 0, nrighecolonne-1), crop(posx-1, 0, nrighecolonne-1), pedina, 0) ) 
			 ||
			 ( matrix[crop(posy -1, 0, nrighecolonne -1)][crop(posx, 0, nrighecolonne -1)] == avversa(pedina) &&
			   check_upper(matrix, crop(posy -1, 0, nrighecolonne -1), crop(posx, 0, nrighecolonne-1), pedina, 0) )
			 || 
			 ( matrix[crop(posy -1, 0, nrighecolonne-1)][crop(posx +1, 0, nrighecolonne-1)] == avversa(pedina) &&
			   check_upperright(matrix, crop(posy -1, 0, nrighecolonne-1), crop(posx+1, 0, nrighecolonne-1),  pedina, 0) )
			 ||
			 ( matrix[crop(posy , 0, nrighecolonne-1)][crop(posx+1, 0, nrighecolonne-1)] == avversa(pedina) &&
			   check_right(matrix, crop(posy , 0, nrighecolonne-1), crop(posx+1, 0, nrighecolonne-1), pedina, 0) )
			 || 
			 ( matrix[crop(posy , 0, nrighecolonne-1)][crop(posx-1, 0, nrighecolonne-1)] == avversa(pedina) &&
			   check_left(matrix, crop(posy , 0, nrighecolonne-1), crop(posx-1, 0, nrighecolonne-1), pedina, 0) )
			 ||
			 ( matrix[crop(posy +1, 0, nrighecolonne-1)][crop(posx+1, 0, nrighecolonne-1)] == avversa(pedina) &&
			   check_lowerright(matrix, crop(posy +1, 0, nrighecolonne-1), crop(posx+1, 0, nrighecolonne-1), pedina, 0) )
			 ||
			 ( matrix[crop(posy +1, 0, nrighecolonne-1)][crop(posx, 0, nrighecolonne-1)] == avversa(pedina) &&
			   check_lower(matrix, crop(posy +1, 0, nrighecolonne-1), crop(posx, 0, nrighecolonne-1), pedina, 0) )
			 ||
			 ( matrix[crop(posy +1, 0, nrighecolonne-1)][crop(posx-1, 0, nrighecolonne-1)] == avversa(pedina) &&
			   check_lowerleft(matrix, crop(posy +1, 0, nrighecolonne-1), crop(posx-1, 0, nrighecolonne-1), pedina, 0) )
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

/* check_mossa_e_conta prende in input una matrice "matrix" rappresentante la nostra situazione, una "posy" e una "posx", e una "pedina", e ritorna un valore > 0 se la mossa "posy","posx" è legale per il colore rappresentato da "pedina", nella nostra situazione. In questo caso, il valore ritornato è anche il numero di pedine catturate dalla mossa. Se la mossa non è legale, viene ritornato -1. */

int check_mossa_e_conta(int **matrix, int posy, int posx, int pedina ) {
	
	int prese = 0;
	int ret = 0;

	if (matrix[posy][posx] != VUOTO ) {
		// Mossa non valida
		return -1;
	}
	else {
		if ( matrix[crop(posy -1, 0, nrighecolonne -1)][crop(posx-1, 0, nrighecolonne -1)] == avversa(pedina)) {
			prese += check_upperleft(matrix, crop(posy -1, 0, nrighecolonne -1), crop(posx-1, 0, nrighecolonne -1), pedina, 0);
			ret = 1;
		}
		if ( matrix[crop(posy -1, 0, nrighecolonne -1)][crop(posx, 0, nrighecolonne -1)] == avversa(pedina)) {
			prese += check_upper(matrix, crop(posy -1, 0, nrighecolonne -1), crop(posx, 0, nrighecolonne -1), pedina, 0);
			ret = 1;
		}
		if  ( matrix[crop(posy -1, 0, nrighecolonne -1)][crop(posx +1, 0, nrighecolonne -1)] == avversa(pedina)) {
			prese += check_upperright(matrix, crop(posy -1, 0, nrighecolonne -1), crop(posx+1, 0, nrighecolonne -1),  pedina, 0);
			ret = 1;
		}
		if ( matrix[crop(posy , 0, nrighecolonne -1)][crop(posx+1, 0, nrighecolonne -1)] == avversa(pedina)) {
			prese += check_right(matrix, crop(posy , 0, nrighecolonne -1), crop(posx+1, 0, nrighecolonne -1), pedina, 0);
			ret = 1;
		}
		if ( matrix[crop(posy , 0, nrighecolonne -1)][crop(posx-1, 0, nrighecolonne -1)] == avversa(pedina)) {
			prese += check_left(matrix, crop(posy , 0, nrighecolonne -1), crop(posx-1, 0, nrighecolonne -1), pedina, 0);
			ret = 1;
		}
		if ( matrix[crop(posy +1, 0, nrighecolonne -1)][crop(posx+1, 0, nrighecolonne -1)] == avversa(pedina)) {
			prese += check_lowerright(matrix, crop(posy +1, 0, nrighecolonne -1), crop(posx+1, 0, nrighecolonne -1), pedina, 0);
			ret = 1;
		}
		if ( matrix[crop(posy +1, 0, nrighecolonne -1)][crop(posx, 0, nrighecolonne -1)] == avversa(pedina)) {
			prese += check_lower(matrix, crop(posy +1, 0, nrighecolonne -1), crop(posx, 0, nrighecolonne -1), pedina, 0);
			ret = 1;
		}
		if ( matrix[crop(posy +1, 0, nrighecolonne -1)][crop(posx-1, 0, nrighecolonne -1)] == avversa(pedina)) {
			prese += check_lowerleft(matrix, crop(posy +1, 0, nrighecolonne -1), crop(posx-1, 0, nrighecolonne -1), pedina, 0);
			ret = 1;
		}

		if (ret == 1) return prese; else return 0;

	}
}

/* check_mossa_ed_esegui_mat è in gran parte simile a check_mossa_e_conta, con la differenza che check_mossa_ed_esegui_mat
   effettua anche le opportune modifiche alla matrice passata in ingresso. */

int check_mossa_ed_esegui_mat(int** matrix, int posy, int posx, int pedina) {

	// Controlla che la mossa in ingresso sia valida e, nel caso, la esegue, sostituendo le pedine necessarie.

	int ret = -1;

	if (matrix[posy][posx] != VUOTO ) {
		// Mossa non valida
		return -1;
	}
	else {

		if ( matrix[crop(posy -1, 0, nrighecolonne -1)][crop(posx-1, 0, nrighecolonne -1)] == avversa(pedina) ) {
			if (check_upperleft(matrix, crop(posy -1, 0, nrighecolonne -1), crop(posx-1, 0, nrighecolonne -1), pedina, 1)) ret = 1;
		}

		if ( matrix[crop(posy -1, 0, nrighecolonne -1)][crop(posx, 0, nrighecolonne -1)] == avversa(pedina) ) {
			if (check_upper(matrix, crop(posy -1, 0, nrighecolonne -1), crop(posx, 0, nrighecolonne -1), pedina, 1)) ret = 1;
		}

		if ( matrix[crop(posy -1, 0, nrighecolonne -1)][crop(posx +1, 0, nrighecolonne -1)] == avversa(pedina) ) {
			if ( check_upperright(matrix, crop(posy -1, 0, nrighecolonne-1), crop(posx+1, 0, nrighecolonne-1),  pedina, 1) ) ret = 1;
		}

		if ( matrix[crop(posy , 0, nrighecolonne -1)][crop(posx+1, 0, nrighecolonne -1)] == avversa(pedina) ) {
			if ( check_right(matrix, crop(posy , 0, nrighecolonne- 1), crop(posx+1, 0, nrighecolonne -1), pedina, 1)  ) ret = 1;
		}

		if ( matrix[crop(posy , 0, nrighecolonne -1)][crop(posx-1, 0, nrighecolonne -1)] == avversa(pedina) ) {
			if ( check_left(matrix, crop(posy , 0, nrighecolonne -1), crop(posx-1, 0, nrighecolonne -1), pedina, 1) ) ret = 1;
		}

		if ( matrix[crop(posy +1, 0, nrighecolonne -1)][crop(posx+1, 0, nrighecolonne -1)] == avversa(pedina) ) {
			if ( check_lowerright(matrix, crop(posy +1, 0, nrighecolonne -1), crop(posx+1, 0, nrighecolonne -1), pedina, 1) ) ret = 1;
		}

		if ( matrix[crop(posy +1, 0, nrighecolonne -1)][crop(posx, 0, nrighecolonne -1)] == avversa(pedina) ) {
			if ( check_lower(matrix,  crop(posy +1, 0, nrighecolonne -1), crop(posx, 0, nrighecolonne -1), pedina, 1) ) ret = 1;
		}

		if ( matrix[crop(posy +1, 0, nrighecolonne -1)][crop(posx-1, 0, nrighecolonne -1)] == avversa(pedina) ) {
			if ( check_lowerleft(matrix, crop(posy +1, 0, nrighecolonne -1), crop(posx-1, 0, nrighecolonne -1), pedina, 1) ) ret = 1;
		}

	}

	return ret;

}

/* check_mossa_ed_esequi_mattomat è come check_mossa_ed_esegui_mat, con la differenza che i controlli vengono prima fatti sulla matrice
   "matrixscr", mentre gli effetti della mossa vengono scritti nella matrice "matrixsdst", di cui viene preso in ingresso un puntatore */

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

		for (i=0; i< nrighecolonne; i++) {
			for (j=0; j < nrighecolonne; j++) {
				matrixdst[i][j] = matrixsrc[i][j];
			}
		}

		// Faccio i check ed eseguo
		if ( matrixdst[crop(posy -1, 0, nrighecolonne -1)][crop(posx-1, 0, nrighecolonne -1)] == avversa(pedina) ) {
			if (check_upperleft(matrixdst, crop(posy -1, 0, nrighecolonne -1), crop(posx-1, 0, nrighecolonne -1), pedina, 2)) ret = 1;
		}

		if ( matrixdst[crop(posy -1, 0, nrighecolonne -1)][crop(posx, 0, nrighecolonne -1)] == avversa(pedina) ) {
			if (check_upper(matrixdst, crop(posy -1, 0, nrighecolonne -1), crop(posx, 0, nrighecolonne -1), pedina, 2)) ret = 1;
		}

		if ( matrixdst[crop(posy -1, 0, nrighecolonne -1)][crop(posx +1, 0, nrighecolonne -1)] == avversa(pedina) ) {
			if ( check_upperright(matrixdst, crop(posy -1, 0, nrighecolonne-1), crop(posx+1, 0, nrighecolonne-1),  pedina, 2) ) ret = 1;
		}

		if ( matrixdst[crop(posy , 0, nrighecolonne -1)][crop(posx+1, 0, nrighecolonne -1)] == avversa(pedina) ) {
			if ( check_right(matrixdst, crop(posy , 0, nrighecolonne- 1), crop(posx+1, 0, nrighecolonne -1), pedina, 2) ) ret = 1;
		}

		if ( matrixdst[crop(posy , 0, nrighecolonne -1)][crop(posx-1, 0, nrighecolonne -1)] == avversa(pedina) ) {
			if ( check_left(matrixdst, crop(posy , 0, nrighecolonne -1), crop(posx-1, 0, nrighecolonne -1), pedina, 2) ) ret = 1;
		}

		if ( matrixdst[crop(posy +1, 0, nrighecolonne -1)][crop(posx+1, 0, nrighecolonne -1)] == avversa(pedina) ) {
			if ( check_lowerright(matrixdst, crop(posy +1, 0, nrighecolonne -1), crop(posx+1, 0, nrighecolonne -1), pedina, 2) ) ret = 1;
		}

		if ( matrixdst[crop(posy +1, 0, nrighecolonne -1)][crop(posx, 0, nrighecolonne -1)] == avversa(pedina) ) {
			if ( check_lower(matrixdst, crop(posy +1, 0, nrighecolonne -1), crop(posx, 0, nrighecolonne -1), pedina, 2) ) ret = 1;
		}

		if ( matrixdst[crop(posy +1, 0, nrighecolonne -1)][crop(posx-1, 0, nrighecolonne -1)] == avversa(pedina) ) {
			if ( check_lowerleft(matrixdst, crop(posy +1, 0, nrighecolonne -1), crop(posx-1, 0, nrighecolonne -1), pedina, 2) ) ret = 1;
		}

		//printf("Fine di MATTOMAT\n");

	}

	//printf("Valore mattomat: %d.\n", matrixdst[3][3]);
	//printf("Pointer matrixdst: %d.\n", matrixdst);
	return ret;

}


/* ritorna 1 se la partita è finita */
int controlla_fine(int** matrix, int pedina) {
	int okmossa = 0;

	if (numerobianche == 0 || numeronere == 0 || casellevuote < 1) {
		okmossa = 0;
	}
	else {
		int j = 0;
		int i = 0;
		for (i=0; i < nrighecolonne && okmossa == 0; i++) {
			for (j=0; j< nrighecolonne && okmossa == 0; j++) {
				if (check_mossa_mat(matrix, i, j, pedina) == 1) {
					// se c'è almeno una mossa, la partita non è finita. Break per uscire dal ciclo e poi return.
					okmossa = 1;
					break;
				}
			}
		}
	}

	return (1 - okmossa);
}

/* funzione di valutazione parziale usata da minmax */
float valuta_matrice(int** matrix, int righe, int colonne, int pedina) {
	int i,j;
	float val = 0.0;
	int count = 0;

	if (pedina > 2) {
		printf("Valore di pedina scorretto! valuta_matrice esce...\n");
		exit(-1);
	}

	for (i=0; i< righe; i++ ) {
		for (j= 0; j < colonne; j++) {
			if ( matrix[i][j] > 2) {
				printf("Valore nella matrice scorretto: %d! valuta_matrice esce...\n", matrix[i][j]);
				exit(-1);
			}
			if ( matrix[i][j] == pedina ) {
				// Se stiamo analizzando una casella d'angolo
				if ( (i == 0 && j ==0) ||
						(i == 0 && j == (colonne -1) ) || 
						(i == (righe -1) && j == 0  ) ||
						(i == (righe -1) && j == (colonne -1) )
						) val = val + 10.0;
				count++;
			}
			else if ( matrix[i][j] == VUOTO) {
				// Controllo se il VUOTO è una possibile mossa
				if ( check_mossa_mat(matrix, i, j, pedina) == 1) {
					val = val + 1.0;
				}
			}
		}
	}
	
	//printf("Pedine trovate: %d.\n", count);
	//printf("Valore in perc: %f.\n", ((float )count) / 100);
	val = val + ( ((float) count) / 100);
	//printf("valuta_matrice ritorna %f.\n", val);
	return val;
}

/* Funzioni di IA */


// la funzione genera una mossa LEGALE per la pedina selezionata in modo casuale, e la ESEGUE.
void ai_mossa_random(int **matrix, int pedina, FILE * logfile) {

	int py = rand() % nrighecolonne;
	int px = rand() % nrighecolonne;

	while ( check_mossa_mat(matrix, py, px, pedina) != 1 ) {
		py = rand() % nrighecolonne;
		px = rand() % nrighecolonne;
	}

	check_mossa_ed_esegui_mat(matrix, py, px, pedina);

	scacchieraPrinc[py][px] = pedina;
	if (pedina == NERO) numeronere++; else numerobianche++;
	if (logfile != NULL) {
		if (pedina == NERO) {
			fprintf(logfile, "MOSSA: Nero, Riga: %d, Colonna: %d.\n\n",py+1, px+1);
		}
		else if (pedina == BIANCO) fprintf(logfile, "MOSSA: Bianco, Riga: %d, Colonna: %d.\n\n",py+1, px+1);
	}
	fflush(logfile);

}


// la funzione controlla tutte le mosse legali, ed ESEGUE quella che massimizza il numero di pedine catturate.
void ai_mossa_presa(int **matrix, int pedina, FILE * logfile) {

	int py;
	int px;
	int maxpx = 0;
	int maxpy = 0;
	int max = 0;

	for ( py= 0; py< nrighecolonne; py++) {
		for ( px = 0; px < nrighecolonne; px++) {
			int num = check_mossa_e_conta(matrix, py, px, pedina);
			if (num > max ) {
				max = num;
				maxpx = px;
				maxpy = py;
			}
		}
	}
	
	if (max == 0) {
		ai_mossa_random(matrix, pedina, logfile);
	}
	else {
		check_mossa_ed_esegui_mat(matrix, maxpy, maxpx, pedina);

		scacchieraPrinc[maxpy][maxpx] = pedina;
		if (pedina == NERO) numeronere++; else numerobianche++;
		if (logfile != NULL) {
			if (pedina == NERO) {
				fprintf(logfile, "MOSSA: Nero, Riga: %d, Colonna: %d.\n\n",py+1, px+1);
			}
			else if (pedina == BIANCO) fprintf(logfile, "MOSSA: Bianco, Riga: %d, Colonna: %d.\n\n",py+1, px+1);
		}
		
		fflush(logfile);
	}
}

/* funzione per l'algoritmo minmax */
nodo* minmax(nodo *input_nodo, int myturno, int depth ) {

	nodo *bestnodo = NULL;
	float bestnodo_valore;
	int i,j;

	// Caso base, di terminazione eventuale
	// effettuo per  (myturno % 2) +1, che è l'avverso della pedina in input, ma è quindi l'ultimo che ha giocato
	if ( controlla_fine(input_nodo->status, myturno) == 1 || depth == 0 ) {
		//printf("Sono in ENDCASE\n");
		input_nodo->value = valuta_matrice(input_nodo->status, nrighecolonne, nrighecolonne, (myturno % 2) +1);

		return input_nodo;
	}

	// TURNO è variabile GLOBALE rappresentante il turno di gioco
	if (myturno == ((turno % 2) +1) ) {
		// Giocatore MAX
		//printf("Sono in Giocatore MAX\n");
		bestnodo_valore = -640.0;

		input_nodo->figli = malloc( sizeof(nodo*) * MAX_FIGLI );

		for (i=0; i < nrighecolonne; i++) {
			for (j=0; j < nrighecolonne; j++) {
				if ( check_mossa_mat(input_nodo->status, i, j, myturno) == 1) {
					//printf("Mossa valida: y %d e x %d.\n",i,j);
					// La mossa è valida.
					//Aggiungo la mossa alla lista delle mosse, se posso.
					if (input_nodo->nfigli +1 > MAX_FIGLI) {
						printf("Troppe mosse. Esco...\n");
						exit(-1);
					}
					else {
						// la mossa è possibile ED è possibile aggiungerla. Aggiungo un figlio al mio nodo.
						input_nodo->nfigli++;
						int thisfiglio = input_nodo->nfigli;
						input_nodo->figli[thisfiglio -1] = malloc( sizeof(nodo) );
						// i figli del nuovo figlio ovviamente sono 0
						input_nodo->figli[thisfiglio -1]->nfigli = 0;

						// alloco il vettore delle mosse del nuovo figlio
						int thismossa = input_nodo->figli[thisfiglio -1]->nmosse = input_nodo->nmosse +1;
						input_nodo->figli[thisfiglio -1]->vettoremosse = malloc( sizeof(mossa*) * thismossa );

						// copio prima le precedenti mosse del padre nel nuovo figlio
						int k;
						for (k=0; k < thismossa-1; k++) {
							input_nodo->figli[thisfiglio -1]->vettoremosse[k] = malloc(sizeof(mossa));
							input_nodo->figli[thisfiglio -1]->vettoremosse[k]->my = input_nodo->vettoremosse[k]->my;
							input_nodo->figli[thisfiglio -1]->vettoremosse[k]->mx = input_nodo->vettoremosse[k]->mx;
						}
						// poi aggiungo questa ultima mossa
						input_nodo->figli[thisfiglio -1]->vettoremosse[thismossa - 1] = malloc( sizeof(mossa) );
						input_nodo->figli[thisfiglio -1]->vettoremosse[thismossa - 1]->my = i;
						input_nodo->figli[thisfiglio -1]->vettoremosse[thismossa - 1]->mx = j;
						
						// ora creo la nuova situazione
						nodo *tempnodo;

						// alloco status del figlio, che conterrà la nuova situazione
						input_nodo->figli[thisfiglio -1]->status = (int **) malloc(sizeof(int*) * nrighecolonne);
						for (k=0; k< nrighecolonne; k++) {
							input_nodo->figli[thisfiglio -1]->status[k] = (int *) malloc (sizeof(int) * nrighecolonne );
						}

						// avevo già controllato che la mossa fosse possibile. Ora, la effettuo, impostando lo status del figlio come situazione modificata
						if ( check_mossa_ed_esegui_mattomat(input_nodo->status, input_nodo->figli[thisfiglio -1]->status, i, j, myturno) == 1) {
							//printf("Prima della ricorsione - giocatore MAX. Mossa Y: %d e X: %d.\n",i,j);
							//printf("Prima della ricorsione - valore status: %d.\n",input_nodo->figli[thisfiglio -1]->status[i][j]);
							
							// check_mossa_ed_esegui non aggiunge la pedina nella posizione i,j. Lo faccio manualmente
							input_nodo->figli[thisfiglio -1]->status[i][j] = myturno;
							tempnodo = minmax(input_nodo->figli[thisfiglio-1], ((myturno % 2) +1), depth -1);
							
							// controllo se ho un nuovo valore migliore
							if (tempnodo->value > bestnodo_valore) {
								//printf("Ho un nuovo BEST - MAX\n");
								bestnodo_valore = tempnodo->value;
								bestnodo = tempnodo;
							}
							//free(tempnodo);
						}

					}
				}
			}
		}

		return bestnodo;
	}

	else {
		// Giocatore MIN
		//printf("Sono in giocatore MIN\n");
		bestnodo_valore = 640.0;

		input_nodo->figli = malloc( sizeof(nodo*) * MAX_FIGLI );
		input_nodo->nfigli = 0;

		for (i=0; i < nrighecolonne; i++) {
			for (j=0; j < nrighecolonne; j++) {
				if ( check_mossa_mat(input_nodo->status, i, j, myturno) == 1) {
					// La mossa è valida.
					//Aggiungo la mossa alla lista delle mosse, se posso.
					if (input_nodo->nfigli +1 > MAX_FIGLI) {
						printf("Troppe mosse. Esco...\n");
						exit(-1);
					}
					else {
						// la mossa è possibile ED è possibile aggiungerla. Aggiungo un figlio al mio nodo.
						input_nodo->nfigli++;
						int thisfiglio = input_nodo->nfigli;
						input_nodo->figli[thisfiglio -1] = malloc( sizeof(nodo) );
						// i figli del nuovo figlio ovviamente sono 0
						input_nodo->figli[thisfiglio -1]->nfigli = 0;

						// alloco spazio per il vettore delle mosse nel figlio, che saranno una in più rispetto al padre
						int thismossa = input_nodo->figli[thisfiglio -1]->nmosse = input_nodo->nmosse +1;
						input_nodo->figli[thisfiglio -1]->vettoremosse = malloc( sizeof(mossa*) * thismossa );

						// prima copio le precedenti mosse
						int k;
						for (k=0; k < thismossa-1; k++) {
							input_nodo->figli[thisfiglio -1]->vettoremosse[k] = malloc(sizeof(mossa));
							input_nodo->figli[thisfiglio -1]->vettoremosse[k]->my = input_nodo->vettoremosse[k]->my;
							input_nodo->figli[thisfiglio -1]->vettoremosse[k]->mx = input_nodo->vettoremosse[k]->mx;
						}
						// poi aggiungo questa ultima mossa
						input_nodo->figli[thisfiglio -1]->vettoremosse[thismossa - 1] = malloc( sizeof(mossa) );
						input_nodo->figli[thisfiglio -1]->vettoremosse[thismossa - 1]->my = i;
						input_nodo->figli[thisfiglio -1]->vettoremosse[thismossa - 1]->mx = j;

						// ora creo la nuova situazione
						// alloco lo spazio per la matrice status nel figlio
						nodo *tempnodo;
						input_nodo->figli[thisfiglio -1]->status = (int **) malloc(sizeof(int*) * nrighecolonne);
						for (k=0; k< nrighecolonne; k++) {
							input_nodo->figli[thisfiglio -1]->status[k] = (int *) malloc (sizeof(int) * nrighecolonne );
						}
						if ( check_mossa_ed_esegui_mattomat(input_nodo->status, input_nodo->figli[thisfiglio -1]->status, i, j, myturno) == 1) {
							// ho già verificato che la mossa era possibile. Ora, la eseguo e metto il risultato nella matrice status del figlio 
							input_nodo->figli[thisfiglio -1]->status[i][j] = myturno;
							//printf("Prima della ricorsione - giocatore MIN. Mossa Y: %d e X: %d.\n",i,j);
							tempnodo = minmax(input_nodo->figli[thisfiglio-1], ((myturno % 2) +1), depth -1);
							
							if (tempnodo->value < bestnodo_valore) {
								// se ho un nuovo valore migliore, aggiorno
								//printf("Nuovo BEST - MIN\n");
								bestnodo_valore = tempnodo->value;
								bestnodo = tempnodo;
 
								//printf("Totale mosse finora: %d.\n",bestnodo->nmosse );
								//printf("Mossa precedente: Y %d e X %d.\n", bestnodo->vettoremosse[0].my, bestnodo->vettoremosse[0].mx);
							}
						}

					}
				}
			}
		}

		return bestnodo;

	}
}



/* Funzione per l'algoritmo minmax con potatura alpha/beta */
nodo* minmax_ab(nodo *input_nodo, int myturno, int depth, float alpha, float beta ) {

	nodo *bestnodo = NULL;
	float bestnodo_valore;
	int i,j;

	//printf("Sono dentro %s\n",__func__);
	//printf("Le mosse dentro sono %d\n", input_nodo->nmosse);

	//printf("Un certo valore è: %d\n", input_nodo->status[3][3]);

	if ( controlla_fine(input_nodo->status, myturno) == 1 || depth == 0 ) {
		//printf("Sono in ENDCASE\n");
		input_nodo->value = valuta_matrice(input_nodo->status, nrighecolonne, nrighecolonne, (myturno % 2) +1);

		return input_nodo;
	}

	if (myturno == ((turno % 2) +1) ) {
		// Giocatore MAX
		//printf("Sono in Giocatore MAX\n");
		bestnodo_valore = -640.0;

		input_nodo->figli = malloc( sizeof(nodo*) * MAX_FIGLI );

		for (i=0; i < nrighecolonne; i++) {
			for (j=0; j < nrighecolonne; j++) {
				if ( check_mossa_mat(input_nodo->status, i, j, myturno) == 1) {
					//printf("Mossa valida: y %d e x %d.\n",i,j);
					// La mossa è valida.
					//Aggiungo la mossa alla lista delle mosse, se posso.
					if (input_nodo->nfigli +1 > MAX_FIGLI) {
						printf("Troppe mosse. Esco...\n");
						exit(-1);
					}
					else {
						// la mossa è legale e ho spazio per un nuovo figlio. Ne alloco la memoria
						int thisfiglio = ++input_nodo->nfigli;
						input_nodo->figli[thisfiglio -1] = malloc( sizeof(nodo) );
						input_nodo->figli[thisfiglio -1]->nfigli = 0;

						// aggiungo la mossa
						int thismossa = input_nodo->figli[thisfiglio -1]->nmosse = input_nodo->nmosse +1;
						input_nodo->figli[thisfiglio -1]->vettoremosse = malloc( sizeof(mossa*) * thismossa );

						// copio le precedenti mosse
						int k;
						for (k=0; k < thismossa-1; k++) {
							input_nodo->figli[thisfiglio -1]->vettoremosse[k] = malloc(sizeof(mossa));
							input_nodo->figli[thisfiglio -1]->vettoremosse[k]->my = input_nodo->vettoremosse[k]->my;
							input_nodo->figli[thisfiglio -1]->vettoremosse[k]->mx = input_nodo->vettoremosse[k]->mx;
						}
						// aggiungo questa ultima mossa
						input_nodo->figli[thisfiglio -1]->vettoremosse[thismossa - 1] = malloc( sizeof(mossa) );
						input_nodo->figli[thisfiglio -1]->vettoremosse[thismossa - 1]->my = i;
						input_nodo->figli[thisfiglio -1]->vettoremosse[thismossa - 1]->mx = j;
						
						// creo la nuova situazione
						nodo *tempnodo;

						input_nodo->figli[thisfiglio -1]->status = (int **) malloc(sizeof(int*) * nrighecolonne);
						for (k=0; k< nrighecolonne; k++) {
							input_nodo->figli[thisfiglio -1]->status[k] = (int *) malloc (sizeof(int) * nrighecolonne );
						}

						if ( check_mossa_ed_esegui_mattomat(input_nodo->status, input_nodo->figli[thisfiglio -1]->status, i, j, myturno) == 1) {
							//printf("Prima della ricorsione - giocatore MAX. Mossa Y: %d e X: %d.\n",i,j);
							//printf("Prima della ricorsione - valore status: %d.\n",input_nodo->figli[thisfiglio -1]->status[i][j]);
							input_nodo->figli[thisfiglio -1]->status[i][j] = myturno;
							tempnodo = minmax_ab(input_nodo->figli[thisfiglio-1], ((myturno % 2) +1), depth -1, alpha, beta);
							
							if (tempnodo->value > bestnodo_valore) {
								//printf("Ho un nuovo BEST - MAX\n");
								bestnodo_valore = tempnodo->value;
								bestnodo = tempnodo;
								alpha = bestnodo_valore;
							}
							//free(tempnodo);
						}

						if (beta > alpha) {
							// effettuo la potatura - beta
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
		//printf("Sono in giocatore MIN\n");
		bestnodo_valore = 640.0;

		input_nodo->figli = malloc( sizeof(nodo*) * MAX_FIGLI );
		input_nodo->nfigli = 0;

		for (i=0; i < nrighecolonne; i++) {
			for (j=0; j < nrighecolonne; j++) {
				if ( check_mossa_mat(input_nodo->status, i, j, myturno) == 1) {
					// La mossa è valida.
					//Aggiungo la mossa alla lista delle mosse, se posso.
					if (input_nodo->nfigli +1 > MAX_FIGLI) {
						printf("Troppe mosse. Esco...\n");
						exit(-1);
					}
					else {
						// la mossa è legale e ho spazio per un nuovo figlio. Alloco nuovo figlio
						int thisfiglio = ++input_nodo->nfigli;
						input_nodo->figli[thisfiglio -1] = malloc( sizeof(nodo) );
						input_nodo->figli[thisfiglio -1]->nfigli = 0;

						// alloco spazio per il vettore delle mosse nel figlio
						int thismossa = input_nodo->figli[thisfiglio -1]->nmosse = input_nodo->nmosse +1;
						input_nodo->figli[thisfiglio -1]->vettoremosse = malloc( sizeof(mossa*) * thismossa );

						// copio le precedenti mosse
						int k;
						for (k=0; k < thismossa-1; k++) {
							input_nodo->figli[thisfiglio -1]->vettoremosse[k] = malloc(sizeof(mossa));
							input_nodo->figli[thisfiglio -1]->vettoremosse[k]->my = input_nodo->vettoremosse[k]->my;
							input_nodo->figli[thisfiglio -1]->vettoremosse[k]->mx = input_nodo->vettoremosse[k]->mx;
						}
						// aggiungo questa mossa
						input_nodo->figli[thisfiglio -1]->vettoremosse[thismossa - 1] = malloc( sizeof(mossa) );
						input_nodo->figli[thisfiglio -1]->vettoremosse[thismossa - 1]->my = i;
						input_nodo->figli[thisfiglio -1]->vettoremosse[thismossa - 1]->mx = j;

						// creo la nuova situazione

						nodo *tempnodo;
						input_nodo->figli[thisfiglio -1]->status = (int **) malloc(sizeof(int*) * nrighecolonne);
						for (k=0; k< nrighecolonne; k++) {
							input_nodo->figli[thisfiglio -1]->status[k] = (int *) malloc (sizeof(int) * nrighecolonne );
						}
						if ( check_mossa_ed_esegui_mattomat(input_nodo->status, input_nodo->figli[thisfiglio -1]->status, i, j, myturno) == 1) {
							input_nodo->figli[thisfiglio -1]->status[i][j] = myturno;
							//printf("Prima della ricorsione - giocatore MIN. Mossa Y: %d e X: %d.\n",i,j);
							tempnodo = minmax_ab(input_nodo->figli[thisfiglio-1], ((myturno % 2) +1), depth -1, alpha, beta);
							
							if (tempnodo->value < bestnodo_valore) {
								//printf("Nuovo BEST - MIN\n");
								bestnodo_valore = tempnodo->value;
								bestnodo = tempnodo;
								beta = bestnodo_valore;
 
								//printf("Totale mosse finora: %d.\n",bestnodo->nmosse );
								//printf("Mossa precedente: Y %d e X %d.\n", bestnodo->vettoremosse[0].my, bestnodo->vettoremosse[0].mx);
							}
						}

						if (alpha > beta) {
							// effettuo la potatura - alpha
							printf("ALPHA cut in MIN\n");
							return bestnodo;
						}
					}
				}
			}
		}

		return bestnodo;

	}
}




/* Funzioni grafiche, di disegno */
int draw_menu_iniziale(TTF_Font *font ) {

	SDL_Surface * iniz_screen;
	SDL_Event event;
	int result;
	// startpartita sarà 1 se alla fine delle configurazioni, verrà chiesto di iniziare una partita vera e propria
	int startpartita = 0;
	// se refreshleft è 1, devo ridisegnare la parte sinistra della finestra, la parte non menu
	int refreshleft = 0;

	/*  0 : 800x600
		1: 1024x768 */
	int scelta_ris = 0; 

	// diventerà poi "nrighecolonne", variabile globale
	int nelem = 8;

	// iniz_screen è la superficie che rappresenterà la mia finestra. La inizializzo
	iniz_screen = SDL_SetVideoMode(800, 400, 32, SDL_HWSURFACE | SDL_DOUBLEBUF );
	if(iniz_screen==NULL)
	{
		fprintf(stderr,"Can't set video mode: %s\n",SDL_GetError());
		SDL_Quit();
		//return ;
	}

	// skybg sarà l'immagine di sfondo.
	skybg = GC_LoadImage("skylow.png", &result);
	if ( result == 0) {
		fprintf(stderr, "Impossibile caricare l'immagine skyhi.png!\n");
		SDL_Quit();
		//return(0);
	}

	// bioimage è l'immagine per la bio
	bioimage = GC_LoadImage("myimage.jpg", &result);
	if ( result == 0) {
		fprintf(stderr, "Impossibile caricare l'immagine myimage.png!\n");
		SDL_Quit();
		//return(0);
	}

    SDL_WM_SetCaption("LMOthello - Menu Iniziale",NULL);

	// Disegno lo sfondo. Taglio solo la parte di immagine (skybg) che mi serve
	GC_DrawImage(skybg, 0, 0, 800, 400, iniz_screen, 0, 0);

	int px, py, w, h;
	// px e py sono inizialmente dove inizio a disegnare il mio menu
	px = 600;
	py = 50;

	// lunga parte per disegnare il menu a destra utilizzando le stutture di tipo menuentry.
	// il menu rimane fisso, lo disegno una volta per tutte.
	GC_DrawText(iniz_screen, font, 0, 0, 0, 0, 100, 100, 100, 0, primomenu_settings.entry, px, py, &w, &h, shaded);
	primomenu_settings.x = px;
	primomenu_settings.y = py;
	primomenu_settings.w = w;
	primomenu_settings.h = h;
	py = py + h + 10;

	GC_DrawText(iniz_screen, font, 0, 0, 0, 0, 100, 100, 100, 255, primomenu_scheda.entry, px, py, &w, &h, shaded);
	primomenu_scheda.x = px;
	primomenu_scheda.y = py;
	primomenu_scheda.w = w;
	primomenu_scheda.h = h;
	py = py + h + 10;

	GC_DrawText(iniz_screen, font, 0, 0, 0, 0, 100, 100, 100, 255, primomenu_regole.entry, px, py, &w, &h, shaded);
	primomenu_regole.x = px;
	primomenu_regole.y = py;
	primomenu_regole.w = w;
	primomenu_regole.h = h;
	py = py + h + 10;

	GC_DrawText(iniz_screen, font, 0, 0, 0, 0, 100, 100, 100, 255, primomenu_game.entry, px, py, &w, &h, shaded);
	primomenu_game.x = px;
	primomenu_game.y = py;
	primomenu_game.w = w;
	primomenu_game.h = h;
	py = py + h + 10;

	GC_DrawText(iniz_screen, font, 0, 0, 0, 0, 100, 100, 100, 255, primomenu_esci.entry, px, py, &w, &h, shaded);
	primomenu_esci.x = px;
	primomenu_esci.y = py;
	primomenu_esci.w = w;
	primomenu_esci.h = h;
	py = py + h + 10;
	//printf("Larg testo: %d\n", wid);
	//printf("Alt testo: %d\n", hei);

	int uscita = 0;

	/* La variabile sceltamenu regola la scelta nel menu di sinistra.
	   0 = Nessuna scelta
	   1 = Menu settings a sinistra
	   2 = Scheda
	   3 = Regole
	*/
	int sceltamenu = 0;

	while (uscita != 1) {
		// Prima disegno
	
		// ridisegno lo sfondo
		if (refreshleft == 1) {
			GC_DrawImage(skybg, 0, 0,
					settings_player_bianco_ia_minmax_ab.x+settings_player_bianco_ia_minmax_ab.w + 20,
					settings_nelementi.y+settings_nelementi.h + 50,
					iniz_screen, 0,0);
			
			//GC_DrawImage(skybg, 0, 0, 650, 400, iniz_screen, 0, 0);
			refreshleft = 0;
		}

		if (sceltamenu == 1) {
			// Disegno la parte di settaggi
			int sx = 150;
			int sy = 25;
			int sw, sh;

			GC_DrawText(iniz_screen, font, 200, 0, 0, 0, 255, 255, 255, 255, "Settings:", sx, sy, &sw, &sh, blended);
			sy = 50;

			sx = 25;

			GC_DrawText(iniz_screen, font, 0, 0, 0, 0, 255, 255, 255, 255, "Giocatore Nero:", sx, sy, &sw, &sh, blended);
			sy = sy + sh + 10;

			sx+= 20;

			GC_DrawText(iniz_screen, font, 230, 0, 0, 0, 255, 255, 255, 255, settings_player_nero_mode.entry, sx, sy, &sw, &sh, blended);

			settings_player_nero_mode.x = sx;
			settings_player_nero_mode.y = sy;
			settings_player_nero_mode.h = sh;
			settings_player_nero_mode.w = sw;
			sy = sy + sh + 10;

			// Disegna le opzioni per il bianco
			sx = 250; // <- importante che non dipenda da sw precedente
			sy = 50;

			GC_DrawText(iniz_screen, font, 0, 0, 0, 0, 255, 255, 255, 255, "Giocatore Bianco:", sx, sy, &sw, &sh, blended);
			sy = sy + sh + 10;
			sx += 25;


			GC_DrawText(iniz_screen, font, 230, 0, 0, 0, 255, 255, 255, 255, settings_player_bianco_mode.entry, sx, sy, &sw, &sh, blended);

			settings_player_bianco_mode.x = sx;
			settings_player_bianco_mode.y = sy;
			settings_player_bianco_mode.h = sh;
			settings_player_bianco_mode.w = sw;
			sy = sy + sh + 10;

			sx = 25;
			sy += 10;

			GC_DrawText(iniz_screen, font, 0, 0, 0, 0, 255, 255, 255, 255, "Tipo IA Nero (se minmax):", sx, sy, &sw, &sh, blended);
			sx += sw + 10;

			if (minmax_nero_scelta == 1) {
				// Per ora ho scelto NO potatura
				GC_DrawText(iniz_screen, font, 230, 0, 0, 0, 255, 255, 255, 255, settings_player_nero_ia_minmax.entry, sx, sy, &sw, &sh, blended);
			}
			else if (minmax_nero_scelta == 2) {
				// Per ora ho scelto potatura
				GC_DrawText(iniz_screen, font, 0, 0, 0, 0, 255, 255, 255, 255, settings_player_nero_ia_minmax.entry, sx, sy, &sw, &sh, blended);
			}
			settings_player_nero_ia_minmax.x = sx;
			settings_player_nero_ia_minmax.y = sy;
			settings_player_nero_ia_minmax.h = sh;
			settings_player_nero_ia_minmax.w = sw;

			sx += sw + 20;

			if (minmax_nero_scelta == 1) {
				// Per ora ho scelto NO potatura
				GC_DrawText(iniz_screen, font, 0, 0, 0, 0, 255, 255, 255, 255, settings_player_nero_ia_minmax_ab.entry, sx, sy, &sw, &sh, blended);
			}
			else if (minmax_nero_scelta == 2) {
				// Per ora ho scelto potatura
				GC_DrawText(iniz_screen, font, 230, 0, 0, 0, 255, 255, 255, 255, settings_player_nero_ia_minmax_ab.entry, sx, sy, &sw, &sh, blended);
			}
			settings_player_nero_ia_minmax_ab.x = sx;
			settings_player_nero_ia_minmax_ab.y = sy;
			settings_player_nero_ia_minmax_ab.h = sh;
			settings_player_nero_ia_minmax_ab.w = sw;

			sx = 25;
			sy += sh + 10;

			// IA minmax BIANCO
			GC_DrawText(iniz_screen, font, 0, 0, 0, 0, 255, 255, 255, 255, "Tipo IA Bianco (se minmax):", sx, sy, &sw, &sh, blended);
			sx += sw + 10;

			if (minmax_bianco_scelta == 2) {
				// Per ora ho scelto NO potatura
				GC_DrawText(iniz_screen, font, 0, 0, 0, 0, 255, 255, 255, 255, settings_player_bianco_ia_minmax.entry, sx, sy, &sw, &sh, blended);
			}
			else if (minmax_bianco_scelta == 1) {
				// Per ora ho scelto potatura
				GC_DrawText(iniz_screen, font, 230, 0, 0, 0, 255, 255, 255, 255, settings_player_bianco_ia_minmax.entry, sx, sy, &sw, &sh, blended);
			}
			settings_player_bianco_ia_minmax.x = sx;
			settings_player_bianco_ia_minmax.y = sy;
			settings_player_bianco_ia_minmax.h = sh;
			settings_player_bianco_ia_minmax.w = sw;

			sx += sw + 20;

			if (minmax_bianco_scelta == 2) {
				// Per ora ho scelto NO potatura
				GC_DrawText(iniz_screen, font, 230, 0, 0, 0, 255, 255, 255, 255, settings_player_bianco_ia_minmax_ab.entry, sx, sy, &sw, &sh, blended);
			}
			else if (minmax_bianco_scelta == 1) {
				// Per ora ho scelto potatura
				GC_DrawText(iniz_screen, font, 0, 0, 0, 0, 255, 255, 255, 255, settings_player_bianco_ia_minmax_ab.entry, sx, sy, &sw, &sh, blended);
			}
			settings_player_bianco_ia_minmax_ab.x = sx;
			settings_player_bianco_ia_minmax_ab.y = sy;
			settings_player_bianco_ia_minmax_ab.h = sh;
			settings_player_bianco_ia_minmax_ab.w = sw;

			sy += sh + 10;
		
			sx = 25;
			sy += 25;
			
			GC_DrawText(iniz_screen, font, 0, 0, 0, 255, 255, 255, 255, 255, "Test Mode:", sx, sy, &sw, &sh, blended);
			sx += sw + 10;

			if (test_mode == 0) GC_DrawText(iniz_screen, font, 0, 0, 0, 255, 255, 255, 255, 255, settings_test_yes.entry, sx, sy, &sw, &sh, blended);
			else if (test_mode == 1) GC_DrawText(iniz_screen, font, 230, 0, 0, 255, 255, 255, 255, 255, settings_test_yes.entry, sx, sy, &sw, &sh, blended);
			settings_test_yes.x = sx;
			settings_test_yes.y = sy;
			settings_test_yes.w = sw;
			settings_test_yes.h = sh;

			sx += sw + 20;

			if (test_mode == 0) GC_DrawText(iniz_screen, font, 230, 0, 0, 255, 255, 255, 255, 255, settings_test_no.entry, sx, sy, &sw, &sh, blended);
			else if (test_mode == 1) GC_DrawText(iniz_screen, font, 0, 0, 0, 255, 255, 255, 255, 255, settings_test_no.entry, sx, sy, &sw, &sh, blended);
			settings_test_no.x = sx;
			settings_test_no.y = sy;
			settings_test_no.w = sw;
			settings_test_no.h = sh;


			sx = 25;
			sy += 25;
			
			GC_DrawText(iniz_screen, font, 0, 0, 0, 0, 255, 255, 255, 255, "Risoluzione:", sx, sy, &sw, &sh, blended);

			sx += sw + 10;

			if (scelta_ris == 0) {
				GC_DrawText(iniz_screen, font, 230, 0, 0, 0, 255, 255, 255, 255, settings_lowres.entry, sx, sy, &sw, &sh, blended);
			}
			else if (scelta_ris == 1) {
				GC_DrawText(iniz_screen, font, 0, 0, 0, 0, 255, 255, 255, 255, settings_lowres.entry, sx, sy, &sw, &sh, blended);
			}
			settings_lowres.x = sx;
			settings_lowres.y = sy;
			settings_lowres.h = sh;
			settings_lowres.w = sw;

			sx = sx + sw + 10;

			if (scelta_ris == 1) {
				GC_DrawText(iniz_screen, font, 230, 0, 0, 0, 255, 255, 255, 255, settings_hires.entry, sx, sy, &sw, &sh, blended);
			}
			else if (scelta_ris == 0) {
				GC_DrawText(iniz_screen, font, 0, 0, 0, 0, 255, 255, 255, 255, settings_hires.entry, sx, sy, &sw, &sh, blended);
			}
			settings_hires.x = sx;
			settings_hires.y = sy;
			settings_hires.h = sh;
			settings_hires.w = sw;

			// Disegno la scelta del numero di caselle
			sy = sy + sh + 10;
			sx = 25;

			GC_DrawText(iniz_screen, font, 0, 0, 0, 0, 255, 255, 255, 255, "Numero di caselle per riga e colonna:", sx, sy, &sw, &sh, blended);
			sx = sx + sw + 10;

			GC_DrawText(iniz_screen, font, 230, 0, 0, 0, 255, 255, 255, 255, settings_nelementi.entry, sx, sy, &sw, &sh, blended);
			settings_nelementi.x = sx;
			settings_nelementi.y = sy;
			settings_nelementi.h = sh;
			settings_nelementi.w = sw;		

		}
		else if (sceltamenu == 2) {
			// visualizzo la scheda.

			int sx = 25;
			int sy = 25;
			int sw, sh;

			// FOTO
			// Usa una foto che sia ALTA ESATTAMENTE 150 pixels e più stretta di boh, 400
			// Una proporzione standard è altezza 150 e lunghezza 200
			// chiama la foto myimage.jpg (formato .jpg quindi)

			// AL POSTO DI 267 SOTTO METTI LA TUA LUNGHEZZA DELLA FOTO
			GC_DrawImage(bioimage, 0, 0, 267, 150, iniz_screen, sx, sy);

			sy += 150 + 10;

			GC_DrawText(iniz_screen, font, 0, 0, 0, 0, 0, 0, 0, 255, bio1, sx, sy, &sw, &sh, blended);
			sy += sh + 5;
			GC_DrawText(iniz_screen, font, 0, 0, 0, 0, 0, 0, 0, 255, bio2, sx, sy, &sw, &sh, blended);
			sy += sh +5;
			GC_DrawText(iniz_screen, font, 0, 0, 0, 0, 0, 0, 0, 255, bio3, sx, sy, &sw, &sh, blended);
			sy += sh + 5;

		}
		else if (sceltamenu == 3) {
			// visualizzo le regole
			int sx = 25;
			int sy = 25;
			int sw, sh;


			GC_DrawText(iniz_screen, font, 0, 0, 0, 0, 0, 0, 0, 255, regole1, sx, sy, &sw, &sh, blended);
			sy += sh + 5;
			GC_DrawText(iniz_screen, font, 0, 0, 0, 0, 0, 0, 0, 255, regole2, sx, sy, &sw, &sh, blended);
			sy += sh +5;
			GC_DrawText(iniz_screen, font, 0, 0, 0, 0, 0, 0, 0, 255, regole3, sx, sy, &sw, &sh, blended);
			sy += sh + 5;
			GC_DrawText(iniz_screen, font, 0, 0, 0, 0, 0, 0, 0, 255, regole4, sx, sy, &sw, &sh, blended);
			sy += sh + 5;
			GC_DrawText(iniz_screen, font, 0, 0, 0, 0, 0, 0, 0, 255, regole5, sx, sy, &sw, &sh, blended);
			sy += sh + 5;
			GC_DrawText(iniz_screen, font, 0, 0, 0, 0, 0, 0, 0, 255, regole6, sx, sy, &sw, &sh, blended);
			sy += sh + 5;
			GC_DrawText(iniz_screen, font, 0, 0, 0, 0, 0, 0, 0, 255, regole7, sx, sy, &sw, &sh, blended);
			sy += sh +5;
			GC_DrawText(iniz_screen, font, 0, 0, 0, 0, 0, 0, 0, 255, regole8, sx, sy, &sw, &sh, blended);
			sy += sh + 5;
			GC_DrawText(iniz_screen, font, 0, 0, 0, 0, 0, 0, 0, 255, regole9, sx, sy, &sw, &sh, blended);
			sy += sh + 5;
			GC_DrawText(iniz_screen, font, 0, 0, 0, 0, 0, 0, 0, 255, regole10, sx, sy, &sw, &sh, blended);
			sy += sh + 5;
			GC_DrawText(iniz_screen, font, 0, 0, 0, 0, 0, 0, 0, 255, regole11, sx, sy, &sw, &sh, blended);
			sy += sh + 5;

			//GC_DrawText(iniz_screen, font, 200, 0, 0, 0, 255, 255, 255, 255, regole1, sx, sy, &sw, &sh, shaded);

		}

		//SDL_Flip(iniz_screen);

		// Ora gli eventi
		
		if (SDL_PollEvent(&event)) {

			switch(event.type) {
				
				case SDL_QUIT:
					uscita = 1;
					break;
				
				case SDL_KEYDOWN:
					if(event.key.keysym.sym == SDLK_ESCAPE) {
						printf("ESC premuto.\n");
						uscita = 1;
					}
					break;

				case SDL_MOUSEBUTTONDOWN:
					if(event.button.button==1) {
						int myx = event.button.x;
						int myy = event.button.y;
						
						if (myx > primomenu_settings.x && myx < primomenu_settings.x+primomenu_settings.w &&
								myy > primomenu_settings.y && myy < primomenu_settings.y+primomenu_settings.h) {
							printf("Ho premuto SETTINGS!\n");

							if (sceltamenu != 1) {
								sceltamenu = 1;
								refreshleft = 1;
							}
						}

						else if (myx > primomenu_scheda.x && myx < primomenu_scheda.x+primomenu_scheda.w &&
								myy > primomenu_scheda.y && myy < primomenu_scheda.y+primomenu_scheda.h) {
							printf("Ho premuto SCHEDA\n");

							if (sceltamenu != 2) {
								sceltamenu = 2;
								refreshleft = 1;
							}
						}

						else if (myx > primomenu_regole.x && myx < primomenu_regole.x+primomenu_regole.w &&
								myy > primomenu_regole.y && myy < primomenu_regole.y+primomenu_regole.h) {
							printf("Ho premuto REGOLE\n");

							if (sceltamenu != 3) {
								sceltamenu = 3;
								refreshleft = 1;
							}
						}

						else if (myx > primomenu_game.x && myx < primomenu_game.x+primomenu_game.w &&
								myy > primomenu_game.y && myy < primomenu_game.y+primomenu_game.h) {
							printf("Ho premuto PARTITA\n");
							// se viene premuto inizia partita, setto la variabile per l'uscita, ricordandomi anche però che poi dovrò iniziare la partita
							startpartita = 1;
							uscita = 1;
						}

						else if (myx > primomenu_esci.x && myx < primomenu_esci.x+primomenu_esci.w &&
								myy > primomenu_esci.y && myy < primomenu_esci.y+primomenu_esci.h) {
							printf("Ho premuto ESCI\n");
							// se viene premuto solo esci, setto solo la variabile per l'uscita
							uscita = 1;
						}

						if (sceltamenu == 1) {
							// Gestisco anche gli eventi dei settaggi

								if (myx > settings_player_nero_mode.x && myx < settings_player_nero_mode.x+settings_player_nero_mode.w 
										&& myy > settings_player_nero_mode.y && myy < settings_player_nero_mode.y+settings_player_nero_mode.h) {
									// Ho cliccato per cambiare la MODALITA' ia del giocatore NERO.
									playerneromode++;
									switch (playerneromode) {
										case 8: 
											playerneromode = -1;
											break;
										case -1: 
											settings_player_nero_mode.entry = "Nero Umano";
											break;
										case 0: 
											settings_player_nero_mode.entry = "Nero IA Random";
											break;
										case 1: 
											settings_player_nero_mode.entry = "Nero IA Simple";
											break;
										case 2: 
											settings_player_nero_mode.entry = "Nero IA MinMax Depth 2";
											break;
										case 3: 
											settings_player_nero_mode.entry = "Nero IA MinMax Depth 3";
											break;
										case 4: 
											settings_player_nero_mode.entry = "Nero IA MinMax Depth 4";
											break;
										case 5:
											settings_player_nero_mode.entry = "Nero IA MinMax Depth 5";
											break;
										case 6: 
											settings_player_nero_mode.entry = "Nero IA MinMax Depth 6";
											break;
										case 7: 
											settings_player_nero_mode.entry = "Nero IA MinMax Depth 7";
											break;
										default: 
											settings_player_nero_mode.entry = "Nero IA MinMax Depth 7";
											break;
									}

									// ridisegno l'immagine di sfondo, Sopra di essa verra, al prossimo ciclo, disegnata la nuova stringa corretta.
									GC_DrawImage(skybg, settings_player_nero_mode.x, settings_player_nero_mode.y,
											settings_player_nero_mode.w +20,
											settings_player_nero_mode.h +20,
											iniz_screen, settings_player_nero_mode.x, settings_player_nero_mode.y);	

								}

								// Eventi relativi al bianco
								else if (myx > settings_player_bianco_mode.x && myx < settings_player_bianco_mode.x+settings_player_bianco_mode.w && myy > settings_player_bianco_mode.y && myy < settings_player_bianco_mode.y+settings_player_bianco_mode.h) {

									// Ho cliccato per cambiare la MODALITA' ia del giocatore NERO.	
									playerbiancomode++;

									switch (playerbiancomode) {
										case 8: 
											playerbiancomode = -1;
											break;
										case -1: 
											settings_player_bianco_mode.entry = "Bianco Umano";
											break;
										case 0: 
											settings_player_bianco_mode.entry = "Bianco IA Random";
											break;
										case 1: 
											settings_player_bianco_mode.entry = "Bianco IA Simple";
											break;
										case 2: 
											settings_player_bianco_mode.entry = "Bianco IA MinMax Depth 2";
											break;
										case 3: 
											settings_player_bianco_mode.entry = "Bianco IA MinMax Depth 3";
											break;
										case 4: 
											settings_player_bianco_mode.entry = "Bianco IA MinMax Depth 4";
											break;
										case 5:
											settings_player_bianco_mode.entry = "Bianco IA MinMax Depth 5";
											break;
										case 6: 
											settings_player_bianco_mode.entry = "Bianco IA MinMax Depth 6";
											break;
										case 7: 
											settings_player_bianco_mode.entry = "Bianco IA MinMax Depth 7";
											break;
										default: 
											settings_player_bianco_mode.entry = "Bianco IA MinMax Depth 7";
											break;
									}

									GC_DrawImage(skybg, settings_player_bianco_mode.x, settings_player_bianco_mode.y,
											settings_player_bianco_mode.w +20,
											settings_player_bianco_mode.h +20,
											iniz_screen, settings_player_bianco_mode.x, settings_player_bianco_mode.y);	

								}

							else if ( myx > settings_test_yes.x && myx < (settings_test_yes.x + settings_test_yes.w +5) && 
									myy > settings_test_yes.y && myy < (settings_test_yes.y + settings_test_yes.h) ) {
								// Rilevato click per ATTIVARE la TEST MODE
										test_mode = 1;
							}

							else if ( myx > settings_test_no.x && myx < (settings_test_no.x + settings_test_no.w +5) && 
									myy > settings_test_no.y && myy < (settings_test_no.y + settings_test_no.h) ) {
								// Rilevato click per DISATTIVARE la TEST MODE
										test_mode = 0;
							}

							else if (myx > (settings_lowres.x) && myx < (settings_lowres.x + settings_lowres.w +10) &&
									myy > settings_lowres.y && myy < settings_lowres.y + settings_lowres.h +10) {
								// Rilevato click per scegliere 800x600 come risoluzione
										scelta_ris = 0;
										printf("LOWres\n");
							}

							else if (myx > (settings_hires.x) && myx < (settings_hires.x + settings_hires.w +10) &&
									myy > settings_hires.y && myy < settings_hires.y + settings_hires.h +10) {
								// Rilevato click per scegliere 1024x768 come risoluzione
										scelta_ris = 1;
										printf("HIwres\n");
							}

							// Scelta tipo minmax NERO
							else if (myx > settings_player_nero_ia_minmax.x && myx < (settings_player_nero_ia_minmax.x + settings_player_nero_ia_minmax.w) && myy > settings_player_nero_ia_minmax.y && myy < (settings_player_nero_ia_minmax.y + settings_player_nero_ia_minmax.h) ) {
								// Minmax SENZA potatura
								minmax_nero_scelta = 1;
							}

							else if (myx > settings_player_nero_ia_minmax_ab.x && myx < (settings_player_nero_ia_minmax_ab.x + settings_player_nero_ia_minmax_ab.w) && myy > settings_player_nero_ia_minmax_ab.y && myy < (settings_player_nero_ia_minmax_ab.y + settings_player_nero_ia_minmax_ab.h) ) {
								// Minmax CON potatura
								minmax_nero_scelta = 2;
							}

							// Scelta tipo minmax NERO
							else if (myx > settings_player_bianco_ia_minmax.x && myx < (settings_player_bianco_ia_minmax.x + settings_player_bianco_ia_minmax.w) && myy > settings_player_bianco_ia_minmax.y && myy < (settings_player_bianco_ia_minmax.y + settings_player_bianco_ia_minmax.h) ) {
								// Minmax SENZA potatura
								minmax_bianco_scelta = 1;
							}

							else if (myx > settings_player_bianco_ia_minmax_ab.x && myx < (settings_player_bianco_ia_minmax_ab.x + settings_player_bianco_ia_minmax_ab.w) && myy > settings_player_bianco_ia_minmax_ab.y && myy < (settings_player_bianco_ia_minmax_ab.y + settings_player_bianco_ia_minmax_ab.h) ) {
								// Minmax CON potatura
								minmax_bianco_scelta = 2;
							}	

							else if (myx > (settings_nelementi.x -5) && myx < (settings_nelementi.x + settings_nelementi.w + 10) &&
									myy > (settings_nelementi.y -5) && myy < settings_nelementi.y + settings_nelementi.h +10 ) {
								// Rilevato click per cambiare il numero di caselle. Verrà cambiata la stringa e ridisegnato lo sfondo del quadrante.
								nelem++;
								if (nelem > 8) nelem = 4;

								if (nelem == 4) settings_nelementi.entry = "4 Elementi";
								else if (nelem == 5) settings_nelementi.entry = "5 Elementi";
								else if (nelem == 6) settings_nelementi.entry = "6 Elementi";
								else if (nelem == 7) settings_nelementi.entry = "7 Elementi";
								else if (nelem == 8) settings_nelementi.entry = "8 Elementi";

								GC_DrawImage(skybg, settings_nelementi.x, settings_nelementi.y,
										settings_nelementi.w +10,
										settings_nelementi.h +10,
										iniz_screen, settings_nelementi.x, settings_nelementi.y);

							}

						}

					}
				
				default:
					break;
			}
		}

		SDL_Flip(iniz_screen);
	}
	
	// sono alla fine di tutto. Faccio free di ciò che non serve e setto le variabili GLOBALI necessarie, usate poi dalla finestra della partita
	SDL_FreeSurface(skybg);
	if (startpartita == 1) {

		if (scelta_ris == 0) {
			winwidth = 800;
			winheight = 600;
		}
		else if (scelta_ris == 1) {
			winwidth = 1024;
			winheight = 768;
		}

		nrighecolonne = nelem;

		SDL_FreeSurface(iniz_screen);
		//SDL_Quit();
		return 1;
	}
	else {
		SDL_FreeSurface(iniz_screen);
		//SDL_Quit();
		return 0;
	}

}

int main(int argc, char** argv) {
	SDL_Event event;
	TTF_Font *font;
	int uscita = 0;
	int result;

	// questa variabili mantengono informazioni sui riquadri che mostrano il numero di pedine, considerato che non sono cliccabili, non si è usata la struttura menuentry
	int label_nero_posx = 0;
	int label_nero_posy = 0;
	int label_nero_w;
	int label_nero_h;
	int label_bianco_posx = 0;
	int label_bianco_posy = 0;
	int label_bianco_w;
	int label_bianco_h;

	/*
	int label_sugg_posx = 0;
	int label_sugg_posy = 0;
	int label_sugg_h = 0;
	int label_sugg_w = 0;
	*/

	// msgtobox è una stringa che conterrà il messaggio di supporto da visualizzare
	char msgtobox[100] = "Benvenuto!";
	// le variabili per ricordarsi di ridisegnare le varie parti a schermo, come il messaggio, la scacchiera o gli indicatori del numero di pedine
	int refreshmsg = 1;
	int refreshscacch = 1;
	int refreshlabelnero = 0;
	int refreshlabelbianco = 0;

	// turno all'inizio è 0 
	turno = 0;
	// playing é 1 finchè la partita NON è finita
	int playing = 1;

	// variabili per ricordarsi se il log delle mosse è attivo e nel caso puntatore alla struttura file
	int historyattiva = 0;
	FILE * historyfile = NULL;

	// nodi per l'IA minmax
	nodo *ai_nodo_nero = NULL;
	nodo *ai_nodo_bianco = NULL;
	int coda_mosse_ai_nero = 0;
	int coda_mosse_ai_bianco = 0;

	// il vettore imgscacch contiene le superfici rappresentanti le due parti della scacchiera
	SDL_Surface *imgscacch[2];
	// superfici per le immagini delle pedine e per lo sfondo
	SDL_Surface *pedinaB, *pedinaN, *skybg = NULL;
	// screen sarà la superficie che rappresenterà la nostra finestra, su cui disegneremo
	SDL_Surface *screen = NULL;

	int i,j;

	/* per rendere personalizzabile il numero di partite da effettuare in una eventuale test_mode, si è pensato di ricevere un argomento in input
	   da linea di comando, in quanto era complesso fare input via funzioni grafiche. Quindi, se il numero di argomenti è maggiore di 1,
	   si interpreta il primo come numero di partite di test eventuali. Il numero non deve essere maggiore della costante MAX_PARTITE_TEST.
	   */

	if (argc > 1) {
		int n;
		n = atoi(argv[1]);
		if ( n > MAX_PARTITE_TEST) {
			printf("Richiesta troppe partite di test. Numero ridotto.\n");
			n = MAX_PARTITE_TEST;
		}
		numero_partite_test = n;
		printf("Numero partite di test: %d\n",numero_partite_test);
	}

	/* Inizializza SDL */
	if(SDL_Init(SDL_INIT_VIDEO)<0)
	{
		fprintf(stderr,"Non riesco ad inizializzare il display video: %s\n",SDL_GetError());
		return(0);
	}
	
	// Inizializza SDL_ttf e un font 
	if (TTF_Init() < 0)
	{
		fprintf(stderr, "Couldn't initialize TTF: %s\n",SDL_GetError());
		//SDL_Quit();
		return(0);
	}
	font = TTF_OpenFont("FreeSans.ttf", 16);
	if (font == NULL)
	{
		fprintf(stderr, "Couldn't load font\n");
	}

	// carico le immagini che mi servono: scacchiera, pedine
	imgscacch[0] = GC_LoadImage("greens.png", &result);
	if ( result == 0) {
		fprintf(stderr, "Impossibile caricare l'immagine greens.png!\n");
		return(0);
	}
	imgscacch[1] = GC_LoadImage("whites.png", &result);
	if ( result == 0) {
		fprintf(stderr, "Impossibile caricare l'immagine whites.png!\n");
		return(0);
	}
	pedinaB = GC_LoadImage("pedw.png", &result);
	if ( result == 0) {
		fprintf(stderr, "Impossibile caricare l'immagine pedw.png!\n");
		return(0);
	}
	pedinaN = GC_LoadImage("pedn.png", &result);
	if ( result == 0) {
		fprintf(stderr, "Impossibile caricare l'immagine pedn.png!\n");
		return(0);
	}

	// disegno la finestra con il menu per i settaggi.
	int iniziopartita = draw_menu_iniziale(font);

	// se la finestra preliminare richiede l'inizio di una partita, inizio
	if (iniziopartita == 1) {

		// Alloco la matrice che usero come scacchiera principale
		// Alloco il primo vettore di puntatori ad int
		scacchieraPrinc = malloc (sizeof(int*) * nrighecolonne);
		if (scacchieraPrinc == NULL) {
			printf("MALLOC fallita! Esco...\n");
			SDL_Quit();
			return -1;
		}
		for (i=0; i< nrighecolonne; i++) {
			// Alloco le mie "colonne", vettori di int
			scacchieraPrinc[i] = malloc (sizeof(int) * nrighecolonne );
			if (scacchieraPrinc[i] == NULL ) {
				printf("MALLOC fallita! Esco...\n");
				SDL_Quit();
				return -1;
			}
		}

		// carico l'immagine correttamente scalata a seconda di quale risoluzione è stata selezionata
		if (winwidth == 800 && winheight == 600) {
			skybg = GC_LoadImage("skylow.png", &result);
			if ( result == 0) {
				fprintf(stderr, "Impossibile caricare l'immagine skylow.png!\n");
				return(0);
			}
		}
		else if (winwidth == 1024 && winheight == 768) {
			skybg = GC_LoadImage("skyhi.png", &result);
			if ( result == 0) {
				fprintf(stderr, "Impossibile caricare l'immagine skyhi.png!\n");
				return(0);
			}
		}
	
		// setto la modalità video
		screen = SDL_SetVideoMode(winwidth, winheight, 32, SDL_HWSURFACE | SDL_DOUBLEBUF );
		if(screen==NULL) {
			fprintf(stderr,"Can't set video mode: %s\n",SDL_GetError());
			SDL_Quit();
			//return ;
		}

		// setto il titolo della finestra
		SDL_WM_SetCaption("LMOthello - Partita!",NULL);


		// Setup della nuova partita
		srand(time(0));

		casellevuote = nrighecolonne * nrighecolonne;

		// Pulisco la memoria appena allocata
		for (i=0; i<nrighecolonne; i++) {
			for (j=0; j<nrighecolonne; j++) {
				scacchieraPrinc[i][j] = VUOTO;
			}
		}

		int startX = (nrighecolonne / 2) -1;
		int startY = (nrighecolonne / 2) -1;
		scacchieraPrinc[startY][startX] = BIANCO;
		scacchieraPrinc[startY][startX + 1] = NERO;
		scacchieraPrinc[startY + 1][startX] = NERO;
		scacchieraPrinc[startY + 1][startX + 1] = BIANCO;

		numerobianche = 2;
		numeronere = 2;

		// Disegno lo sfondo
		GC_DrawImage(skybg, 0, 0, winwidth, winheight, screen, 0, 0);
		

		//init_game();

		// Disegno menu principale

		/* Per disegnare i menu si utilizzano riquadri rappresentati dalla struttura menuentry: essa mantiene le coordinate e l'estensione dei riquadri, in modo da facilitare il processo di rilevamento dei riquadri cliccati. La funzione GC_DrawText è stata modificata così che ritorni altezza e larghezza della stringa disegnata. */
		int px = winwidth - 200;
		int py = 50;
		int w,h;

		GC_DrawText(screen, font, 230, 0, 0, 0, 255, 255, 255, 255, partita_save.entry, px, py, &w, &h, shaded);
		partita_save.x = px;
		partita_save.y = py;
		partita_save.h = h;
		partita_save.w = w;
		py += h + 10;

		GC_DrawText(screen, font, 230, 0, 0, 0, 255, 255, 255, 255, partita_load.entry, px, py, &w, &h, shaded);
		partita_load.x = px;
		partita_load.y = py;
		partita_load.h = h;
		partita_load.w = w;
		py += h + 10;

		GC_DrawText(screen, font, 230, 0, 0, 0, 255, 255, 255, 255, partita_info.entry, px, py, &w, &h, shaded);
		partita_info.x = px;
		partita_info.y = py;
		partita_info.h = h;
		partita_info.w = w;
		py += h + 10;

		GC_DrawText(screen, font, 230, 0, 0, 0, 255, 255, 255, 255, partita_history.entry, px, py, &w, &h, shaded);
		partita_history.x = px;
		partita_history.y = py;
		partita_history.h = h;
		partita_history.w = w;
		py += h + 10;

		GC_DrawText(screen, font, 230, 0, 0, 0, 255, 255, 255, 255, partita_restart.entry, px, py, &w, &h, shaded);
		partita_restart.x = px;
		partita_restart.y = py;
		partita_restart.h = h;
		partita_restart.w = w;
		py += h + 10;

		GC_DrawText(screen, font, 230, 0, 0, 0, 255, 255, 255, 255, partita_esci.entry, px, py, &w, &h, shaded);
		partita_esci.x = px;
		partita_esci.y = py;
		partita_esci.h = h;
		partita_esci.w = w;
		py += h + 10;


		// Disegno la scacchiera
		int mystart;
		char mystr[3];
		int scacchx = 50;
		int scacchy = 50;
		int winutile, hinutile;	
	
		// Prima disegno la numerazione delle caselle
		for (i=0; i< nrighecolonne; i++) {
			sprintf(mystr, "%d", i+1);
			GC_DrawText(screen, font, 255, 255, 255, 0, 0, 0, 0, 255, mystr, scacchx + 20 + i * 50, scacchy -30, &winutile, &hinutile, blended);
			GC_DrawText(screen, font, 255, 255, 255, 0, 0, 0, 0, 255, mystr, scacchx - 20, scacchy + 20 + i * 50, &winutile, &hinutile, blended);
		}	

		// Ora il riquadro vero e proprio
		for (i=0; i < nrighecolonne; i++) {
			mystart = i % 2;
			for (j = 0; j < nrighecolonne; j++) {	
				GC_DrawImage(imgscacch[(mystart+j) % 2], 0, 0, 50, 50, screen, scacchx+j*50, scacchy+i*50);
				if (scacchieraPrinc[i][j] == NERO) {
					GC_DrawImage(pedinaN, 0, 0, 50, 50, screen, scacchx + j * 50 + 5, scacchy + i * 50 + 5);
				}
				else if (scacchieraPrinc[i][j] == BIANCO) {	
					GC_DrawImage(pedinaB, 0, 0, 50, 50, screen, scacchx + j * 50 + 5, scacchy + i * 50 + 5);
				}
			}
		}


		// Setto e disegno due aree che visualizzano il numero di pedine
		label_nero_posx = scacchx + nrighecolonne * 50 + 20;
		label_nero_posy = scacchy;
		label_bianco_posx = label_nero_posx;
		label_bianco_posy = label_nero_posy + 50;

		char mytext[20];
		sprintf(mytext, "N. NERE: %d", numeronere);
		GC_DrawText(screen, font, 0, 0, 0, 0, 0, 0, 0, 255, mytext, label_nero_posx, label_nero_posy, &label_nero_w, &label_nero_h, blended);
		sprintf(mytext, "N. BIANCHE: %d", numeronere);
		GC_DrawText(screen, font, 255, 255, 255, 0, 0, 0, 0, 255, mytext, label_bianco_posx, label_bianco_posy, &label_bianco_w, &label_bianco_h, blended);

		// Posizione del bottone per chiedere un suggerimento
		GC_DrawText(screen, font, 0, 255, 0, 0, 0, 0, 0, 255, partita_suggerimento.entry, label_bianco_posx, label_bianco_posy + 70, &(partita_suggerimento.w), &(partita_suggerimento.h), blended);	
		partita_suggerimento.x = label_bianco_posx;
		partita_suggerimento.y = label_bianco_posy + 70;

		// Disegno l'area per i messaggi
		int messy = scacchy + 50 * nrighecolonne + 30;
		int messx = scacchx;
		int messw, messh;
		GC_DrawText(screen, font, 255, 255, 255, 0, 0, 0, 0, 255, msgtobox, messx, messy, &messw, &messh, blended);

		if (test_mode == 1) {
			// Esecuzione della modalità test. Tutte le mosse verranno salvate su un nuovo file di log, othello_test.txt
			testfile = fopen("othello_test.txt", "w");
			if (testfile == NULL) {
				printf("Errore nell'apertura del file di test. Esco...");
				exit(-1);
			}

			// se non ho settato nessun tipo di IA, verranno usate le opzioni di default, ovvero Minmax depth 2
			if (playerneromode == -1) playerneromode = 2;
			if (playerbiancomode == -1) playerbiancomode = 2;
			sprintf(msgtobox,"TEST MODE: Si eseguiranno %d partite CPU vs CPU SENZA interfaccia grafica.",numero_partite_test);
			refreshmsg = 1;

			if (playerneromode == 0) {
				fprintf(testfile, "IA CPU NERO: Random IA.\n");
			}
			else if (playerneromode == 1) {
				fprintf(testfile, "IA CPU NERO: Cattura Semplice.\n");
			}
			else if (playerneromode > 1) {
				fprintf(testfile,"IA CPU NERO: MinMax Depth %d.\n", playerneromode);
			}

			if (playerbiancomode == 0) {
				fprintf(testfile, "IA CPU BIANCO: Random IA.\n");
			}
			else if (playerbiancomode == 1) {
				fprintf(testfile, "IA CPU BIANCO: Cattura Semplice.\n");
			}
			else if (playerbiancomode > 1) {
				fprintf(testfile, "IA CPU BIANCO: MinMax Depth %d.\n", playerneromode);
			}

			fprintf(testfile,"\nInizio Partita CPU vs CPU.\n");
		}
		else {
			if (playerneromode > -1 && playerbiancomode > -1) {
				sprintf(msgtobox, "Partita CPU vs CPU! Muove il NERO, fare click sulla scacchiera per procedere.");
				refreshmsg = 1;
			}
			else {
				sprintf(msgtobox, "Muove il NERO.");
				refreshmsg = 1;
			}
		}

		while (uscita != 1) {
			// vero e proprio ciclo di gioco

			if ( controlla_fine(scacchieraPrinc, ((turno % 2) +1) ) == 1 ) {
				if (  controlla_fine(scacchieraPrinc, (( (turno +1) % 2) +1)) == 1 ) {
					// nessun giocatore può fare più mosse. Partita finita
					playing = 0;
				}
				// altrimenti, semplicemente il giocatore attuale salta un turno
				else turno++;
			}

			// Se la partita è finita, visualizzo la scritta apposita
			if (playing == 0) {
				if (test_mode == 1 && numero_partite_test > 0) {
					// le cose sono un po' particolari se la modalità era quella di test
					if (numeronere > numerobianche) {
						fprintf(testfile, "Partita %d finita. Il NERO ha VINTO!\n", numero_partite_test);
					}
					else if (numerobianche > numeronere) {
						fprintf(testfile, "Partita %d finita. Il BIANCO ha VINTO!\n", numero_partite_test);
					}
					else fprintf(testfile, "Partita %d finita. Incredibile, PAREGGIO!\n", numero_partite_test);

					numero_partite_test--;

					// ho appena terminato una partita in test_mode. Se devo effettuarne altre, riazzero la scacchiera
					if (numero_partite_test > 0) {

						casellevuote = nrighecolonne * nrighecolonne;

						for (i=0; i<nrighecolonne; i++) {
							for (j=0; j<nrighecolonne; j++) {
								scacchieraPrinc[i][j] = VUOTO;
							}
						}

						// ripresento una situazione di partenza
						int startX = (nrighecolonne / 2) -1;
						int startY = (nrighecolonne / 2) -1;
						scacchieraPrinc[startY][startX] = BIANCO;
						scacchieraPrinc[startY][startX + 1] = NERO;
						scacchieraPrinc[startY + 1][startX] = NERO;
						scacchieraPrinc[startY + 1][startX + 1] = BIANCO;

						numerobianche = 2;
						numeronere = 2;

						fprintf(testfile,"\nInizio Partita CPU vs CPU.\n");

						playing = 1;

					}
					else {
						sprintf(msgtobox,"Test Mode TERMINATA. Log: othello_test.txt.");
						refreshmsg = 1;
						//test_mode = 1;
					}
				}
				else {
					// NON SONO in test_mode
					// decreto il vincitore
					if (numeronere > numerobianche) {
						sprintf(msgtobox, "Partita finita. Il NERO ha VINTO!");
						if (historyattiva == 1) fprintf(historyfile, "Partita finita. Il NERO ha VINTO!");
					}
					else if (numerobianche > numeronere ) {
						sprintf(msgtobox, "Partita finita. Il BIANCO ha VINTO!");
						if (historyattiva == 1) fprintf(historyfile, "Partita finita. Il BIANCO ha VINTO!");
					}
					else {
						sprintf(msgtobox, "Incredibile, PAREGGIO!");
						if (historyattiva == 1) fprintf(historyfile, "Incredibile, PAREGGIO!");
					}
					refreshmsg = 1;
				}

				//printf("Numero nere: %d. Numero bianche: %d.\n",numeronere,numerobianche);
			}

			// FINE controlli in caso di fine partita.
			// da ora in poi, ciclo di gioco

			if (refreshmsg == 1) {
				// Ridisegno il messaggio
				GC_DrawImage(skybg, messx, messy, messw, messh, screen, messx, messy);
				GC_DrawText(screen, font, 255, 255, 255, 0, 0, 0, 0, 255, msgtobox, messx, messy, &messw, &messh, blended);
				refreshmsg = 0;
			}

			if (refreshscacch == 1) {
				// Disegno la scacchiera
				for (i=0; i < nrighecolonne; i++) {
					mystart = i % 2;
					for (j = 0; j < nrighecolonne; j++) {	
						GC_DrawImage(imgscacch[(mystart+j) % 2], 0, 0, 50, 50, screen, scacchx+j*50, scacchy+i*50);
						if (scacchieraPrinc[i][j] == NERO) {
							GC_DrawImage(pedinaN, 0, 0, 50, 50, screen, scacchx + j * 50 + 5, scacchy + i * 50 + 5);
						}
						else if (scacchieraPrinc[i][j] == BIANCO) {	
							GC_DrawImage(pedinaB, 0, 0, 50, 50, screen, scacchx + j * 50 + 5, scacchy + i * 50 + 5);
						}
					}
				}
				refreshscacch = 0;
			}

			if (refreshlabelnero == 1) {
				// Ridisegno il counter delle nere
				GC_DrawImage(skybg, label_nero_posx, label_nero_posy, label_nero_w, label_nero_h, screen, label_nero_posx, label_nero_posy);
				sprintf(mytext, "N. NERE: %d", numeronere);
				GC_DrawText(screen, font, 0, 0, 0, 0, 0, 0, 0, 255, mytext, label_nero_posx, label_nero_posy, &label_nero_w, &label_nero_h, blended);
				refreshlabelnero = 0;
			}
			if (refreshlabelbianco == 1) {
				// Ridisegno il counter delle bianche
				GC_DrawImage(skybg, label_bianco_posx, label_bianco_posy, label_bianco_w, label_bianco_h, screen, label_bianco_posx, label_bianco_posy);
				sprintf(mytext, "N. BIANCHE: %d", numerobianche);
				GC_DrawText(screen, font, 255, 255, 255, 0, 0, 0, 0, 255, mytext, label_bianco_posx, label_bianco_posy, &label_bianco_w, &label_bianco_h, blended);
				refreshlabelbianco = 0;
			}

			SDL_Flip(screen);

			// Lunga parte di AI per la CPU vs UMANO -- caso NERO CPU e BIANCO UMANO
			if ( playing && ((turno % 2) +1) == NERO && (playerneromode >= 0 && playerbiancomode == -1) ) {
				if (playerneromode == 0) {
					// AI Random
					if (test_mode == 1) ai_mossa_random(scacchieraPrinc, NERO, testfile);
					else ai_mossa_random(scacchieraPrinc, NERO, historyfile );
					casellevuote--;
					sprintf(msgtobox, "Mossa NERA via AI Random. Muove il BIANCO ora.");
					turno++;
				}
				else if (playerneromode == 1) {
					// AI max_presa
					if (test_mode == 1) ai_mossa_presa(scacchieraPrinc, NERO, testfile);
					else ai_mossa_presa(scacchieraPrinc, NERO, historyfile );
					casellevuote--;
					sprintf(msgtobox, "Mossa NERA via AI MAX_PRESA. Muove il BIANCO ora.");
					turno++;
				}
				else if (playerneromode > 1) {
					//printf("Sono qui NERO\n");
					// AI MinMax Depth [playerneromode]
					nodo *tempnodo = malloc(sizeof(nodo));
					tempnodo->status = scacchieraPrinc;
					tempnodo->vettoremosse = NULL;
					tempnodo->nfigli = 0;
					tempnodo->nmosse = 0;

					// mi preparo a calcolare il tempo in secondi che impiegherò per trovare la mossa
					time_t start, end;
					if (minmax_nero_scelta == 1) {
						time(&start);
						ai_nodo_nero = minmax(tempnodo, (turno % 2) +1, playerneromode  );
						time(&end);
					}
					else if (minmax_nero_scelta == 2) {
						time(&start);
						ai_nodo_nero = minmax_ab(tempnodo, (turno % 2) +1, playerneromode, 0.0, 0.0);
						time(&end);
					}
					float tempo_passato = difftime(end, start);
					
					coda_mosse_ai_nero = ai_nodo_nero->nmosse;
				
					// ho trovato una serie di mosse. Eseguo la prima
					if (coda_mosse_ai_nero > 0) {
						if ( check_mossa_ed_esegui_mat(scacchieraPrinc, ai_nodo_nero->vettoremosse[0]->my, ai_nodo_nero->vettoremosse[0]->mx, (turno % 2) +1) == 1) {
							//printf("Mossa eseguita: Y %d e X %d.\n", ai_nodo_nero->vettoremosse[0]->my, ai_nodo_nero->vettoremosse[0]->mx);
							scacchieraPrinc[ai_nodo_nero->vettoremosse[0]->my][ai_nodo_nero->vettoremosse[0]->mx] = (turno % 2) + 1;
							numeronere++;
							casellevuote--;
							sprintf(msgtobox, "Mossa NERA via AI MINMAX_AB depth %d, calcolata in %.2lf secondi. Muove il BIANCO ora.", playerneromode, tempo_passato);
							if (historyattiva == 1) fprintf(historyfile, "Mossa NERA via AI MINMAX_AB depth %d, calcolata in %.2lf secondi. Muove il BIANCO ora.\n", playerneromode, tempo_passato);
							turno++;
						}
					}
					free(tempnodo);
					tempnodo = NULL;

					if (test_mode != 1) {
						refreshscacch = 1;
						refreshlabelnero = 1;
						refreshlabelbianco = 1;
						refreshmsg = 1;
					}
				
				}
			}

			// Lunga parte di AI per la CPU vs UMANO -- caso NERO UMANO e BIANCO CPU
			else if ( playing && ((turno % 2) +1) == BIANCO && (playerbiancomode >= 0 && playerneromode == -1) ) {
				if (playerbiancomode == 0) {
					// AI Random
					if (test_mode == 1) ai_mossa_random(scacchieraPrinc, BIANCO, testfile);
					else ai_mossa_random(scacchieraPrinc, BIANCO, historyfile );
					casellevuote--;
					sprintf(msgtobox, "Mossa BIANCA via AI Random. Muove il NERO ora.");
					if (historyattiva ) 
					turno++;
				}
				else if (playerbiancomode == 1) {
					// AI max_presa
					if (test_mode == 1) ai_mossa_presa(scacchieraPrinc, BIANCO, testfile);
					else ai_mossa_presa(scacchieraPrinc, BIANCO, historyfile );
					casellevuote--;					
					sprintf(msgtobox, "Mossa BIANCA via AI MAX_PRESA. Muove il NERO ora.");
					turno++;
				}
				else if (playerbiancomode > 1) {
					// AI MinMax Depth [playerbiancomode]
					//printf("Sono qui BIANCO\n");
					nodo *tempnodo = malloc(sizeof(nodo));
					tempnodo->status = scacchieraPrinc;
					tempnodo->vettoremosse = NULL;
					tempnodo->nfigli = 0;
					tempnodo->nmosse = 0;
					//printf("Depth: %d\n",playerbiancomode);

					// mi preparo a calcolare il tempo in secondi che impiegherò per trovare la mossa
					time_t start, end;
					if (minmax_bianco_scelta == 1) {
						time(&start);
						ai_nodo_bianco = minmax(tempnodo, (turno % 2) +1, playerbiancomode  );
						time(&end);
					}
					else if (minmax_bianco_scelta == 2) {
						time(&start);
						ai_nodo_bianco = minmax_ab(tempnodo, (turno % 2) +1, playerbiancomode, 0.0, 0.0);
						time(&end);
					}
					float tempo_passato = difftime(end, start);

					coda_mosse_ai_bianco = ai_nodo_bianco->nmosse;

					// ho trovato una serie di mosse. Eseguo la prima
					if (coda_mosse_ai_bianco > 0) {
						if ( check_mossa_ed_esegui_mat(scacchieraPrinc, ai_nodo_bianco->vettoremosse[0]->my, ai_nodo_bianco->vettoremosse[0]->mx, (turno % 2) +1) == 1) {
							//printf("Mossa eseguita: Y %d e X %d.\n", ai_nodo_bianco->vettoremosse[0]->my, ai_nodo_bianco->vettoremosse[0]->mx);
							scacchieraPrinc[ai_nodo_bianco->vettoremosse[0]->my][ai_nodo_bianco->vettoremosse[0]->mx] = (turno % 2) + 1;
							numerobianche++;
							casellevuote--;
							sprintf(msgtobox, "Mossa BIANCA via AI MINMAX_AB depth %d, calcolata in %.2lf secondi. Muove il NERO ora.", playerbiancomode, tempo_passato);
							if (historyattiva == 1) fprintf(historyfile, "Mossa BIANCA via AI MINMAX_AB depth %d, calcolata in %.2lf secondi. Muove il NERO ora.\n", playerbiancomode, tempo_passato);
							turno++;

							for (i=0; i < ai_nodo_bianco->nmosse; i++) {
								//printf("\tMossa %d: Y: %d e X %d.\n",i, ai_nodo_bianco->vettoremosse[i]->my +1 , ai_nodo_bianco->vettoremosse[i]->mx +1);
							}
							//SDL_Quit();
							//exit(-1);

						}
					}
					free(tempnodo);
					tempnodo = NULL;

					// SE SONO IN TEST_MODE, non ridisegno NULLA sulla finestra
					if (test_mode != 1) {
						refreshscacch = 1;
						refreshlabelbianco = 1;
						refreshlabelnero = 1;
						refreshmsg = 1;
					}
				}
			}

			/* Effettuo un ridisegno anche qui perchè l'aver eseguito una mossa avrà modificato, probabilmente, il numero di pedine per colore */

			if (refreshlabelnero == 1) {
				// Ridisegno il counter delle nere
				GC_DrawImage(skybg, label_nero_posx, label_nero_posy, label_nero_w, label_nero_h, screen, label_nero_posx, label_nero_posy);
				sprintf(mytext, "N. NERE: %d", numeronere);
				GC_DrawText(screen, font, 0, 0, 0, 0, 0, 0, 0, 255, mytext, label_nero_posx, label_nero_posy, &label_nero_w, &label_nero_h, blended);
				refreshlabelnero = 0;
			}
			if (refreshlabelbianco == 1) {
				// Ridisegno il counter delle bianche
				GC_DrawImage(skybg, label_bianco_posx, label_bianco_posy, label_bianco_w, label_bianco_h, screen, label_bianco_posx, label_bianco_posy);
				sprintf(mytext, "N. BIANCHE: %d", numerobianche);
				GC_DrawText(screen, font, 255, 255, 255, 0, 0, 0, 0, 255, mytext, label_bianco_posx, label_bianco_posy, &label_bianco_w, &label_bianco_h, blended);
				refreshlabelbianco = 1;
			}

			// Eventi
			int richiestasugg = 0;

			if (test_mode == 1) {
				SDL_Event my_event;
				my_event.type = SDL_MOUSEBUTTONDOWN;
				my_event.button.type = SDL_MOUSEBUTTONDOWN;
				my_event.button.button = SDL_BUTTON_LEFT;
				//my_event.state = SDL_PRESSED;
				//my_event.x = 10;
				//my_event.y = 10;
				SDL_PushEvent(&my_event);
			}

			if (SDL_PollEvent(&event))
			{
				switch(event.type)
				{
					case SDL_QUIT:
						uscita = 1;
						break;

					case SDL_KEYDOWN:
						if(event.key.keysym.sym == SDLK_ESCAPE) {
							//printf("ESC premuto.\n");
							uscita = 1;
						}
						/* richiedere i suggerimenti: per richiedere i suggerimenti, un giocatore può premere un numero da 2 a 7, richiedendo
						   un suggerimento utilizzando l'IA del tipo MinMax con depth pari al numero premuto E potatura alpha/beta SOLO SE
						   attivata in precedenza dalla finestra di settaggi. Ovviamente, questo è possibile solo se si è un giocatore umano,
						   ovvero playerneromode/playerbiancomode == -1 */
						
						if ( ( ( ((turno % 2) +1) == NERO && playerneromode == -1)) ||
							   ( ((turno % 2) +1) == BIANCO && playerbiancomode == -1)) {

							switch (event.key.keysym.sym) {
							case SDLK_2:
								//printf("Premuto 2\n");
								sugg_depth = 2;
								break;
							case SDLK_3:
								//printf("Premuto 3\n");
								sugg_depth = 3;
								break;
							case SDLK_4:
								//printf("Premuto 4\n");
								sugg_depth = 4;
								break;
							case SDLK_5:
								//printf("Premuto 5\n");
								sugg_depth = 5;
								break;
							case SDLK_6:
								//printf("Premuto 6\n");
								sugg_depth = 6;
								break;
							case SDLK_7:
								//printf("Premuto 7\n");
								sugg_depth = 7;
								break;
							case SDLK_p:
								//printf("Richiesta mossa via TASTIERA\n");
								richiestasugg = 1;
							default:
								break;
							}

						}
						break;

					case SDL_MOUSEBUTTONDOWN:
						if (event.button.button == SDL_BUTTON_LEFT) {
							int myx = event.button.x;
							int myy = event.button.y;

							if (  ( (myx < ( scacchx + (nrighecolonne * 50) + 10)) || test_mode) && (playing == 1) ) {
								// Probabile click sulla scacchiera
								
								if ( (playerneromode > -1) && (playerbiancomode > -1) ) {
									
									/* *** Partita tra sole CPU ***.
									   A volte, è possibile che il giocatore scelga una partita CPU vs CPU ma NON in test_mode. In questo caso,
									   viene giocata una partita normale, con ridisegno delle mosse sulla scacchiera ecc., e il giocatore deve
									   ogni volta cliccare sulla scacchiera per far fare una mossa. Per questo, viene gestito questo evento.
									   */
									if ( ((turno % 2) +1) == NERO ) {
										if (playerneromode == 0) {
											// AI Random
											if (test_mode == 1) ai_mossa_random(scacchieraPrinc, NERO, testfile);
											else ai_mossa_random(scacchieraPrinc, NERO, historyfile );
											casellevuote--;
											sprintf(msgtobox, "Mossa NERA via AI Random. Muove il BIANCO ora, fare click sulla scacchiera per avanzare");
											turno++;
										}
										else if (playerneromode == 1) {
											// AI max_presa
											if (test_mode == 1) ai_mossa_presa(scacchieraPrinc, NERO, testfile);
											else ai_mossa_presa(scacchieraPrinc, NERO, historyfile );
											casellevuote--;
											sprintf(msgtobox, "Mossa NERA via AI MAX_PRESA. Muove il BIANCO ora, fare click sulla scacchiera per avanzare");
											turno++;
										}
										else if (playerneromode > 1) {
											// AI MinMax Depth [playerneromode]
											nodo *tempnodo = malloc(sizeof(nodo));
											tempnodo->status = scacchieraPrinc;
											tempnodo->vettoremosse = NULL;
											tempnodo->nfigli = 0;
											tempnodo->nmosse = 0;

											time_t start, end;
											if (minmax_nero_scelta == 1) {
												time(&start);
												ai_nodo_nero = minmax(tempnodo, (turno % 2) +1, playerneromode );
												time(&end);
											}
											else if (minmax_nero_scelta == 2) {
												time(&start);
												ai_nodo_nero = minmax_ab(tempnodo, (turno % 2) +1, playerneromode, 0.0, 0.0);
												time(&end);
											}
											float tempo_passato = difftime(end, start);

											coda_mosse_ai_nero = ai_nodo_nero->nmosse;

											if (coda_mosse_ai_nero > 0) {
												if ( check_mossa_ed_esegui_mat(scacchieraPrinc, ai_nodo_nero->vettoremosse[0]->my, ai_nodo_nero->vettoremosse[0]->mx, (turno % 2) +1) == 1) {
													//printf("Mossa eseguita: Y %d e X %d.\n", ai_nodo_nero->vettoremosse[0]->my, ai_nodo_nero->vettoremosse[0]->mx);
													numeronere++;
													scacchieraPrinc[ai_nodo_nero->vettoremosse[0]->my][ai_nodo_nero->vettoremosse[0]->mx] = (turno % 2) + 1;
													casellevuote--;
													sprintf(msgtobox, "Mossa NERA via AI MINMAX_AB depth %d, calcolata in %.2lf secondi. Muove il BIANCO ora.", playerneromode, tempo_passato);
													if (test_mode == 1) fprintf(testfile, "Mossa NERA via AI MINMAX_AB depth %d, calcolata in %.2lf secondi: Y %d e X %d.\n", playerneromode, tempo_passato, ai_nodo_nero->vettoremosse[0]->my +1, ai_nodo_nero->vettoremosse[0]->mx +1);
													else if (historyattiva == 1) fprintf(historyfile, "Mossa NERA via AI MINMAX_AB depth %d, calcolata in %.2lf secondi: Y %d e X %d.\n", playerneromode, tempo_passato, ai_nodo_nero->vettoremosse[0]->my +1, ai_nodo_nero->vettoremosse[0]->mx +1);
													turno++;
												}
											}
											free(tempnodo);
											tempnodo = NULL;
										}

										if (test_mode != 1) {
											refreshlabelnero = 1;
											refreshlabelbianco = 1;
											refreshscacch = 1;
											refreshmsg = 1;
										}
									}

									else if ( ((turno % 2) +1) == BIANCO ) {
										if (playerbiancomode == 0) {
											// AI Random
											if (test_mode == 1) ai_mossa_random(scacchieraPrinc, BIANCO, testfile);
											else ai_mossa_random(scacchieraPrinc, BIANCO, historyfile );
											casellevuote--;
											sprintf(msgtobox, "Mossa BIANCA via AI Random. Muove il NERO ora, fare click sulla scacchiera per avanzare");
											turno++;
										}
										else if (playerbiancomode == 1) {
											// AI max_presa
											if (test_mode == 1) ai_mossa_presa(scacchieraPrinc, BIANCO, testfile);
											else ai_mossa_presa(scacchieraPrinc, BIANCO, historyfile );
											casellevuote--;
											sprintf(msgtobox, "Mossa BIANCA via AI MAX_PRESA. Muove il NERO ora, fare click sulla scacchiera per avanzare");
											turno++;
										}
										else if (playerbiancomode > 1) {
											// AI MinMax Depth [playerbiancomode]
											nodo *tempnodo = malloc(sizeof(nodo));
											tempnodo->status = scacchieraPrinc;
											tempnodo->vettoremosse = NULL;
											tempnodo->nfigli = 0;
											tempnodo->nmosse = 0;
					
											time_t start, end;
											if (minmax_bianco_scelta == 1) {
												time(&start);
												ai_nodo_bianco = minmax(tempnodo, (turno % 2) +1, playerbiancomode  );
												time(&end);
											}
											else if (minmax_bianco_scelta == 2) {
												time(&start);
												ai_nodo_bianco = minmax_ab(tempnodo, (turno % 2) +1, playerbiancomode, 0.0, 0.0);
												time(&end);
											}
											float tempo_passato = difftime(end, start);
											
											coda_mosse_ai_bianco = ai_nodo_bianco->nmosse;
											if (coda_mosse_ai_bianco > 0) {
												if ( check_mossa_ed_esegui_mat(scacchieraPrinc, ai_nodo_bianco->vettoremosse[0]->my, ai_nodo_bianco->vettoremosse[0]->mx, (turno % 2) +1) == 1) {
													//printf("Mossa eseguita: Y %d e X %d.\n", ai_nodo_bianco->vettoremosse[0]->my, ai_nodo_bianco->vettoremosse[0]->mx);
													scacchieraPrinc[ai_nodo_bianco->vettoremosse[0]->my][ai_nodo_bianco->vettoremosse[0]->mx] = (turno % 2) + 1;
													numerobianche++;
													casellevuote--;
													sprintf(msgtobox, "Mossa BIANCA via AI MINMAX_AB depth %d, calcolata in %.2lf secondi. Muove il NERO ora.", playerbiancomode, tempo_passato);
													if (test_mode == 1) fprintf(testfile, "Mossa BIANCA via AI MINMAX_AB depth %d, calcolata in %.2lf secondi: Y %d e X %d.\n", playerbiancomode, tempo_passato, ai_nodo_bianco->vettoremosse[0]->my +1, ai_nodo_bianco->vettoremosse[0]->mx+1);
													else if (historyattiva == 1) fprintf(historyfile, "Mossa BIANCA via AI MINMAX_AB depth %d, calcolata in %.2lf secondi: Y %d e X %d.\n", playerbiancomode, tempo_passato, ai_nodo_bianco->vettoremosse[0]->my+1, ai_nodo_bianco->vettoremosse[0]->mx+1);
													turno++;
												}
											}
											free(tempnodo);
											tempnodo = NULL;
										}

										if (test_mode != 1) {
											refreshlabelbianco = 1;
											refreshlabelnero = 1;
											refreshscacch = 1;
											refreshmsg = 1;
										}
									}
								}

								else {
									// Click sulla SCACCHIERA ma Partita tra UMANO e CPU o tra 2 UMANI
									// cerco di capire quale casella è stata cliccata
									int casellax = ((myx - scacchx) / 50);
									int casellay = ((myy - scacchy) / 50);
									//sprintf(msgtobox, "Scelta casella RIGA: %d e COLONNA: %d.", casellay, casellax);
									// a seconda di chi doveva muovere come umano, provo ad effettuare la mossa
									// turno NERO e NERO è UMANO
									if (  ((turno % 2) +1) == NERO && playerneromode == -1 &&
											casellax >= 0 && casellax < nrighecolonne &&
											casellay >= 0 && casellay < nrighecolonne) {
										int ret = check_mossa_ed_esegui_mat(scacchieraPrinc, casellay, casellax, NERO);
										if (ret != 1) {
											sprintf(msgtobox, "Mossa NON VALIDA!");
											//printf("Mossa: Y %d e X %d.\n", casellay, casellax);
											refreshmsg = 1;
										}
										else {
											scacchieraPrinc[casellay][casellax] = NERO;
											numeronere++;
											turno++;
											casellevuote--;
											sprintf(msgtobox, "Muove il BIANCO.");
											refreshmsg = 1;
											refreshscacch = 1;

											//printf("Fatto\n");
											//exit(-1);

											if (historyattiva == 1) {
												// Logga
												fprintf(historyfile, "MOSSA: Nero, Riga %d, Colonna %d.\n", casellay+1, casellax+1);
											}
											refreshlabelnero = 1;
											refreshlabelbianco = 1;
										}
									}
									// turno BIANCO e BIANCO è UMANO
									else if (  ((turno % 2) +1) == BIANCO && playerbiancomode == -1 &&
											casellax >= 0 && casellax < nrighecolonne &&
											casellay >= 0 && casellay < nrighecolonne ) {
										int ret = check_mossa_ed_esegui_mat(scacchieraPrinc, casellay, casellax, BIANCO);
										if (ret != 1) {
											sprintf(msgtobox, "Mossa NON VALIDA!");
											//printf("Mossa: Y %d e X %d.\n", casellay, casellax);
											refreshmsg = 1;
										}
										else {
											scacchieraPrinc[casellay][casellax] = BIANCO;
											numerobianche++;
											turno++;
											casellevuote--;
											sprintf(msgtobox, "Muove il NERO.");
											refreshmsg = 1;
											refreshscacch = 1;

											//printf("Fatto\n");
											//exit(-1);

											if (historyattiva == 1) {
												// Logga
												fprintf(historyfile, "MOSSA: Bianco, Riga %d, Colonna %d.\n", casellay+1, casellax+1);
												fprintf(historyfile,"\n");
											}
											refreshlabelbianco = 1;
											refreshlabelnero = 1;
										}
									}
								}
							}

							// FINE parte che gestisce evento del click sulla scacchiera
							else if ( myx > partita_suggerimento.x && myx < (partita_suggerimento.x + partita_suggerimento.w) &&
									myy > partita_suggerimento.y && myy < (partita_suggerimento.y + partita_suggerimento.h) ) {
								// Click sul suggerimento
								richiestasugg = 1;
							}

							else if (myy < partita_esci.y + partita_esci.h) {
								// Probabile click sul menu
								if (myx > partita_save.x && myx < partita_save.x + partita_save.w &&
										myy > partita_save.y && myy < partita_save.y + partita_save.h && (playing == 1) ) {
									//printf("Ho cliccato su SAVE\n");
									// Salvo lo schema sul file othello_scheme.txt, cancellandone prima il contenuto
									if ( write_game_to_file() == 1) {
										sprintf(msgtobox, "SAVE andato a buon fine!");
										refreshmsg = 1;
									}
									else {
										sprintf(msgtobox, "SAVE FALLITO!");
										refreshmsg = 1;
									}
								}
								else if (myx > partita_load.x && myx < partita_load.x + partita_load.w &&
										myy > partita_load.y && myy < partita_load.y + partita_load.h && (playing == 1) ) {
									//printf("Ho cliccato su LOAD\n");
									// Leggo uno schema da file e lo importo
									if ( read_game_from_file() == 1) {
										refreshscacch = 1;
										sprintf(msgtobox, "LOAD andato a buon fine!");
										refreshmsg = 1;
										// Sistemo il logging
										if (historyattiva == 1) {
											historyfile = fopen("othello_history.txt", "w");
											historyattiva = 1;
											fprintf(historyfile, "Inizio log della PARTITA.\n");
											if (playerneromode == -1) fprintf(historyfile,"Player NERO: UMANO.\n");
											else if (playerneromode == 0) fprintf(historyfile, "Player NERO: CPU - AI Random.\n");
											else if (playerneromode == 1) fprintf (historyfile, "Player NERO: CPU - AI MAX_PRESA");
											else fprintf(historyfile, "Player NERO: CPU - AI MinMax depth %d.\n", playerneromode);

											if (playerbiancomode == -1) fprintf(historyfile,"Player BIANCO: UMANO.\n");
											else if (playerbiancomode == 0) fprintf(historyfile, "Player BIANCO: CPU - AI Random.\n");
											else if (playerbiancomode == 1) fprintf(historyfile, "Player BIANCO: CPU - AI MAX_PRESA");
											else fprintf(historyfile, "Player BIANCO: CPU - AI MinMax depth %d.\n", playerbiancomode);
										}
									}
								}
								else if (myx > partita_info.x && myx < partita_info.x + partita_info.w &&
										myy > partita_info.y && myy < partita_info.y + partita_info.h ) {
									printf("Ho cliccato su INFO\n");
									// display delle regole
								}
								else if (myx > partita_history.x && myx < partita_history.x + partita_history.w &&
										myy > partita_history.y && myy < partita_history.y + partita_history.h && (playing == 1) ) {
									//printf("Ho cliccato su HISTORY\n");
									// Attivo il salvataggio su file delle mosse
									historyfile = fopen("othello_history.txt", "w");
									historyattiva = 1;
									fprintf(historyfile, "Inizio log della PARTITA.\n");
									if (playerneromode == -1) fprintf(historyfile,"Player NERO: UMANO.\n");
									else if (playerneromode == 0) fprintf(historyfile, "Player NERO: CPU - AI Random.\n");
									else if (playerneromode == 1) fprintf(historyfile, "Player NERO: CPU - AI MAX_PRESA.\n");
									else fprintf(historyfile, "Player NERO: CPU - AI depth %d.\n", playerneromode);

									if (playerbiancomode == -1) fprintf(historyfile,"Player BIANCO: UMANO.\n");
									else if (playerbiancomode == 0) fprintf(historyfile, "Player BIANCO: CPU - AI Random.\n");
									else if (playerneromode == 1) fprintf(historyfile, "Player BIANCO: CPU - AI MAX_PRESA.\n");
									else fprintf(historyfile, "Player BIANCO: CPU - AI depth %d.\n", playerbiancomode);

									sprintf(msgtobox,"Inizio LOG delle mosse su othello_history.txt");
									refreshmsg = 1;

								}
								else if (myx > partita_restart.x && myx < partita_restart.x + partita_restart.w &&
										myy > partita_restart.y && myy < partita_restart.y + partita_restart.h ) {
									printf("Ho cliccato su RESTART\n");

									if (test_mode ==1) {
										// Premuto restart in modalità test. Non permesso.
										printf("TEST MODE. RESTART NON PERMESSO.\n");
									}
									else {
										
										SDL_FreeSurface(screen);
										SDL_FreeSurface(skybg);

										// ricreo una finestra come quella iniziale.
										int iniziopartita = draw_menu_iniziale(font);

										if (iniziopartita == 1) {
											// come ho fatto all'inizio, se dai settaggi viene richiesto di iniziare una partita, riporto tutto a condizioni inizili

											playing = 1;

											if (historyattiva == 1) {
												historyfile = fopen("othello_history.txt", "w");
												historyattiva = 1;
												fprintf(historyfile, "Inizio log della PARTITA.\n");
												if (playerneromode == -1) fprintf(historyfile,"Player NERO: UMANO.\n");
												else if (playerneromode == 0) fprintf(historyfile, "Player NERO: CPU - AI Random.\n");
												else if (playerneromode == 1) fprintf (historyfile, "Player NERO: CPU - AI MAX_PRESA");
												else fprintf(historyfile, "Player NERO: CPU - AI MinMax depth %d.\n", playerneromode);

												if (playerbiancomode == -1) fprintf(historyfile,"Player BIANCO: UMANO.\n");
												else if (playerbiancomode == 0) fprintf(historyfile, "Player BIANCO: CPU - AI Random.\n");
												else if (playerbiancomode == 1) fprintf(historyfile, "Player BIANCO: CPU - AI MAX_PRESA");
												else fprintf(historyfile, "Player BIANCO: CPU - AI MinMax depth %d.\n", playerbiancomode);
											}

											if (winwidth == 800 && winheight == 600) {
												skybg = GC_LoadImage("skylow.png", &result);
												if ( result == 0) {
													fprintf(stderr, "Impossibile caricare l'immagine skylow.png!\n");
													return(0);
												}
											}
											else if (winwidth == 1024 && winheight == 768) {
												skybg = GC_LoadImage("skyhi.png", &result);
												if ( result == 0) {
													fprintf(stderr, "Impossibile caricare l'immagine skyhi.png!\n");
													return(0);
												}
											}
											
											screen = SDL_SetVideoMode(winwidth, winheight, 32, SDL_HWSURFACE | SDL_DOUBLEBUF );
											if(screen==NULL) {
												fprintf(stderr,"Can't set video mode: %s\n",SDL_GetError());
												SDL_Quit();
												//return ;
											}

											SDL_WM_SetCaption("LMOthello - Partita!",NULL);

											casellevuote = nrighecolonne * nrighecolonne;
											for (i=0; i<nrighecolonne; i++) {
												for (j=0; j<nrighecolonne; j++) {
													scacchieraPrinc[i][j] = VUOTO;
												}
											}

											int startX = (nrighecolonne / 2) -1;
											int startY = (nrighecolonne / 2) -1;
											scacchieraPrinc[startY][startX] = BIANCO;
											scacchieraPrinc[startY][startX + 1] = NERO;
											scacchieraPrinc[startY + 1][startX] = NERO;
											scacchieraPrinc[startY + 1][startX + 1] = BIANCO;
											numerobianche = 2;
											numeronere = 2;
											turno = 0;

											GC_DrawImage(skybg, 0, 0, winwidth, winheight, screen, 0, 0);

											// Devo anche ridisegnare il menu principale
											px = winwidth - 200;
											py = 50;

											GC_DrawText(screen, font, 230, 0, 0, 0, 255, 255, 255, 255, partita_save.entry, px, py, &w, &h, shaded);
											partita_save.x = px;
											partita_save.y = py;
											partita_save.h = h;
											partita_save.w = w;
											py += h + 10;

											GC_DrawText(screen, font, 230, 0, 0, 0, 255, 255, 255, 255, partita_load.entry, px, py, &w, &h, shaded);
											partita_load.x = px;
											partita_load.y = py;
											partita_load.h = h;
											partita_load.w = w;
											py += h + 10;

											GC_DrawText(screen, font, 230, 0, 0, 0, 255, 255, 255, 255, partita_info.entry, px, py, &w, &h, shaded);
											partita_info.x = px;
											partita_info.y = py;
											partita_info.h = h;
											partita_info.w = w;
											py += h + 10;

											GC_DrawText(screen, font, 230, 0, 0, 0, 255, 255, 255, 255, partita_history.entry, px, py, &w, &h, shaded);
											partita_history.x = px;
											partita_history.y = py;
											partita_history.h = h;
											partita_history.w = w;
											py += h + 10;

											GC_DrawText(screen, font, 230, 0, 0, 0, 255, 255, 255, 255, partita_restart.entry, px, py, &w, &h, shaded);
											partita_restart.x = px;
											partita_restart.y = py;
											partita_restart.h = h;
											partita_restart.w = w;
											py += h + 10;

											GC_DrawText(screen, font, 230, 0, 0, 0, 255, 255, 255, 255, partita_esci.entry, px, py, &w, &h, shaded);
											partita_esci.x = px;
											partita_esci.y = py;
											partita_esci.h = h;
											partita_esci.w = w;
											py += h + 10;

											sprintf(msgtobox, "RESTART della partita.");
											refreshmsg = 1;
											refreshscacch = 1;
											refreshlabelnero = 1;
											refreshlabelbianco = 1;

											if (test_mode == 1) {
												if (testfile != NULL) fclose(testfile);
												testfile = fopen("othello_test.txt","w");

												if (playerneromode == -1) playerneromode = 2;
												if (playerbiancomode == -1) playerbiancomode = 2;
											}
										}
										
										else if (iniziopartita == 0) {
											uscita = 1;
										}

									}

								}

								// Ultima possibilità: ho cliccato su esci dalla partita.
								else if (myx > partita_esci.x && myx < partita_esci.x + partita_esci.w &&
										myy > partita_esci.y && myy < partita_esci.y + partita_esci.h ) {
									printf("Ho cliccato su ESCI\n");
									uscita = 1;
								}
							}

						}

					default:
						break;
				}
			}

			if (richiestasugg == 1) {
				// Richiesta di un suggerimento
				sprintf(msgtobox, "Richiesta suggerimento DEPTH %d. Penso...", sugg_depth);
				// Ridisegno il messaggio
				GC_DrawImage(skybg, messx, messy, messw, messh, screen, messx, messy);
				GC_DrawText(screen, font, 255, 255, 255, 0, 0, 0, 0, 255, msgtobox, messx, messy, &messw, &messh, blended);
				refreshmsg = 0;
				SDL_UpdateRect(screen, messx, messy, messw, messh);
				//nodo *tempnodo;
				// AI MinMax Depth [playerneromode]
				nodo *tempnodo = malloc(sizeof(nodo));
				tempnodo->status = scacchieraPrinc;
				tempnodo->vettoremosse = NULL;
				tempnodo->nfigli = 0;
				tempnodo->nmosse = 0;

				// Richiedo un suggerimento per il nero
				if ( ((turno % 2) +1) == NERO ) {
					time_t start, end;
					if (minmax_nero_scelta == 1) {
						time(&start);
						ai_nodo_nero = minmax(tempnodo, (turno % 2) +1, sugg_depth);
						time(&end);
					}
					else if (minmax_nero_scelta == 2) {
						time(&start);
						ai_nodo_nero = minmax_ab(tempnodo, (turno % 2) +1, sugg_depth, 0.0, 0.0);
						time(&end);
					}
					float tempo_passato = difftime(end, start);

					coda_mosse_ai_nero = ai_nodo_nero->nmosse;
					// la prima mossa della lista di mosse trovata viene scritta a schermo nel box usato comunemente
					if (coda_mosse_ai_nero > 0) {
						sprintf(msgtobox, "Suggerimento mossa NERA a depth %d: Y=%d e X=%d, calcolata in %.2lf secondi.", sugg_depth, ai_nodo_nero->vettoremosse[0]->my +1, ai_nodo_nero->vettoremosse[0]->mx +1, tempo_passato);
						refreshmsg = 1;
					}
					else {
						printf("Fallimento AI MINMAX depth %d, richiesta suggerimento. Esco...\n", sugg_depth);
						SDL_Quit();
						exit(-1);
					}
					free(tempnodo);
					tempnodo = NULL;

					//free(ai_nodo_nero);
					//ai_nodo_nero = NULL;
				}
				else if ( ((turno % 2) +1) == BIANCO ) {
					time_t start, end;
					if (minmax_bianco_scelta == 1) {
						time(&start);
						ai_nodo_bianco = minmax(tempnodo, (turno % 2) +1, sugg_depth);
						time(&end);
					}
					else if (minmax_bianco_scelta == 2) {
						time(&start);
						ai_nodo_bianco = minmax_ab(tempnodo, (turno % 2) +1, sugg_depth, 0.0, 0.0);
						time(&end);
					}
					float tempo_passato = difftime(end, start);

					coda_mosse_ai_bianco = ai_nodo_bianco->nmosse;
					if (coda_mosse_ai_bianco > 0) {
						sprintf(msgtobox, "Suggerimento mossa BIANCA a depth %d: Y=%d e X=%d, calcolato in %.2lf secondi.", sugg_depth, ai_nodo_bianco->vettoremosse[0]->my +1, ai_nodo_bianco->vettoremosse[0]->mx +1, tempo_passato);
						refreshmsg = 1;
					}
					else {
						printf("Fallimento AI MINMAX depth %d, richiesta suggerimento. Esco...\n", sugg_depth);
						SDL_Quit();
						exit(-1);
					}
					free(tempnodo);
					tempnodo = NULL;

					//free(ai_nodo_bianco);
					//ai_nodo_bianco = NULL;
				}

				//richiestasugg = 0;
			}

			SDL_Flip(screen);
			if (historyattiva == 1) fflush(historyfile);
			if (test_mode == 1) fflush(testfile);
		}
	}

	// Prima di chiudere:
	// close dei file
	if (historyattiva == 1) fclose(historyfile);
	if (testfile != NULL) fclose(testfile);
	// chiudo i font
	TTF_CloseFont(font);
	// free delle superfici usate
	SDL_FreeSurface(skybg);
	SDL_FreeSurface(imgscacch[0]);
	SDL_FreeSurface(imgscacch[1]);
	SDL_FreeSurface(pedinaB);
	SDL_FreeSurface(pedinaN);
	SDL_FreeSurface(screen);

	SDL_Quit();

	return 1;
}

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <locale.h>
#include "GCGraLib.h"

/* Definizione di costanti */
#define RAGGIO 80
#define DIFFORA 30
#define DIFFMIN 15

/* Funzioni statiche*/
static void disegnaOrologio(void);
static void disegna(void);
static void updateSecondi(int secondo);
static void updateMinuti(int minuto);
static void updateOre(int ora, int minuto);

/* Struttura per le variabili globali */
typedef struct sVarGlob
{
    SDL_Surface *ecran, *points;
    int init;
}
SVarGlob;

/* Struttura Variable globale */
SVarGlob param = { NULL, NULL, 0 };

/* Funzione main */
int main (int argc, char *argv[])
{
SDL_Event event;
int done = 0;

    if (SDL_Init(SDL_INIT_VIDEO) == -1)
    {
        fprintf(stderr, "Errore di inizializzazione di SDL : %s\n", SDL_GetError());
        exit (EXIT_FAILURE);
    }

    SDL_Surface *icone = IMG_Load("orologio.gif");
    if (!icone)
    {
        fprintf(stderr, "Errore nel caricare l'immagine : %s.\n",SDL_GetError());
        exit(EXIT_FAILURE);
    }
    SDL_WM_SetIcon(icone, NULL);
    SDL_FreeSurface(icone);

    param.ecran = SDL_SetVideoMode(200, 200, 32, SDL_HWSURFACE|SDL_DOUBLEBUF);
    if ( param.ecran == NULL )
    {
        fprintf(stderr, "Impossibile aprire una 200x200x8 : %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    param.points = IMG_Load("points.gif");
    if (!param.points)
    {
        fprintf(stderr, "Errore nel caricare l'immagine : %s.\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    SDL_WM_SetCaption("Orologio",NULL);

    while(!done)
    {
        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
                case SDL_QUIT:
                done = 1;
                break;

                default:
                break;
            }
        }
        disegna();
    }

    SDL_Quit();
    return EXIT_SUCCESS;
}

/* Disegna l'orologio; il quadrante e' composto da 12 punti*/
void disegnaOrologio(void)
{
SDL_Rect Pixel;
int conta_ore;
int angolo = 30;  /* angolo in cui sono posizionati i punti sul cerchio/quadrante*/
int trasla = 10;  /* per centrare il quadrante */

    SDL_FillRect(param.ecran, NULL, SDL_MapRGB(param.ecran->format,0,0,0));

/*Il ciclo permette di calcolare la posizione dei punti (trigonometria)*/
    for (conta_ore = 12; conta_ore > 0; conta_ore --)
    {
        Pixel.x = RAGGIO * cos((M_PI * (angolo) / 180) * conta_ore) + RAGGIO
                          + param.points->w + trasla;
        Pixel.y = RAGGIO * sin((M_PI * (angolo) / 180) * conta_ore) + RAGGIO
                          + param.points->h + trasla;
        SDL_BlitSurface(param.points, NULL, param.ecran, &Pixel);
    }
}

/*lancetta dei secondi*/
void updateSecondi(int secondo)
{
SDL_Rect Centro;
SDL_Rect Fine;
Uint32 rosa;
double R, A; /* raggio e angolo */ 

    Centro.x = param.ecran->w/2;
    Centro.y = param.ecran->h/2;

    rosa = SDL_MapRGB(param.ecran->format, 255, 4, 255);

    R = RAGGIO;
    A = (M_PI_2)-secondo*(M_PI)/30;

    /* calcolo della posizione finale della lancetta */
    /* la lancetta consiste in un segmento che parte dal centro del quadrante */
    Fine.x = (int)(cos(A)*R + Centro.x);
    Fine.y = (int)(-sin(A)*R + Centro.y);

    GC_DrawLine(param.ecran, Centro.x, Centro.y, Fine.x, Fine.y, rosa);
}

/*lancetta dei minuti*/
void updateMinuti(int minuto)
{
SDL_Rect Centro;
SDL_Rect Fine;
Uint32 blu;
double R, A;

    Centro.x = param.ecran->w/2;
    Centro.y = param.ecran->h/2;

    blu = SDL_MapRGB(param.ecran->format, 4, 4, 255);

    R = RAGGIO - DIFFMIN;
    A = (M_PI_2)-minuto*(M_PI)/30;
    Fine.x = (int)(cos(A)*R + Centro.x);
    Fine.y = (int)(-sin(A)*R + Centro.y);

    GC_DrawLine(param.ecran,Centro.x, Centro.y, Fine.x, Fine.y, blu);
}

/*lancetta delle ore*/
void updateOre(int ora, int minuto)
{
SDL_Rect Centro;
SDL_Rect Fine;
Uint32 bianco;
double R, AInter;
int min;

    Centro.x = param.ecran->w/2;
    Centro.y = param.ecran->h/2;

    bianco = SDL_MapRGB(param.ecran->format, 255, 255, 255);
    R = RAGGIO-DIFFORA;
    
    /*Interpolazione*/
    min = minuto/60.0;
    
    /*Calcola l'ora attuale*/
    AInter =(M_PI_2)-(ora + min)*(M_PI)/6;
       
    /* contrariamente ai minuti e ai secondi, si fa avanzare la lancetta delle */ 
    /* in modo diverso. Sono movimenti basati sui giri dei minuti */
    Fine.x = (int)(cos(AInter)*R + Centro.x);
    Fine.y = (int)(-sin(AInter)*R + Centro.y);
    
    GC_DrawLine(param.ecran,Centro.x, Centro.y, Fine.x, Fine.y, bianco);
}

void disegna()
{
struct tm *tPre; /* tm e time_t sono predefinite in time.h */
time_t timePresent;

/* legge l'ora attuale*/
    timePresent = time (NULL);
    tPre = localtime(&timePresent);

    disegnaOrologio();
    updateMinuti(tPre->tm_min);
    updateOre(tPre->tm_hour, tPre->tm_min);
    updateSecondi(tPre->tm_sec);
    SDL_Flip(param.ecran);
}

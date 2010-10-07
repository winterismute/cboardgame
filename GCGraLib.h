/*
 * file GCGraLib.h
 * by Giulio Casciola 2008
 */
#ifndef __GCGraLib_H__
#define __GCGraLib_H__

#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_video.h>

enum textquality {solid, shaded, blended};

/* PROGETTO LAURA MICHELA D'ASTORE
   La funzione GC_DrawText ha 2 nuovi parametri, due *int, come 14esimo e 15esimo parametro
   */
extern void GC_DrawText(SDL_Surface *, TTF_Font *, char , char , char , char ,
              char , char , char , char , char* , int , int , int*, int*, enum textquality );

extern void GC_DrawRect(SDL_Surface * ,int ,int ,int ,int , Uint32 );

extern SDL_Surface* GC_LoadImage(char *file, int *exitstate);

extern void GC_DrawImage(SDL_Surface *srcimg, int sx, int sy, int sw, int sh, 
                       SDL_Surface *dstimg, int dx, int dy);

extern Uint32 GC_GetPixelImage(SDL_Surface *image, int x, int y);

extern Uint32 GC_GetPixel(SDL_Surface *surface, int x, int y);

extern void GC_PutPixel(SDL_Surface *surface, int x, int y, Uint32 pixel);

extern void GC_HorizLine(SDL_Surface *s, int x, int x2, int y, Uint32 color);

extern void GC_DrawCircle(SDL_Surface *s, int x1, int y1, int ir, Uint32 color);

extern void GC_FillCircle(SDL_Surface *s, int xin, int yin, int rad, Uint32 color);

extern void GC_DrawLine(SDL_Surface *s, int x0, int y0, int x1, int y1, Uint32 color);

#endif/*__GCGraLib_H__*/

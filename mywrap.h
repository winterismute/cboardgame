
#ifndef __MYWRAP_H__
#define __MYWRAP_H__

#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_video.h>

enum textquality {solid, shaded, blended};

extern void mywrap_DrawText(SDL_Surface *, TTF_Font *, char , char , char , char ,
              char , char , char , char , char* , int , int , int*, int*, enum textquality );

extern SDL_Surface* mywrap_LoadImage(char *file, int *exitstate);

extern void mywrap_DrawImage(SDL_Surface *srcimg, int sx, int sy, int sw, int sh, 
                       SDL_Surface *dstimg, int dx, int dy);

extern Uint32 mywrap_GetPixel(SDL_Surface *surface, int x, int y);

extern void mywrap_PutPixel(SDL_Surface *surface, int x, int y, Uint32 pixel);

#endif/*__MYWRAP_H__*/

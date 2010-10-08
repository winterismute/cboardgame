
#include <string.h>
#include "mywrap.h"

/************************************************************************
mywrap_DrawText: function to draw text using SDL_ttf.
Input:
-(arg.1) pointer to surface blit on;
-(arg.2) pointer to font;
-(arg.3-6) RGBA foreground;
-(arg.7-10) RGBA background;
-(arg.11) string;
-(arg.12-13) posx and posy on the surface, starting from top-left corner
-(arg.14-15) pointer to int which will hold the width and height of text written;
-(arg.16) type of rendering: "solid", "shaded", "blended";
***************************************************************************/
void mywrap_DrawText(SDL_Surface *s, TTF_Font *fonttodraw, char fgR, char fgG, char fgB, char fgA,
              char bgR, char bgG, char bgB, char bgA, char text[], int x, int y, int *rectw, int *recth,
              enum textquality quality)
{
SDL_Color tmpfontcolor = {fgR,fgG,fgB,fgA};
SDL_Color tmpfontbgcolor = {bgR, bgG, bgB, bgA};
SDL_Rect dstrect;
SDL_Surface *resulting_text;

  resulting_text=NULL;
  if (quality == solid)
    resulting_text = TTF_RenderText_Solid(fonttodraw, text, tmpfontcolor);
  else
    if (quality == shaded)
       resulting_text = TTF_RenderText_Shaded(fonttodraw, text, tmpfontcolor, tmpfontbgcolor);
    else
      if (quality == blended)
         resulting_text = TTF_RenderText_Blended(fonttodraw, text, tmpfontcolor);

  if ( resulting_text != NULL ) 
  {
    dstrect.x = x;
    dstrect.y = y;
    dstrect.w = resulting_text->w;
    dstrect.h = resulting_text->h;
    SDL_BlitSurface(resulting_text, NULL, s, &dstrect);
    SDL_UpdateRect(s,0,0,0,0);
  }
  // Patch: ritorno altezza e larghezza del testo, approssimativamente.
  (*rectw) = dstrect.w;
  (*recth) = dstrect.h;
  SDL_FreeSurface(resulting_text);
}


/*******************************************************************************
mywrap_LoadImage: function to load images in PPM, PNG or JPG format from file.
RETURNS SDL_SURFACE* of the image.

Input: 
-(arg.1) file pointer;
-(arg.2) will hold 1 if loading succeded, otherwise 0.

*******************************************************************************/
SDL_Surface* mywrap_LoadImage(char *file, int *exitstate)
{
SDL_Surface *tmp;

 tmp = IMG_Load(file);
 *exitstate = 0;
 if (tmp == NULL)
 {
   fprintf(stderr, "Error: '%s' could not be opened: %s\n", file, IMG_GetError());
 }
 else
 {
//   if(SDL_SetColorKey(tmp, SDL_SRCCOLORKEY | SDL_RLEACCEL, SDL_MapRGB(tmp->format, 255, 0, 255)) == -1)
//     fprintf(stderr, "Warning: colorkey will not be used, reason: %s\n", SDL_GetError());
   *exitstate = 1;
 }
 return tmp;
}

/********************************************************************************
mywrap_DrawImage: function used to blit an image over another one.
Input: 
-(arg.1) src surface pointer;
-(arg.2-3) posx and posy defining the top-left angle IN THE SRC image;
-(arg.4-5) width and height of the SRC image to be drawn;
-(arg.6) dst surface pointer;
-(arg.7-8) posx and posy defining the top-left angle IN THE DST image;
*********************************************************************************/
void mywrap_DrawImage(SDL_Surface *srcimg, int sx, int sy, int sw, int sh, SDL_Surface *dstimg,
               int dx, int dy)
{
SDL_Rect src, dst;

  if (srcimg == NULL)
    return; //If theres no image, or its 100% transparent.

  src.x = sx;  src.y = sy;  src.w = sw;  src.h = sh;
  dst.x = dx;  dst.y = dy;  dst.w = src.w;  dst.h = src.h;
  SDL_BlitSurface(srcimg, &src, dstimg, &dst);
}

/*
static inline void swap(int *a, int *b)
{
  int tmp=*a;
  *a=*b;
  *b=tmp;
}
*/

/********************************************************************
mywrap_GetPixel: function to read a pixel from a surface.
 * Return the pixel value at (x, y)
 * NOTE: The surface must be locked before calling this!
RETURNS color of the pixel.
********************************************************************/
Uint32 mywrap_GetPixel(SDL_Surface *surface, int x, int y)
{
int bpp = surface->format->BytesPerPixel;
/* Here p is the address to the pixel we want to retrieve */
Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

  switch(bpp) {
  case 1:
    return *p;
  case 2:
    return *(Uint16 *)p;
  case 3:
    if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
      return p[0] << 16 | p[1] << 8 | p[2];
    else
//original
      return p[0] | p[1] << 8 | p[2] << 16;
//new
//      printf("getpixel %d %d %d\n",p[2],p[1],p[0]);
      return p[2] | p[1] << 8 | p[0] << 16;
  case 4:
    return *(Uint32 *)p;
  default:
    return 0; /* shouldn't happen, but avoids warnings */
  }
}

/***********************************************************************
mywrap_PutPixel: Set the pixel at (x, y) to the given value.
 * NOTE: The surface must be locked before calling this!
************************************************************************/
void mywrap_PutPixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to set */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    if((x>surface->w)||(y>surface->h)||(x<0)||(y<0)) return;

    switch(bpp) {
    case 1:
        *p = pixel;
        break;

    case 2:
        *(Uint16 *)p = pixel;
        break;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            p[0] = (pixel >> 16) & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = pixel & 0xff;
        } else {
//versione originale
            p[0] = pixel & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = (pixel >> 16) & 0xff;
//printf("%ld\n",pixel);
//printf("%d %d %d\n",p[2],p[1],p[0]);
//versione modificata
//            p[2] = pixel & 0xff;
//            p[1] = (pixel >> 8) & 0xff;
//            p[0] = (pixel >> 16) & 0xff;
        }
        break;

    case 4:
        *(Uint32 *)p = pixel;
        break;
    }
}



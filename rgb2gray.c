/* rgb2gray.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "GCGraLib.h"

#define DEFAULT_PTSIZE  18

typedef struct
{
   int centerx;
   int centery;
   int radius;
   char option[16];
} CIRC;

/* conversione di un intero da base 10 a base b */
void conv(Uint32 m, int b, int a[])
{
int p;

a[0]=a[1]=a[2]=0;
p=-1;
while (m != 0)
  {
    p++;
    a[p]= m % b;
    m= m / b;
  }
}

/* funzione di determinazione opzione del menu' scelta */
void opt_menu(CIRC menu[],int n,int ix,int iy,int *choice)
{
int i;

  *choice=-1;
  for(i=0; i<n; i++)
  {
    if (sqrt(pow((double)ix-menu[i].centerx,2)+pow((double)iy-menu[i].centery,2))<=menu[i].radius)
	    *choice=i;
  }
}

int main()
{
SDL_Surface *screen,*tux;
int res = 0;
SDL_Event event;
TTF_Font *font;
int i,j,pix_int[3],pix[3];
int w,h,color,dw,px,py;
Uint32 pixelval,pixelvalue1,pixelvalue2,pixelvalue3;
int esc=1,choice;
CIRC menu[10];

/* load an image by file */ 
  tux = GC_LoadImage("foto_rgb.ppm",&res);
  if (res == 0)
    return 1;

/* Initialize the graphics session */
  if(SDL_Init(SDL_INIT_VIDEO)<0)
  {
    fprintf(stderr,"Couldn't init video: %s\n",SDL_GetError());
    return(1);
  }

/* Initialize the TTF library */
  if(TTF_Init() < 0)
  {
    fprintf(stderr, "Couldn't initialize TTF: %s\n",SDL_GetError());
    SDL_Quit();
    return(2);
  }

  font = TTF_OpenFont("FreeSans.ttf", DEFAULT_PTSIZE);
  if(font == NULL)
  {
    fprintf(stderr, "Couldn't load font\n");
  }
//  TTF_SetFontStyle(font, TTF_STYLE_NORMAL);

/* create the image window */
  w=tux->w;
  h=tux->h;
  dw=w+150;
//  screen=SDL_SetVideoMode(w+dw,h,32,SDL_SWSURFACE|SDL_ANYFORMAT);
  screen=SDL_SetVideoMode(w+dw,h,24,SDL_HWSURFACE);
  if(screen==NULL)
  {
    fprintf(stderr,"Can't set video mode: %s\n",SDL_GetError());
    SDL_Quit();
    return(2);
  }

/* copy the image into the window */ 
  GC_DrawImage(tux, 0,0, tux->w, tux->h, screen, 0, 0);
  SDL_UpdateRect(screen, 0, 0, 0, 0);
//  for (i=0; i<tux->h; i++)
//  {
//    GC_DrawImage(tux, 0,0, tux->w, i, screen, 0, 0);
//    SDL_UpdateRect(screen, 0, 0, 0, 0);
//  }

//  printf("screen %d\n",screen->format->BytesPerPixel);
//  printf("image %d\n",tux->format->BytesPerPixel);

/* set menu coordinates: center and radius*/
    for (i=0; i<=4; i++)
    {
      menu[i].centerx=w+20;
      menu[i].centery=20+i*30;
      menu[i].radius=10;
    }
    strcpy(menu[0].option, "Negative");
    strcpy(menu[1].option, "Gray");
    strcpy(menu[2].option, "Neg-Gray");
    strcpy(menu[3].option, "B/W");
    strcpy(menu[4].option, "Neg-B/W");
/* draw menu check-points */
    color=SDL_MapRGB(screen->format,240,240,240);
    for (i=0; i<=4; i++)
    {
      GC_FillCircle(screen,menu[i].centerx,menu[i].centery,menu[i].radius,color);
      px=menu[i].centerx+menu[i].radius+5;
      py=menu[i].centery-menu[i].radius;
      GC_DrawText(screen,font,255,255,255,0,0,0,0,0,menu[i].option,px,py,shaded);
    }
    GC_DrawText(screen,font,255,255,255,0,0,0,0,0,"<Esc>",w+30,h-40,shaded);
    SDL_UpdateRect(screen,0,0,0,0);

/* main loop for events */
    do
    {
     if (SDL_PollEvent(&event))
      switch(event.type)
      {
        case SDL_MOUSEBUTTONDOWN:
           if(event.button.button==1)
           {
            opt_menu(menu,5,event.button.x,event.button.y,&choice);
            switch(choice)
            {
	    case 0:
/* transform image to be negative */
	      SDL_LockSurface(screen);
	      pixelvalue3 = 256*256*256;
              for (i=0;i<h;i++)
                 for (j=0;j<w;j++)
                 {
	           pixelvalue1=GC_GetPixelImage(tux,j,i);
	           pixelvalue2=pixelvalue3-pixelvalue1;
                   GC_PutPixel(screen,j+dw,i,pixelvalue2);
                 }
	      SDL_UnlockSurface(screen);
              SDL_UpdateRect(screen, 0, 0, 0, 0);
              break;
            case 1:
/* transform image colors to gray levels */
	      SDL_LockSurface(screen);
              for (i=0;i<h;i++)
                 for (j=0;j<w;j++)
                 {
	           conv(GC_GetPixelImage(tux,j,i),256,pix);
	           pix_int[0]=pix_int[1]=pix_int[2]=(int)((pix[0]+pix[1]+pix[2])/3.0);
                   pixelval=pix_int[2]*256*256+pix_int[1]*256+pix_int[0];
                   GC_PutPixel(screen,j+dw,i,pixelval);
                 }
	      SDL_UnlockSurface(screen);
              SDL_UpdateRect(screen, 0, 0, 0, 0);
              break;
	    case 2:
/* transform image colors to negative-gray levels */
	      SDL_LockSurface(screen);
	      pixelvalue3 = 256*256*256;
              for (i=0;i<h;i++)
                 for (j=0;j<w;j++)
                 {
	           conv(GC_GetPixelImage(tux,j,i),256,pix);
	           pix_int[0]=pix_int[1]=pix_int[2]=(int)((pix[0]+pix[1]+pix[2])/3.0);
                   pixelval=pix_int[2]*256*256+pix_int[1]*256+pix_int[0];
	           pixelvalue2=pixelvalue3-pixelval;
                   GC_PutPixel(screen,j+dw,i,pixelvalue2);
                 }
	      SDL_UnlockSurface(screen);
              SDL_UpdateRect(screen, 0, 0, 0, 0);
              break;
            case 3:
/* transform image colors to Black and White */
	      SDL_LockSurface(screen);
              for (i=0;i<h;i++)
                 for (j=0;j<w;j++)
                 {
	           conv(GC_GetPixelImage(tux,j,i),256,pix);
	           pix_int[0]=(int)((pix[0]+pix[1]+pix[2])/3.0);
	           if(pix_int[0]<128)
		       pix_int[0]=pix_int[1]=pix_int[2]=0;
	           else
		       pix_int[0]=pix_int[1]=pix_int[2]=255;
                   pixelval=pix_int[2]*256*256+pix_int[1]*256+pix_int[0];
                   GC_PutPixel(screen,j+dw,i,pixelval);
                 }
	      SDL_UnlockSurface(screen);
              SDL_UpdateRect(screen, 0, 0, 0, 0);
              break;
            case 4:
/* transform image colors to Negative Black and White */
	      SDL_LockSurface(screen);
              for (i=0;i<h;i++)
                 for (j=0;j<w;j++)
                 {
	           conv(GC_GetPixelImage(tux,j,i),256,pix);
	           pix_int[0]=(int)((pix[0]+pix[1]+pix[2])/3.0);
	           if(pix_int[0]>=128)
		       pix_int[0]=pix_int[1]=pix_int[2]=0;
	           else
		       pix_int[0]=pix_int[1]=pix_int[2]=255;
                   pixelval=pix_int[2]*256*256+pix_int[1]*256+pix_int[0];
                   GC_PutPixel(screen,j+dw,i,pixelval);
                 }
	      SDL_UnlockSurface(screen);
              SDL_UpdateRect(screen, 0, 0, 0, 0);
              break;
	    }
          }
	  break;
	  case SDL_KEYDOWN:
           if(event.key.keysym.sym == SDLK_ESCAPE)
	     esc=0;
	  break;
      }
    }while(esc);

    TTF_CloseFont(font);
    TTF_Quit();
    SDL_FreeSurface(tux);
    SDL_Quit();
    return(0);
}

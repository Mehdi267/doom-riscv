// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
// $Log:$
//
// DESCRIPTION:
//	DOOM graphics stuff for X11, UNIX.
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id: i_x.c,v 1.6 1997/02/03 22:45:10 b1 Exp $";

#include <stdlib.h>
#include <unistd.h>
// #include <sys/ipc.h>
// Had to dig up XShm.c for this one.
// It is in the libXext, but not in the XFree86 headers.
#ifdef LINUX
int XShmGetEventBase( Display* dpy ); // problems with g++?
#endif

#include <stdarg.h>
#include <sys/time.h>
#include <sys/types.h>
// #include <sys/socket.h>

// #include <netinet/in.h>
#include <errno.h>

#include "doomstat.h"
#include "i_system.h"
#include "v_video.h"
#include "m_argv.h"
#include "d_main.h"

#include "doomdef.h"

#define POINTER_WARP_COUNTDOWN	1

// Colormap	X_cmap;
// XEvent		X_event;
typedef struct XImage_personal{
  void* data;
} XImage;
XImage img;
XImage*		image = &img;
typedef struct rgba_pixel {
  unsigned char red;
  unsigned char green;
  unsigned char blue;
  unsigned char alpha;
} rgba_pixel;
rgba_pixel* rgba_pixelmap;
int		X_width;
int		X_height;

typedef struct {
	unsigned long pixel;
	unsigned short red, green, blue;
	char flags;  /* do_red, do_green, do_blue */
	char pad;
} XColor;
static XColor	colors[256];
typedef unsigned int Colormap;


// Blocky mode,
// replace each 320x200 pixel with multiply*multiply pixels.
// According to Dave Taylor, it still is a bonehead thing
// to use ....
static int	multiply=1;


//
//  Translates the key currently in X_event
//

// int xlatekey(void)
// {

//     int rc;

//     switch(rc = XKeycodeToKeysym(X_display, X_event.xkey.keycode, 0))
//     {
//       case XK_Left:	rc = KEY_LEFTARROW;	break;
//       case XK_Right:	rc = KEY_RIGHTARROW;	break;
//       case XK_Down:	rc = KEY_DOWNARROW;	break;
//       case XK_Up:	rc = KEY_UPARROW;	break;
//       case XK_Escape:	rc = KEY_ESCAPE;	break;
//       case XK_Return:	rc = KEY_ENTER;		break;
//       case XK_Tab:	rc = KEY_TAB;		break;
//       case XK_F1:	rc = KEY_F1;		break;
//       case XK_F2:	rc = KEY_F2;		break;
//       case XK_F3:	rc = KEY_F3;		break;
//       case XK_F4:	rc = KEY_F4;		break;
//       case XK_F5:	rc = KEY_F5;		break;
//       case XK_F6:	rc = KEY_F6;		break;
//       case XK_F7:	rc = KEY_F7;		break;
//       case XK_F8:	rc = KEY_F8;		break;
//       case XK_F9:	rc = KEY_F9;		break;
//       case XK_F10:	rc = KEY_F10;		break;
//       case XK_F11:	rc = KEY_F11;		break;
//       case XK_F12:	rc = KEY_F12;		break;
	
//       case XK_BackSpace:
//       case XK_Delete:	rc = KEY_BACKSPACE;	break;

//       case XK_Pause:	rc = KEY_PAUSE;		break;

//       case XK_KP_Equal:
//       case XK_equal:	rc = KEY_EQUALS;	break;

//       case XK_KP_Subtract:
//       case XK_minus:	rc = KEY_MINUS;		break;

//       case XK_Shift_L:
//       case XK_Shift_R:
// 	rc = KEY_RSHIFT;
// 	break;
	
//       case XK_Control_L:
//       case XK_Control_R:
// 	rc = KEY_RCTRL;
// 	break;
	
//       case XK_Alt_L:
//       case XK_Meta_L:
//       case XK_Alt_R:
//       case XK_Meta_R:
// 	rc = KEY_RALT;
// 	break;
	
//       default:
// 	if (rc >= XK_space && rc <= XK_asciitilde)
// 	    rc = rc - XK_space + ' ';
// 	if (rc >= 'A' && rc <= 'Z')
// 	    rc = rc - 'A' + 'a';
// 	break;
//     }

//     return rc;

// }

void I_ShutdownGraphics(void)
{
  image->data = NULL;
}



//
// I_StartFrame
//
void I_StartFrame (void)
{
    // er?

}

static int	lastmousex = 0;
static int	lastmousey = 0;
boolean		mousemoved = false;
boolean		shmFinished;

int first = 1;
int iter = 0;
int second = 0;
void I_GetEvent(void)
{
  iter++;
  if (iter > 1000){ 
  if (first){
    event_t event;
    event.type = ev_keydown;
    event.data1 = KEY_DOWNARROW;
    D_PostEvent(&event);
    event.type = ev_keydown;
    event.data1 = KEY_ENTER;
    D_PostEvent(&event);
    event.type = ev_keydown;
    event.data1 = KEY_ENTER;
    D_PostEvent(&event);
    first = 0;
    second = 1;
  }
  if (second){
    event_t event;
    event.type = ev_keydown;
    event.data1 = KEY_ENTER;
    D_PostEvent(&event);
    second = 0;
  }
  }
}

//
// I_StartTic
//
void I_StartTic (void)
{
  int i = 0;
  while (i == 0){
    i++;
    I_GetEvent();
  }
}

int printed = 1;

void transformToRGBA(XColor palette[], unsigned char* srcMap, struct rgba_pixel* destMap, int width, int height, unsigned char alphaValue) {
  for (int x = 0; x < width; x++) {
    for (int y = 0; y < height; y++) {
      if (x < SCREENWIDTH && y <  SCREENHEIGHT) {
        unsigned char pixelValue = *(srcMap+y*SCREENWIDTH+x);  // Use unsigned char instead of unsigned long
        (destMap+y*SCREENWIDTH+x)->red = (unsigned char) ((int)(palette[pixelValue].red*255)/65535);
        (destMap+y*SCREENWIDTH+x)->green = (unsigned char) ((int)(palette[pixelValue].green*255)/65535);
        (destMap+y*SCREENWIDTH+x)->blue = (unsigned char) ((int)(palette[pixelValue].blue*255)/65535);
        (destMap+y*SCREENWIDTH+x)->alpha = alphaValue;
      }
    }
  }
  printed = 0;
}


//
// I_UpdateNoBlit
//
void I_UpdateNoBlit (void)
{
    // what is this?
}

int printeds = 1;
//
// I_FinishUpdate
//
void I_FinishUpdate (void)
{

    static int	lasttic;
    int		tics;
    int		i;
    // UNUSED static unsigned char *bigscreen=0;

    // draws little dots on the bottom of the screen
    if (devparm)
    {

	i = I_GetTime();
	tics = i - lasttic;
	lasttic = i;
	if (tics > 20) tics = 20;

	for (i=0 ; i<tics*2 ; i+=2)
	    screens[0][ (SCREENHEIGHT-1)*SCREENWIDTH + i] = 0xff;
	for ( ; i<20*2 ; i+=2)
	    screens[0][ (SCREENHEIGHT-1)*SCREENWIDTH + i] = 0x0;
    
    }

    // scales the screen size before blitting it
    if (multiply == 2)
    {
	unsigned int *olineptrs[2];
	unsigned int *ilineptr;
	int x, y, i;
	unsigned int twoopixels;
	unsigned int twomoreopixels;
	unsigned int fouripixels;

	ilineptr = (unsigned int *) (screens[0]);
	for (i=0 ; i<2 ; i++)
	    olineptrs[i] = (unsigned int *) &image->data[i*X_width];

	y = SCREENHEIGHT;
	while (y--)
	{
	    x = SCREENWIDTH;
	    do
	    {
		fouripixels = *ilineptr++;
		twoopixels =	(fouripixels & 0xff000000)
		    |	((fouripixels>>8) & 0xffff00)
		    |	((fouripixels>>16) & 0xff);
		twomoreopixels =	((fouripixels<<16) & 0xff000000)
		    |	((fouripixels<<8) & 0xffff00)
		    |	(fouripixels & 0xff);
#ifdef __BIG_ENDIAN__
		*olineptrs[0]++ = twoopixels;
		*olineptrs[1]++ = twoopixels;
		*olineptrs[0]++ = twomoreopixels;
		*olineptrs[1]++ = twomoreopixels;
#else
		*olineptrs[0]++ = twomoreopixels;
		*olineptrs[1]++ = twomoreopixels;
		*olineptrs[0]++ = twoopixels;
		*olineptrs[1]++ = twoopixels;
#endif
	    } while (x-=4);
	    olineptrs[0] += X_width/4;
	    olineptrs[1] += X_width/4;
	}

    }
    else if (multiply == 3)
    {
	unsigned int *olineptrs[3];
	unsigned int *ilineptr;
	int x, y, i;
	unsigned int fouropixels[3];
	unsigned int fouripixels;

	ilineptr = (unsigned int *) (screens[0]);
	for (i=0 ; i<3 ; i++)
	    olineptrs[i] = (unsigned int *) &image->data[i*X_width];

	y = SCREENHEIGHT;
	while (y--)
	{
	    x = SCREENWIDTH;
	    do
	    {
		fouripixels = *ilineptr++;
		fouropixels[0] = (fouripixels & 0xff000000)
		    |	((fouripixels>>8) & 0xff0000)
		    |	((fouripixels>>16) & 0xffff);
		fouropixels[1] = ((fouripixels<<8) & 0xff000000)
		    |	(fouripixels & 0xffff00)
		    |	((fouripixels>>8) & 0xff);
		fouropixels[2] = ((fouripixels<<16) & 0xffff0000)
		    |	((fouripixels<<8) & 0xff00)
		    |	(fouripixels & 0xff);
#ifdef __BIG_ENDIAN__
		*olineptrs[0]++ = fouropixels[0];
		*olineptrs[1]++ = fouropixels[0];
		*olineptrs[2]++ = fouropixels[0];
		*olineptrs[0]++ = fouropixels[1];
		*olineptrs[1]++ = fouropixels[1];
		*olineptrs[2]++ = fouropixels[1];
		*olineptrs[0]++ = fouropixels[2];
		*olineptrs[1]++ = fouropixels[2];
		*olineptrs[2]++ = fouropixels[2];
#else
		*olineptrs[0]++ = fouropixels[2];
		*olineptrs[1]++ = fouropixels[2];
		*olineptrs[2]++ = fouropixels[2];
		*olineptrs[0]++ = fouropixels[1];
		*olineptrs[1]++ = fouropixels[1];
		*olineptrs[2]++ = fouropixels[1];
		*olineptrs[0]++ = fouropixels[0];
		*olineptrs[1]++ = fouropixels[0];
		*olineptrs[2]++ = fouropixels[0];
#endif
	    } while (x-=4);
	    olineptrs[0] += 2*X_width/4;
	    olineptrs[1] += 2*X_width/4;
	    olineptrs[2] += 2*X_width/4;
	}

    }
    else if (multiply == 4)
    {
	// Broken. Gotta fix this some day.
	void Expand4(unsigned *, int *);
  	Expand4 ((unsigned *)(screens[0]), (int *) (image->data));
  }
	// I_GetEvent();
  transformToRGBA(colors, screens[0], rgba_pixelmap,  SCREENWIDTH, SCREENHEIGHT, 125);
  upd_data_display(rgba_pixelmap, 0, 0, SCREENWIDTH, SCREENHEIGHT); 
}


//
// I_ReadScreen
//
void I_ReadScreen (byte* scr)
{
    memcpy (scr, screens[0], SCREENWIDTH*SCREENHEIGHT);
}


#define DoRed			(1<<0)
#define DoGreen			(1<<1)
#define DoBlue			(1<<2)

//
// Palette stuff.
//
void UploadNewPalette(Colormap cmap, byte *palette)
{

    register int	i;
    register int	c;
    static boolean	firstcall = true;
#ifdef __cplusplus
    if (X_visualinfo.c_class == PseudoColor && X_visualinfo.depth == 8)
#else
    // if (X_visualinfo.class == PseudoColor && X_visualinfo.depth == 8)
#endif
	{
	    // initialize the colormap
	    if (firstcall)
	    {
		firstcall = false;
		for (i=0 ; i<256 ; i++)
		{
		    colors[i].pixel = i;
		    colors[i].flags = DoRed|DoGreen|DoBlue;
		}
	    }

	    // set the X colormap entries
	    for (i=0 ; i<256 ; i++)
	    {
		c = gammatable[usegamma][*palette++];
		colors[i].red = c + (c<<8) ;
		c = gammatable[usegamma][*palette++];
		colors[i].green = c + (c<<8) ;
		c = gammatable[usegamma][*palette++];
		colors[i].blue = c + (c<<8) ;
	    }

	}
}

//
// I_SetPalette
//
void I_SetPalette (byte* palette)
{
    UploadNewPalette(0, palette);
}


void I_InitGraphics(void)
{

    char*		displayname;
    char*		d;
    int			n;
    int			pnum;
    int			x=0;
    int			y=0;
    
    // warning: char format, different type arg
    char		xsign=' ';
    char		ysign=' ';
    
    int			oktodraw;
    unsigned long	attribmask;
    int			valuemask;
    static int		firsttime=1;

    if (!firsttime)
	return;
    firsttime = 0;

  X_width = SCREENWIDTH * multiply;
  X_height = SCREENHEIGHT * multiply;

	// KeyPressMask
	// | KeyReleaseMask
	// // | PointerMotionMask | ButtonPressMask | ButtonReleaseMask
	// | ExposureMask;

  //   attribs.colormap = X_cmap;
  //   attribs.border_pixel = 0;
	
  image->data = (void*) malloc (SCREENWIDTH * SCREENHEIGHT);
	rgba_pixelmap = malloc(sizeof(rgba_pixelmap)*SCREENWIDTH * SCREENHEIGHT); 

}


unsigned	exptable[256];

void InitExpand (void)
{
    int		i;
	
    for (i=0 ; i<256 ; i++)
	exptable[i] = i | (i<<8) | (i<<16) | (i<<24);
}

int		exptable2[256*256];

void InitExpand2 (void)
{
    int		i;
    int		j;
    // UNUSED unsigned	iexp, jexp;
    int*	exp;
    union
    {
	int 		d;
	unsigned	u[2];
    } pixel;
	
    printf ("building exptable2...\n");
    exp = exptable2;
    for (i=0 ; i<256 ; i++)
    {
	pixel.u[0] = i | (i<<8) | (i<<16) | (i<<24);
	for (j=0 ; j<256 ; j++)
	{
	    pixel.u[1] = j | (j<<8) | (j<<16) | (j<<24);
	    *exp++ = pixel.d;
	}
    }
    printf ("done.\n");
}

int	inited;

void
Expand4
( unsigned*	lineptr,
  int*	xline )
{
    int	dpixel;
    unsigned	x;
    unsigned 	y;
    unsigned	fourpixels;
    unsigned	step;
    int*	exp;
	
    exp = exptable2;
    if (!inited)
    {
	inited = 1;
	InitExpand2 ();
    }
		
		
    step = 3*SCREENWIDTH/2;
	
    y = SCREENHEIGHT-1;
    do
    {
	x = SCREENWIDTH;

	do
	{
	    fourpixels = lineptr[0];
			
	    dpixel = *(int *)( (int)exp + ( (fourpixels&0xffff0000)>>13) );
	    xline[0] = dpixel;
	    xline[160] = dpixel;
	    xline[320] = dpixel;
	    xline[480] = dpixel;
			
	    dpixel = *(int *)( (int)exp + ( (fourpixels&0xffff)<<3 ) );
	    xline[1] = dpixel;
	    xline[161] = dpixel;
	    xline[321] = dpixel;
	    xline[481] = dpixel;

	    fourpixels = lineptr[1];
			
	    dpixel = *(int *)( (int)exp + ( (fourpixels&0xffff0000)>>13) );
	    xline[2] = dpixel;
	    xline[162] = dpixel;
	    xline[322] = dpixel;
	    xline[482] = dpixel;
			
	    dpixel = *(int *)( (int)exp + ( (fourpixels&0xffff)<<3 ) );
	    xline[3] = dpixel;
	    xline[163] = dpixel;
	    xline[323] = dpixel;
	    xline[483] = dpixel;

	    fourpixels = lineptr[2];
			
	    dpixel = *(int *)( (int)exp + ( (fourpixels&0xffff0000)>>13) );
	    xline[4] = dpixel;
	    xline[164] = dpixel;
	    xline[324] = dpixel;
	    xline[484] = dpixel;
			
	    dpixel = *(int *)( (int)exp + ( (fourpixels&0xffff)<<3 ) );
	    xline[5] = dpixel;
	    xline[165] = dpixel;
	    xline[325] = dpixel;
	    xline[485] = dpixel;

	    fourpixels = lineptr[3];
			
	    dpixel = *(int *)( (int)exp + ( (fourpixels&0xffff0000)>>13) );
	    xline[6] = dpixel;
	    xline[166] = dpixel;
	    xline[326] = dpixel;
	    xline[486] = dpixel;
			
	    dpixel = *(int *)( (int)exp + ( (fourpixels&0xffff)<<3 ) );
	    xline[7] = dpixel;
	    xline[167] = dpixel;
	    xline[327] = dpixel;
	    xline[487] = dpixel;

	    lineptr+=4;
	    xline+=8;
	} while (x-=16);
	xline += step;
    } while (y--);
}



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
void* input_page; //Used for event handling

typedef struct {
	unsigned long pixel;
	unsigned short red, green, blue;
	char flags;  /* do_red, do_green, do_blue */
	char pad;
} XColor;
static XColor	colors[256];
typedef unsigned int Colormap;

//Used to communicate with the event process
extern void *shm_create(const char*);
extern void *shm_acquire(const char*);
extern void shm_release(const char*);
extern int start(const char *process_name, unsigned long ssize, int prio, void *arg);
extern int scount(int sem);
extern int screate(short count);
extern int sdelete(int sem);
extern int signal(int sem);
extern int signaln(int sem, short count);
extern int sreset(int sem, short count);
extern int try_wait(int sem);
extern int wait(int sem);
extern int getpid(void);
extern int getprio(int pid);

// Blocky mode,
// replace each 320x200 pixel with multiply*multiply pixels.
// According to Dave Taylor, it still is a bonehead thing
// to use ....
static int	multiply=1;

void I_ShutdownGraphics(void)
{
  image->data = NULL;
}



//
// I_StartFrame
//
void I_StartFrame (void){}


typedef struct event_action {
  int event;
  evtype_t press_type;
} event_action;

typedef struct event_com {
  int event;
  evtype_t press_type;
  unsigned char beingused;
  int next_event_id;
  int before_event_id;
  //Don't use this pointer here
  struct event_com* next;
} event_com;

#define MAX_EVENTS 100
#define NO_EVENT	0xff
typedef struct page_struct{
  int write_mutex;
  int nb_events_in;
  int event_head_id;
  int event_tail_id;
  event_com events[MAX_EVENTS+1];
} page_struct;

void get_next_event(event_action* action){
  page_struct* page = (page_struct*) input_page;
  wait(page->write_mutex);
  // // printf("[Doom]Got mutex\n");
  if (page->nb_events_in > 0){
    event_com* ret_event = &page->events[page->event_head_id];
    action->event = ret_event->event;
    action->press_type = ret_event->press_type;
    if (page->event_head_id == page->event_tail_id
        && page->nb_events_in == 1){
      // printf("List is now empty\n");
      page->event_head_id = -1;
      page->event_tail_id = -1;
    } else{
      if (ret_event->before_event_id != -1){
        // printf("ERROR CAUGHT! FIX PLEASE\n");
      }
      page->events[ret_event->next_event_id].before_event_id = 
            ret_event->before_event_id;
      page->event_head_id = ret_event->next_event_id;
    }
    ret_event->beingused = 0;
    page->nb_events_in--;
  }
  else{
    action->event = NO_EVENT;
  }
  signal(page->write_mutex);
  // // printf("[Doom]Release mutex\n");
}

static int	lastmousex = 0;
static int	lastmousey = 0;
boolean		mousemoved = false;
boolean		shmFinished;
int old_event = 0xff;
//Used to detect stand alone events
int iter_event = 0;
void I_GetEvent(void){
  event_action com;
  while(true){
    get_next_event(&com);
    evtype_t event_type = com.press_type;
    int event_pressed = com.event;
    iter_event++;
    if (event_pressed != old_event && 
        event_type != ev_keyup && 
        iter_event > 10 &&
        old_event != NO_EVENT){
      // printf("###");
      // printf("[Doom][Release]Releasing event due to timer = %d\n", old_event);
      event_t event;
      event.type = ev_keyup;
      event.data1 = old_event;
      D_PostEvent(&event);
      iter_event = 0;
      old_event = NO_EVENT;
    }
    if (event_pressed != NO_EVENT){
      // printf("^^^");
      if (com.press_type == ev_keyup &&
          old_event == event_pressed){
        // printf("[Doom][Release]Detected event release= %d\n", old_event);
        iter_event = 0;
        old_event = NO_EVENT;
      } else {
        old_event = event_pressed;
      }
      event_t event;
      event.type = event_type;
      event.data1 = event_pressed;
      D_PostEvent(&event);
      // printf("[Doom][Press]event_pressed = %d\n", event_pressed);
    }else{
      break;
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
	
  image->data = (void*) malloc (SCREENWIDTH * SCREENHEIGHT);
	rgba_pixelmap = malloc(sizeof(rgba_pixelmap)*SCREENWIDTH * SCREENHEIGHT); 
  start("doomevents", 10000, getprio(getpid()) + 1, NULL);
  input_page = shm_acquire("doom_events");
  if (input_page == NULL){
    I_Error("Input page cannot be found\n");
  }
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



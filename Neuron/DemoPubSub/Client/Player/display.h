/*-------------------------------------------------
	Project:	XvdPlayer
	File:		display.h
	Author:		Manjesh Malavalli
	Date:		02/19/2010
-------------------------------------------------*/
//--------------------------------------- INCLUDES ------------------------------------------------

//---------------------------------------- DEFINES ------------------------------------------------
#define MIN_DELAY_BETN_FRMS_MS 	5
#define FF_REFRESH_EVENT 		(SDL_USEREVENT+1)
#define FF_REFRESH_SRCLIST_EVENT (SDL_USEREVENT+2)

#define SCHED_DISP_REFRESH( ptsd, delay_ms )	SDL_AddTimer( delay_ms, refresh_callback, ptsd )
#define GET_REL_TIME_MUS( ref_time )			(av_gettime()-(ref_time))
//---------------------------------------- GLOBALS ------------------------------------------------
extern double		*bandwidth;
extern double		*cpu_usage;
extern SDL_mutex	*mutex;
extern SDL_cond		*cond;
//--------------------------------------- STRUCTURES ----------------------------------------------
typedef struct
{
	int	ovl_type;
	int	pix_fmt;
	int	width;
	int height;
	
	SDL_Surface	*pScreen;
	SDL_Overlay	*pOvl;
	SDL_Rect	dispRect;
	
} SDLDisplay;

typedef struct
{
	GtkWidget	*window;
	GtkWidget	*fixedFrame;
	GtkWidget	*scrollWin;
	GtkWidget	*videoScreen;
	GtkWidget	*bandwidthBar;
	GtkWidget	*bwBarLabel[2];
	GtkWidget	*cpuusageBar;
	GtkWidget	*cpuBarLabel[2];
	GtkWidget	*fpsButtons[3];
	GtkWidget	*srcList;
	
	SDLDisplay	*pSdlDisp;
} NeuronGuiObject;
//---------------------------- Button Press & SDL Refresh Loop FUNCTIONS --------------------------
void button_click( GtkWidget *, gpointer );
//void SDL_refresh_loop( gpointer );
//---------------------------------- Neuron GUI FUNCTIONS -----------------------------------------
void setup_src_list( NeuronGuiObject * );
void neu_gui_open( NeuronGuiObject *, int, int, char * );
//---------------------------------- SDLDisplay FUNCTIONS -----------------------------------------
int SDLDInit( NeuronGuiObject *, int, int, int, int, char * );
int SDLDDispPCQFrame( SDLDisplay *, TSDPtr );
//--------------------------------- DISPLAY THREAD FUNCTIONS --------------------------------------
Uint32 	refresh_callback( Uint32, void * );
int 	update_display_stats( NeuronGuiObject * );
int		refresh_display( NeuronGuiObject *, void * );
//--------------------------------------- MISC FUNCTIONS ------------------------------------------
int img_convert( AVPicture *, int, AVPicture *, int, int, int );


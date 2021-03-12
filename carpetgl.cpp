// Average flag waving thingy, courtesy of me, Red Phoenix aka Roy Massad
// built onto Nehe's framework, thx Nehe <3
// nb: sorry there isn't too much(more like nil) OOP programming going on, 
// but this isn't the place nor time to concern myself with it, 
// it's only a demo for testing purposes, not a full blown 3d engine wannabe

// can u believe i made it without a 2d grid structure?
// but i'm gonna add a 2d array structure to hold uv coordinates for the grid
// because i need to hold to uv/s to apply effects on them, like warping and stuff


// and by the way, the weirdness in the algo is due to the fact that it's
// optimized for speed,haha, or so i think !
// finally: have fun, read it, leave it, i don't care, i'm going to sleep now ;)...

// License: GPL, Date 2005

// ********************************************************************************
// UPDATE: UPDATED in 2021 to work on Visual studio 2019 before uploading to GitHub
// i also removed some curse words and fixed typos in the comments
// ********************************************************************************

// now let's start here
#pragma once
#define MAX_LOADSTRING 100
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define _USE_MATH_DEFINES
#define _WIN32_WINNT 0x0601		// windows 7

// dear base mfc classes
#include "stdafx.h"
// deared CImage class
#include <afxstr.h>
#include <atlimage.h>

#include "resource.h"
// Windows Header Files:
#include <windows.h>
#include <Winuser.h>
// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <math.h>
#include <time.h>
#include <string>
using std::string; 

#include	<stdio.h>							// Header File For Standard Input/Output ( ADD )
#include	<gl\gl.h>							// Header File For The OpenGL32 Library
#include	<gl\glu.h>							// Header File For The GLu32 Library


#include	<Shellapi.h>	// for drag n drop support


// custom globals

const int gx = 70, gz = 35;	// grid sizes

const int lutprecision = gx;
float lut[lutprecision];	 // sine look-up table of 1 hundreds' precision

float amplitude = -0.234f; //= 0.15;		// amplitude factor of the wave
int pos[lutprecision];			  // position in wave
int freq = 4;					// this frequency value is multipied by PI

// precalculated u/v and x/z increments
float udif = 1/float(gx);
float vdif = 1/float(gz);
float xdif = udif * 4;			  // aspect ratio considerations
float zdif = vdif * 2;

int maxfiles = 0;				// max number of files (ellegible to open) (it's zero based[0])
HDROP hDrop;					// contains the list of dropped files
int listpos = 0;				// position inside the list( the one used to select the pic to display)

clock_t start;					//  timer
long mposx, mposy;				// mouse position
GLenum frame = GL_QUADS;		// are we gonna display with wireframes or not ?
bool carpetmode = true;		// keeps tab of magic carpet mode !
bool warpeffect = false;		// this keeps tab of the warp effect
bool pixelate = false;			// for the pixelate effect
bool pause	= false;			// to pause/unpause
bool scroll = false;					// will the images automatically scroll ?

// first create a node structure for the uv array grid
typedef struct
	{
		float u;
		float v;
	} uvnode;

// the create a grid/array of the appropriate size
uvnode uvgrid[gx][gz];


HDC			hDC=NULL;							// Private GDI Device Context
HGLRC		hRC=NULL;							// Permanent Rendering Context
HWND		hWnd=NULL;							// Holds Our Window Handle
HINSTANCE	hInstance;							// Holds The Instance Of The Application

bool		keys[256];							// Array Used For The Keyboard Routine
bool		active = TRUE;						// Window Active Flag
bool		fullscreen = false;					// Fullscreen Flag


GLfloat	xrot=65.0f; //28.5f; //45;								// X Rotation
GLfloat	yrot=0.0f; //91.29f;//60;								// Y Rotation
GLfloat	zrot=0.0f;
GLfloat xspeed;									// X Rotation Speed
GLfloat yspeed;									// Y Rotation Speed
GLfloat	z=-4.89f;//-6f;								// Depth Into The Screen

GLfloat LightAmbient[]= { 1.5f, 1.5f, 1.8f, 1.0f }; 			// Ambient Light Values ( NEW )
GLfloat LightDiffuse[]= { 1.0f, 1.0f, 2.0f, 1.0f };				// Diffuse Light Values ( NEW )
GLfloat LightPosition[]= { 0.0f, 2.0f, 0.0f, 1.0f };			// Light Position ( NEW )

GLuint	filter;									// Which Filter To Use
GLuint	texture[1];								// Storage for 3 textures

BITMAP	BMP;														// Bitmap Structure

LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);				// Declaration For WndProc


// fix found online for glaux lib
bool NeHeLoadBitmap(LPTSTR szFileName, GLuint& texid)					// Creates Texture From A Bitmap File
{
	
	HBITMAP hBMP;														// Handle Of The Bitmap

	glGenTextures(1, &texid);											// Create The Texture
	hBMP = (HBITMAP)LoadImage(GetModuleHandle(NULL), szFileName, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);

	if (!hBMP)															// Does The Bitmap Exist?
		return FALSE;													// If Not Return False

	GetObject(hBMP, sizeof(BMP), &BMP);									// Get The Object
																		// hBMP:        Handle To Graphics Object
																		// sizeof(BMP): Size Of Buffer For Object Information
																		// &BMP:        Buffer For Object Information

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);								// Pixel Storage Mode (Word Alignment / 4 Bytes)

	// Typical Texture Generation Using Data From The Bitmap
	glBindTexture(GL_TEXTURE_2D, texid);								// Bind To The Texture ID
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	// Linear Min Filter
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	// Linear Mag Filter
	glTexImage2D(GL_TEXTURE_2D, 0, 3, BMP.bmWidth, BMP.bmHeight, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, BMP.bmBits);

	DeleteObject(hBMP);													// Delete The Object
	frame = GL_QUADS; // disable wireframe if successful
	return TRUE;														// Loading Was Successful
}


int LoadGLTextures(LPTSTR name)							// Load Bitmaps And Convert To Textures
{

	int Status = FALSE;							// Status Indicator



	if (NeHeLoadBitmap(name, texture[0]))
	{
		Status = TRUE;							// Set The Status To TRUE

		// Create MipMapped Texture with LINEAR FILTERING ALONE !
		glBindTexture(GL_TEXTURE_2D, texture[0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		
		gluBuild2DMipmaps(GL_TEXTURE_2D, 4, BMP.bmWidth, BMP.bmHeight, GL_BITMAP,  GL_UNSIGNED_BYTE,  BMP.bmBits);

	}



	return Status;								// Return The Status
}



// this functions resets the uvgrid , a good thing to remove uv effects
void resetUVgrid(uvnode array[][gz])
{
	// we figure out the true uv for each  by incrementing each position with a small value 
						// the sum of this values starts from 0 to end at 1

	float u,v;		 // temporary variables for holding results
	float flipV;

	// we need to remember the important role that vdiff and udiff play in all this, for they provide us with the right amout of increments to use
	
	v = 0;		// reset v for zero
	// start scanning the grid first from up til down going through rows each time

	for (int z = 0 ; z < gz ; z++)	// note: we won't be skipping any grid points here, contrary to when we draw the gird in glDrawScene
	{		
		u = 0;						// reset u to zero for each new row
        flipV = 1 - v;					// flip v, remember cuz +v is from down to up
		
		for (int x = 0; x < gx ; x++)	// run through the row
		{
			u += udif ;					// adding increments
		
			// then assigning the values to their proper location
			array[x][z].u = u;
			array[x][z].v = flipV;
		}

		v += vdif ;						// after that increment for v
	}

	return;
}

// the pixelate effeft
void pixelateGrid(uvnode array[][gz])
{
	// we figure out the true uv for each  by incrementing each position with a small value 
						// the sum of this values starts from 0 to end at 1

	float u,v;		 // temporary variables for holding results
	float flipV;

	// we need to remember the important role that vdiff and udiff play in all this, for they provide us with the right amout of increments to use
	 
	v = 0;		// reset v for zero
	// start scanning the grid first from up til down going through rows each time
	  
	// not the step+2, bcuz we r only interest in hook to the first vertex of each quad
	for (int z = 0 ; z < gz-2 ; z += 2)	// note: we won't be skipping any grid points here, contrary to when we draw the gird in glDrawScene
	{		
		u = 0;						// reset u to zero for each new row
        flipV = 1 - v;					// flip v, remember cuz +v is from down to up
		
		for (int x = 0; x < gx-2 ; x += 2)	// run through the row
		{
			u += udif*2 ;					// adding increments, note the +2
		
			// this is how we pixelate, by giving the 4 vertex of the quad the same
			// uv coordinates for each one of them
			array[x][z].u = u;
			array[x][z].v = flipV;

			array[x][z+1].u = u;
			array[x][z+1].v = flipV;
			
			array[x+1][z+1].u = u;
			array[x+1][z+1].v = flipV;
			
			array[x+1][z].u = u;
			array[x+1][z].v = flipV;


		}

		v += vdif*2 ;						// after that increment for v
	}

	return;
}

// this is suppose to induce a "warping" effect on the uvgrid but applying random jittering
void warpUVgrid(uvnode array[][gz])
{
				
	// if we want warping to simulate cheap quality water, then we have to reset ever
	// couple of cycles unless we want to end up with a mushed up image
	
	// use this level to decrease the speed of the filter x2
	static bool lever = true;
	
	
	if (lever == false) 
	{
		lever = true;
		return;			// exit without applying filter one time each 2 entries
	}

	lever = false;
    
	
	// for keeping count we to resart the image for the filter
	static int counter = 0;		// use this counter
	counter++;
    if (counter > 3)
	{
		counter = 0;
		resetUVgrid(array);	  // to decide when to reset
	}

	// now let's apply this particular filter to the uvarray
	for (int z = 0 ; z < gz ; z++)	
	{		
		for (int x = 0; x < gx ; x++)	
		{	
			
			float temp;
			
			// check out the small value this will yield
			// it has ten choices which range from 1/5000 to 9/5000
			// good for our purposes, since we want to preserve locality here
			temp = (float((rand() % 10)+ 1) / 4000.0f);  // not the + 1 that avoids us the pain of the diveide by one
			
			// now do a 50/50 dice throw to see if we are gonna add or substart this gittering number
			if ((rand() % 10) > 4) array[x][z].u += temp;
			else array[x][z].u -= temp; 
			
			// now, do the same thing with v
			temp = (float((rand() % 10)+ 1) / 4000.0f); 
	
			if ((rand() % 10) > 4) array[x][z].v += temp;
			else array[x][z].v -= temp; 

		}
	}
	// and voila , finished storing

	// u will note that this filter is heavy on the cpu cycles
	// but then are all the filters, especially interactive ones

	return;
}



// populate array with normalize sine values
void PopulateSine(float* array, int count, int fq)
{
	// choose with PI how many rimples u want to have
	// fq is frequency	// this can wwarp around it's values !!! .009x!@#@#
	float value = 0.095, step = float(M_PI*fq)/float(count);
    float jitter;	 // to spice things up with a little data noise!

	for (int i = 0; i < count ; i++)
	{
		// roll the dice
		 
		jitter = float(float(rand() % 11)-5.0f) / 250.0f;
        array[i] = sin(value) + jitter;	   // jitter it

		 
		jitter = float(float(rand() % 11)-5.0f) / 250.0f;
		value += step + jitter;		  // double the jitter order
		pos[i] = i;			// and setup initial positions
		
		jitter=0;
	}

		
	return;
}


GLvoid ReSizeGLScene(GLsizei width, GLsizei height)				// Resize And Initialize The GL Window
{
	if (height==0)								// Prevent A Divide By Zero By
	{
		height=1;								// Making Height Equal One
	}

	glViewport(0, 0, width, height);	

	glMatrixMode(GL_PROJECTION);				// Select The Projection Matrix
	glLoadIdentity();							// Reset The Projection Matrix

	// Calculate The Aspect Ratio Of The Window
	gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);

	glMatrixMode(GL_MODELVIEW);					// Select The Modelview Matrix
	glLoadIdentity();							// Reset The Modelview Matrix
}

int InitGL(GLvoid)								// All Setup For OpenGL Goes Here
{
	// Allocate enough memory for ONE textures throughout the lifetime of the app
	glGenTextures(1, &texture[0]);				

	// set to wireframe b4 attempting to open the default picture, in case of image missing=> wireframe is more aestheticly pleasing to view
	frame = GL_LINE_LOOP;
	// load default windows wallpaper as default image( if present)

	LPTSTR s = "open.bmp";
	
	if (!LoadGLTextures(s))	
	{
		return FALSE;							// If Texture Didn't Load Return FALSE
	}

	glEnable(GL_TEXTURE_2D);					// Enable Texture Mapping
	glShadeModel(GL_SMOOTH);					// Enable Smooth Shading
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);					// Black Background
	glClearDepth(1.0f);							// Depth Buffer Setup
	//glEnable(GL_DEPTH_TEST);					// Enables Depth Testing
	//glDepthFunc(GL_LEQUAL);						// The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);			// Really Nice Perspective Calculations

	glBlendFunc(GL_SRC_ALPHA,GL_ONE);			// Set The Blending Function For Translucency
	glEnable(GL_BLEND);	

	glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient);				// Setup The Ambient Light
	glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse);				// Setup The Diffuse Light
	glLightfv(GL_LIGHT1, GL_POSITION,LightPosition);			// Position The Light
	glEnable(GL_LIGHT1);						// Enable Light One
	
	glEnable(GL_LIGHTING);
	return TRUE;								// Initialization Went OK
}

// remember, magic carpet mode mix two waves, vertival and horizontal
int DrawGLScene(GLvoid)									// Here's Where We Do All The Drawing
{
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear The Screen And The Depth Buffer
	glLoadIdentity();
	glTranslatef(0.0f,0.0f,z);							// Translate Into/Out Of The Screen By z

	glRotatef(xrot,1.0f,0.0f,0.0f);						// Rotate On The X Axis By xrot
	glRotatef(yrot,0.0f,1.0f,0.0f);						// Rotate On The Y Axis By yrot
	glRotatef(zrot,0.0f,0.0f,1.0f);						// Rotate On The Z Axis By yrot


	glBindTexture(GL_TEXTURE_2D, texture[0]);		// Select A Texture Based On filter

	// apply warp effect ?
	if (warpeffect) warpUVgrid(uvgrid);

	//test for wireframe or normal quad options
	if (frame == GL_LINE_LOOP) glDisable(GL_TEXTURE_2D);
	if (frame == GL_QUADS) glEnable(GL_TEXTURE_2D);									

	glNormal3f( 0.0f, 1.0f, 0.0f);					
		
		// numb of x/y quads in grid is equal x-1 and y-1 vertices
		float xx = 0,zz = 0;	// holds actual pos and mapping coordinates for each vertex
				
		float ZplusDif = 0, XplusDif = 0;  // and their no brainer complements, these vars aren't necessary, but they r easier to read than inline calculations
		
		float waveStart, waveEnd; // start of the wave til it's end // horizontal
		float waveStart2 = 0.0f, waveEnd2 = 0.0f; // reset the vertical ones in case we don't use them
		float averager = 1;	// of the two waves // default only for 1 wave

		// the end fo the previous wave must hook with the start of the current wave
		      
		// ps: kids it's sinful to overshadow the z variable for trivials reasons, like for a loop counter, so whatever u do in life, don't do what i'm about to do now, name ur variables reasonably
		
		// remember we are gonna draw the grid in increments like we did in resetUvgrid()
		zz = 0;					// reset z cursor
		for (int z = 1 ; z < gz-1 ; z++)	// run through grid matrix   starting from 1 and not from 0 because we are gonna use z-1 and x-1 soon
		{														// the end results is that the edges of the "grid" are never displayed but always calculated...
			
			ZplusDif = zz + zdif;	//  calculate  zz complement
			// vertical wave on the z axis //uses only half of the look up table
			if (carpetmode == true)	  // check first if mode is enabled
			{
				waveEnd2 = lut[pos[z]] * amplitude;		//+ amplitude;
				waveStart2 = lut[pos[z-1]] * amplitude;
				
				averager = 2;
			}

			xx = 0;						   // reset the xx counter
			for (int x = 1; x < gx-1 ; x++)
			{
				  
				xx += xdif ;	 // add porper increment
				
				// pin and nail using increments
				// this quad is drawn counter clock wise
				
				// "precalcutate" the necessasry complements
				XplusDif = xx + xdif;
			
				// horizontal wave on the X axis
				waveEnd = lut[pos[x]] * amplitude;		//+ amplitude;
				waveStart = lut[pos[x-1]] * amplitude;
										
				// just checking out how excessive triangulation looks like ! hehe
				//glDisable(GL_TEXTURE_2D);
				//glPolygonMode(GL_FRONT, GL_LINE);
				//glPolygonMode(GL_BACK, GL_LINE);
				//frame = GL_TRIANGLE_STRIP;

				
				// note how the mapping of heights using the average of the two 
				// waves is used to make the guads "click" together on all edges
				// ps: this order is the only correct one since  each quad
				// is pinned Counter Clock wise starting from upper/left.
				// , the horizontal wave comes from left to right => WS then WE
				// , the vertical wave comes from top to bottom => WS2 then WE2
				// now imagine where each vertex of the quad lies, on the criss
				// crossing of any two waves, as in the vertex of the upper left
				// need the average of both starting waves, hence the next line.
				
				glBegin(frame);
				// plz note that translations are taking place(-2,1), precalculated variables
				// might be better here for speed, but in this case i believe that they will
				// clutter up the comprehension of this section to an unacceptable point
				// so switch them at ur own will
				glTexCoord2f(uvgrid[x][z].u, uvgrid[x][z].v);
				glVertex3f(xx - 2, (waveStart + waveStart2)/averager, zz - 1);	
				
				glTexCoord2f(uvgrid[x][z+1].u, uvgrid[x][z+1].v);
				glVertex3f(xx - 2, (waveStart + waveEnd2)/averager, ZplusDif - 1);

				glTexCoord2f(uvgrid[x+1][z+1].u, uvgrid[x+1][z+1].v);
				glVertex3f(XplusDif - 2, (waveEnd + waveEnd2)/averager, ZplusDif - 1);	

				glTexCoord2f(uvgrid[x+1][z].u, uvgrid[x+1][z].v);
				glVertex3f(XplusDif - 2, (waveEnd + waveStart2)/averager, zz - 1);	
				// this was heavy on the math with the divisions, but hey
				// at least we ain't using no sin trigonomitry	!!!
				glEnd();	
			
			}
		   	zz += zdif;				// increment z cursor
			
		}
						

	// advance all wave position in this column by one ( using pos array)
	if (pause == false)				// and only if unpaused
	{
		for (int x = 0; x < gx-1 ; x++)
		{
			pos[x] += 1;				
			if (pos[x] > lutprecision) pos[x] = 0;	// wrap if need be
		}
	}

	// amplify rotational increments +
	xrot+=xspeed;								// Add xspeed To xrot
	yrot+=yspeed;								// Add yspeed To yrot
	
	// if automaticall scrolling is on, tilt a bit back and forth on the z axis
	if (scroll == true)
	{
		// we will bounce lineary back and forth from -8 to 8, starting from -7,-6 ...
		static float zinc = -7.0f;		// Z increment, not the zinc chemical element
		static float sign = 0.3f;	// this is the elementery incremental unit
		
		zinc+=sign;
		if (abs(zinc) > 8.0f)  {sign *= -1.0f; zinc+=sign;}		// backtrack by flipping the sign 

		zrot = zinc;	// finally assign the value
	}
	else zrot = 0.0f;
	
	// slowly neutralize motional inertia
	xspeed /= 1.3f;
	yspeed /= 1.3f;
	
	// restart clock here
	start = clock();
	return TRUE;								// Keep Going
}

GLvoid KillGLWindow(GLvoid)						// Properly Kill The Window
{

	if (fullscreen)								// Are We In Fullscreen Mode?
	{	
		ChangeDisplaySettings(NULL,0);			// If So Switch Back To The Desktop
		ShowCursor(TRUE);						// Show Mouse Pointer
	}

	if (hRC)									// Do We Have A Rendering Context?
	{
		if (!wglMakeCurrent(NULL,NULL))			// Are We Able To Release The DC And RC Contexts?
			{
				MessageBox(NULL,"Release Of DC And RC Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
			}

		if (!wglDeleteContext(hRC))				// Are We Able To Delete The RC?
			{
				MessageBox(NULL,"Release Rendering Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
			}
		
			hRC=NULL;							// Set RC To NULL
	}

	if (hDC && !ReleaseDC(hWnd,hDC))			// Are We Able To Release The DC
	{
		MessageBox(NULL,"Release Device Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hDC=NULL;								// Set DC To NULL
	}
	
	if (hWnd && !DestroyWindow(hWnd))			// Are We Able To Destroy The Window?
	{
		MessageBox(NULL,"Could Not Release hWnd.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hWnd=NULL;								// Set hWnd To NULL
	}

	if (!UnregisterClass("OpenGL",hInstance))	// Are We Able To Unregister Class
	{
		MessageBox(NULL,"Could Not Unregister Class.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hInstance=NULL;							// Set hInstance To NULL
	}
}

BOOL CreateGLWindow(char* title, int width, int height, int bits, bool fullscreenflag)
{

	GLuint		PixelFormat;					// Holds The Results After Searching For A Match
	WNDCLASS	wc;								// Windows Class Structure
	DWORD		dwExStyle;						// Window Extended Style
	DWORD		dwStyle;						// Window Style

	RECT WindowRect;							// Grabs Rectangle Upper Left / Lower Right Values
	WindowRect.left=(long)0;						// Set Left Value To 0
	WindowRect.right=(long)width;					// Set Right Value To Requested Width
	WindowRect.top=(long)0;							// Set Top Value To 0
	WindowRect.bottom=(long)height;					// Set Bottom Value To Requested Height
	
	fullscreen=fullscreenflag;						// Set The Global Fullscreen Flag

	hInstance		= GetModuleHandle(NULL);			// Grab An Instance For Our Window
	wc.style		= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Move, And Own DC For Window
	wc.lpfnWndProc		= (WNDPROC) WndProc;				// WndProc Handles Messages
	wc.cbClsExtra		= 0;						// No Extra Window Data
	wc.cbWndExtra		= 0;						// No Extra Window Data
	wc.hInstance		= hInstance;					// Set The Instance
	wc.hIcon		= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));			// Load The Default Icon
	wc.hCursor		= LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
	wc.hbrBackground	= NULL;						// No Background Required For GL
	wc.lpszMenuName		= NULL;						// We Don't Want A Menu
	wc.lpszClassName	= "OpenGL";					// Set The Class Name

	if (!RegisterClass(&wc))						// Attempt To Register The Window Class
	{
		MessageBox(NULL,"Failed To Register The Window Class.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;							// Exit And Return FALSE
	}

	if (fullscreen)								// Attempt Fullscreen Mode?
	{
		DEVMODE dmScreenSettings;				// Device Mode
		memset(&dmScreenSettings,0,sizeof(dmScreenSettings));	// Makes Sure Memory's Cleared
		dmScreenSettings.dmSize=sizeof(dmScreenSettings);		// Size Of The Devmode Structure
		dmScreenSettings.dmPelsWidth	= width;			// Selected Screen Width
		dmScreenSettings.dmPelsHeight	= height;			// Selected Screen Height
		dmScreenSettings.dmBitsPerPel	= bits;				// Selected Bits Per Pixel
		dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

		// Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
		if (ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL)
		{
			// If The Mode Fails, Offer Two Options.  Quit Or Run In A Window.
			if (MessageBox(NULL,"The Requested Fullscreen Mode Is Not Supported By\nYour Video Card. Use Windowed Mode Instead?","GL",MB_YESNO|MB_ICONEXCLAMATION)==IDYES)
			{
					fullscreen=FALSE;				// Select Windowed Mode (Fullscreen=FALSE)
			}
			else
			{
			// Pop Up A Message Box Letting User Know The Program Is Closing.
				MessageBox(NULL,"Program Will Now Close.","ERROR",MB_OK|MB_ICONSTOP);
				return FALSE;					// Exit And Return FALSE
			}
		}
	}

	if (fullscreen)								// Are We Still In Fullscreen Mode?
	{
		dwExStyle=WS_EX_APPWINDOW;				// Window Extended Style
		dwStyle=WS_POPUP;						// Windows Style
		ShowCursor(FALSE);						// Hide Mouse Pointer
	}
	else
	{
		dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;		// Window Extended Style
		dwStyle=WS_OVERLAPPEDWINDOW;					// Windows Style
	}
	
	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);		// Adjust Window To True Requested Size

	// always on top
	if (!(hWnd=CreateWindowEx(	dwExStyle | WS_EX_TOPMOST,				// Extended Style For The Window
					"OpenGL",				// Class Name
					title,					// Window Title
					WS_CLIPSIBLINGS |		// Required Window Style
					WS_CLIPCHILDREN |		// Required Window Style
					dwStyle,				// Selected Window Style
					100, 100,					// Window Position
					WindowRect.right-WindowRect.left,	// Calculate Adjusted Window Width
					WindowRect.bottom-WindowRect.top,	// Calculate Adjusted Window Height
					NULL,					// No Parent Window
					NULL,					// No Menu
					hInstance,				// Instance
					NULL)))					// Don't Pass Anything To WM_CREATE
	{
		KillGLWindow();							// Reset The Display
		MessageBox(NULL,"Window Creation Error.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;							// Return FALSE
	}

	static	PIXELFORMATDESCRIPTOR pfd=	// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),	// Size Of This Pixel Format Descriptor
		1,								// Version Number
		PFD_DRAW_TO_WINDOW |			// Format Must Support Window
		PFD_SUPPORT_OPENGL |			// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,				// Must Support Double Buffering
		PFD_TYPE_RGBA,					// Request An RGBA Format
		bits,							// Select Our Color Depth
		0, 0, 0, 0, 0, 0,				// Color Bits Ignored
		0,								// No Alpha Buffer
		0,								// Shift Bit Ignored
		0,								// No Accumulation Buffer
		0, 0, 0, 0,						// Accumulation Bits Ignored
		16,								// 16Bit Z-Buffer (Depth Buffer)
		0,								// No Stencil Buffer
		0,								// No Auxiliary Buffer
		PFD_MAIN_PLANE,					// Main Drawing Layer
		0,								// Reserved
		0, 0, 0							// Layer Masks Ignored
	};

	if (!(hDC=GetDC(hWnd)))						// Did We Get A Device Context?
	{
		KillGLWindow();							// Reset The Display
		MessageBox(NULL,"Can't Create A GL Device Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;							// Return FALSE
	}

	if (!(PixelFormat=ChoosePixelFormat(hDC,&pfd)))		// Did Windows Find A Matching Pixel Format?
	{
		KillGLWindow();							// Reset The Display
		MessageBox(NULL,"Can't Find A Suitable PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;							// Return FALSE
	}

	if(!SetPixelFormat(hDC,PixelFormat,&pfd))	// Are We Able To Set The Pixel Format?
	{
		KillGLWindow();							// Reset The Display
		MessageBox(NULL,"Can't Set The PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;							// Return FALSE
	}

	if (!(hRC=wglCreateContext(hDC)))			// Are We Able To Get A Rendering Context?
	{
		KillGLWindow();							// Reset The Display
		MessageBox(NULL,"Can't Create A GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;							// Return FALSE
	}

	if(!wglMakeCurrent(hDC,hRC))				// Try To Activate The Rendering Context
	{
		KillGLWindow();							// Reset The Display
		MessageBox(NULL,"Can't Activate The GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;							// Return FALSE
	}

	ShowWindow(hWnd,SW_SHOW);					// Show The Window
	SetForegroundWindow(hWnd);					// Slightly Higher Priority
	SetFocus(hWnd);								// Sets Keyboard Focus To The Window
	ReSizeGLScene(width, height);				// Set Up Our Perspective GL Screen

	if (!InitGL())								// Initialize Our Newly Created GL Window
	{
		KillGLWindow();							// Reset The Display
		MessageBox(NULL,"Initialization Failed.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;							// Return FALSE
	}
		
		// enable drap n drop for this window
		DragAcceptFiles(hWnd,true);
		return TRUE;							// Success
}

LRESULT CALLBACK WndProc(	HWND	hWnd,		// Handle For This Window
				UINT	uMsg,					// Message For This Window
				WPARAM	wParam,					// Additional Message Information
				LPARAM	lParam)					// Additional Message Information
{

	switch (uMsg)								// Check For Windows Messages
	{

	case WM_ACTIVATE:							// Watch For Window Activate Message
		{
			if (!HIWORD(wParam))				// Check Minimization State
			{
				active=TRUE;					// Program Is Active
			}
			else
			{
				active=FALSE;					// Program Is No Longer Active
			}

			return 0;							// Return To The Message Loop
		}


	case WM_SYSCOMMAND:							// Intercept System Commands
		{
			switch (wParam)						// Check System Calls
			{
				case SC_SCREENSAVE:				// Screensaver Trying To Start?
				case SC_MONITORPOWER:			// Monitor Trying To Enter Powersave?
				return 0;					// Prevent From Happening
			}
			break;							// Exit
		}

	case WM_CLOSE:							// Did We Receive A Close Message?
		{
			PostQuitMessage(0);				// Send A Quit Message
			return 0;						// Jump Back
		}

	case WM_KEYDOWN:						// Is A Key Being Held Down?
		{
			keys[wParam] = TRUE;			// If So, Mark It As TRUE
			return 0;						// Jump Back
		}

	case WM_KEYUP:							// Has A Key Been Released?
		{
			keys[wParam] = FALSE;			// If So, Mark It As FALSE
			return 0;						// Jump Back
		}

	case WM_SIZE:							// Resize The OpenGL Window
		{
			ReSizeGLScene(LOWORD(lParam),HIWORD(lParam));		// LoWord=Width, HiWord=Height
			return 0;						// Jump Back
		}
	
	case WM_DROPFILES:	  // handles dropping .bmp files inside this window
		{
			
			char buf[200];
			
			hDrop = (HDROP) wParam;
			maxfiles = DragQueryFile(hDrop,0xFFFFFFFF,buf,200);
			DragQueryFile(hDrop,0,buf,200);

			//MessageBox(NULL, buf, "DEBUG", MB_OK);	   //debug
			LoadGLTextures(buf);	   // load the new texture

			return 0;
		}

	case WM_LBUTTONDOWN:
		{
			// hook original position
			mposx = LOWORD (lParam) ;
			mposy = HIWORD (lParam) ;
			return 0 ;
		}
          
    case WM_MOUSEMOVE:
		{
          
            // my very own mouse orbiter !!!! //crappy edition
			if (wParam & MK_LBUTTON)
			{
				// calculate delta
				int dx,dy;
				
				// counter is useful to reversing or backtracking mouse motion 
				// on a certain line

				// every coouple of tics or so we drop a new ancher origin 
				// so we can backtrack

				// try it, strafe the mouse from left to right without letting go
				// of the mouse button

				static int counter;
				counter++;
				if (counter>30) 
				{
					counter = 0 ;	
					mposx = LOWORD (lParam) ; mposy = HIWORD (lParam) ;
				}

				dx = mposx - LOWORD (lParam);
				dy = mposy - HIWORD (lParam);
				
				// calculate the diff of delta with the original position
				// once u got the diff2, take into consideration both 
				// the mouse buffer indentation(~=15) and delta priority( dx>dy and vice versa)
				
				if ((dx > 0) && (dx > abs(dy)) && (dx > 15))   { yspeed += 1.5f ; }	 
				if ((dx < 0) && (abs(dx) > abs(dy)) && (dx < -15)) { yspeed -= 1.5f ; }

				if ((dy > 0) && (dy > abs(dx)) && (dy > 15)) { xspeed -= 1.5f ; }
				if ((dy < 0) && (abs(dy) > abs(dx)) && (dy < -15)) { xspeed += 1.5f ; }

			}   
            
			
			if (wParam & MK_RBUTTON)	 // same as above, diff functionality
			{
				int dx,dy;
			
				static int counter2;
				counter2++;
				if (counter2>30) 
				{
					counter2 = 0 ;	
					mposx = LOWORD (lParam) ; mposy = HIWORD (lParam) ;
				}

				dx = mposx - LOWORD (lParam);
				dy = mposy - HIWORD (lParam);
				
				if ((dx > 0) && (dx > abs(dy)) && (dx > 15))   { z += .05f ; break;}	 
				if ((dx < 0) && (abs(dx) > abs(dy)) && (dx < -15)) { z -= .05f ; break;}

				if ((dy > 0) && (dy > abs(dx)) && (dy > 15)) { amplitude -= .005f ; break;}
				if ((dy < 0) && (abs(dy) > abs(dx)) && (dy < -15)) { amplitude += .005f ; break;}

			}   
			
			return 0 ;
        }
          
	
	
	}

		// Pass All Unhandled Messages To DefWindowProc
	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

int WINAPI WinMain(	HINSTANCE	hInstance,				// Instance
			HINSTANCE	hPrevInstance,				// Previous Instance
			LPSTR		lpCmdLine,				// Command Line Parameters
			int		nCmdShow)				// Window Show State
{

	MSG	msg;									// Windows Message Structure
	BOOL	done=FALSE;							// Bool Variable To Exit Loop

	srand( (unsigned)time( NULL ) );			// seed the rand generate with time ...
	
	// populate sine 
	/////////////////////////////////////////
	PopulateSine(lut,lutprecision,freq);
	///////////
	resetUVgrid(uvgrid);	// initialize UV grid with default coordinates;

	start = clock();	  // initialize first timer
	static clock_t imagetimer;			// the timer for scrolling images
	/////////////////////////////////////////

    	
	// Ask The User Which Screen Mode They Prefer
	//if (MessageBox(NULL,"Would You Like To Run In Fullscreen Mode?", "Start FullScreen?",MB_YESNO|MB_ICONQUESTION)==IDNO)
	//{
	//	fullscreen=FALSE;						// Windowed Mode
	//}
	
	// Create Our OpenGL Window
	if (!CreateGLWindow("MagicCarpet, the OpenGl image browser v0.9!! PRESS F1 for HELP",1080,800,32,fullscreen))
	{
		return 0;								// Quit If Window Was Not Created
	}

	
	while(!done)								// Loop That Runs Until done=TRUE
	{
		
		if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))			// Is There A Message Waiting?
		{
			if (msg.message==WM_QUIT)			// Have We Received A Quit Message?
			{
				done=TRUE;						// If So done=TRUE
			}
			else								// If Not, Deal With Window Messages
			{
				TranslateMessage(&msg);			// Translate The Message
				DispatchMessage(&msg);			// Dispatch The Message
			}
		}
											// If There Are No Messages
		
			// Draw The Scene.  Watch For ESC Key And Quit Messages From DrawGLScene()
			if (active)							// Program Active?
			{
				if (keys[VK_ESCAPE])			// Was ESC Pressed?
				{
					done=TRUE;					// ESC Signalled A Quit
				}
				else							// Not Time To Quit, Update Screen
				{
								
					if ((clock() - start) > 30)   // timer control b4 drawing, 31 seems like a decent estimate => 1000/33 gives a stable 333 fps
					{ 
						DrawGLScene();				// Draw The Scene
						SwapBuffers(hDC);			// Swap Buffers (Double Buffering)
					}
					
							
				
				// is scrolling enabled? and more than 1 file got dragged into the queue ?
				// also check that enough time has passed
				if ((scroll && maxfiles > 0)   && ((clock() - imagetimer) > 5000))
				{
					listpos++;					// move one step to the right
					if (listpos > maxfiles-1)	listpos = 0;	// if out of bound, wrap to the left side
					char buf[200];
				
					DragQueryFile(hDrop,listpos,buf,200);

					LoadGLTextures(buf);	   // load the new texture
					imagetimer = clock();		//reset the timer for scrolling
				}
				
				
				if (keys[VK_ADD])					 // add frequency
				{
					freq += 2;
					PopulateSine(lut,lutprecision,freq);   // recreate look up table
					keys[VK_ADD] = FALSE;
				}

				if (keys[VK_SUBTRACT])					 // invierse dito
				{
					freq -= 2;
					PopulateSine(lut,lutprecision,freq);
					keys[VK_SUBTRACT] = FALSE;
				}

				if (keys['W'])				// wireframe toggle
				{
					if (frame == GL_QUADS) frame = GL_LINE_LOOP;
					else if (frame == GL_LINE_LOOP) frame = GL_QUADS;
					keys['W'] = false;
				}

				if (keys['M'])				// magic carpet toggle
				{
					carpetmode = !carpetmode;
			
					keys['M'] = false;
				}

				if (keys[VK_F1])			// help	
				{
					ShowWindow(hWnd,SW_SHOWMINIMIZED);
					MessageBox(NULL,"Drag any 24bit/high-color bitmap pictures onto the window to make them the new selection,\\
								\nPress RETURN to start/stop automatic scrolling of images (5s/image),\\
								\nPress LEFT and RIGHT to scroll through the list of selected images,\\
								\nPress and hold LMB then move in any direction to rotate the pic,\\
								\nPress and hold RMB then move vertically to zoom,\\
								\nPress and hold RMB then move horizontally to change wave amplitude,\\
								\nPress + or - on the numpade to affect frequency of waves,\\
								\nPress W to toggle wireframe mode,\\
								\nPress M to toggle magic carpet mode!,\\
								\nPress numbers 1 to 3 to toggle (installed) texture effects,\\
								\n\nPress SPACEBAR to pause/unpause the motion of the mesh,\\
								\nPress R to reset all this,\\
								\n\nBrought to u by the Red Phoenix aka Roy Massaad! freeware til the extreme! ;)\\
								\n\troy.masad@gmail.com", "Help", MB_OK );
					ShowWindow(hWnd,SW_SHOWNORMAL);
					SetForegroundWindow(hWnd);					// Slightly Higher Priority
					SetFocus(hWnd);	
					
					keys[VK_F1] = false;
				}

				if (keys['R'])				// reset everything
				{
					amplitude = -0.234f;
					freq = 4;
					frame = GL_QUADS;
					xrot=75.0f;
					yrot=0.0f;
					zrot=0.0f;
					z=-4.89f;
					PopulateSine(lut,lutprecision,freq);
					warpeffect = false;
					carpetmode = true;
                    pixelate = false;
					pause = false;
					scroll = false;
					resetUVgrid(uvgrid);
					keys['R'] = false;
 				}

				if (keys[VK_SPACE])				// set's the pause sentinel on/off
				{
					if (pause) pause = false;
					else pause = true;

					keys[VK_SPACE] = false;
				}
				
				if (keys[VK_RIGHT])				// scroll right of the list of dragged files
				{
					scroll = false;				// disable auto-scrolling first
					listpos++;					// move one step to the right
					if (listpos > maxfiles-1)	listpos = 0;	// if out of bound, wrap to the left side
					char buf[200];
					
					DragQueryFile(hDrop,listpos,buf,200);

					LoadGLTextures(buf);	   // load the new texture
					keys[VK_RIGHT]  = false;
				}					
				
				if (keys[VK_LEFT])				// scroll right of the list of dragged files
				{
					scroll = false;				// disable auto-scrolling first

					listpos--;					// move one step to the right
					if (listpos < 0)	listpos = maxfiles-1;	// if out of bound, wrap to the left side
					char buf[200];
					
					DragQueryFile(hDrop,listpos,buf,200);

					LoadGLTextures(buf);	   // load the new texture
					keys[VK_LEFT]  = false;
				}			

				if (keys['1'])				// apply first effect (warping)
				{
					
					// reset grid, then enable or disable effect
					if (warpeffect) { resetUVgrid(uvgrid); warpeffect = false; }
					else { resetUVgrid(uvgrid); warpeffect = true;}

					keys['1'] = false;
				}

				if (keys['2'])				// apply second effect (swapping)
				{
					// we don't need to reset the grid first here
					// because it swaps the u and v values alternitavly
					// only disable the warpeffect and pixelate effect

					if (warpeffect)  { warpeffect = false; resetUVgrid(uvgrid);}
					if (pixelate)  { pixelate = false; resetUVgrid(uvgrid);}
					
					float temp;

					for (int y = 0; y < gz ; y++)
					{					
						for (int x = 0; x < gx ; x++)
						{
						temp = uvgrid[x][y].u;
						uvgrid[x][y].u = uvgrid[x][y].v;
						uvgrid[x][y].v = temp;

						}
					}
					keys['2'] =  false;
				}

				if (keys['3'])				// apply PIXELATE effect
				{
										
					// reset grid first, then enable or disable effect
					// disable warp effect if it's on
					warpeffect = false;
					if (pixelate) {resetUVgrid(uvgrid); pixelate = false; }
					else {resetUVgrid(uvgrid); pixelate = true; pixelateGrid(uvgrid);}

					keys['3'] =  false;
				}

				// enable/disable the image time scroller
				if (keys[VK_RETURN])
				{

					if (scroll) scroll = false;
					else {scroll = true; imagetimer = clock();}
					
					keys[VK_RETURN] = false;
				}

				

				}
			}
		
	}

	// Shutdown
	KillGLWindow();								// Kill The Window
	return (msg.wParam);						// Exit The Program
	
}



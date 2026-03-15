// WL_DRAW.C

#include "WL_DEF.H"
#include <DOS.H>
#pragma hdrstop

#define DOORWALL	(PMSpriteStart-20)

#define ACTORSIZE	0x4000

extern byte far shadetable[SHADE_COUNT][256];
extern int LSHADE_flag;
void DeathWarning (void);

long playx,playy;
extern int dithershift;

#ifdef DEBUGWALLS
unsigned screenloc[3]= {0,0,0};
#else
unsigned screenloc[3]= {PAGE1START,PAGE2START,PAGE3START};
#endif
unsigned freelatch = FREESTART;

long 	lasttimecount;
long 	frameon;
int   messagetime=0;

unsigned	wallheight[MAXVIEWWIDTH],min_wallheight;

fixed	tileglobal	= TILEGLOBAL;
fixed	mindist		= MINDIST;

int			pixelangle[MAXVIEWWIDTH];
long		far finetangent[FINEANGLES/4];
fixed 		far sintable[ANGLES+ANGLES/4],far *costable = sintable+(ANGLES/4);

fixed	viewx,viewy;
int		viewangle;
fixed	viewsin,viewcos;

fixed	FixedByFrac (fixed a, fixed b);
void	TransformActor (objtype *ob);
void	BuildTables (void);
void	ClearScreen (void);
int		CalcRotate (objtype *ob);
void	DrawScaleds (void);
void	CalcTics (void);
void	FixOfs (void);
void	ThreeDRefresh (void);

int		lastside;
long	lastintercept;
int		lasttilehit;

int			focaltx,focalty,viewtx,viewty;

int			midangle,angle;
unsigned	xpartial,ypartial;
unsigned	xpartialup,xpartialdown,ypartialup,ypartialdown;
unsigned	xinttile,yinttile;

unsigned	tilehit;
unsigned	pixx;

int		xtile,ytile;
int		xtilestep,ytilestep;
long	xintercept,yintercept;
long	xstep,ystep;

int		horizwall[MAXWALLTILES],vertwall[MAXWALLTILES];

void AsmRefresh (void);

#pragma warn -rvl

fixed FixedByFrac (fixed a, fixed b)
{
asm	mov	si,[WORD PTR b+2]
asm	mov	ax,[WORD PTR a]
asm	mov	cx,[WORD PTR a+2]

asm	or	cx,cx
asm	jns	aok:
asm	neg	cx
asm	neg	ax
asm	sbb	cx,0
asm	xor	si,0x8000
aok:

asm	mov	bx,[WORD PTR b]
asm	mul	bx
asm	mov	di,dx
asm	mov	ax,cx
asm	mul	bx
asm add	ax,di
asm	adc	dx,0

asm	test	si,0x8000
asm	jz	ansok:
asm	neg	dx
asm	neg	ax
asm	sbb	dx,0
ansok:;
}

#pragma warn +rvl

void TransformActor (objtype *ob)
{
	int ratio;
	fixed gx,gy,gxt,gyt,nx,ny;
	long	temp;

	gx = ob->x-viewx;
	gy = ob->y-viewy;

	gxt = FixedByFrac(gx,viewcos);
	gyt = FixedByFrac(gy,viewsin);
	nx = gxt-gyt-ACTORSIZE;
	gxt = FixedByFrac(gx,viewsin);
	gyt = FixedByFrac(gy,viewcos);
	ny = gyt+gxt;

	ob->transx = nx;
	ob->transy = ny;

	if (nx<mindist)
	{
	  ob->viewheight = 0;
	  return;
	}

	ob->viewx = centerx + ny*scale/nx;
	asm	mov	ax,[WORD PTR heightnumerator]
	asm	mov	dx,[WORD PTR heightnumerator+2]
	asm	idiv	[WORD PTR nx+1]
	asm	mov	[WORD PTR temp],ax
	asm	mov	[WORD PTR temp+2],dx

	ob->viewheight = temp;
}

boolean TransformTile (int tx, int ty, int *dispx, int *dispheight)
{
	int ratio;
	fixed gx,gy,gxt,gyt,nx,ny;
	long	temp;

	gx = ((long)tx<<TILESHIFT)+0x8000-viewx;
	gy = ((long)ty<<TILESHIFT)+0x8000-viewy;

	gxt = FixedByFrac(gx,viewcos);
	gyt = FixedByFrac(gy,viewsin);
	nx = gxt-gyt-0x2000;
	gxt = FixedByFrac(gx,viewsin);
	gyt = FixedByFrac(gy,viewcos);
	ny = gyt+gxt;

	if (nx<mindist)
	{
		*dispheight = 0;
		return false;
	}

	*dispx = centerx + ny*scale/nx;
	asm	mov	ax,[WORD PTR heightnumerator]
	asm	mov	dx,[WORD PTR heightnumerator+2]
	asm	idiv	[WORD PTR nx+1]
	asm	mov	[WORD PTR temp],ax
	asm	mov	[WORD PTR temp+2],dx

	*dispheight = temp;

	if (nx<TILEGLOBAL && ny>-TILEGLOBAL/2 && ny<TILEGLOBAL/2)
		return true;
	else
		return false;
}

#pragma warn -rvl
int   CalcHeight (void)
{
   int   transheight;
   int ratio;
   fixed gxt,gyt,nx,ny;
   long   gx,gy;

   gx = xintercept-viewx;
   gxt = FixedByFrac(gx,viewcos);

   gy = yintercept-viewy;
   gyt = FixedByFrac(gy,viewsin);

   nx = gxt-gyt;

   if (nx<mindist)
      nx=mindist;
   asm   mov   ax,[WORD PTR heightnumerator]
   asm   mov   dx,[WORD PTR heightnumerator+2]
   asm   idiv   [WORD PTR nx+1]
  asm mov bx,[min_wallheight]
  asm cmp ax,bx
  asm jae noupdate:
  asm mov [min_wallheight],ax
  noupdate:;
}

long		postsource;
unsigned	postx;
unsigned	postwidth;

#ifdef WITH_VGA
void	near ScalePost (void)		// VGA version
{
	asm	mov	ax,SCREENSEG
	asm	mov	es,ax

	asm	mov	bx,[postx]
	asm	shl	bx,1
	asm	mov	bp,WORD PTR [wallheight+bx]		// fractional height (low 3 bits frac)
	asm	and	bp,0xfff8				// bp = heightscaler*4
	asm	shr	bp,1
	asm	cmp	bp,[maxscaleshl2]
	asm	jle	heightok
	asm	mov	bp,[maxscaleshl2]
heightok:
	asm	add	bp,OFFSET fullscalefarcall
	//
	// scale a byte wide strip of wall
	//
	asm	mov	bx,[postx]
	asm	mov	di,bx
	asm	shr	di,2						// X in bytes
	asm	add	di,[bufferofs]

	asm	and	bx,3
	asm	shl	bx,3						// bx = pixel*8+pixwidth
	asm	add	bx,[postwidth]

	asm	mov	al,BYTE PTR [mapmasks1-1+bx]	// -1 because no widths of 0
	asm	mov	dx,SC_INDEX+1
	asm	out	dx,al						// set bit mask register
	asm	lds	si,DWORD PTR [postsource]
	asm	call DWORD PTR [bp]				// scale the line of pixels

	asm	mov	al,BYTE PTR [ss:mapmasks2-1+bx]   // -1 because no widths of 0
	asm	or	al,al
	asm	jz	nomore

	//
	// draw a second byte for vertical strips that cross two bytes
	//
	asm	inc	di
	asm	out	dx,al						// set bit mask register
	asm	call DWORD PTR [bp]				// scale the line of pixels

	asm	mov	al,BYTE PTR [ss:mapmasks3-1+bx]	// -1 because no widths of 0
	asm	or	al,al
	asm	jz	nomore
	//
	// draw a third byte for vertical strips that cross three bytes
	//
	asm	inc	di
	asm	out	dx,al						// set bit mask register
	asm	call DWORD PTR [bp]				// scale the line of pixels


nomore:
	asm	mov	ax,ss
	asm	mov	ds,ax
}
#else
void	near ScalePost (void)		// CGA version
{
	BEGIN_PROFILE(PROF_SCALEPOST)
	asm mov cx,[dithershift]

	asm mov ax,[activebackbufferseg]
	asm	mov	es,ax

	asm	mov	bx,[postx]
	asm	shl	bx,1
	asm	mov	bp,WORD PTR [wallheight+bx]		// fractional height (low 3 bits frac)
	asm	and	bp,0xfff8				// bp = heightscaler*4
	asm	shr	bp,1
	asm	cmp	bp,[maxscaleshl2]
	asm	jle	heightok
	asm	mov	bp,[maxscaleshl2]
heightok:
	asm	add	bp,OFFSET fullscalefarcall
	//
	// scale a byte wide strip of wall
	//
	asm	mov	bx,[postx]
	asm	mov	di,bx
	asm	shr	di,1						// X in bytes
	asm	shr	di,1						//
	asm	add	di,[screenofs]

	asm	lds	si,DWORD PTR [postsource]
	asm	call DWORD PTR [bp]				// scale the line of pixels

	asm	mov	ax,ss
	asm	mov	ds,ax
	END_PROFILE(PROF_SCALEPOST)
}
#endif

void  FarScalePost (void)				// just so other files can call
{
	ScalePost ();
}


/*
====================
=
= HitVertWall
=
= tilehit bit 7 is 0, because it's not a door tile
= if bit 6 is 1 and the adjacent tile is a door tile, use door side pic
=
====================
*/

void HitVertWall (void)
{
	int			wallpic;
	unsigned	texture;
	byte	tile;

	texture = (yintercept>>2)&0x3f80;
	if (xtilestep == -1)
	{
		texture = 0x3f80-texture;
		xintercept += TILEGLOBAL;
	}
	wallheight[pixx] = CalcHeight();

#ifdef WITH_VGA
	if (lastside==1 && lastintercept == xtile && lasttilehit == tilehit)
	{
		if (texture == (unsigned)postsource)
		{
			postwidth++;
			wallheight[pixx] = wallheight[pixx-1];
			return;
		}
		else
		{
			ScalePost ();
			(unsigned)postsource = texture;
			postwidth = 1;
			postx = pixx;
		}
	}
	else
#endif
	{
		if (lastside != -1)
			ScalePost ();

		lastside = true;
		lastintercept = xtile;

		lasttilehit = tilehit;
		postx = pixx;
		postwidth = 1;

		if (CheckAdjacentTile(xtile,ytile))
		{								// check for adjacent doors
			ytile = yintercept>>TILESHIFT;
			tile = tilemap[xtile-xtilestep][ytile];
			if (CheckHighBits(tile))
			{
				switch (doorobjlist[tile & 0x7f].lock)
				{
					case dr_lock1:
						wallpic = DOORWALL+9;
						break;
					case dr_lock2:
						wallpic = DOORWALL+11;
						break;
					case dr_lock3:
						wallpic = DOORWALL+13;
						break;
					case dr_lock4:
					case dr_normal:
					case dr_elevator:
					case dr_green:
					case dr_brown:
					default:
						wallpic = DOORWALL+3;
						break;
					}
				}
			else
				wallpic = vertwall[tilehit & ~0x40];
		}
		else
			wallpic = vertwall[tilehit];

		AssignWall (wallpic);
		(unsigned)postsource = texture;
	}

}


/*
====================
=
= HitHorizWall
=
= tilehit bit 7 is 0, because it's not a door tile
= if bit 6 is 1 and the adjacent tile is a door tile, use door side pic
=
====================
*/

void HitHorizWall (void)
{
	int			wallpic;
	unsigned	texture;
	byte tile;

	texture = (xintercept>>2)&0x3f80;
	if (ytilestep == -1)
		yintercept += TILEGLOBAL;
	else
		texture = 0x3f80-texture;
	wallheight[pixx] = CalcHeight();

#ifdef WITH_VGA
	if (lastside==0 && lastintercept == ytile && lasttilehit == tilehit)
	{
		if (texture == (unsigned)postsource)
		{
			postwidth++;
			wallheight[pixx] = wallheight[pixx-1];
			return;
		}
		else
		{
			ScalePost ();
			(unsigned)postsource = texture;
			postwidth = 1;
			postx = pixx;
		}
	}
	else
#endif
	{
		if (lastside != -1)
			ScalePost ();

		lastside = 0;
		lastintercept = ytile;

		lasttilehit = tilehit;
		postx = pixx;
		postwidth = 1;

		if (CheckAdjacentTile(xtile,ytile))
		{								// check for adjacent doors
			xtile = xintercept>>TILESHIFT;
			tile = tilemap[xtile][ytile-ytilestep];
			if (CheckHighBits(tile))
			{
				switch (doorobjlist[tile & 0x7f].lock)
				{
					case dr_lock1:
						wallpic = DOORWALL+8;
						break;
					case dr_lock2:
						wallpic = DOORWALL+10;
						break;
					case dr_lock3:
						wallpic = DOORWALL+12;
						break;
					case dr_normal:
					case dr_elevator:
					case dr_lock4:
					case dr_green:
					case dr_brown:
					default:
						wallpic = DOORWALL+2;
						break;
					}
				}
			else
				wallpic = horizwall[tilehit & ~0x40];
		}
		else
			wallpic = horizwall[tilehit];

		AssignWall(wallpic);
		(unsigned)postsource = texture;
	}

}

void HitHorizDoor (void)
{
	unsigned	texture,doorpage,doornum;

	doornum = tilehit&0x7f;
	texture = ( (xintercept-doorposition[doornum]) >> 2) &0x3f80;

	wallheight[pixx] = CalcHeight();

#ifdef WITH_VGA
	if (lasttilehit == tilehit)
	{
		if (texture == (unsigned)postsource)
		{
			postwidth++;
			wallheight[pixx] = wallheight[pixx-1];
			return;
		}
		else
		{
			ScalePost ();
			(unsigned)postsource = texture;
			postwidth = 1;
			postx = pixx;
		}
	}
	else
#endif
	{
		if (lastside != -1)
			ScalePost ();
		lastside = 2;
		lasttilehit = tilehit;
		postx = pixx;
		postwidth = 1;

		switch (doorobjlist[doornum].lock)
		{
		case dr_normal:
			doorpage = DOORWALL;
			break;
		case dr_lock1:
		case dr_lock2:
		case dr_lock3:
		case dr_brown:
			doorpage = DOORWALL+6;
			break;
		case dr_lock4:
			doorpage = DOORWALL+16;
			break;
		case dr_green:
			doorpage = DOORWALL+18;
			break;
		case dr_elevator:
			doorpage = DOORWALL+4;
			break;
		}

		*( ((unsigned *)&postsource)+1) = (unsigned)PM_GetPage(doorpage);
		(unsigned)postsource = texture;
	}
}

void HitVertDoor (void)
{
	unsigned	texture,doorpage,doornum;

	doornum = tilehit&0x7f;
	texture = ( (yintercept-doorposition[doornum]) >> 2) &0x3f80;

	wallheight[pixx] = CalcHeight();

#ifdef WITH_VGA
	if (lasttilehit == tilehit)
	{
		if (texture == (unsigned)postsource)
		{
			postwidth++;
			wallheight[pixx] = wallheight[pixx-1];
			return;
		}
		else
		{
			ScalePost ();
			(unsigned)postsource = texture;
			postwidth = 1;
			postx = pixx;
		}
	}
	else
#endif
	{
		if (lastside != -1)
			ScalePost ();
		lastside = 2;
		lasttilehit = tilehit;
		postx = pixx;
		postwidth = 1;

		switch (doorobjlist[doornum].lock)
		{
		case dr_normal:
			doorpage = DOORWALL;
			break;
		case dr_lock1:
		case dr_lock2:
		case dr_lock3:
		case dr_brown:
			doorpage = DOORWALL+6;
			break;
		case dr_lock4:
			doorpage = DOORWALL+16;
			break;
		case dr_green:
			doorpage = DOORWALL+18;
			break;
		case dr_elevator:
			doorpage = DOORWALL+4;
			break;
		}

		*( ((unsigned *)&postsource)+1) = (unsigned)PM_GetPage(doorpage+1);
		(unsigned)postsource = texture;
	}
}

void HitHorizPWall (void)
{
	int			wallpic;
	unsigned	texture,offset;

	texture = (xintercept>>2)&0x3f80;
	offset = pwallpos<<10;
	if (ytilestep == -1)
		yintercept += TILEGLOBAL-offset;
	else
	{
		texture = 0x3f80-texture;
		yintercept += offset;
	}

	wallheight[pixx] = CalcHeight();

#ifdef WITH_VGA
	if (lasttilehit == tilehit)
	{
		if (texture == (unsigned)postsource)
		{
			postwidth++;
			wallheight[pixx] = wallheight[pixx-1];
			return;
		}
		else
		{
			ScalePost ();
			(unsigned)postsource = texture;
			postwidth = 1;
			postx = pixx;
		}
	}
	else
#endif
	{
		if (lastside != -1)
			ScalePost ();

		lasttilehit = tilehit;
		postx = pixx;
		postwidth = 1;

		wallpic = horizwall[tilehit&63];

		*( ((unsigned *)&postsource)+1) = (unsigned)PM_GetPage(wallpic);
		(unsigned)postsource = texture;
	}

}

void HitVertPWall (void)
{
	int			wallpic;
	unsigned	texture,offset;

	texture = (yintercept>>2)&0x3f80;
	offset = pwallpos<<10;
	if (xtilestep == -1)
	{
		xintercept += TILEGLOBAL-offset;
		texture = 0x3f80-texture;
	}
	else
		xintercept += offset;

	wallheight[pixx] = CalcHeight();

#ifdef WITH_VGA
	if (lasttilehit == tilehit)
	{
		if (texture == (unsigned)postsource)
		{
			postwidth++;
			wallheight[pixx] = wallheight[pixx-1];
			return;
		}
		else
		{
			ScalePost ();
			(unsigned)postsource = texture;
			postwidth = 1;
			postx = pixx;
		}
	}
	else
#endif
	{
		if (lastside != -1)
			ScalePost ();

		lasttilehit = tilehit;
		postx = pixx;
		postwidth = 1;

		wallpic = vertwall[tilehit&63];

		*( ((unsigned *)&postsource)+1) = (unsigned)PM_GetPage(wallpic);
		(unsigned)postsource = texture;
	}

}

#if 0

void ClearScreen (void)
{
 unsigned floor=egaFloor[gamestate.episode*10+mapon],
	  ceiling=egaCeiling[gamestate.episode*10+mapon];

asm	mov	dx,GC_INDEX
asm	mov	ax,GC_MODE + 256*2
asm	out	dx,ax
asm	mov	ax,GC_BITMASK + 255*256
asm	out	dx,ax

asm	mov	dx,40
asm	mov	ax,[viewwidth]
asm	shr	ax,3
asm	sub	dx,ax

asm	mov	bx,[viewwidth]
asm	shr	bx,4
asm	mov	bh,BYTE PTR [viewheight]
asm	shr	bh,1

asm	mov	ax,[ceiling]
asm	mov	es,[screenseg]
asm	mov	di,[bufferofs]

toploop:
asm	mov	cl,bl
asm	rep	stosw
asm	add	di,dx
asm	dec	bh
asm	jnz	toploop

asm	mov	bh,BYTE PTR [viewheight]
asm	shr	bh,1
asm	mov	ax,[floor]

bottomloop:
asm	mov	cl,bl
asm	rep	stosw
asm	add	di,dx
asm	dec	bh
asm	jnz	bottomloop


asm	mov	dx,GC_INDEX
asm	mov	ax,GC_MODE + 256*10
asm	out	dx,ax
asm	mov	al,GC_BITMASK
asm	out	dx,al

}
#endif

#define USE_TEX(page) (0x0000|(page))
unsigned vgaCeiling[]=
{
#ifndef SPEAR
 0x1d1d,0x1d1d,0x1d1d,0x1d1d,0x1d1d,0x1d1d,0x1d1d,0x1d1d,0x1d1d,0xbfbf,
 0x4e4e,0x4e4e,0x4e4e,0x1d1d,0x8d8d,0x4e4e,0x1d1d,0x2d2d,0x1d1d,0x8d8d,
 0x1d1d,0x1d1d,0x1d1d,0x1d1d,0x1d1d,0x2d2d,0xdddd,0x1d1d,0x1d1d,0x9898,

 0x1d1d,0x9d9d,0x2d2d,0xdddd,0xdddd,0x9d9d,0x2d2d,0x4d4d,0x1d1d,0xdddd,
 0x7d7d,0x1d1d,0x2d2d,0x2d2d,0xdddd,0xd7d7,0x1d1d,0x1d1d,0x1d1d,0x2d2d,
 0x1d1d,0x1d1d,0x1d1d,0x1d1d,0xdddd,0xdddd,0x7d7d,0xdddd,0xdddd,0xdddd
#else
 0x6f6f,0x4f4f,0x1d1d,0xdede,0xdfdf,0x2e2e,0x7f7f,0x9e9e,0xaeae,0x7f7f,
 0x1d1d,0xdede,0xdfdf,0xdede,0xdfdf,0xdede,0xe1e1,0xdcdc,0x2e2e,0x1d1d,0xdcdc
#endif
};

t_floorcolors floorcolors[] =
{
	// CGA_MODE5,
	{	0xcccc,	0x3333,	0xc0c0,	0x0c0c },
	// CGA_MODE4,
	{	0xcccc,	0x3333,	0xc0c0,	0x0c0c },
	// CGA_COMPOSITE_MODE,
	{	0x5555,	0x5555,	0x8888,	0x8888 },
	// CGA_INVERSE_MONO,
	{	0x5555,	0xaaaa,	0xdddd,	0x7777 },
	// TANDY_MODE
	{	0x8888,	0x8888,	0x8080,	0x0808 },
	// HERCULES720_MODE,
	{	0x5555,	0xaaaa,	0x2222,	0x8888 },
	// HERCULES640_MODE,
	{	0x5555,	0xaaaa,	0x2222,	0x8888 },
};


/*
=====================
=
= VGAClearScreen
=
=====================
*/

void VGAClearScreen (void)
{
 unsigned ceiling=vgaCeiling[gamestate.episode*10+mapon];

  //
  // clear the screen
  //
asm	mov	dx,SC_INDEX
asm	mov	ax,SC_MAPMASK+15*256
asm	out	dx,ax

asm	mov	dx,80
asm	mov	ax,[viewwidth]
asm	shr	ax,1
asm	shr	ax,1
asm	sub	dx,ax					// dx = 40-viewwidth/2

asm	mov	bx,[viewwidth]
asm	shr	bx,1					// bl = viewwidth/8
asm	shr	bx,1					//
asm	shr	bx,1					//
asm	mov	bh,BYTE PTR [viewheight]
asm	shr	bh,1

asm	mov	es,[screenseg]
asm	mov	di,[bufferofs]
asm	mov	ax,[ceiling]

toploop:
asm	mov	cl,bl
asm	rep	stosw
asm	add	di,dx
asm	dec	bh
asm	jnz	toploop

asm	mov	bh,BYTE PTR [viewheight]
asm	shr	bh,1
asm	mov	ax,0x6d6d

bottomloop:
asm	mov	cl,bl
asm	rep	stosw
asm	add	di,dx
asm	dec	bh
asm	jnz	bottomloop
}
void DrawParallax(int startpage)
{
   word xtex,nextxtex,offs;
   byte far *skytex;
   byte far *destbuf=(byte far *)(0xa0000000L+bufferofs);
   int i,x=0,x2,curtex,nextx=0,texoffs,t,tend;
   byte mask;
   int midangle=player->angle*(FINEANGLES/ANGLES);
   int skyheight=viewheight>>1;
   startpage+=15;

   t=pixelangle[0]+midangle;
   while(t<0) t+=FINEANGLES;
   while(t>=FINEANGLES) t-=FINEANGLES;
   xtex=(word)((((long)t)<<11)/FINEANGLES);

   do
   {
      curtex=xtex>>7;
      skytex=(byte far *)(((long)(unsigned) PM_GetPage(startpage-curtex))<<16);
      nextx=0x7fff;
      for(i=0;i<4;i++)
      {
	 mask=1<<((x+i)&3);
	 VGAMAPMASK(mask);
	 for(x2=x+i;x2<viewwidth;x2+=4)
	 {
	    t=pixelangle[x2]+midangle;
	    while(t<0) t+=FINEANGLES;
	    while(t>=FINEANGLES) t-=FINEANGLES;
	    xtex=(word)((((long)t)<<11)/FINEANGLES);

	    t=xtex>>7;
	    if(t!=curtex)
	    {
	       if(x2<nextx) nextx=x2,nextxtex=xtex;
	       break;
	    }

	    texoffs=0x3f80-((xtex&127)<<7);
	    tend=skyheight-(wallheight[x2]>>3);
	    if(tend<=0) continue;
	    for(t=0,offs=x2>>2;t<tend;t++,offs+=80)
	    {
	    if (switches.paralaxsky)
	      destbuf[offs]=skytex[texoffs+((t<<7)/skyheight)];
              else
	      destbuf[offs]=vgaCeiling2[gamestate.mapon+gamestate.episode*10];
            }
	 }

	 if(x2>=viewwidth+3) nextx=viewwidth;

      }
      x=nextx;
      xtex=nextxtex;
   }
   while(x<viewwidth);
}
int	CalcRotate (objtype *ob)
{
	int	angle,viewangle;

	viewangle = player->angle + (centerx - ob->viewx)/8;

	switch (ob->obclass)
	{
	case rocketobj:
	case revballobj:
	case bossballobj:
	case fatballobj:
		angle = (viewangle-180)- ob->angle;
		break;
	default:
		angle = (viewangle-180)- dirangle[ob->dir];
		break;
	}

	angle+=ANGLES/16;
	while (angle>=ANGLES)
		angle-=ANGLES;
	while (angle<0)
		angle+=ANGLES;

	if (ob->state->rotate == 2)
		return 4*(angle/(ANGLES/2));

	return angle/(ANGLES/8);
}

#define MAXVISABLE	50

typedef struct
{
	int	viewx,
		viewheight,
		shapenum;
	statobj_t *transsprite;
} visobj_t;

visobj_t	vislist[MAXVISABLE],*visptr,*visstep,*farthest;

void Scale3DShape(statobj_t *ob);

void DrawScaleds (void)
{
	int 		i,j,least,numvisable,height;
	memptr		shape;
	byte		*tilespot,*visspot;
	int			shapenum;
	unsigned	spotloc;

	statobj_t	*statptr;
	objtype		*obj;

	BEGIN_PROFILE(PROF_DRAWSCALED)

	visptr = &vislist[0];

	for (statptr = &statobjlist[0] ; statptr !=laststatobj ; statptr++)
	{
		if ((visptr->shapenum = statptr->shapenum) == -1)
			continue;						// object has been deleted

		if (!*statptr->visspot)
			continue;

		switch (statptr->shapenum)
		{
		case SPR_STAT_0:
		case SPR_STAT_2:
		case SPR_STAT_4:
		case SPR_STAT_21:
		case SPR_STAT_23:
		case SPR_STAT3_0:
		case SPR_STAT3_8:
		case SPR_STAT3_42:
		case SPR_STAT_51:
			visptr->shapenum+=(frameon>>4)%2;
			break;
		case SPR_STAT_43:
		case SPR_STAT3_2:
		case SPR_STAT3_5:
			visptr->shapenum+=(frameon>>4)%3;
			break;
		case SPR_STAT_25:
		case SPR_STAT_29:
		case SPR_STAT_37:
		case SPR_STAT_54:
		case SPR_STAT_58:
		case SPR_STAT3_10:
		case SPR_STAT3_14:
		case SPR_STAT3_18:
		case SPR_STAT3_22:
		case SPR_STAT3_26:
		case SPR_STAT3_30:
		case SPR_STAT3_34:
		case SPR_STAT3_38:
		case SPR_STAT3_44:
		case SPR_HELLORB:
			visptr->shapenum+=(frameon>>3)%4;
			break;
		}

		if (TransformTile (statptr->tilex,statptr->tiley,&visptr->viewx,&visptr->viewheight) && statptr->flags & FL_BONUS)
		{
			GetBonus (statptr);
			continue;
		}

		if (!visptr->viewheight)
			continue;						// to close to the object
if(statptr->flags & (FL_DIRSOUTH | FL_DIREAST))
	 visptr->transsprite=statptr;
      else
	 visptr->transsprite=NULL;
		if (visptr < &vislist[MAXVISABLE-1])
			visptr++;
	}

	for (obj = player->next;obj;obj=obj->next)
	{
		if (!(visptr->shapenum = obj->state->shapenum))
			continue;

		spotloc = (obj->tilex<<6)+obj->tiley;
		visspot = &spotvis[0][0]+spotloc;
		tilespot = &tilemap[0][0]+spotloc;

		if (*visspot
		|| ( *(visspot-1) && !*(tilespot-1) )
		|| ( *(visspot+1) && !*(tilespot+1) )
		|| ( *(visspot-65) && !*(tilespot-65) )
		|| ( *(visspot-64) && !*(tilespot-64) )
		|| ( *(visspot-63) && !*(tilespot-63) )
		|| ( *(visspot+65) && !*(tilespot+65) )
		|| ( *(visspot+64) && !*(tilespot+64) )
		|| ( *(visspot+63) && !*(tilespot+63) ) )
		{
			obj->active = true;
			TransformActor (obj);
			if (!obj->viewheight)
				continue;

			visptr->viewx = obj->viewx;
			visptr->viewheight = obj->viewheight;
			if (visptr->shapenum == -1)
				visptr->shapenum = obj->temp1;

			if (obj->state->rotate)
				visptr->shapenum += CalcRotate (obj);

			if (visptr < &vislist[MAXVISABLE-1])
			{
	    visptr->transsprite=NULL;
	    visptr++;
	 }
			obj->flags |= FL_VISABLE;
		}
		else
			obj->flags &= ~FL_VISABLE;
	}

	numvisable = visptr-&vislist[0];

	if (!numvisable)
	{
		END_PROFILE(PROF_DRAWSCALED)
		return;									// no visable objects
	}

	for (i = 0; i<numvisable; i++)
	{
		least = 32000;
		for (visstep=&vislist[0] ; visstep<visptr ; visstep++)
		{
			height = visstep->viewheight;
			if (height < least)
			{
				least = height;
				farthest = visstep;
			}
		}
		if(farthest->transsprite)
	 Scale3DShape(farthest->transsprite);
      else
		ScaleShape(farthest->viewx,farthest->shapenum,farthest->viewheight);

		farthest->viewheight = 32000;
	}

	END_PROFILE(PROF_DRAWSCALED)

}

unsigned weaponviewheight;
int	weaponscale[NUMWEAPONS] = {SPR_FISTSREADY,SPR_PISTOLREADY,SPR_SHOTGUNREADY,SPR_SUPERSHOTGUNREADY,SPR_CHAINREADY,SPR_LAUNCHERREADY,SPR_PLASMAGUNREADY,SPR_BFGREADY,SPR_CHAINSAWREADY,SPR_HELLORB_READY};

void DrawPlayerWeapon (void)
{
	int	shapenum;

	if (gamestate.weapon != -1)
	{
		shapenum = weaponscale[gamestate.weapon]+gamestate.weaponframe;


		if (shapenum == SPR_CHAINSAWREADY){shapenum+=(frameon>>3)%2; SD_PlaySound (CHAINSAWIDLESND);}
		if (shapenum == SPR_HELLORB_READY)shapenum+=(frameon>>3)%4;
		if (shapenum == SPR_SHOTGUNRLD1)SimpleScaleShape(viewwidth/2-30,shapenum,weaponviewheight);
//		SimpleScaleShape(viewwidth/2,shapenum,152);
		else if (shapenum == SPR_SHOTGUNRLD2)SimpleScaleShape(viewwidth/2-40,shapenum,weaponviewheight+1);
		else if (shapenum == SPR_SHOTGUNRLD3)SimpleScaleShape(viewwidth/2-50,shapenum,weaponviewheight+1);
		else if (gamestate.weapon==wp_hellorb)
			SimpleScaleShape(viewwidth-80+gamestate.bobber2,shapenum,weaponviewheight+1+gamestate.bobber);
		else
			SimpleScaleShape(viewwidth/2-20+gamestate.bobber2,shapenum,weaponviewheight+1+gamestate.bobber+gamestate.weapchange);
	}

	if (!timedemo && (demorecord || demoplayback))
		SimpleScaleShape(viewwidth/2,SPR_DEMO,weaponviewheight);
}


//==========================================================================


/*
=====================
=
= CalcTics
=
=====================
*/

void CalcTics (void)
{
	long	newtime,oldtimecount;
	if (lasttimecount > TimeCount)
		TimeCount = lasttimecount;

	do
	{
		newtime = TimeCount;
		tics = newtime-lasttimecount;
	} while (!tics);

	lasttimecount = newtime;

#ifdef FILEPROFILE
		strcpy (scratch,"\tTics:");
		itoa (tics,str,10);
		strcat (scratch,str);
		strcat (scratch,"\n");
		write (profilehandle,scratch,strlen(scratch));
#endif

	if (tics>MAXTICS)
	{
		TimeCount -= (tics-MAXTICS);
		tics = MAXTICS;
	}
}

void	FixOfs (void)
{
	VW_ScreenToScreen (displayofs,bufferofs,viewwidth/8,viewheight);
}

void WallRefresh (void)
{
//
// set up variables for this view
//
	viewangle = player->angle;
	midangle = viewangle*(FINEANGLES/ANGLES);
	viewsin = sintable[viewangle];
	viewcos = costable[viewangle];
	viewx = player->x - FixedByFrac(focallength,viewcos);
	viewy = player->y + FixedByFrac(focallength,viewsin);

	focaltx = viewx>>TILESHIFT;
	focalty = viewy>>TILESHIFT;

	viewtx = player->x >> TILESHIFT;
	viewty = player->y >> TILESHIFT;

	xpartialdown = viewx&(TILEGLOBAL-1);
	xpartialup = TILEGLOBAL-xpartialdown;
	ypartialdown = viewy&(TILEGLOBAL-1);
	ypartialup = TILEGLOBAL-ypartialdown;

#ifdef WITH_VGA
	lastside = -1;			// the first pixel is on a new wall
#else
	lastside = 0;
#endif
	AsmRefresh ();
	ScalePost ();			// no more optimization on last post
	END_PROFILE(PROF_WALLREFRESH)
}

//==========================================================================

void CGAClearScreen()
{
	unsigned ceiling1 = floorcolors[cgamode].ceiling1, ceiling2 = floorcolors[cgamode].ceiling2;
	unsigned floor1 = floorcolors[cgamode].floor1, floor2 = floorcolors[cgamode].floor2;

	//
	// clear the screen
	//
	asm	mov	dx,80
	asm	mov	ax,[viewwidth]
	asm	shr	ax,1
	asm	shr	ax,1
	asm	sub	dx,ax					// dx = 40-viewwidth/2

	asm	mov	bx,[viewwidth]
	asm	shr	bx,1					// bl = viewwidth/8
	asm	shr	bx,1					//
	asm	shr	bx,1					//
	asm	mov	bh,BYTE PTR [viewheight]
	asm	shr	bh,1					// quarter height
	asm	shr	bh,1					//

	asm	mov	es,[cgabackbufferseg]
	asm	mov	di,[screenofs]

	toploop:
	asm	mov	cl,bl
	asm	mov	ax, [ceiling1] //0x8888 //0xc0c0
	asm	rep	stosw
	asm	add	di,dx

	asm	mov	cl,bl
	asm	mov	ax, [ceiling2] //0x8888 //0x0c0c
	asm	rep	stosw
	asm	add	di,dx

	asm	dec	bh
	asm	jnz	toploop

	asm	mov	bh,BYTE PTR [viewheight]
	asm	shr	bh,1					// quarter height
	asm	shr	bh,1					//

	bottomloop:
	asm	mov	cl,bl
	asm	mov	ax, [floor1] //0x5555 //0xcccc
	asm	rep	stosw
	asm	add	di,dx

	asm	mov	cl,bl
	asm	mov	ax, [floor2] // 0x5555 //0x3333
	asm	rep	stosw
	asm	add	di,dx

	asm	dec	bh
	asm	jnz	bottomloop
}

void HerculesClearScreen()
{
	unsigned ceiling1 = floorcolors[cgamode].ceiling1, ceiling2 = floorcolors[cgamode].ceiling2;
	unsigned floor1 = floorcolors[cgamode].floor1, floor2 = floorcolors[cgamode].floor2;

	//
	// clear the screen
	//
	asm	mov	dx,[linewidth]
	asm	mov	ax,[viewwidth]
	asm mov cl, 2
	asm	shr	ax,cl
	asm	sub	dx,ax					// dx = 40-viewwidth/2

	asm	mov	bx,[viewwidth]
	asm mov cl,3
	asm	shr	bx,cl					// bl = viewwidth/8

	// First bank
	asm	mov	es,[activebackbufferseg]
	asm	mov	di,[screenofs]

	asm	mov	bh,BYTE PTR [viewheight]
	asm mov cl,3
	asm	shr	bh,cl					// 1/8 height
	asm	mov	ax, [ceiling1]

	toploop1:
	asm	mov	cl,bl
	asm	rep	stosw
	asm	add	di,dx
	asm	dec	bh
	asm	jnz	toploop1

	asm	mov	bh,BYTE PTR [viewheight]
	asm mov cl,3
	asm	shr	bh,cl					// 1/8 height
	asm	mov	ax, [floor1]

	bottomloop1:
	asm	mov	cl,bl
	asm	rep	stosw
	asm	add	di,dx
	asm	dec	bh
	asm	jnz	bottomloop1

	// Second bank
	asm mov ax,[activebackbufferseg]
	asm add ax,0x200
	asm	mov	es,ax
	asm	mov	di,[screenofs]

	asm	mov	bh,BYTE PTR [viewheight]
	asm mov cl,3
	asm	shr	bh,cl					// 1/8 height
	asm	mov	ax, [ceiling2]

	toploop2:
	asm	mov	cl,bl
	asm	rep	stosw
	asm	add	di,dx
	asm	dec	bh
	asm	jnz	toploop2

	asm	mov	bh,BYTE PTR [viewheight]
	asm mov cl,3
	asm	shr	bh,cl					// 1/8 height
	asm	mov	ax, [floor2]

	bottomloop2:
	asm	mov	cl,bl
	asm	rep	stosw
	asm	add	di,dx
	asm	dec	bh
	asm	jnz	bottomloop2

	// Third bank
	asm mov ax,[activebackbufferseg]
	asm add ax,0x400
	asm	mov	es,ax
	asm	mov	di,[screenofs]

	asm	mov	bh,BYTE PTR [viewheight]
	asm mov cl,3
	asm	shr	bh,cl					// 1/8 height
	asm	mov	ax, [ceiling1]

	toploop3:
	asm	mov	cl,bl
	asm	rep	stosw
	asm	add	di,dx
	asm	dec	bh
	asm	jnz	toploop3

	asm	mov	bh,BYTE PTR [viewheight]
	asm mov cl,3
	asm	shr	bh,cl					// 1/8 height
	asm	mov	ax, [floor1]

	bottomloop3:
	asm	mov	cl,bl
	asm	rep	stosw
	asm	add	di,dx
	asm	dec	bh
	asm	jnz	bottomloop3

	// Fourth bank
	asm mov ax,[activebackbufferseg]
	asm add ax,0x600
	asm	mov	es,ax
	asm	mov	di,[screenofs]

	asm	mov	bh,BYTE PTR [viewheight]
	asm mov cl,3
	asm	shr	bh,cl					// 1/8 height
	asm	mov	ax, [ceiling2]

	toploop4:
	asm	mov	cl,bl
	asm	rep	stosw
	asm	add	di,dx
	asm	dec	bh
	asm	jnz	toploop4

	asm	mov	bh,BYTE PTR [viewheight]
	asm mov cl,3
	asm	shr	bh,cl					// 1/8 height
	asm	mov	ax, [floor2]

	bottomloop4:
	asm	mov	cl,bl
	asm	rep	stosw
	asm	add	di,dx
	asm	dec	bh
	asm	jnz	bottomloop4
}

void Hercules640ClearScreen()
{
	unsigned ceiling1 = floorcolors[cgamode].ceiling1, ceiling2 = floorcolors[cgamode].ceiling2;
	unsigned floor1 = floorcolors[cgamode].floor1, floor2 = floorcolors[cgamode].floor2;

	//
	// clear the screen
	//
	asm	mov	dx,[linewidth]
	asm	mov	ax,[viewwidth]
	asm mov cl, 2
	asm	shr	ax,cl
	asm	sub	dx,ax					// dx = 40-viewwidth/2

	asm	mov	bx,[viewwidth]
	asm mov cl,3
	asm	shr	bx,cl					// bl = viewwidth/8

	// First bank
	asm	mov	es,[activebackbufferseg]
	asm	mov	di,[screenofs]

	asm	mov	bh,BYTE PTR [viewheight]
	asm mov cl,2
	asm	shr	bh,cl					// 1/8 height
	asm	mov	ax, [ceiling1]

	toploop1:
	asm	mov	cl,bl
	asm	rep	stosw
	asm	add	di,dx
	asm	dec	bh
	asm	jnz	toploop1

	asm	mov	bh,BYTE PTR [viewheight]
	asm mov cl,2
	asm	shr	bh,cl					// 1/8 height
	asm	mov	ax, [floor1]

	bottomloop1:
	asm	mov	cl,bl
	asm	rep	stosw
	asm	add	di,dx
	asm	dec	bh
	asm	jnz	bottomloop1

	// Second bank
	asm mov ax,[activebackbufferseg]
	asm add ax,0x200
	asm	mov	es,ax
	asm	mov	di,[screenofs]

	asm	mov	bh,BYTE PTR [viewheight]
	asm mov cl,2
	asm	shr	bh,cl					// 1/8 height
	asm	mov	ax, [ceiling2]

	toploop2:
	asm	mov	cl,bl
	asm	rep	stosw
	asm	add	di,dx
	asm	dec	bh
	asm	jnz	toploop2

	asm	mov	bh,BYTE PTR [viewheight]
	asm mov cl,2
	asm	shr	bh,cl					// 1/8 height
	asm	mov	ax, [floor2]

	bottomloop2:
	asm	mov	cl,bl
	asm	rep	stosw
	asm	add	di,dx
	asm	dec	bh
	asm	jnz	bottomloop2

	// Third bank
	asm mov ax,[activebackbufferseg]
	asm add ax,0x400
	asm	mov	es,ax
	asm	mov	di,[screenofs]

	asm	mov	bh,BYTE PTR [viewheight]
	asm mov cl,2
	asm	shr	bh,cl					// 1/8 height
	asm	mov	ax, [ceiling1]

	toploop3:
	asm	mov	cl,bl
	asm	rep	stosw
	asm	add	di,dx
	asm	dec	bh
	asm	jnz	toploop3

	asm	mov	bh,BYTE PTR [viewheight]
	asm mov cl,2
	asm	shr	bh,cl					// 1/8 height
	asm	mov	ax, [floor1]

	bottomloop3:
	asm	mov	cl,bl
	asm	rep	stosw
	asm	add	di,dx
	asm	dec	bh
	asm	jnz	bottomloop3

	// Fourth bank
	asm mov ax,[activebackbufferseg]
	asm add ax,0x600
	asm	mov	es,ax
	asm	mov	di,[screenofs]

	asm	mov	bh,BYTE PTR [viewheight]
	asm mov cl,2
	asm	shr	bh,cl					// 1/8 height
	asm	mov	ax, [ceiling2]

	toploop4:
	asm	mov	cl,bl
	asm	rep	stosw
	asm	add	di,dx
	asm	dec	bh
	asm	jnz	toploop4

	asm	mov	bh,BYTE PTR [viewheight]
	asm mov cl,2
	asm	shr	bh,cl					// 1/8 height
	asm	mov	ax, [floor2]

	bottomloop4:
	asm	mov	cl,bl
	asm	rep	stosw
	asm	add	di,dx
	asm	dec	bh
	asm	jnz	bottomloop4

}

void CGABlit()
{
	BEGIN_PROFILE(PROF_CGABLIT)
	asm mov dx, [viewheight]
	asm shr dx, 1
	asm mov si, [screenofs]
	asm mov di, [cgascreenofs]

	asm mov bx, [viewwidth]
	asm mov cl, 2
	asm shr bx, cl				// bx is number of bytes to copy

	asm mov ax, 80
	asm sub ax, bx				// ax is the number of bytes to step to the next line

	asm push ds
	asm mov ds, [cgabackbufferseg]

blitlines:
	asm mov cx, 0xb800
	asm mov es, cx

	asm mov cx, bx
	asm rep movsb

	asm sub di, bx
	asm add si, ax

	asm mov cx, 0xba00
	asm mov es, cx

	asm mov cx, bx
	asm rep movsb

	asm add di, ax
	asm add si, ax

	asm dec dx
	asm jnz blitlines

	asm pop ds
}

/*
========================
=
= ThreeDRefresh
=
========================
*/

void	ThreeDRefresh (void)
{
	int tracedir;

	BEGIN_PROFILE(PROF_THREEDREFRESH);

// this wouldn't need to be done except for my debugger/video wierdness
//	outportb (SC_INDEX,SC_MAPMASK);

asm	mov	ax,ds
asm	mov	es,ax
asm	mov	di,OFFSET spotvis
asm	xor	ax,ax
asm	mov	cx,2048
asm	rep stosw


//
// follow the walls from there to the right, drawwing as we go
//

	if(cgamode == HERCULES720_MODE)
	{
		HerculesClearScreen();
	}
	else if(cgamode == HERCULES640_MODE)
	{
		Hercules640ClearScreen();
	}
	else
	{
		CGAClearScreen();
	}

	//VGAClearScreen ();


	WallRefresh ();

//
// draw all the scaled images
//
	DrawScaleds();			// draw scaled stuff
	DrawPlayerWeapon ();	// draw player's hands

//
// show screen and time last cycle
//

	if(cgamode == HERCULES720_MODE || cgamode == HERCULES640_MODE)
	{
		VL_PageFlip(false);
	}
	else
	{
		CGABlit();
	}

//	VL_BlitCGA();

/*
	if (fizzlein)
	{
		FizzleFade(bufferofs,displayofs+screenofs,viewwidth,viewheight,20,false);
		fizzlein = false;

		lasttimecount = TimeCount = 0;
	}
*/
	//bufferofs -= screenofs;
/*	displayofs = bufferofs;

	asm	cli
	asm	mov	cx,[displayofs]
	asm	mov	dx,3d4h
	asm	mov	al,0ch
	asm	out	dx,al
	asm	inc	dx
	asm	mov	al,ch
	asm	out	dx,al
	asm	sti

	bufferofs += SCREENSIZE;
	if (bufferofs > PAGE3START)
		bufferofs = PAGE1START;
*/

/*
	asm	cli
	asm	mov	cx,[bufferofs]
	asm	mov	dx,3d4h		// CRTC address register
	asm	mov	al,0ch		// start address high register
	asm	out	dx,al
	asm	inc	dx
	asm	mov	al,ch
	asm	out	dx,al   	// set the high byte
	asm	sti
*/
	frameon++;
	PM_NextFrame();
	END_PROFILE(PROF_THREEDREFRESH);
}

void GetMessage (char *lastmessage)
{
   messagetime = 210;

   strcpy(gamestate.message, lastmessage);
}

void DrawMessage (void)
{
	if (switches.messages)
{
   messagetime-=tics;

   fontnumber = 0;
   SETFONTCOLOR(0,184);
   PrintX=9; PrintY=3;
   US_Print(gamestate.message);
   SETFONTCOLOR(184,0);
   PrintX=8; PrintY=2;
   US_Print(gamestate.message);
    DrawAllPlayBorderSides ();
}
}
void DeathWarning (void)
{
	fontnumber = 0;
	SETFONTCOLOR(0,184);
	PrintX=9; PrintY=27;
	US_Print("Liv
		es: ");
	US_PrintUnsigned(gamestate.lives);
	SETFONTCOLOR(184,0);
	PrintX=8; PrintY=26;
	US_Print("Lives: ");
	US_PrintUnsigned(gamestate.lives);
	DrawAllPlayBorderSides ();
}
void DrawGameScore (void)
{
	fontnumber=0;
	SETFONTCOLOR(0,184);
	PrintX=9; PrintY=15;
	if (!DebugOk)
	{
	US_Print("Score: ");
	US_PrintUnsigned(gamestate.score);
	}
	else
	US_Print("disabled");
	SETFONTCOLOR(184,0);
	PrintX=8; PrintY=14;
	if (!DebugOk)
	{
	US_Print("Score: ");
	US_PrintUnsigned(gamestate.score);
	}
	else
	US_Print("disabled");
}


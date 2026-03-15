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

void near ScalePost()
{
   byte far *vmem;
   int i,ywcount,yoffs,yw,yd,yendoffs;
   byte col,mask;
#ifdef SHADE_COUNT
   byte shade;
   byte far *curshades;
#endif
   *((unsigned *)&vmem)=bufferofs;
   *(((unsigned *)&vmem)+1)=0xa000;

   for(i=0;i<postwidth;i++)
   {
      mask=1<<((postx+i)&3);
      VGAMAPMASK(mask);
      ywcount=yd=wallheight[postx+i]>>3;

#ifdef SHADE_COUNT
      shade=(yd<<2)/((maxscaleshl2>>8)+1+LSHADE_flag);
      if (gamestate.goggles==false)
      {
      if(shade>32) shade=32;
      else if(shade<1) shade=1;
      shade=32-shade;
      }
      else
      shade=0;

      curshades=shadetable[shade];
#endif

      if(yd<=0) yd=100;
      yoffs=(viewheight/2-ywcount)*80;
      if(yoffs<0) yoffs=0;
      yoffs+=((postx+i)>>2);
      yendoffs=viewheight/2+ywcount-1;
      yw=127;
      while(yendoffs>=viewheight)
      {
	 ywcount-=64;
	 while(ywcount<=0)
	 {
	    ywcount+=yd;
	    yw--;
	 }
	 yendoffs--;
      }
      if(yw<0) break;
#ifdef SHADE_COUNT
      col=curshades[((byte far *)postsource)[yw]];
#else
      col=((byte far *)postsource)[yw];
#endif
      yendoffs=yendoffs*80+((postx+i)>>2);
      while(yoffs<=yendoffs)
      {
	 vmem[yendoffs]=col;
	 ywcount-=64;
	 if(ywcount<=0)
	 {
	    do
            {
               ywcount+=yd;
               yw--;
            }
            while(ywcount<=0);
            if(yw<0) break;
#ifdef SHADE_COUNT
            col=curshades[((byte far *)postsource)[yw]];
#else
            col=((byte far *)postsource)[yw];
#endif
         }
	 yendoffs-=80;
      }
   }
}

void  FarScalePost (void)
{
	ScalePost ();
}

int CheckHighBits (int value)
{
   if ((value & 0x40) && (value & 0x80))
      { return 0; }
   else
      {
   if (value & 0x80)
     { return 1; }
   else
     { return 0; }
      }
}

int CheckAdjacentTile (int x, int y)
{
   if ((y-1 >= 0) && (tilemap[x][y-1] & 0x80)
   || (y+1 <= 63) && (tilemap[x][y+1] & 0x80)
   || (x-1 >= 0) && (tilemap[x-1][y] & 0x80)
   || (x+1 <= 63) && (tilemap[x+1][y] & 0x80))
    return 1;
   else
    return 0;
}
byte   far patchedwall[PMPageSize];
int      lastpatchnum;
#define FIRSTPATCHOBJ    2
#define NUMPATCHES       20
#define PATCHSTART       DOORWALL-NUMPATCHES*2

unsigned GetPatchNum(void)
{
   unsigned spot;

   spot = MAPSPOT(xtile,ytile,2);
   if (spot < FIRSTPATCHOBJ || spot >= FIRSTPATCHOBJ+NUMPATCHES )
      return 0;
   else
      return spot;
}

boolean ApplyPatch(int wallpic)
{
      byte   far *scan;
      int    pixel;
      unsigned spot;

   spot = GetPatchNum();
   if (spot > 0 && wallpic < DOORWALL)
   {
      lastpatchnum = spot;
      spot -= FIRSTPATCHOBJ;
      spot = spot * 2 + (lastside?1:0);
	 scan = PM_GetPage(wallpic);
	 for (pixel=0; pixel<PMPageSize; pixel++,scan++)
	  patchedwall[pixel] = *scan;
	 scan = PM_GetPage(PATCHSTART+spot);
	 for (pixel=0; pixel<PMPageSize; pixel++,scan++)
	  if (*scan < 255)
	     patchedwall[pixel] = *scan;
      return true;
   }
   lastpatchnum = 0;
   return false;
}
void AssignWall(int wallpic)
{
   if (ApplyPatch (wallpic))
      *( ((unsigned *)&postsource)+1) = (unsigned)(memptr)patchedwall;
   else
      *( ((unsigned *)&postsource)+1) = (unsigned)PM_GetPage(wallpic);
}
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

	if (lastside==1 && lastintercept == xtile && lasttilehit == tilehit && !CheckAdjacentTile(xtile,ytile) && lastpatchnum == GetPatchNum())
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

	if (lastside==0 && lastintercept == ytile && lasttilehit == tilehit && !CheckAdjacentTile(xtile,ytile)&& lastpatchnum == GetPatchNum())
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
 USE_TEX(51),USE_TEX(51),USE_TEX(51),USE_TEX(53),USE_TEX(53),USE_TEX(53),USE_TEX(53),USE_TEX(53),USE_TEX(22),USE_TEX(53),
 USE_TEX(50),USE_TEX(50),USE_TEX(50),USE_TEX(22),USE_TEX(50),USE_TEX(39),USE_TEX(50),USE_TEX(50),USE_TEX(50),USE_TEX(50),
 USE_TEX(53),USE_TEX(53),USE_TEX(53),USE_TEX(36),USE_TEX(53),USE_TEX(66),USE_TEX(50),USE_TEX(50),USE_TEX(52),USE_TEX(50),
 USE_TEX(55),USE_TEX(55),USE_TEX(55),USE_TEX(55),USE_TEX(55),USE_TEX(55),USE_TEX(55),USE_TEX(55),USE_TEX(55),USE_TEX(55),USE_TEX(55)
};
unsigned vgaCeiling2[]=
{
 0x5959,0x5656,0x5656,0x5656,0x5656,0x5656,0x5656,0x5656,0x5656,0x5656,
 0x8d8d,0x8d8d,0x8d8d,0x8d8d,0x8d8d,0x8d8d,0x8d8d,0x8d8d,0x8d8d,0xd8d8,
 0x2e2e,0x2e2e,0x2e2e,0x2e2e,0x2e2e,0x2e2e,0x2e2e,0x2e2e,0x2e2e,0xd8d8,
 0x2f2f,0x2f2f,0x2f2f,0x2f2f,0x2f2f,0xb4b4,0xb4b4,0xb4b4,0xb4b4,0xd8d8
};
void DrawFlats(unsigned tex_f, unsigned tex_c)
{
   int x, y, y0, halfheight,shade;
   unsigned top_offset0, bot_offset0, top_offset, bot_offset;
   unsigned top_add, bot_add;
   byte p, color;
   byte far *src_bot, far *src_top;
   fixed dist;            // distance to row projection
   fixed tex_step;         // global step per one screen pixel
   fixed gu, gv, du, dv;   // global texture coordinates
   int u, v;            // local texture coordinates
   int floorx,floory;
   unsigned curfloortex, lastfloortex;
   int ceilingx,ceilingy;
   unsigned curceilingtex, lastceilingtex;


   // ------ * prepare * --------
   halfheight=viewheight>>1;
   y0=min_wallheight>>3;      // starting y value
   if(y0>halfheight)
      return;     // view obscued by walls
   if(y0==0)
      y0=1;       // don't let division by zero
   top_offset0=80*(halfheight-y0-1);   // and will decrease by 80 each row
   bot_offset0=80*(halfheight+y0);     // and will increase by 80 each row
   src_top=PM_GetPage(tex_c);          // load ceiling texture
lastceilingtex=-1;
   // draw horizontal lines
   for(p=0; p<4; p++)
   {
      asm mov ax,0x0102
      asm mov cl,[p]
      asm shl ah,cl
      asm mov dx,0x3c4
      asm out dx,ax

      for(y=y0, top_offset=top_offset0; y<halfheight; y++, top_offset-=80)
      {
	 dist=(heightnumerator/y)<<5;
	 gu= viewx+FixedByFrac(dist, viewcos);
	 gv=-viewy+FixedByFrac(dist, viewsin);
	 tex_step=(dist<<8)/viewwidth/175;
	 du= FixedByFrac(tex_step, viewsin);
	 dv=-FixedByFrac(tex_step, viewcos);
	 gu-=((viewwidth>>1)-p)*du;
	 gv-=((viewwidth>>1)-p)*dv; // starting point (leftmost)
	 du<<=2; // 4pix step
	 dv<<=2;
	 shade=(y<<2)/((maxscaleshl2>>8)+1+LSHADE_flag);
	if (gamestate.goggles==false)
{
      if(shade>32) shade=32;
      shade=32-shade;
}
else
	shade=0;
	 for(x=p, top_add=top_offset; x<viewwidth; x+=4, top_add++)
	 {
	    if(wallheight[x]>>3<=y)
	    {
		ceilingx = (gu>>TILESHIFT)&127;
		ceilingy = (-(gv>>TILESHIFT)-1)&127;
		if (switches.floorsceilings)
		{
		curceilingtex=MAPSPOT(ceilingx,ceilingy,4);
		if (curceilingtex != lastceilingtex)
		{
			lastceilingtex=curceilingtex;
			src_top=PM_GetPage(curceilingtex);
		}
	       u=(gu>>9)&127;
	       v=(gv>>9)&127;
	       color=shadetable[shade][*(src_top+((127-u)<<7)+(127-v))];
	       }
	       else
	       {
	       curceilingtex=MAPSPOT(ceilingx,ceilingy,4)&0xff;
		if (curceilingtex==226)
		 color=0xba;
		else if (curceilingtex==228)
		 color=50;
		else
		 color=0x6d;
		 }
	       // draw top pixel using <color>

	if (!curceilingtex==0)
	{
	asm mov es,[screenseg]
	asm mov di,[bufferofs]
	asm add di,[top_add]
	asm mov al,[color]
	asm mov es:[di],al
	}
	    }
	    gu+=du;
	    gv+=dv;
	 }
      }
   }
   src_bot=PM_GetPage(tex_f); // load floor texture
   lastfloortex=-1;
   for(p=0; p<4; p++)
   {
      asm mov ax,0x0102
      asm mov cl,[p]
      asm shl ah,cl
      asm mov dx,0x3c4
      asm out dx,ax

      for(y=y0, bot_offset=bot_offset0; y<halfheight; y++, bot_offset+=80)
      {
	 dist=(heightnumerator/y)<<5;
	 gu= viewx+FixedByFrac(dist, viewcos);
	 gv=-viewy+FixedByFrac(dist, viewsin);
	 tex_step=(dist<<8)/viewwidth/175;
	 du= FixedByFrac(tex_step, viewsin);
	 dv=-FixedByFrac(tex_step, viewcos);
	 gu-=((viewwidth>>1)-p)*du;
	 gv-=((viewwidth>>1)-p)*dv; // starting point (leftmost)
	 du<<=2; // 4pix step
	 dv<<=2;
	shade=(y<<2)/((maxscaleshl2>>8)+1+LSHADE_flag);
	if (gamestate.goggles==false)
{
      if(shade>32) shade=32;
      shade=32-shade;
}
else
	shade=0;
	 for(x=p, bot_add=bot_offset; x<viewwidth; x+=4, bot_add++)
	 {
	    if(wallheight[x]>>3<=y)
	    {
	       floorx = (gu>>TILESHIFT)&127;
	       floory = (-(gv>>TILESHIFT)-1)&127;
	       if (switches.floorsceilings)
	       {
	       curfloortex=MAPSPOT(floorx,floory,3);
	       if (curfloortex==230)
		curfloortex+=((gamestate.TimeCount>>4)%4)<<1;
	       if (curfloortex==238)
		curfloortex+=((gamestate.TimeCount>>4)%3)<<1;
	       if (curfloortex==244)
		curfloortex+=((gamestate.TimeCount>>4)%3)<<1;
		}
		else
		{
		curfloortex=MAPSPOT(floorx,floory,3)&0xff;
		if (curfloortex==230)
		 color=0xca;
		else if (curfloortex==238)
		 color=0x7f;
		else if (curfloortex==226)
		 color=0xba;
		else if (curfloortex==228)
		 color=0x50;
		else if (curfloortex==244)
		 color=0xd8;
		else
		 color=0x6d;
		}
		if (switches.floorsceilings)
		{
	       if (curfloortex != lastfloortex)
	       {
		  lastfloortex=curfloortex;
		  src_bot=PM_GetPage(curfloortex);
	       }
	       u=(gu>>9)&127;
	       v=(gv>>9)&127;
	       color=shadetable[shade][*(src_bot+(u<<7)+(127-v))];
	       }
	       // draw bottom pixel using <color>
	       asm mov es,[screenseg]
	       asm mov di,[bufferofs]
	       asm add di,[bot_add]
	       asm mov al,[color]
	       asm mov es:[di],al
	    }
	    gu+=du;
	    gv+=dv;
	 }
      }
   }
}
void VGAClearScreen (unsigned ceiling)
{
asm	mov	dx,SC_INDEX
asm	mov	ax,SC_MAPMASK+15*256
asm	out	dx,ax

asm	mov	dx,80
asm	mov	ax,[viewwidth]
asm	shr	ax,2
asm	sub	dx,ax

asm	mov	bx,[viewwidth]
asm	shr	bx,3
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
		return;									// no visable objects

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

}

int	weaponscale[NUMWEAPONS] = {SPR_FISTSREADY,SPR_PISTOLREADY,SPR_SHOTGUNREADY,SPR_SUPERSHOTGUNREADY,SPR_CHAINREADY,SPR_LAUNCHERREADY,SPR_PLASMAGUNREADY,SPR_BFGREADY,SPR_CHAINSAWREADY,SPR_HELLORB_READY};

void DrawPlayerWeapon (void)
{
	int	shapenum;

	if (gamestate.weapon != -1)
	{
		shapenum = weaponscale[gamestate.weapon]+gamestate.weaponframe;


		if (shapenum == SPR_CHAINSAWREADY){shapenum+=(frameon>>3)%2; SD_PlaySound (CHAINSAWIDLESND);}
		if (shapenum == SPR_HELLORB_READY)shapenum+=(frameon>>3)%4;
		if (shapenum == SPR_SHOTGUNRLD1)SimpleScaleShape(viewwidth/2-30,shapenum,viewheight+1);
		else if (shapenum == SPR_SHOTGUNRLD2)SimpleScaleShape(viewwidth/2-40,shapenum,viewheight+1);
		else if (shapenum == SPR_SHOTGUNRLD3)SimpleScaleShape(viewwidth/2-50,shapenum,viewheight+1);
		else if (gamestate.weapon==wp_hellorb)
			SimpleScaleShape(viewwidth-80+gamestate.bobber2,shapenum,viewheight+1+gamestate.bobber);
		else
			SimpleScaleShape(viewwidth/2-20+gamestate.bobber2,shapenum,viewheight+1+gamestate.bobber+gamestate.weapchange);
	}

	if (demorecord || demoplayback)
		SimpleScaleShape(viewwidth/2,SPR_DEMO,viewheight+1);
}

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

	lastside = -1;
	AsmRefresh ();
	ScalePost ();
}

void	ThreeDRefresh (void)
{
	int tracedir;
	unsigned ceiling;
	boolean flats;
	boolean paralax;
	outportb (SC_INDEX,SC_MAPMASK);

asm	mov	ax,ds
asm	mov	es,ax
asm	mov	di,OFFSET spotvis
asm	xor	ax,ax
asm	mov	cx,2048
asm	rep stosw

	bufferofs += screenofs;

    ceiling=vgaCeiling[gamestate.episode*10+mapon];

    if((ceiling>>8)==(ceiling&0xFF))
    {
	  VGAClearScreen(ceiling);
	  flats=false;
    }
    else
    {
	  flats=true;
	  ceiling=((ceiling&0xFF)-1)<<1;
    }

    WallRefresh();

	switch (gamestate.mapon+gamestate.episode*10)
{
	case 0:
	case 2:
	case 3:
	case 6:
	case 9:
		paralax=true;
		startpage=130; // mossed mountains
		break;
	case 10:
	case 11:
	case 12:
	case 13:
	case 15:
	case 16:
	case 17:
	case 18:
		paralax=true; // sunset clouds
		startpage=146;
		break;
	case 19:
	case 29:
		paralax=true;
		startpage=210; // yellow sky
		break;
	case 20:
	case 21:
	case 23:
	case 24:
	case 25:
	case 26:
	case 27:
	case 28:
		paralax=true;
		startpage=162; // souls in sky
		break;
	case 30:
	case 31:
	case 32:
	case 33:
	case 35:
	case 38:
	case 40:
		paralax=true;
		startpage=178; // mountains w\red back
		break;
	case 39:
		paralax=true;
		startpage=194; // hell columns
		break;
	default:
		paralax=false;
		break;
	}

	if (paralax)
		DrawParallax (startpage);

    if(flats)
	  DrawFlats(ceiling, ceiling+1);

	playy=(player->y+FixedByFrac(0x7300,viewsin));
   playx=(player->x-FixedByFrac(0x7300,viewcos));
	DrawScaleds();
	DrawPlayerWeapon ();
	if (messagetime > 0)
   DrawMessage ();
   DrawGameScore();
   DeathWarning();
if (biosuit)
	StartBioFlash ();
if (godmode && !godflag)
	StartBonusFlash();
if (berserk)
	StartDamageFlash(1);
	if (fizzlein)
	{
		FizzleFade(bufferofs,displayofs+screenofs,viewwidth,viewheight,20,false);
		fizzlein = false;

		lasttimecount = TimeCount = 0;
	}
	bufferofs -= screenofs;
	displayofs = bufferofs;

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

	frameon++;
	PM_NextFrame();
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
	US_Print("Lives: ");
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
// WL_SCALE.C

#include "WL_DEF.H"
#pragma hdrstop

#define OP_RETF	0xcb

/*
=============================================================================

						  GLOBALS

=============================================================================
*/

#ifdef SHADE_COUNT                              // use shading ?
extern byte far shadetable[SHADE_COUNT][256];
extern int LSHADE_flag;
#endif

int			maxscale,maxscaleshl2;

boolean	insetupscaling;

//===========================================================================

/*
==========================
=
= SetupScaling
=
==========================
*/

void SetupScaling (int maxscaleheight)
{
	maxscaleheight/=2;			// one scaler every two pixels

	maxscale = maxscaleheight-1;
	maxscaleshl2 = maxscale<<2;
}

//===========================================================================

/*
=======================
=
= ScaleShape
=
= Draws a compiled shape at [scale] pixels high
=
= each vertical line of the shape has a pointer to segment data:
= 	end of segment pixel*2 (0 terminates line) used to patch rtl in scaler
= 	top of virtual line with segment in proper place
=	start of segment pixel*2, used to jsl into compiled scaler
=	<repeat>
=
= Setup for call
= --------------
= GC_MODE			read mode 1, write mode 2
= GC_COLORDONTCARE  set to 0, so all reads from video memory return 0xff
= GC_INDEX			pointing at GC_BITMASK
=
=======================
*/

void ScaleShape (int xcenter, int shapenum, unsigned height)
{
	t_compshape   _seg *shape;
	unsigned scale,pixheight;
	unsigned starty,endy;
	unsigned far *cmdptr;
	unsigned far *cline;
	unsigned far *line;
	int actx,newstart,i,upperedge;
	int scrstarty,screndy,j,lpix,rpix,pixcnt,ycnt;
	byte mask,col;
	byte far *vmem;
#ifdef SHADE_COUNT
	byte shade;
	byte far *curshades;
#endif

	shape=PM_GetSpritePage(shapenum);

	scale=height>>3;                 // low three bits are fractional
	if(!scale || scale>maxscale) return;   // too close or far away

#ifdef SHADE_COUNT
	switch(shapenum)
	{
		case 23:
		case 24:
		case 25:
		case 26:
		case 27:
		case 28:
		case 29:
		case 30:
		case 31:
		case 32:
		case 33:
		case 34:
		case 39:
		case 40:
		case 41:
		case 42:
		case 45:
		case 46:
		case 47:
		case 49:
		case 57:
		case 58:
		case 59:
		case 60:
		case 61:
		case 62:
		case 63:
		case 64:
		case 1217:
                case 1218: 
		case 1219:
		case 1220:
		case 1221:
		case 1222:
		case 1223:
		case 1224:
		case 1225:
		case 1226:
		case 1227:
		case 1228:
		case 1229:
		case 1230:
		case 1231:
		case 1232:
		case 1233:
		case 1234:
		case 1235:
		case 1236:
		case 1237:
		case 1238:
		case 1239:
		case 1240:
		case 1241:
		case 1242:
		case 1243:
		case 1244:
		case 1245:
		case 1246:
		case 1247:
		case 1248:
		case 1249:
		case 1250:
		case 1251:
		case 1252:
		case 1253:
		case 1254:
			shade=0;   // let lamps "shine" in the dark
			break;
		default:
			shade=(scale<<2)/((maxscaleshl2>>8)+1+LSHADE_flag);
			if (gamestate.goggles == false)
			{
			if(shade>32) shade=32;
			else if(shade<1) shade=1;
			shade=32-shade;
			}
			else shade=0;
			break;
	}
	curshades=shadetable[shade];
#endif

	pixheight=scale;
	actx=xcenter-scale;
	upperedge=viewheight/2-scale;

	*(((unsigned *)&cline)+1)=(unsigned)shape;		// seg of shape
	cmdptr=shape->dataofs;
	*(((unsigned *)&vmem)+1)=0xa000;

	for(i=shape->leftpix,pixcnt=i*pixheight,rpix=(pixcnt>>6)+actx;i<=shape->rightpix;i++,cmdptr++)
	{
		lpix=rpix;
		if(lpix>=viewwidth) break;
		pixcnt+=pixheight;
		rpix=(pixcnt>>6)+actx;
		if(lpix!=rpix && rpix>0)
		{
			if(lpix<0) lpix=0;
			if(rpix>viewwidth) rpix=viewwidth,i=shape->rightpix+1;
			(unsigned)cline=*cmdptr;
			while(lpix<rpix)
			{
				if(wallheight[lpix]<=height)
				{
					mask=1<<(lpix&3);
					VGAMAPMASK(mask);
					line=cline;
					while(*line)
					{
						starty=(*(line+2))>>1;
						endy=(*line)>>1;
						newstart=(*(line+1));
						j=starty;
						ycnt=j*pixheight;
						screndy=(ycnt>>6)+upperedge;
						if(screndy<0) (unsigned)vmem=bufferofs+(lpix>>2);
						else (unsigned)vmem=bufferofs+screndy*80+(lpix>>2);
						for(;j<endy;j++)
						{
							scrstarty=screndy;
							ycnt+=pixheight;
							screndy=(ycnt>>6)+upperedge;
							if(scrstarty!=screndy && screndy>0)
							{
#ifdef SHADE_COUNT
								col=curshades[((byte _seg *)shape)[newstart+j]];
#else
								col=((byte _seg *)shape)[newstart+j];
#endif
								if(scrstarty<0) scrstarty=0;
								if(screndy>viewheight) screndy=viewheight,j=endy;

								while(scrstarty<screndy)
								{
									*vmem=col;
									vmem+=80;
									scrstarty++;
								}
							}
						}
						line+=3;
					}
				}
				lpix++;
			}
		}
	}
}


/*
=======================
=
= SimpleScaleShape
=
= NO CLIPPING, height in pixels
=
= Draws a compiled shape at [scale] pixels high
=
= each vertical line of the shape has a pointer to segment data:
= 	end of segment pixel*2 (0 terminates line) used to patch rtl in scaler
= 	top of virtual line with segment in proper place
=	start of segment pixel*2, used to jsl into compiled scaler
=	<repeat>
=
= Setup for call
= --------------
= GC_MODE			read mode 1, write mode 2
= GC_COLORDONTCARE  set to 0, so all reads from video memory return 0xff
= GC_INDEX			pointing at GC_BITMASK
=
=======================
*/

void SimpleScaleShape (int xcenter, int shapenum, unsigned height)
{
	t_compshape   _seg *shape;
	unsigned scale,pixheight;
	unsigned starty,endy;
	unsigned far *cmdptr;
	unsigned far *cline;
	unsigned far *line;
	int actx,newstart,i,upperedge;
	int scrstarty,screndy,j,lpix,rpix,pixcnt,ycnt;
	byte mask,col;
	byte far *vmem;

	shape=PM_GetSpritePage(shapenum);

	scale=height>>1;                 // low three bits are fractional
	if(!scale || scale>maxscale) return;   // too close or far away

	pixheight=scale;
	actx=xcenter-scale;
	upperedge=viewheight/2-scale;

	*(((unsigned *)&cline)+1)=(unsigned)shape;		// seg of shape
	cmdptr=shape->dataofs;
	*(((unsigned *)&vmem)+1)=0xa000;

	for(i=shape->leftpix,pixcnt=i*pixheight,rpix=(pixcnt>>6)+actx;i<=shape->rightpix;i++,cmdptr++)
	{
		lpix=rpix;
		if(lpix>=viewwidth) break;
		pixcnt+=pixheight;
		rpix=(pixcnt>>6)+actx;
		if(lpix!=rpix && rpix>0)
		{
			if(lpix<0) lpix=0;
			if(rpix>viewwidth) rpix=viewwidth,i=shape->rightpix+1;
			(unsigned)cline=*cmdptr;
			while(lpix<rpix)
			{
				mask=1<<(lpix&3);
				VGAMAPMASK(mask);
				line=cline;
				while(*line)
				{
					starty=(*(line+2))>>1;
					endy=(*line)>>1;
					newstart=(*(line+1));
					j=starty;
					ycnt=j*pixheight;
					screndy=(ycnt>>6)+upperedge;
					if(screndy<0) (unsigned)vmem=bufferofs+(lpix>>2);
					else (unsigned)vmem=bufferofs+screndy*80+(lpix>>2);
					for(;j<endy;j++)
					{
						scrstarty=screndy;
						ycnt+=pixheight;
						screndy=(ycnt>>6)+upperedge;
						if(scrstarty!=screndy && screndy>0)
						{
							col=((byte _seg *)shape)[newstart+j];
							if(scrstarty<0) scrstarty=0;
							if(screndy>viewheight) screndy=viewheight,j=endy;

							while(scrstarty<screndy)
							{
								*vmem=col;
								vmem+=80;
								scrstarty++;
							}
						}
					}
					line+=3;
				}
				lpix++;
			}
		}
	}
}

#ifdef FL_DIRSOUTH  // use directional 3d sprites ?

void Scale3DShaper (int x1,int x2,int shapenum,unsigned height1,unsigned height2,
		fixed ny1,fixed ny2,fixed nx1,fixed nx2)
{
	t_compshape _seg *shape;
	unsigned scale1,scale2,starty,endy;
	unsigned far *cmdptr;
	unsigned far *line;
	byte far *vmem;
	int dx=x2-x1,len,i,j,newstart,ycnt,pixheight,screndy,upperedge,scrstarty;
	fixed height,dheight;
	int xpos[129];
	int slinex;
	fixed dxx=(ny2-ny1)<<8,dzz=(nx2-nx1)<<8;
	fixed dxa=0,dza=0;
	byte mask,col;
#ifdef SHADE_COUNT
	byte shade;
	byte far *curshades;
#endif

	if(!dx) return;

	height=(((fixed)height1)<<12)+2048;
	dheight=(((fixed)height2-(fixed)height1)<<12)/(fixed)dx;

	// Get length/address of pixeldata
	shape = PM_GetSpritePage (shapenum);

	scale1 = height1>>3; // low three bits are fractional
	scale2 = height2>>3; // low three bits are fractional
	if (!scale1 && !scale2 || scale1>maxscale && scale2>maxscale)
		return; // too close or far away*/

	len=shape->rightpix-shape->leftpix+1;
	if(!len) return;

	ny1+=dxx>>9;
	nx1+=dzz>>9;

	dxa=-(dxx>>1),dza=-(dzz>>1);
	dxx>>=7,dzz>>=7;
	dxa+=shape->leftpix*dxx,dza+=shape->leftpix*dzz;

	xpos[0]=(int)((ny1+(dxa>>8))*scale/(nx1+(dza>>8))+centerx);

	for(i=1;i<=len;i++)
	{
		dxa+=dxx,dza+=dzz;
		xpos[i]=(int)((ny1+(dxa>>8))*scale/(nx1+(dza>>8))+centerx);
		if(xpos[i-1]>viewwidth) break;
	}
	len=i-1;

	*(((unsigned *)&line)+1)=(unsigned)shape; // seg of shape
	cmdptr = &shape->dataofs[0];
	*(((unsigned *)&vmem)+1)=0xa000;

	i=0;
	if(x2>viewwidth) x2=viewwidth;

	for(i=0;i<len;i++)
	{
		for(slinex=xpos[i];slinex<xpos[i+1] && slinex<x2;slinex++)
		{
			height+=dheight;
			if(slinex<0) continue;

			scale1=(unsigned)(height>>15);

			if(wallheight[slinex]<(height>>12) && scale1 && scale1<=maxscale)
			{
#ifdef SHADE_COUNT
				shade=(scale1<<2)/((maxscaleshl2>>8)+1+LSHADE_flag);
				if (gamestate.goggles == false)
				{
				if(shade>32) shade=32;
				else if(shade<1) shade=1;
				shade=32-shade;
				}
                                else shade=0;
				curshades=shadetable[shade];
#endif

				pixheight=scale1;
				upperedge=viewheight/2-scale1;

				mask=1<<(slinex&3);
				VGAMAPMASK(mask);
				(unsigned)line=cmdptr[i];
				while(*line)
				{
					starty=(*(line+2))>>1;
					endy=(*line)>>1;
					newstart=(*(line+1));
					j=starty;
					ycnt=j*pixheight;
					screndy=(ycnt>>6)+upperedge;
					if(screndy<0) (unsigned)vmem=bufferofs+(slinex>>2);
					else (unsigned)vmem=bufferofs+screndy*80+(slinex>>2);
					for(;j<endy;j++)
					{
						scrstarty=screndy;
						ycnt+=pixheight;
						screndy=(ycnt>>6)+upperedge;
						if(scrstarty!=screndy && screndy>0)
						{
#ifdef SHADE_COUNT
							col=curshades[((byte _seg *)shape)[newstart+j]];
#else
							col=((byte _seg *)shape)[newstart+j];
#endif
							if(scrstarty<0) scrstarty=0;
							if(screndy>viewheight) screndy=viewheight,j=endy;

							while(scrstarty<screndy)
							{
								*vmem=col;
								vmem+=80;
								scrstarty++;
							}
						}
					}
					line+=3;
				}
			}
		}
	}
}

extern long playx,playy;

void Scale3DShape(statobj_t *ob)
{
   fixed nx1,nx2,ny1,ny2;
	int viewx1,viewx2;
   long height1,height2;
   fixed diradd;

//
// the following values for "diradd" aren't optimized yet
// if you have problems with sprites being visible through wall edges
// where they shouldn't, you can try to adjust these values and
// the -1024 and +2048 in both "if" blocks
//

   if(ob->flags & FL_DIRFORWARD) diradd=0x7ff0+0x8000;
   else if(ob->flags & FL_DIRBACKWARD) diradd=-0x7ff0+0x8000;
   else diradd=0x8000;

   if(ob->flags & FL_DIREAST)
	{
      fixed gx1,gx2,gy,gxt1,gxt2,gyt;
//
// translate point to view centered coordinates
//
      gx1 = (((long)ob->tilex) << TILESHIFT)+0x8000-playx-0x8000L-1024;
      gx2 = gx1+0x10000L+2048;
      gy = (((long)ob->tiley) << TILESHIFT)+diradd-playy;

//
// calculate newx
//
      gxt1 = FixedByFrac(gx1,viewcos);
      gxt2 = FixedByFrac(gx2,viewcos);
      gyt = FixedByFrac(gy,viewsin);
      nx1 = gxt1-gyt-0x2000;
      nx2 = gxt2-gyt-0x2000;

//
// calculate newy
//
      gxt1 = FixedByFrac(gx1,viewsin);
		gxt2 = FixedByFrac(gx2,viewsin);
      gyt = FixedByFrac(gy,viewcos);
      ny1 = gyt+gxt1;
		ny2 = gyt+gxt2;
	}
   else
   {
      fixed gy1,gy2,gx,gyt1,gyt2,gxt;
//
// translate point to view centered coordinates
//
      gy1 = (((long)ob->tiley) << TILESHIFT)+0x8000-playy-0x8000L-1024;
      gy2 = gy1+0x10000L+2048;
      gx = (((long)ob->tilex) << TILESHIFT)+diradd-playx;

//
// calculate newx
//
		gxt = FixedByFrac(gx,viewcos);
      gyt1 = FixedByFrac(gy1,viewsin);
      gyt2 = FixedByFrac(gy2,viewsin);
      nx1 = gxt-gyt1-0x2000;
      nx2 = gxt-gyt2-0x2000;

//
// calculate newy
//
      gxt = FixedByFrac(gx,viewsin);
		gyt1 = FixedByFrac(gy1,viewcos);
      gyt2 = FixedByFrac(gy2,viewcos);
      ny1 = gyt1+gxt;
      ny2 = gyt2+gxt;
   }

//
// calculate perspective ratio
//
   if(nx1>=0 && nx1<=1792) nx1=1792;
	if(nx1<0 && nx1>=-1792) nx1=-1792;
   if(nx2>=0 && nx2<=1792) nx2=1792;
   if(nx2<0 && nx2>=-1792) nx2=-1792;

   viewx1=(int)(centerx+ny1*scale/nx1);
   viewx2=(int)(centerx+ny2*scale/nx2);

//
// calculate height (heightnumerator/(nx>>8)) (heightnumerator=0x36800)
//
   asm mov ax,word ptr heightnumerator
   asm mov dx,word ptr heightnumerator+2
	asm idiv word ptr nx1+1 // nx1>>8
   asm mov word ptr height1,ax
   asm mov word ptr height1+2,dx

	asm mov ax,word ptr heightnumerator
   asm mov dx,word ptr heightnumerator+2
   asm idiv word ptr nx2+1 // nx2>>8
   asm mov word ptr height2,ax
   asm mov word ptr height2+2,dx

   if(viewx2 < viewx1)
      Scale3DShaper(viewx2,viewx1,ob->shapenum,(unsigned)height2,
         (unsigned)height1,ny2,ny1,nx2,nx1);
   else
      Scale3DShaper(viewx1,viewx2,ob->shapenum,(unsigned)height1,
         (unsigned)height2,ny1,ny2,nx1,nx2);
}

#endif         // #ifdef FL_DIRSOUTH
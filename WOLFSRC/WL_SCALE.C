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

t_compscale _seg *scaledirectory[MAXSCALEHEIGHT+1];
long			fullscalefarcall[MAXSCALEHEIGHT+1];

boolean		usewiderendering;
boolean		adjustherculesaspect = false;
int			maxscale,maxscaleshl2;

boolean	insetupscaling;
int			dithershift = 2;

/*
=============================================================================

						  LOCALS

=============================================================================
*/

t_compscale 	_seg *work;
unsigned BuildCompScale (int height, memptr *finalspot);

int			stepbytwo;

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
	int i;
	maxscaleheight/=2;			// one scaler every two pixels

	maxscale = maxscaleheight-1;
	maxscaleshl2 = maxscale<<2;

#ifdef WITH_VGA
	dithershift = 0;
#else
	switch(cgamode)
	{
		case CGA_MODE4:
		case CGA_MODE5:
		dithershift = 2;
		break;
		case CGA_INVERSE_MONO:
		dithershift = 3;
		break;
		case TANDY_MODE:
		case CGA_COMPOSITE_MODE:
		dithershift = 4;
		break;
		case HERCULES720_MODE:
		case HERCULES640_MODE:
		dithershift = 3;
		break;
	}
	if (MS_CheckParm("nodither"))
	{
		dithershift = 0;
	}
#endif

//
// free up old scalers
//
	for (i=1;i<MAXSCALEHEIGHT;i++)
	{
		if (scaledirectory[i])
			MM_FreePtr (&(memptr)scaledirectory[i]);
		if (i>=stepbytwo)
			i += 2;
	}
	memset (scaledirectory,0,sizeof(scaledirectory));

	MM_SortMem ();

//
// build the compiled scalers
//
	stepbytwo = viewheight/2;	// save space by double stepping
	MM_GetPtr (&(memptr)work,20000);

	for (i=1;i<=maxscaleheight;i++)
	{
		BuildCompScale (i*2,&(memptr)scaledirectory[i]);
		if (i>=stepbytwo)
			i+= 2;
	}
	MM_FreePtr (&(memptr)work);

//
// compact memory and lock down scalers
//
	MM_SortMem ();
	for (i=1;i<=maxscaleheight;i++)
	{
		MM_SetLock (&(memptr)scaledirectory[i],true);
		fullscalefarcall[i] = (unsigned)scaledirectory[i];
		fullscalefarcall[i] <<=16;
		fullscalefarcall[i] += COMPSCALECODESTART; //scaledirectory[i]->codeofs[0];
		if (i>=stepbytwo)
		{
			scaledirectory[i+1] = scaledirectory[i];
			fullscalefarcall[i+1] = fullscalefarcall[i];
			scaledirectory[i+2] = scaledirectory[i];
			fullscalefarcall[i+2] = fullscalefarcall[i];
			i+=2;
		}
	}
	scaledirectory[0] = scaledirectory[1];
	fullscalefarcall[0] = fullscalefarcall[1];

//
// check for oversize wall drawing
//

	insetupscaling = false;
}

//===========================================================================

/*
========================
=
= BuildCompScale
=
= Builds a compiled scaler object that will scale a 64 tall object to
= the given height (centered vertically on the screen)
=
= height should be even
=
= Call with
= ---------
= DS:SI		Source for scale
= ES:DI		Dest for scale
= CL		Dither rotation amount (bits)
=
= Calling the compiled scaler only destroys AL
=
========================
*/

unsigned BuildCompScale (int height, memptr *finalspot)
{
	byte		far *code;

	int			i;
	long		fix,step;
	unsigned	src,totalscaled,totalsize;
	int			startpix,endpix,toppix,pix,bottompix;

	step = ((long)height<<16) / 64;
	fix = 0;

	for (src=0;src<=64;src++)
	{
		startpix = fix>>16;
		fix += step;
		endpix = fix>>16;

		if (endpix>startpix)
		{
			work->width[src] = endpix-startpix;
		}
		else
			work->width[src] = 0;
	}

	// Aspect ratio correction
	if(usewiderendering)
	{
		height <<= 1;
	}
	else if(cgamode == HERCULES720_MODE)// && adjustherculesaspect)
	{
		height += height >> 1;
	}

	code = &work->code[0];
	step = ((long)height<<16) / 64;
	toppix = (viewheight-height)/2;
	bottompix = toppix+height;
	fix = 0;

	for (src=0;src<=64;src++)
	{
		startpix = fix>>16;
		fix += step;
		endpix = fix>>16;

//
// mark the start of the code
//
		work->codeofs[src] = FP_OFF(code);

//
// compile some code if the source pixel generates any screen pixels
//
		startpix+=toppix;
		endpix+=toppix;

		if (startpix == endpix || endpix < 0 || startpix >= viewheight || src == 64)
			continue;

	//
	// mov al,[si+src]
	//
		*code++ = 0x8a;
		*code++ = 0x44;
		*code++ = src;

		if(!dithershift)
		{
			for (pix=(startpix);pix<endpix;pix++)
			{
				if (pix >= viewheight)
					break;						// off the bottom of the view area
				if (pix < 0)
					continue;					// not into the view area

			//
			// mov [es:di+heightofs],al
			//
				*code++ = 0x26;
				*code++ = 0x88;
				*code++ = 0x85;

				if(cgamode == HERCULES720_MODE)
				{
					int planeoffset = (pix & 3) * 0x2000;
					int pixline = pix >> 2;
					*((unsigned far *)code)++ = planeoffset + pixline*linewidth;
				}
				else if(cgamode == HERCULES640_MODE)
				{
					int planeoffset = (pix & 1) * 0x4000;
					int pixline = pix >> 1;
					int writeLocation = planeoffset + pixline*linewidth;
					*((unsigned far *)code)++ = writeLocation;
					*code++ = 0x26;
					*code++ = 0x88;
					*code++ = 0x85;
					*((unsigned far *)code)++ = writeLocation + 0x2000;
				}
				else
				{
					*((unsigned far *)code)++ = pix*linewidth;
				}
			}
		}
		else
		{
			// First draw even lines
			for (pix=(startpix+1)&0xfffe;pix<endpix;pix+=2)
			{
				if (pix >= viewheight)
					break;						// off the bottom of the view area
				if (pix < 0)
					continue;					// not into the view area

			//
			// mov [es:di+heightofs],al
			//
				*code++ = 0x26;
				*code++ = 0x88;
				*code++ = 0x85;

				if(cgamode == HERCULES720_MODE)
				{
					int planeoffset = (pix & 3) * 0x2000;
					int pixline = pix >> 2;
					*((unsigned far *)code)++ = planeoffset + pixline*linewidth;
				}
				else if(cgamode == HERCULES640_MODE)
				{
					int planeoffset = (pix & 1) * 0x4000;
					int pixline = pix >> 1;
					int writeLocation = planeoffset + pixline*linewidth;
					*((unsigned far *)code)++ = writeLocation;
					*code++ = 0x26;
					*code++ = 0x88;
					*code++ = 0x85;
					*((unsigned far *)code)++ = writeLocation + 0x2000;
				}
				else
				{
					*((unsigned far *)code)++ = pix*linewidth;
				}
			}

			if((startpix | 1) < endpix)
			{
				// Rotate the dither pattern for odd lines
				// ror al, cl
				*code++ = 0xd2;
				*code++ = 0xc8;

				// Draw odd lines
				for (pix=(startpix | 0x1);pix<endpix;pix+=2)
				{
					if (pix >= viewheight)
						break;						// off the bottom of the view area
					if (pix < 0)
						continue;					// not into the view area

				//
				// mov [es:di+heightofs],al
				//
					*code++ = 0x26;
					*code++ = 0x88;
					*code++ = 0x85;
					if(cgamode == HERCULES720_MODE)
					{
						int planeoffset = (pix & 3) * 0x2000;
						int pixline = pix >> 2;
						*((unsigned far *)code)++ = planeoffset + pixline*linewidth;
					}
					else if(cgamode == HERCULES640_MODE)
					{
						int planeoffset = (pix & 1) * 0x4000;
						int pixline = pix >> 1;
						int writeLocation = planeoffset + pixline*linewidth;
						*((unsigned far *)code)++ = writeLocation;
						*code++ = 0x26;
						*code++ = 0x88;
						*code++ = 0x85;
						*((unsigned far *)code)++ = writeLocation + 0x2000;
					}
					else
					{
						*((unsigned far *)code)++ = pix*linewidth;
					}
				}
			}
		}
	}

//
// retf
//
	*code++ = 0xcb;

	totalsize = FP_OFF(code);
	MM_GetPtr (finalspot,totalsize);
	_fmemcpy ((byte _seg *)(*finalspot),(byte _seg *)work,totalsize);

	return totalsize;
}


/*
=======================
=
= ScaleLine
=
= linescale should have the high word set to the segment of the scaler
=
=======================
*/

extern	int			slinex,slinewidth;
extern	unsigned	far *linecmds;
extern	long		linescale;
extern	unsigned	maskword;

byte	mask1,mask2,mask3;

#ifdef WITH_VGA
// VGA version
void near ScaleLine (void)
{
asm	mov	cx,WORD PTR [linescale+2]
asm	mov	es,cx						// segment of scaler

asm	mov bp,WORD PTR [linecmds]
asm	mov	dx,SC_INDEX+1				// to set SC_MAPMASK

asm	mov	bx,[slinex]
asm	mov	di,bx
asm	shr	di,2						// X in bytes
asm	add	di,[bufferofs]
asm	and	bx,3
asm	shl	bx,3
asm	add	bx,[slinewidth]				// bx = (pixel*8+pixwidth)
asm	mov	al,BYTE [mapmasks3-1+bx]	// -1 because pixwidth of 1 is first
asm	mov	ds,WORD PTR [linecmds+2]
asm	or	al,al
asm	jz	notthreebyte				// scale across three bytes
asm	jmp	threebyte
notthreebyte:
asm	mov	al,BYTE PTR ss:[mapmasks2-1+bx]	// -1 because pixwidth of 1 is first
asm	or	al,al
asm	jnz	twobyte						// scale across two bytes

//
// one byte scaling
//
asm	mov	al,BYTE PTR ss:[mapmasks1-1+bx]	// -1 because pixwidth of 1 is first
asm	out	dx,al						// set map mask register

scalesingle:

asm	mov	bx,[ds:bp]					// table location of rtl to patch
asm	or	bx,bx
asm	jz	linedone					// 0 signals end of segment list
asm	mov	bx,[es:bx]
asm	mov	dl,[es:bx]					// save old value
asm	mov	BYTE PTR es:[bx],OP_RETF	// patch a RETF in
asm	mov	si,[ds:bp+4]				// table location of entry spot
asm	mov	ax,[es:si]
asm	mov	WORD PTR ss:[linescale],ax	// call here to start scaling
asm	mov	si,[ds:bp+2]				// corrected top of shape for this segment
asm	add	bp,6						// next segment list

asm	mov	ax,SCREENSEG
asm	mov	es,ax
asm	call ss:[linescale]				// scale the segment of pixels

asm	mov	es,cx						// segment of scaler
asm	mov	BYTE PTR es:[bx],dl			// unpatch the RETF
asm	jmp	scalesingle					// do the next segment


//
// done
//
linedone:
asm	mov	ax,ss
asm	mov	ds,ax
return;

//
// two byte scaling
//
twobyte:
asm	mov	ss:[mask2],al
asm	mov	al,BYTE PTR ss:[mapmasks1-1+bx]	// -1 because pixwidth of 1 is first
asm	mov	ss:[mask1],al

scaledouble:

asm	mov	bx,[ds:bp]					// table location of rtl to patch
asm	or	bx,bx
asm	jz	linedone					// 0 signals end of segment list
asm	mov	bx,[es:bx]
asm	mov	cl,[es:bx]					// save old value
asm	mov	BYTE PTR es:[bx],OP_RETF	// patch a RETF in
asm	mov	si,[ds:bp+4]				// table location of entry spot
asm	mov	ax,[es:si]
asm	mov	WORD PTR ss:[linescale],ax	// call here to start scaling
asm	mov	si,[ds:bp+2]				// corrected top of shape for this segment
asm	add	bp,6						// next segment list

asm	mov	ax,SCREENSEG
asm	mov	es,ax
asm	mov	al,ss:[mask1]
asm	out	dx,al						// set map mask register
asm	call ss:[linescale]				// scale the segment of pixels
asm	inc	di
asm	mov	al,ss:[mask2]
asm	out	dx,al						// set map mask register
asm	call ss:[linescale]				// scale the segment of pixels
asm	dec	di

asm	mov	es,WORD PTR ss:[linescale+2] // segment of scaler
asm	mov	BYTE PTR es:[bx],cl			// unpatch the RETF
asm	jmp	scaledouble					// do the next segment


//
// three byte scaling
//
threebyte:
asm	mov	ss:[mask3],al
asm	mov	al,BYTE PTR ss:[mapmasks2-1+bx]	// -1 because pixwidth of 1 is first
asm	mov	ss:[mask2],al
asm	mov	al,BYTE PTR ss:[mapmasks1-1+bx]	// -1 because pixwidth of 1 is first
asm	mov	ss:[mask1],al

scaletriple:

asm	mov	bx,[ds:bp]					// table location of rtl to patch
asm	or	bx,bx
asm	jz	linedone					// 0 signals end of segment list
asm	mov	bx,[es:bx]
asm	mov	cl,[es:bx]					// save old value
asm	mov	BYTE PTR es:[bx],OP_RETF	// patch a RETF in
asm	mov	si,[ds:bp+4]				// table location of entry spot
asm	mov	ax,[es:si]
asm	mov	WORD PTR ss:[linescale],ax	// call here to start scaling
asm	mov	si,[ds:bp+2]				// corrected top of shape for this segment
asm	add	bp,6						// next segment list

asm	mov	ax,SCREENSEG
asm	mov	es,ax
asm	mov	al,ss:[mask1]
asm	out	dx,al						// set map mask register
asm	call ss:[linescale]				// scale the segment of pixels
asm	inc	di
asm	mov	al,ss:[mask2]
asm	out	dx,al						// set map mask register
asm	call ss:[linescale]				// scale the segment of pixels
asm	inc	di
asm	mov	al,ss:[mask3]
asm	out	dx,al						// set map mask register
asm	call ss:[linescale]				// scale the segment of pixels
asm	dec	di
asm	dec	di

asm	mov	es,WORD PTR ss:[linescale+2] // segment of scaler
asm	mov	BYTE PTR es:[bx],cl			// unpatch the RETF
asm	jmp	scaletriple					// do the next segment


}

#else // CGA version

void near ScaleLine (void)
{
asm	mov	cx,WORD PTR [linescale+2]
asm	mov	es,cx						// segment of scaler

asm	mov bp,WORD PTR [linecmds]

asm	mov	bx,[slinex]
asm	mov	di,bx
asm	shr	di,1						// X in bytes
asm	shr	di,1						//
asm	add	di,[screenofs]
asm	mov	ds,WORD PTR [linecmds+2]

scalesingle:
asm	mov	bx,[ds:bp]					// table location of rtl to patch
asm	or	bx,bx
asm	jz	linedone					// 0 signals end of segment list
asm	mov	bx,[es:bx]
asm	mov	dl,[es:bx]					// save old value
asm	mov	BYTE PTR es:[bx],OP_RETF	// patch a RETF in
asm	mov	si,[ds:bp+4]				// table location of entry spot
asm	mov	ax,[es:si]
asm	mov	WORD PTR ss:[linescale],ax	// call here to start scaling
asm	mov	si,[ds:bp+2]				// corrected top of shape for this segment
asm	add	bp,6						// next segment list

asm	mov	ax,ss:[activebackbufferseg]
asm	mov	es,ax

asm mov ax, ss:[dithershift]		// Set up dither shift value
asm mov ah, cl
asm mov cl, al

asm	call ss:[linescale]				// scale the segment of pixels

asm mov cl, ah						// Restore CL

asm	mov	es,cx						// segment of scaler
asm	mov	BYTE PTR es:[bx],dl			// unpatch the RETF
asm	jmp	scalesingle					// do the next segment


//
// done
//
linedone:
asm	mov	ax,ss
asm	mov	ds,ax
}
#endif

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
static	long		longtemp;

#ifdef WITH_VGA
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
#else // CGA version
void ScaleShape (int xcenter, int shapenum, unsigned height)
{
	t_compshape	_seg *shape;
	t_compscale _seg *comptable;
	unsigned	scale,srcx,stopx,tempx;
	int			t;
	unsigned	far *cmdptr;
	boolean		leftvis,rightvis;


	shape = PM_GetSpritePage (shapenum);

	scale = height>>3;						// low three bits are fractional
	if (!scale || scale>maxscale)
		return;								// too close or far away
	comptable = scaledirectory[scale];

	*(((unsigned *)&linescale)+1)=(unsigned)comptable;	// seg of far call
	*(((unsigned *)&linecmds)+1)=(unsigned)shape;		// seg of shape

//
// scale to the left (from pixel 31 to shape->leftpix)
//
	srcx = 32;
	slinex = xcenter;
	stopx = shape->leftpix;
	cmdptr = &shape->dataofs[31-stopx];

	while ( --srcx >=stopx && slinex>0)
	{
		(unsigned)linecmds = *cmdptr--;
		if ( !(slinewidth = comptable->width[srcx]) )
			continue;

		while(slinewidth-- && slinex >= 0)
		{
			slinex--;
			if(slinex < viewwidth && !(slinex & 3))
			{
				if (wallheight[slinex] < height)
				{
					ScaleLine ();
				}
			}
		}
	}


//
// scale to the right
//
	slinex = xcenter;
	stopx = shape->rightpix;
	if (shape->leftpix<31)
	{
		srcx = 31;
		cmdptr = &shape->dataofs[32-shape->leftpix];
	}
	else
	{
		srcx = shape->leftpix-1;
		cmdptr = &shape->dataofs[0];
	}
	slinewidth = 0;

	while ( ++srcx <= stopx && slinex<viewwidth)
	{
		(unsigned)linecmds = *cmdptr++;
		if ( !(slinewidth = comptable->width[srcx]) )
			continue;

		while(slinewidth-- && slinex<viewwidth)
		{
			if(slinex >= 0 && !(slinex & 3) && wallheight[slinex] < height)
			{
				ScaleLine();
			}
			slinex++;
		}
	}
}
#endif


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

#ifdef WITH_VGA
// VGA version
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

	// Get length/address of pixeldata	backbufferseg = FP_SEG(cgabackbuffer);

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
		srcx = shape->leftpix-1;
		cmdptr = &shape->dataofs[0];
	}
	slinewidth = 0;

	while ( ++srcx <= stopx )
	{
		(unsigned)linecmds = *cmdptr++;
		if ( !(slinewidth = comptable->width[srcx]) )
			continue;

		ScaleLine ();
		slinex+=slinewidth;
	}
}

#else // CGA version
void SimpleScaleShape (int xcenter, int shapenum, unsigned height)
{
	t_compshape	_seg *shape;
	t_compscale _seg *comptable;
	unsigned	scale,srcx,stopx,tempx;
	int			t;
	unsigned	far *cmdptr;
	boolean		leftvis,rightvis;

	shape = PM_GetSpritePage (shapenum);

	scale = height>>1;
	comptable = scaledirectory[scale];

	*(((unsigned *)&linescale)+1)=(unsigned)comptable;	// seg of far call
	*(((unsigned *)&linecmds)+1)=(unsigned)shape;		// seg of shape

//
// scale to the left (from pixel 31 to shape->leftpix)
//
	srcx = 32;
	slinex = xcenter;
	stopx = shape->leftpix;
	cmdptr = &shape->dataofs[31-stopx];

	while ( --srcx >=stopx )
	{
		(unsigned)linecmds = *cmdptr--;
		if ( !(slinewidth = comptable->width[srcx]) )
			continue;

		while(slinewidth--)
		{
			slinex--;
			if(!(slinex & 0x3))
				ScaleLine ();
		}
	}


//
// scale to the right
//
	slinex = xcenter;
	stopx = shape->rightpix;
	if (shape->leftpix<31)
	{
		srcx = 31;
		cmdptr = &shape->dataofs[32-shape->leftpix];
	}
	else
	{
		srcx = shape->leftpix-1;
		cmdptr = &shape->dataofs[0];
	}
	slinewidth = 0;

	while ( ++srcx <= stopx )
	{
		(unsigned)linecmds = *cmdptr++;
		if ( !(slinewidth = comptable->width[srcx]) )
			continue;

		while(slinewidth--)
		{
			if(!(slinex & 0x3))
				ScaleLine ();
			slinex++;
		}
	}
}
#endif



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
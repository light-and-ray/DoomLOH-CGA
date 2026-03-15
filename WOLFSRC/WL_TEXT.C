#include "WL_DEF.H"
#pragma	hdrstop
#define BACKCOLOR		0x6c
#define WORDLIMIT		80
#define FONTHEIGHT		10
#define	TOPMARGIN		16
#define BOTTOMMARGIN	32
#define LEFTMARGIN		16
#define RIGHTMARGIN		16
#define PICMARGIN		8
#define TEXTROWS		((200-TOPMARGIN-BOTTOMMARGIN)/FONTHEIGHT)
#define	SPACEWIDTH		7
#define SCREENPIXWIDTH	320
#define SCREENMID		(SCREENPIXWIDTH/2)

int pagenum,numpages;
unsigned leftmargin[TEXTROWS],rightmargin[TEXTROWS];
char far *text;
unsigned rowon;
int picx,picy,picnum,picdelay;
boolean layoutdone;

void RipToEOL (void)
{
	while (*text++ != '\n');
}

int ParseNumber (void)
{
	char ch;
	char num[80],*numptr;ch = *text;

	while (ch < '0' || ch >'9')
		ch = *++text;
		numptr = num;

		do
		{
			*numptr++ = ch;
			ch = *++text;
		} while (ch >= '0' && ch <= '9');
	*numptr = 0;
	return atoi (num);
}

void ParsePicCommand (void)
{
	picy=ParseNumber();
	picx=ParseNumber();
	picnum=ParseNumber();
	RipToEOL ();
}

void ParseTimedCommand (void)
{
	picy=ParseNumber();
	picx=ParseNumber();
	picnum=ParseNumber();
	picdelay=ParseNumber();
	RipToEOL ();
}

void TimedPicCommand (void)
{
	ParseTimedCommand ();
	VW_UpdateScreen ();
	TimeCount = 0;
	while (TimeCount < picdelay);
		VWB_DrawPic (picx&~7,picy,picnum);
}

void HandleCommand (void)
{
	int i,margin,top,bottom;
	int picwidth,picheight,picmid;

	switch (toupper(*++text))
	{
		case 'B':
			picy=ParseNumber();
			picx=ParseNumber();
			picwidth=ParseNumber();
			picheight=ParseNumber();
			VWB_Bar(picx,picy,picwidth,picheight,BACKCOLOR);
			RipToEOL();
			break;
		case ';':
			RipToEOL();
			break;
		case 'P':
		case 'E':
			layoutdone = true;
			text--;
			break;
		case 'C':
			i = toupper(*++text);
			if (i>='0' && i<='9')
				fontcolor = i-'0';
			else if (i>='A' && i<='F')
				fontcolor = i-'A'+10;
			fontcolor *= 16;
			i = toupper(*++text);
			if (i>='0' && i<='9')
				fontcolor += i-'0';
			else if (i>='A' && i<='F')
				fontcolor += i-'A'+10;
			text++;
			break;
		case '>':
			px = 160;
			text++;
			break;
		case 'L':
			py=ParseNumber();
			rowon = (py-TOPMARGIN)/FONTHEIGHT;
			py = TOPMARGIN+rowon*FONTHEIGHT;
			px=ParseNumber();
			while (*text++ != '\n');
			break;
		case 'T':
			TimedPicCommand ();
			break;
		case 'G':
			ParsePicCommand ();
			VWB_DrawPic (picx&~7,picy,picnum);
			picwidth = pictable[picnum-STARTPICS].width;
			picheight = pictable[picnum-STARTPICS].height;
			picmid = picx + picwidth/2;
			if (picmid > SCREENMID)
				margin = picx-PICMARGIN;
			else
				margin = picx+picwidth+PICMARGIN;
			top = (picy-TOPMARGIN)/FONTHEIGHT;
			if (top<0)
				top = 0;
			bottom = (picy+picheight-TOPMARGIN)/FONTHEIGHT;
			if (bottom>=TEXTROWS)
				bottom = TEXTROWS-1;
			for (i=top;i<=bottom;i++)
				if (picmid > SCREENMID)
					rightmargin[i] = margin;
				else
					leftmargin[i] = margin;
			if (px < leftmargin[rowon])
				px = leftmargin[rowon];
			break;
		}
}

void NewLine (void)
{
	char ch;

	if (++rowon == TEXTROWS)
	{
		layoutdone = true;
		do
		{
			if (*text == '^')
			{
				ch = toupper(*(text+1));
				if (ch == 'E' || ch == 'P')
				{
					layoutdone = true;
					return;
				}
			}
			text++;
		} while (1);
	}
	px = leftmargin[rowon];
	py+= FONTHEIGHT;
}

void HandleCtrls (void)
{
	char ch;
	ch = *text++;
	if (ch == '\n')
	{
		NewLine ();
		return;
	}
}

void HandleWord (void)
{
	char word[WORDLIMIT];
	int i,wordindex;
	unsigned wwidth,wheight,newpos;

	word[0] = *text++;
	wordindex = 1;
	while (*text>32)
	{
		word[wordindex] = *text++;
		if (++wordindex == WORDLIMIT)
			Quit ("PageLayout: Word limit exceeded");
	}
	word[wordindex] = 0;
	VW_MeasurePropString (word,&wwidth,&wheight);
	while (px+wwidth > rightmargin[rowon])
	{
		NewLine ();
		if (layoutdone)
			return;
	}
	newpos = px+wwidth;
	VWB_DrawPropString (word);
	px = newpos;
	while (*text == ' ')
	{
		px += SPACEWIDTH;text++;
	}
}

void PageLayout (boolean shownumber)
{
	int i,oldfontcolor;
	char ch;
	oldfontcolor = fontcolor;
	fontcolor = 0;
	CacheLump (PAGEOF_LUMP_START,PAGEOF_LUMP_END);
	VWB_DrawPic (0,0,TOPWINDOWPIC);
	VWB_DrawPic (0,12,LEFTWINDOWPIC);
	VWB_DrawPic (308,12,RIGHTWINDOWPIC);
	VWB_DrawPic (0,188,BOTTOMWINDOWPIC);
	VWB_DrawPic (152,2,PAGE1OF41PIC+pagenum);
	VWB_DrawPic (184,2,PAGE1OF41PIC+numpages-1);

	for (i=0;i<TEXTROWS;i++)
	{
		leftmargin[i] = LEFTMARGIN;
		rightmargin[i] = SCREENPIXWIDTH-RIGHTMARGIN;
	}
	px = LEFTMARGIN;
	py = TOPMARGIN;
	rowon = 0;
	layoutdone = false;
	while (*text <= 32)
	text++;
	if (*text != '^' || toupper(*++text) != 'P')
		Quit ("PageLayout: Text not headed with ^P");
	while (*text++ != '\n');
	do
	{
		ch = *text;
		if (ch == '^')
			HandleCommand ();
		else if (ch == 9)
		{
			px = (px+8)&0xf8;
			text++;
		}
		else if (ch <= 32)
			HandleCtrls ();
		else
			HandleWord ();
		} while (!layoutdone);
			pagenum++;
			if (shownumber)
			{
			 /*	strcpy (str,"pg ");
				itoa (pagenum,str2,10);
				strcat (str,str2);
				strcat (str," of ");
				py = 183;
				px = 213;
				itoa (numpages,str2,10);
				strcat (str,str2);
				fontcolor = 0x4f;      */
				VWB_DrawPropString (str);
			}
			fontcolor = oldfontcolor;
}

void BackPage (void)
{
	pagenum--;
	do
	{
		text--;
		if (*text == '^' && toupper(*(text+1)) == 'P')
			return;
	} while (1);
}

void CacheLayoutGraphics (void)
{
	char far *bombpoint, far *textstart;
	char ch;
	textstart = text;
	bombpoint = text+30000;
	numpages = pagenum = 0;

	do
	{
		if (*text == '^')
		{
			ch = toupper(*++text);
			if (ch == 'P')
				numpages++;
			if (ch == 'E')
			{
				CA_MarkGrChunk(BACKWINDOWPIC);
				CA_MarkGrChunk(TOPWINDOWPIC);
				CA_MarkGrChunk(LEFTWINDOWPIC);
				CA_MarkGrChunk(RIGHTWINDOWPIC);
				CA_MarkGrChunk(BOTTOMWINDOWPIC);
				CA_CacheMarks ();
				text = textstart;
				return;
			}
			if (ch == 'G')
			{
				ParsePicCommand ();
				CA_MarkGrChunk (picnum);
			}
			if (ch == 'T')
			{
				ParseTimedCommand ();
				CA_MarkGrChunk (picnum);
			}
		}
		else
			text++;
		} while (text<bombpoint);
	Quit ("CacheLayoutGraphics: No ^E to terminate file!");
}

void ShowArticle (char far *article)
{
	unsigned oldfontnumber;
	unsigned temp;
	boolean newpage,firstpage;
	text = article;
	oldfontnumber = fontnumber;
	fontnumber = 0;
	CA_MarkGrChunk(STARTFONT);
	VWB_Bar (0,0,320,200,BACKCOLOR);
	CacheLayoutGraphics ();
	newpage = true;
	firstpage = true;
		do
		{
			if (newpage)
			{
				newpage = false;
				PageLayout (true);
				VW_UpdateScreen ();
				if (firstpage)
				{
					VL_FadeIn(0,255,&gamepal,10);
					firstpage = false;
				}
			}
			LastScan = 0;
			while (!LastScan);
			switch (LastScan)
			{
				case sc_UpArrow:
				case sc_PgUp:
				case sc_LeftArrow:
					if (pagenum>1)
					{
						BackPage ();
						BackPage ();
						newpage = true;
					}
					break;
				case sc_Enter:
				case sc_DownArrow:
				case sc_PgDn:
				case sc_RightArrow:
					if (pagenum<numpages)
					{
						newpage = true;
					}
					break;
				}
				if (Keyboard[sc_Tab] && Keyboard[sc_P] && MS_CheckParm("doomdebug"))
					PicturePause();
			} while (LastScan != sc_Escape);

		IN_ClearKeysDown ();
		fontnumber = oldfontnumber;
}

int helpextern = T_HELPART;

void HelpScreens (void)
{
	int artnum;
	char far *text;
	memptr layout;

	CA_UpLevel ();
	MM_SortMem ();

	artnum = helpextern;
	CA_CacheGrChunk (artnum);
	text = (char _seg *)grsegs[artnum];
	MM_SetLock (&grsegs[artnum], true);
	ShowArticle (text);

	MM_FreePtr (&grsegs[artnum]);
	VW_FadeOut();
	FreeMusic ();
	CA_DownLevel ();
	MM_SortMem ();
}

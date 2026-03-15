// WL_PLAY.C

#include "WL_DEF.H"
#pragma hdrstop

#define sc_Question	0x35

boolean		madenoise;
exit_t		playstate;

objtype 	objlist[MAXACTORS],*new,*obj,*player,*lastobj,
			*objfreelist,*killerobj;

unsigned	farmapylookup[MAPSIZE];
byte		*nearmapylookup[MAPSIZE];

boolean		godmode,invisibility,berserk,biosuit,godflag;

byte far shadetable[SHADE_COUNT][256];
int LSHADE_flag;

byte		tilemap[MAPSIZE][MAPSIZE];
byte		spotvis[MAPSIZE][MAPSIZE];
objtype		*actorat[MAPSIZE][MAPSIZE];

unsigned	mapwidth,mapheight,tics;
boolean		compatability;
byte		*updateptr;
unsigned	mapwidthtable[64];
unsigned	uwidthtable[UPDATEHIGH];
unsigned	blockstarts[UPDATEWIDE*UPDATEHIGH];
byte		update[UPDATESIZE];

boolean		mouseenabled,joystickenabled,joypadenabled,joystickprogressive;
int			joystickport;
int			dirscan[4] = {sc_UpArrow,sc_RightArrow,sc_DownArrow,sc_LeftArrow};
int			buttonscan[NUMBUTTONS] =
			{sc_Control,sc_Alt,sc_RShift,sc_Space,sc_1,sc_2,sc_3,sc_4};
int			buttonmouse[4]={bt_attack,bt_strafe,bt_use,bt_nobutton};
int			buttonjoy[4]={bt_attack,bt_strafe,bt_use,bt_run};
int			viewsize;
boolean		buttonheld[NUMBUTTONS];

boolean		demorecord,demoplayback;
char		far *demoptr, far *lastdemoptr;
memptr		demobuffer;

int			controlx,controly,strafe;
boolean		buttonstate[NUMBUTTONS];

void	CenterWindow(word w,word h);
void 	InitObjList (void);
void 	RemoveObj (objtype *gone);
void 	PollControls (void);
void 	StopMusic(void);
void 	StartMusic(void);
void	PlayLoop (void);

int songs[]=
{
D_IN_CIT_MUS,
D_E1M2_MUS,
D_E1M3_MUS,
D_QUADM_MUS,
D_E2M6_MUS,
D_DDTBLU_MUS,
D_STALKS_MUS,
D_DEAD_MUS,
D_SHAWN_MUS,
D_BETWEEN_MUS,

D_E3M2_MUS,
D_E1M9_MUS,
D_UNDERW_MUS,
D_E3M3_MUS,
D_E2M1_MUS,
D_E2M2_MUS,
DESCENT_MUS,
D_E3M1_MUS,
D_E2M8_MUS,
D_E1M4_MUS,

D_RUNNIN_MUS,
D_E2M2_MUS,
D_E2M6_MUS,
D_UNDERW_MUS,
D_E3M3_MUS,
D_DEAD_MUS,
D_E2M1_MUS,
D_IN_CIT_MUS,
D_SHAWN_MUS,
D_E1M2_MUS,

D_DDTBLU_MUS,
D_E3M1_MUS,
D_STALKS_MUS,
D_SHAWN_MUS,
D_E3M3_MUS,
D_UNDERW_MUS,
D_IN_CIT_MUS,
DESCENT_MUS,
D_QUADM_MUS,
D_E2M1_MUS,
D_E2M6_MUS,
};

#define BASEMOVE		35
#define RUNMOVE			70
#define BASETURN		35
#define RUNTURN			70

#define JOYSCALE		2

void PollKeyboardButtons (void)
{
	int		i;

	for (i=0;i<NUMBUTTONS;i++)
		if (Keyboard[buttonscan[i]])
			buttonstate[i] = true;
}

void PollMouseButtons (void)
{
	int	buttons;

	buttons = IN_MouseButtons ();

	if (buttons&1)
		buttonstate[buttonmouse[0]] = true;
	if (buttons&2)
		buttonstate[buttonmouse[1]] = true;
	if (buttons&4)
		buttonstate[buttonmouse[2]] = true;
}

void PollJoystickButtons (void)
{
	int	buttons;
	buttons = IN_JoyButtons ();
	if (joystickport && !joypadenabled)
	{
		if (buttons&4)
			buttonstate[buttonjoy[0]] = true;
		if (buttons&8)
			buttonstate[buttonjoy[1]] = true;
	}
	else
	{
		if (buttons&1)
			buttonstate[buttonjoy[0]] = true;
		if (buttons&2)
			buttonstate[buttonjoy[1]] = true;
		if (joypadenabled)
		{
			if (buttons&4)
				buttonstate[buttonjoy[2]] = true;
			if (buttons&8)
				buttonstate[buttonjoy[3]] = true;
		}
	}
}

void PollKeyboardMove (void)
{
	if (buttonstate[bt_run])
	{
		if (Keyboard[dirscan[di_north]])
			controly -= RUNMOVE*tics;
		if (Keyboard[dirscan[di_south]])
			controly += RUNMOVE*tics;
		if (Keyboard[dirscan[di_west]])
			controlx -= RUNMOVE*tics;
		if (Keyboard[dirscan[di_east]])
			controlx += RUNMOVE*tics;
		if (Keyboard[sc_A])
			strafe -= RUNMOVE*tics;
		if (Keyboard[sc_S])
			strafe += RUNMOVE*tics;
	}
	else
	{
		if (Keyboard[dirscan[di_north]])
			controly -= BASEMOVE*tics;
		if (Keyboard[dirscan[di_south]])
			controly += BASEMOVE*tics;
		if (Keyboard[dirscan[di_west]])
			controlx -= BASEMOVE*tics;
		if (Keyboard[dirscan[di_east]])
			controlx += BASEMOVE*tics;
		if (Keyboard[sc_A])
			strafe -= BASEMOVE*tics;
		if (Keyboard[sc_S])
			strafe += BASEMOVE*tics;
	}
}

void PollMouseMove (void)
{
	int	mousexmove,mouseymove;

	Mouse(MDelta);
	mousexmove = _CX;
	mouseymove = _DX;

	controlx += mousexmove*10/(13-mouseadjustment);
	controly += mouseymove*20/(13-mouseadjustment);
}

void PollJoystickMove (void)
{
	int	joyx,joyy;

	INL_GetJoyDelta(joystickport,&joyx,&joyy);

	if (joystickprogressive)
	{
		if (joyx > 64)
			controlx += (joyx-64)*JOYSCALE*tics;
		else if (joyx < -64)
			controlx -= (-joyx-64)*JOYSCALE*tics;
		if (joyy > 64)
			controlx += (joyy-64)*JOYSCALE*tics;
		else if (joyy < -64)
			controly -= (-joyy-64)*JOYSCALE*tics;
	}
	else if (buttonstate[bt_run])
	{
		if (joyx > 64)
			controlx += RUNMOVE*tics;
		else if (joyx < -64)
			controlx -= RUNMOVE*tics;
		if (joyy > 64)
			controly += RUNMOVE*tics;
		else if (joyy < -64)
			controly -= RUNMOVE*tics;
	}
	else
	{
		if (joyx > 64)
			controlx += BASEMOVE*tics;
		else if (joyx < -64)
			controlx -= BASEMOVE*tics;
		if (joyy > 64)
			controly += BASEMOVE*tics;
		else if (joyy < -64)
			controly -= BASEMOVE*tics;
	}
}

void PollControls (void)
{
	int		max,min,i;
	byte	buttonbits;

	if (demoplayback)
	{
		while (TimeCount<lasttimecount+DEMOTICS)
		;
		TimeCount = lasttimecount + DEMOTICS;
		lasttimecount += DEMOTICS;
		tics = DEMOTICS;
	}
	else if (demorecord)
	{
		while (TimeCount<lasttimecount+DEMOTICS);
		TimeCount = lasttimecount + DEMOTICS;
		lasttimecount += DEMOTICS;
		tics = DEMOTICS;
	}
	else
		CalcTics ();

	controlx = 0;
	controly = 0;
	strafe = 0;
	memcpy (buttonheld,buttonstate,sizeof(buttonstate));
	memset (buttonstate,0,sizeof(buttonstate));

	if (demoplayback)
	{
		buttonbits = *demoptr++;
		for (i=0;i<NUMBUTTONS;i++)
		{
			buttonstate[i] = buttonbits&1;
			buttonbits >>= 1;
		}
		controlx = *demoptr++;
		controly = *demoptr++;
		strafe = *demoptr++;

		if (demoptr == lastdemoptr)
			playstate = ex_completed;

		controlx *= (int)tics;
		controly *= (int)tics;
		strafe *= (int)tics;

		return;
	}
	PollKeyboardButtons ();

	if (mouseenabled)
		PollMouseButtons ();

	if (joystickenabled)
		PollJoystickButtons ();

	PollKeyboardMove ();

	if (mouseenabled)
		PollMouseMove ();

	if (joystickenabled)
		PollJoystickMove ();

	max = 100*tics;
	min = -max;
	if (controlx > max)
		controlx = max;
	else if (controlx < min)
		controlx = min;

	if (controly > max)
		controly = max;
	else if (controly < min)
		controly = min;

	if (strafe > max)
		strafe = max;
	else if (strafe < min)
		strafe = min;

	if (demorecord)
	{
		controlx /= (int)tics;
		controly /= (int)tics;
		strafe /= (int)tics;

		buttonbits = 0;

		for (i=NUMBUTTONS-1;i>=0;i--)
		{
			buttonbits <<= 1;
			if (buttonstate[i])
				buttonbits |= 1;
		}

		*demoptr++ = buttonbits;
		*demoptr++ = controlx;
		*demoptr++ = controly;
		*demoptr++ = strafe;

		if (demoptr >= lastdemoptr)
			Quit ("Demo buffer overflowed!");

		controlx *= (int)tics;
		controly *= (int)tics;
		strafe *= (int)tics;
	}
}

#define MAXX	320
#define MAXY	160

void	CenterWindow(word w,word h)
{
	FixOfs ();
	US_DrawWindow(((MAXX / 8) - w) / 2,((MAXY / 8) - h) / 2,w,h);
}
boolean	KeyIsPressed (void)
{
	byte x;

	for (x=0;x<NumCodes;x++)
		if (Keyboard[x])
			return true;
		return false;
	}
byte KeyDown;

void CheckKeys (void)
{
	int		i;
	byte	scan;
	unsigned	temp;

	if (screenfaded || demoplayback)
		return;

	scan = LastScan;

  if (Keyboard[sc_T])
  {
	if (switches.floorsceilings)
	{
		switches.floorsceilings=0;
		GetMessage("Floors/Ceilings OFF");
	}
	else
	{
		switches.floorsceilings=1;
		GetMessage("Floors/Ceilings ON");
	}
	IN_ClearKeysDown();
  }
  if (Keyboard[sc_P])
  {
	if (switches.paralaxsky)
	{
		switches.paralaxsky=0;
		GetMessage("Parallaxing skies OFF");
	}
	else
	{
		switches.paralaxsky=1;
		GetMessage("Parallaxing skies ON");
	}
	IN_ClearKeysDown();
  }
  if (Keyboard[sc_M])
  {
	if (switches.messages)
		switches.messages=0;
	else
	{
		switches.messages=1;
		GetMessage("Messages ON");
	}
	IN_ClearKeysDown();
  }
  if(Keyboard[sc_Equal] && viewsize<20) // Viewsize by + - keys
  {
      ClearMemory();

      NewViewSize(viewsize+1);

      IN_ClearKeysDown();
      DrawAllPlayBorder();
      PM_CheckMainMem();
  }

  if(Keyboard[sc_Minus] && viewsize>10) // Viewsize by + - keys
  {
      ClearMemory();
      NewViewSize(viewsize-1);
      IN_ClearKeysDown();
      PM_CheckMainMem();

      if(viewsize>20)
      {
	 DrawPlayScreen();
      }
      else
      {
	 DrawAllPlayBorder();
      }

  }
	if (Keyboard[sc_Tab] &&
		Keyboard[sc_BackSpace] &&
		Keyboard[sc_LShift] &&
		MS_CheckParm(PARMCHEAT))
	{
	 ClearMemory ();
	 CA_CacheGrChunk (STARTFONT+1);
	 ClearSplitVWB ();
	 VW_ScreenToScreen (displayofs,bufferofs,80,160);

	 Message("Cheat codes enabled!");
	 UNCACHEGRCHUNK(STARTFONT+1);
	 PM_CheckMainMem ();
	 IN_ClearKeysDown();
	 IN_Ack();

	 DrawAllPlayBorderSides ();
	 DebugOk=1;
	}
	if (Keyboard[sc_D] &&
		Keyboard[sc_L] &&
		Keyboard[sc_H])
	{
	ClearMemory ();
	CA_CacheGrChunk (STARTFONT+1);
	ClearSplitVWB ();
	VW_ScreenToScreen (displayofs,bufferofs,80,160);

	Message("DOOM: Legions of Hell\nCrack Software (C)\nVersion "STR_VERSION);

	UNCACHEGRCHUNK(STARTFONT+1);
	PM_CheckMainMem();
	IN_ClearKeysDown();
	IN_Ack();

	DrawAllPlayBorder ();
  }
	if (Paused)
	{
		bufferofs = displayofs;
		SD_MusicOff();
		IN_Ack();
		IN_ClearKeysDown ();
		SD_MusicOn();
		Paused = false;
		if (MousePresent)
			Mouse(MDelta);
		return;
	}

	if (
#ifndef DEBCHECK
		scan == sc_F10 ||
#endif
		scan == sc_F7)
	{
		ClearMemory ();
		ClearSplitVWB ();
		VW_ScreenToScreen (displayofs,bufferofs,80,160);
		US_ControlPanel(scan);

		 DrawAllPlayBorderSides ();

		if (scan == sc_F9)
		  StartMusic ();

		PM_CheckMainMem ();
		SETFONTCOLOR(0,15);
		IN_ClearKeysDown();
		return;
	}

	if ( (scan >= sc_F1 && scan <= sc_F7) || scan == sc_Escape)
	{
		StopMusic ();
		ClearMemory ();
		VW_FadeOut ();

		US_ControlPanel(scan);

		SETFONTCOLOR(0,15);
		IN_ClearKeysDown();
		DrawPlayScreen ();

		if (!startgame && !loadedgame)
		{
			VW_FadeIn ();
			StartMusic ();
		}
		if (loadedgame)
			playstate = ex_abort;
		lasttimecount = TimeCount;
		if (MousePresent)
			Mouse(MDelta);
		PM_CheckMainMem ();
		return;
	}
	if (Keyboard[sc_Tab] && DebugOk)
	{
		CA_CacheGrChunk (STARTFONT);
		fontnumber=0;
		SETFONTCOLOR(0,15);
		DebugKeys();
		if (MousePresent)
			Mouse(MDelta);
		lasttimecount = TimeCount;
		return;
	}

}

int	objcount;

void InitActorList (void)
{
	int	i;

	for (i=0;i<MAXACTORS;i++)
	{
		objlist[i].prev = &objlist[i+1];
		objlist[i].next = NULL;
	}

	objlist[MAXACTORS-1].prev = NULL;

	objfreelist = &objlist[0];
	lastobj = NULL;

	objcount = 0;

	GetNewActor ();
	player = new;

}

void GetNewActor (void)
{
	if (!objfreelist)
		return;
		//Quit ("GetNewActor: No free spots in objlist!");

	new = objfreelist;
	objfreelist = new->prev;
	memset (new,0,sizeof(*new));

	if (lastobj)
		lastobj->next = new;
	new->prev = lastobj;

	new->active = false;
	lastobj = new;

	objcount++;
}

void RemoveObj (objtype *gone)
{
	objtype **spotat;

	if (gone == player)
		Quit ("RemoveObj: Tried to remove the player!");

	gone->state = NULL;

	if (gone == lastobj)
		lastobj = (objtype *)gone->prev;
	else
		gone->next->prev = gone->prev;

	gone->prev->next = gone->next;

	gone->prev = objfreelist;
	objfreelist = gone;

	objcount--;
}

void StopMusic(void)
{
	int	i;

	SD_MusicOff();
	for (i = 0;i < LASTMUSIC;i++)
		if (audiosegs[STARTMUSIC + i])
		{
			MM_SetPurge(&((memptr)audiosegs[STARTMUSIC + i]),3);
			MM_SetLock(&((memptr)audiosegs[STARTMUSIC + i]),false);
		}
}

void StartMusic(void)
{
	musicnames	chunk;

	SD_MusicOff();
	chunk = songs[gamestate.mapon+gamestate.episode*10];

	MM_BombOnError (false);
	CA_CacheAudioChunk(STARTMUSIC + chunk);
	MM_BombOnError (true);
	if (mmerror)
		mmerror = false;
	else
	{
		MM_SetLock(&((memptr)audiosegs[STARTMUSIC + chunk]),true);
		SD_StartMusic((MusicGroup far *)audiosegs[STARTMUSIC + chunk]);
	}
}

#define NUMREDSHIFTS	6
#define REDSTEPS		8

#define NUMWHITESHIFTS	3
#define WHITESTEPS		20
#define WHITETICS		6

#define NUMGREENSHIFTS	2
#define GREENSTEPS		16
#define GREENTICS	4

byte	far redshifts[NUMREDSHIFTS][768];
byte	far whiteshifts[NUMWHITESHIFTS][768];
byte	far greenshifts[NUMGREENSHIFTS][768];

int		damagecount,bonuscount,biocount;
boolean	palshifted;

extern 	byte	far	gamepal;

void InitRedShifts (void)
{
	byte	far *workptr, far *baseptr;
	int		i,j,delta;

	for (i=1;i<=NUMREDSHIFTS;i++)
	{
		workptr = (byte far *)&redshifts[i-1][0];
		baseptr = &gamepal;

		for (j=0;j<=255;j++)
		{
			delta = 64-*baseptr;
			*workptr++ = *baseptr++ + delta * i / REDSTEPS;
			delta = -*baseptr;
			*workptr++ = *baseptr++ + delta * i / REDSTEPS;
			delta = -*baseptr;
			*workptr++ = *baseptr++ + delta * i / REDSTEPS;
		}
	}

	for (i=1;i<=NUMWHITESHIFTS;i++)
	{
		workptr = (byte far *)&whiteshifts[i-1][0];
		baseptr = &gamepal;

		for (j=0;j<=255;j++)
		{
			delta = 64-*baseptr;
			*workptr++ = *baseptr++ + delta * i / WHITESTEPS;
			delta = 62-*baseptr;
			*workptr++ = *baseptr++ + delta * i / WHITESTEPS;
			delta = 0-*baseptr;
			*workptr++ = *baseptr++ + delta * i / WHITESTEPS;
		}
	}
	for (i=1;i<=NUMGREENSHIFTS;i++)
	{
		workptr = (byte far *)&greenshifts[i-1][0];
		baseptr = &gamepal;

		for (j=0;j<255;j++)
		{
			delta = 0-*baseptr;
			*workptr++ = *baseptr++ + delta * i / GREENSTEPS;
			delta = 64-*baseptr;
			*workptr++ = *baseptr++ + delta * i / GREENSTEPS;
			delta = 0-*baseptr;
			*workptr++ = *baseptr++ + delta * i / GREENSTEPS;
		}
	}
}

void ClearPaletteShifts (void)
{
	bonuscount = damagecount = biocount = 0;
}

void StartBonusFlash (void)
{
	bonuscount = NUMWHITESHIFTS*WHITETICS;
}

void StartDamageFlash (int damage)
{
	damagecount += damage;
}
void StartBioFlash (void)
{
	biocount = NUMGREENSHIFTS*GREENTICS;
}
void UpdatePaletteShifts (void)
{
	int	red,white,green;

	if (bonuscount)
	{
		white = bonuscount/WHITETICS +1;
		if (white>NUMWHITESHIFTS)
			white = NUMWHITESHIFTS;
		bonuscount -= tics;
		if (bonuscount < 0)
			bonuscount = 0;
	}
	else
		white = 0;

	if (damagecount)
	{
		red = damagecount/10 +1;
		if (red>NUMREDSHIFTS)
			red = NUMREDSHIFTS;

		damagecount -= tics;
		if (damagecount < 0)
			damagecount = 0;
	}

	else
		red = 0;
	if (biocount)
	{
		green = biocount/10+1;
		if (green>NUMGREENSHIFTS)
			green=NUMGREENSHIFTS;

		biocount -= tics;
		if (biocount <0)
			biocount = 0;
	}
	else
		green=0;
	if (red)
	{
		VW_WaitVBL(1);
		VL_SetPalette (redshifts[red-1]);
		palshifted = true;
	}
	else if (white)
	{
		VW_WaitVBL(1);
		VL_SetPalette (whiteshifts[white-1]);
		palshifted = true;
	}
	else if (green)
	{
		VW_WaitVBL(1);
		VL_SetPalette (greenshifts[green-1]);
		palshifted = true;
	}
	else if (palshifted)
	{
		VW_WaitVBL(1);
		VL_SetPalette (&gamepal);
		palshifted = false;
	}
}

void FinishPaletteShifts (void)
{
	if (palshifted)
	{
		palshifted = 0;
		VW_WaitVBL(1);
		VL_SetPalette (&gamepal);
	}
}

void DoActor (objtype *ob)
{
	void (*think)(objtype *);

	if (!ob->active && !areabyplayer[ob->areanumber])
		return;

	if (!(ob->flags&(FL_NONMARK|FL_NEVERMARK)) )
		actorat[ob->tilex][ob->tiley] = NULL;


	if (!ob->ticcount)
	{
		think =	ob->state->think;
		if (think)
		{
			think (ob);
			if (!ob->state)
			{
				RemoveObj (ob);
				return;
			}
		}

		if (ob->flags&FL_NEVERMARK)
			return;

		if ( (ob->flags&FL_NONMARK) && actorat[ob->tilex][ob->tiley])
			return;

		actorat[ob->tilex][ob->tiley] = ob;
		return;
	}

	ob->ticcount-=tics;
	while ( ob->ticcount <= 0)
	{
		think = ob->state->action;
		if (think)
		{
			think (ob);
			if (!ob->state)
			{
				RemoveObj (ob);
				return;
			}
		}

		ob->state = ob->state->next;

		if (!ob->state)
		{
			RemoveObj (ob);
			return;
		}

		if (!ob->state->tictime)
		{
			ob->ticcount = 0;
			goto think;
		}

		ob->ticcount += ob->state->tictime;
	}

think:
	think =	ob->state->think;
	if (think)
	{
		think (ob);
		if (!ob->state)
		{
			RemoveObj (ob);
			return;
		}
	}

	if (ob->flags&FL_NEVERMARK)
		return;

	if ( (ob->flags&FL_NONMARK) && actorat[ob->tilex][ob->tiley])
		return;

	actorat[ob->tilex][ob->tiley] = ob;
}
byte GetColor(byte red,byte green,byte blue,byte far *palette)
{
   byte mincol=0;
   double mindist=200000.F,curdist,DRed, DGreen, DBlue;
   int col;

   for(col=0;col<256;col++)
   {
      DRed   = (double) (red - *(palette+(col*3)));
      DGreen = (double) (green - *(palette+(col*3)+1));
      DBlue  = (double) (blue - *(palette+(col*3)+2));
      curdist=DRed * DRed + DGreen * DGreen + DBlue * DBlue;
      if(curdist<mindist)
      {
	 mindist=curdist;
	 mincol=(byte) col;
      }
   }
   return mincol;
}

void GenerateShadeTable(byte red,byte green,byte blue,byte far *palette,int fog)
{
   int count;
   int n;
   double R,G,B,R1,G1,B1,DeltaR,DeltaG,DeltaB,StepR,StepG,StepB;

 if (playstate == ex_died)
      return;

   gamestate.faceframe = 0;
   DrawFace();

   VWB_Bar (63,86,100,12,1);
   PrintX=65;
   PrintY=88;
   fontnumber = 0;
   SETFONTCOLOR (15,0);
   US_Print("Loading level...");
   fontcolor = 44;

   VW_UpdateScreen();
   VW_FadeIn();
   // Set the fog-flag
   LSHADE_flag=fog;

   // Color loop
   for (count=0; count<256; count++)
   {
      // Get original palette color
      R=R1=(double) *(palette+(count*3));
      G=G1=(double) *(palette+(count*3)+1);
      B=B1=(double) *(palette+(count*3)+2);

      // Calculate difference to final color
      DeltaR=(double)red-R1;
      DeltaG=(double)green-G1;
      DeltaB=(double)blue-B1;

      // Calculate increment per step
      StepR=DeltaR / (double)(SHADE_COUNT+8);
      StepG=DeltaG / (double)(SHADE_COUNT+8);
      StepB=DeltaB / (double)(SHADE_COUNT+8);

      // Calc color for each shade of the active color
      for (n=0; n<SHADE_COUNT; n++)
      {
	 shadetable[n][count]=GetColor((byte)R,(byte)G,(byte)B,palette);

	 // Inc to next shade
	 R+=StepR;
	 G+=StepG;
	 B+=StepB;
      }
	if (count%8 == 0)
      {
	 VWB_Bar (165,86,90,12,37);
	 SETFONTCOLOR (15,0);
	 PrintX=167;
	 US_PrintUnsigned(count*100/256);
	 US_Print("% complete");
	 VW_WaitVBL(1);
	 VW_UpdateScreen();
      }
   }
}
long funnyticount;
long ambientcount;
void PlayLoop (void)
{
	int		give;
	int	helmetangle;

	switch (gamestate.episode*10+mapon)
	{

		default: GenerateShadeTable(0,0,0,&gamepal,LSHADE_FOG); break;
		case 2: case 5: case 11: case 20: case 31: case 33: case 34:
		GenerateShadeTable(0,0,0,&gamepal,LSHADE_NORMAL); break;
		case 40: GenerateShadeTable(32,0,0,&gamepal,LSHADE_FOG); break;
	}

	playstate = TimeCount = lasttimecount = 0;
	frameon = 0;
	running = false;
	anglefrac = 0;
	facecount = 0;
	funnyticount = 0;
	memset (buttonstate,0,sizeof(buttonstate));
	ClearPaletteShifts ();

	if (MousePresent)
		Mouse(MDelta);
	if (demoplayback)
		IN_StartAck ();

	do
	{
		if (virtualreality)
		{
			helmetangle = peek (0x40,0xf0);
			player->angle += helmetangle;
			if (player->angle >= ANGLES)
				player->angle -= ANGLES;
		}
		PollControls();
		madenoise = false;
		MoveDoors ();
		MovePWalls ();

		for (obj = player;obj;obj = obj->next)
			DoActor (obj);

		UpdatePaletteShifts ();

		ThreeDRefresh ();
		switch (gamestate.mapon)
		{
			case 0:
				ambientcount+=tics;
				if (ambientcount>55l*70)
				{
				ambientcount=0;
				if (!DigiPlaying)
					SD_PlaySound (GROWLSND);
				}
				break;
			case 6:
			       ambientcount+=tics;
			       if (ambientcount>45l*70)
			       {
			       ambientcount=0;
			       if (!DigiPlaying)
				SD_PlaySound (DMACTSND);
			       }
			       break;
			}
		if (gamestate.changingweapon == true)
		{
			if (gamestate.goingdown == false)
			{
				gamestate.weapchange -=25;
				if (gamestate.weapchange <= 0)
				{
					gamestate.weapchange = 0;
					gamestate.changingweapon = false;
				}
			}
		else
		{
			gamestate.weapchange += 25;
			if (gamestate.weapchange >= 200)
			{
				gamestate.goingdown = false;
				gamestate.weapon = gamestate.nextweapon;
			}
		}
}
		if (!godflag)
		{
			if (gamestate.godmode || godmode)
			{
				godmode = true;
				gamestate.godmodecount += tics;
				if (gamestate.godmodecount > 30l*70)
				{
					gamestate.godmodecount = 0;
					gamestate.godmode = false;
					godmode = false;
				}
			}
		}
			if (gamestate.berserk || berserk)
			{
				berserk = true;
				gamestate.berserkcount += tics;
				if (gamestate.berserkcount > 30l*70)
				{
					gamestate.berserkcount = 0;
					gamestate.berserk = false;
					berserk = false;
				}
			}
			if (gamestate.goggles)
			{
			gamestate.gogglescount += tics;
			if (gamestate.gogglescount >30l*70)
			{
				gamestate.gogglescount = 0;
				gamestate.goggles = false;
				}

			}
			if (gamestate.biosuit || biosuit)
			{
				biosuit=true;
				gamestate.biosuitcount += tics;
				if (gamestate.biosuitcount > 30l*70)
				{
					gamestate.biosuitcount=0;
					gamestate.biosuit=false;
					biosuit=false;
				}
			}
			if (gamestate.invisibility)
			{
			gamestate.invisibilitycount += tics;
			if (gamestate.invisibilitycount >30l*70)
			{
				gamestate.invisibilitycount = 0;
				gamestate.invisibility = false;
				}

			}
		gamestate.TimeCount+=tics;

		SD_Poll ();
		UpdateSoundLoc();

		if (screenfaded)
			VW_FadeIn ();

		CheckKeys();

		if (demoplayback)
		{
			if (IN_CheckAck ())
			{
				IN_ClearKeysDown ();
				playstate = ex_abort;
			}
		}

		if (virtualreality)
		{
			player->angle -= helmetangle;
			if (player->angle < 0)
				player->angle += ANGLES;
		}

	}while (!playstate && !startgame);

	if (playstate != ex_died)
		FinishPaletteShifts ();
}
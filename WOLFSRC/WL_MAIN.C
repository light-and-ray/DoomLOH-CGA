#include <conio.h>
#include "WL_DEF.H"
#pragma hdrstop
#define FOCALLENGTH     (0x5700l)
#define VIEWGLOBAL      0x10000

char str[80],str2[20];
boolean nospr;
boolean IsA386;

int dirangle[9] ={0,ANGLES/8,2*ANGLES/8,3*ANGLES/8,4*ANGLES/8,5*ANGLES/8,6*ANGLES/8,7*ANGLES/8,ANGLES};
fixed focallength;
unsigned screenofs;
unsigned		cgascreenofs;
int viewwidth;
int viewheight;
int centerx;
int shootdelta;
fixed scale,maxslope;
long heightnumerator;
int minheightdiv;
void Quit (char *error);
boolean startgame,loadedgame,virtualreality;
int mouseadjustment;

char	configname[13]="CONFIG.";

#ifdef WITH_PROFILER
profilermarker_t profilermarkers[NUM_PROFILER_MARKERS] =
{
	{ "THREEDREFRESH" },
	{ "WALLREFRESH" },
	{ "SCALEPOST" },
	{ "DRAWSCALED" },
	{ "DRAWWEAPON" },
	{ "CGABLIT" },
};
#endif

/*
=============================================================================

						 LOCAL VARIABLES

=============================================================================
*/


/*
====================
=
= ReadConfig
=
====================
*/

void ReadConfig(void)
{
	int file;
	SDMode sd;
	SMMode sm;
	SDSMode sds;
	if ((file = open(configname,O_BINARY|O_RDONLY))!=-1)
	{
		read(file,Scores,sizeof(HighScore)*MaxScores);
		read(file,&sd,sizeof(sd));
		read(file,&sm,sizeof(sm));
		read(file,&sds,sizeof(sds));
		read(file,&mouseenabled,sizeof(mouseenabled));
		read(file,&joystickenabled,sizeof(joystickenabled));
		read(file,&joypadenabled,sizeof(joypadenabled));
		read(file,&joystickprogressive,sizeof(joystickprogressive));
		read(file,&joystickport,sizeof(joystickport));
		read(file,&dirscan,sizeof(dirscan));
		read(file,&buttonscan,sizeof(buttonscan));
		read(file,&buttonmouse,sizeof(buttonmouse));
		read(file,&buttonjoy,sizeof(buttonjoy));
		read(file,&viewsize,sizeof(viewsize));
		read(file,&mouseadjustment,sizeof(mouseadjustment));
		read(file,&switches,sizeof(switches));
		close(file);
		if (sd == sdm_AdLib && !AdLibPresent && !SoundBlasterPresent)
		{
			sd = sdm_PC;
			sd = smm_Off;
		}
		if ((sds == sds_SoundBlaster && !SoundBlasterPresent) || (sds == sds_SoundSource && !SoundSourcePresent))
			sds = sds_Off;
			if (!MousePresent)
			mouseenabled = false;
			if (!JoysPresent[joystickport])
			joystickenabled = false;
			MainMenu[6].active=1;
			MainItems.curpos=0;
		}
		else
		{
			if (SoundBlasterPresent || AdLibPresent)
			{
				sd = sdm_AdLib;
				sm = smm_AdLib;
			}
			else
			{
				sd = sdm_PC;
				sm = smm_Off;
			}
			if (SoundBlasterPresent)
				sds = sds_SoundBlaster;
			else if (SoundSourcePresent)
				sds = sds_SoundSource;
			else
				sds = sds_Off;
			if (MousePresent)
				mouseenabled = true;
			joystickenabled = false;
			joypadenabled = false;
			joystickport = 0;
			joystickprogressive = false;
			viewsize = 20;
			mouseadjustment=5;
			switches.messages=1;
			switches.floorsceilings=1;
			switches.paralaxsky=1;
		}

	if (MS_CheckParm ("pcspeaker"))
	{
		sds = sds_PC;
	}
		SD_SetMusicMode (sm);
		SD_SetSoundMode (sd);
		SD_SetDigiDevice (sds);
	}

void WriteConfig(void)
{
	int file;
	file=open(configname,O_CREAT|O_BINARY|O_WRONLY,S_IREAD| S_IWRITE|S_IFREG);
	if (file != -1)
	{
		write(file,Scores,sizeof(HighScore) * MaxScores);
		write(file,&SoundMode,sizeof(SoundMode));
		write(file,&MusicMode,sizeof(MusicMode));
		write(file,&DigiMode,sizeof(DigiMode));
		write(file,&mouseenabled,sizeof(mouseenabled));
		write(file,&joystickenabled,sizeof(joystickenabled));
		write(file,&joypadenabled,sizeof(joypadenabled));
		write(file,&joystickprogressive,sizeof(joystickprogressive));
		write(file,&joystickport,sizeof(joystickport));
		write(file,&dirscan,sizeof(dirscan));
		write(file,&buttonscan,sizeof(buttonscan));
		write(file,&buttonmouse,sizeof(buttonmouse));
		write(file,&buttonjoy,sizeof(buttonjoy));
		write(file,&viewsize,sizeof(viewsize));
		write(file,&mouseadjustment,sizeof(mouseadjustment));
		write(file,&switches,sizeof(switches));
		close(file);
	}
}

char *JHParmStrings[]={"no386",nil};

void Patch386 (void)
{
	extern void far jabhack2(void);
	extern int far  CheckIs386(void);
	int i;

	for (i = 1;i < _argc;i++)
	if (US_CheckParm(_argv[i],JHParmStrings) == 0)
	{
		IsA386 = false;
		return;
	}
	if (CheckIs386())
	{
		IsA386 = true;
		jabhack2();
	}
	else
		IsA386 = false;
}

void NewGame (int difficulty,int episode)
{
	memset (&gamestate,0,sizeof(gamestate));
	gamestate.difficulty = difficulty;
   	gamestate.bestweapon = 3;
	gamestate.nextweapon = gamestate.weapon = gamestate.chosenweapon = wp_pistol;
	gamestate.armor = gamestate.ammo2 = gamestate.ammo3 =
	gamestate.ammo4 = 0;
	gamestate.health = 100;
	gamestate.ammo = 10;
	gamestate.ammomax = 200;
	gamestate.ammo2max = gamestate.ammo3max = 50;
	gamestate.ammo4max = 300;
	gamestate.lives = 3;
	gamestate.nextextra = EXTRAPOINTS;
	gamestate.episode=episode;
	startgame = true;
	if (!godflag)
		godmode = gamestate.godmode = false;
}

void DiskFlopAnim(int x,int y)
{
	static char which=0;
	if (!x && !y)
	return;

	VWB_DrawPic(x,y,C_DISKLOADING1PIC+which);
	VW_UpdateScreen();
	which^=1;
}

long DoChecksum(byte far *source,unsigned size,long checksum)
{
	unsigned i;
	for (i=0;i<size-1;i++)
		checksum += source[i]^source[i+1];
		return checksum;
}

boolean SaveTheGame(int file,int x,int y)
{
	struct diskfree_t dfree;
	long avail,size,checksum;
	objtype *ob,nullobj;

	if (_dos_getdiskfree(0,&dfree))
		Quit("Error in _dos_getdiskfree call");
	avail = (long)dfree.avail_clusters * dfree.bytes_per_sector * dfree.sectors_per_cluster;
	size = 0;
	for (ob = player;ob;ob=ob->next)
		size += sizeof(*ob);
		size += sizeof(nullobj);
		size += sizeof(gamestate) + sizeof(LRstruct)*8 + sizeof(tilemap) + sizeof(actorat) + sizeof(laststatobj) +
			sizeof(statobjlist) + sizeof(doorposition) + sizeof(pwallstate) + sizeof(pwallx) +
			sizeof(pwally) + sizeof(pwalldir) + sizeof(pwallpos); if (avail < size){ Message(STR_NOSPACE1"\n"STR_NOSPACE2);
		return false;
	}
		checksum = 0;
		DiskFlopAnim(x,y);
		CA_FarWrite (file,(void far *)&gamestate,sizeof(gamestate));
		checksum = DoChecksum((byte far *)&gamestate,sizeof(gamestate),checksum);
		DiskFlopAnim(x,y);
		CA_FarWrite (file,(void far *)&LevelRatios[0],sizeof(LRstruct)*8); checksum = DoChecksum((byte far *)&LevelRatios[0],sizeof(LRstruct)*8,checksum);
		DiskFlopAnim(x,y);
		CA_FarWrite (file,(void far *)tilemap,sizeof(tilemap));
		checksum = DoChecksum((byte far *)tilemap,sizeof(tilemap),checksum);
		DiskFlopAnim(x,y);
		CA_FarWrite (file,(void far *)actorat,sizeof(actorat));
		checksum = DoChecksum((byte far *)actorat,sizeof(actorat),checksum);
		CA_FarWrite (file,(void far *)areaconnect,sizeof(areaconnect));
		CA_FarWrite (file,(void far *)areabyplayer,sizeof(areabyplayer));
		for (ob = player ; ob ; ob=ob->next)
		{
			DiskFlopAnim(x,y);
			CA_FarWrite (file,(void far *)ob,sizeof(*ob));
		}
		nullobj.active = ac_badobject;
		DiskFlopAnim(x,y);
		CA_FarWrite (file,(void far *)&nullobj,sizeof(nullobj));
		DiskFlopAnim(x,y);
		CA_FarWrite (file,(void far *)&laststatobj,sizeof(laststatobj));
		checksum = DoChecksum((byte far *)&laststatobj,sizeof(laststatobj),checksum); DiskFlopAnim(x,y);
		CA_FarWrite (file,(void far *)statobjlist,sizeof(statobjlist));
		checksum = DoChecksum((byte far *)statobjlist,sizeof(statobjlist),checksum);
		DiskFlopAnim(x,y);
		CA_FarWrite (file,(void far *)doorposition,sizeof(doorposition));
		checksum = DoChecksum((byte far *)doorposition,sizeof(doorposition),checksum);
		DiskFlopAnim(x,y);
		CA_FarWrite (file,(void far *)doorobjlist,sizeof(doorobjlist));
		checksum = DoChecksum((byte far *)doorobjlist,sizeof(doorobjlist),checksum);
		DiskFlopAnim(x,y);
		CA_FarWrite (file,(void far *)&pwallstate,sizeof(pwallstate));
		checksum = DoChecksum((byte far *)&pwallstate,sizeof(pwallstate),checksum);
		CA_FarWrite (file,(void far *)&pwallx,sizeof(pwallx));
		checksum = DoChecksum((byte far *)&pwallx,sizeof(pwallx),checksum);
		CA_FarWrite (file,(void far *)&pwally,sizeof(pwally));
		checksum = DoChecksum((byte far *)&pwally,sizeof(pwally),checksum);
		CA_FarWrite (file,(void far *)&pwalldir,sizeof(pwalldir));
		checksum = DoChecksum((byte far *)&pwalldir,sizeof(pwalldir),checksum);
		CA_FarWrite (file,(void far *)&pwallpos,sizeof(pwallpos));
		checksum = DoChecksum((byte far *)&pwallpos,sizeof(pwallpos),checksum);
		CA_FarWrite (file,(void far *)&checksum,sizeof(checksum));
	return(true);
}

boolean LoadTheGame(int file,int x,int y)
{
	long checksum,oldchecksum;
	objtype *ob,nullobj;
	checksum = 0;
	DiskFlopAnim(x,y);
	CA_FarRead (file,(void far *)&gamestate,sizeof(gamestate));
	checksum = DoChecksum((byte far *)&gamestate,sizeof(gamestate),checksum);
	DiskFlopAnim(x,y);
	CA_FarRead (file,(void far *)&LevelRatios[0],sizeof(LRstruct)*8);
	checksum = DoChecksum((byte far *)&LevelRatios[0],sizeof(LRstruct)*8,checksum);
	DiskFlopAnim(x,y);
	SetupGameLevel ();
	DiskFlopAnim(x,y);
	CA_FarRead (file,(void far *)tilemap,sizeof(tilemap));
	checksum = DoChecksum((byte far *)tilemap,sizeof(tilemap),checksum);
	DiskFlopAnim(x,y);
	CA_FarRead (file,(void far *)actorat,sizeof(actorat));
	checksum = DoChecksum((byte far *)actorat,sizeof(actorat),checksum);
	CA_FarRead (file,(void far *)areaconnect,sizeof(areaconnect));
	CA_FarRead (file,(void far *)areabyplayer,sizeof(areabyplayer));
	InitActorList ();
	DiskFlopAnim(x,y);
	CA_FarRead (file,(void far *)player,sizeof(*player));
	while (1)
	{
		DiskFlopAnim(x,y);
		CA_FarRead (file,(void far *)&nullobj,sizeof(nullobj));
		if (nullobj.active == ac_badobject)
			break;
		GetNewActor ();
		memcpy (new,&nullobj,sizeof(nullobj)-4);
	}
	DiskFlopAnim(x,y);
	CA_FarRead (file,(void far *)&laststatobj,sizeof(laststatobj));
	checksum = DoChecksum((byte far *)&laststatobj,sizeof(laststatobj),checksum);
	DiskFlopAnim(x,y);
	CA_FarRead (file,(void far *)statobjlist,sizeof(statobjlist));
	checksum = DoChecksum((byte far *)statobjlist,sizeof(statobjlist),checksum);
	DiskFlopAnim(x,y);
	CA_FarRead (file,(void far *)doorposition,sizeof(doorposition));
	checksum = DoChecksum((byte far *)doorposition,sizeof(doorposition),checksum);
	DiskFlopAnim(x,y);
	CA_FarRead (file,(void far *)doorobjlist,sizeof(doorobjlist));
	checksum = DoChecksum((byte far *)doorobjlist,sizeof(doorobjlist),checksum);
	DiskFlopAnim(x,y);
	CA_FarRead (file,(void far *)&pwallstate,sizeof(pwallstate));
	checksum = DoChecksum((byte far *)&pwallstate,sizeof(pwallstate),checksum);
	CA_FarRead (file,(void far *)&pwallx,sizeof(pwallx));
	checksum = DoChecksum((byte far *)&pwallx,sizeof(pwallx),checksum);
	CA_FarRead (file,(void far *)&pwally,sizeof(pwally));
	checksum = DoChecksum((byte far *)&pwally,sizeof(pwally),checksum);
	CA_FarRead (file,(void far *)&pwalldir,sizeof(pwalldir));
	checksum = DoChecksum((byte far *)&pwalldir,sizeof(pwalldir),checksum);
	CA_FarRead (file,(void far *)&pwallpos,sizeof(pwallpos));
	checksum = DoChecksum((byte far *)&pwallpos,sizeof(pwallpos),checksum);
	CA_FarRead (file,(void far *)&oldchecksum,sizeof(oldchecksum));
		if (oldchecksum != checksum)
		{
			Message(STR_SAVECHT1"\n"STR_SAVECHT2"\n"STR_SAVECHT3"\n"STR_SAVECHT4);
			IN_ClearKeysDown();
			IN_Ack();
			gamestate.score = 0;
			gamestate.lives = 1;
			gamestate.weapon = gamestate.chosenweapon =
			gamestate.bestweapon = wp_pistol;
			gamestate.ammo = 10;
		}
	return true;
}

void ShutdownId (void)
{
	US_Shutdown ();
	SD_Shutdown ();
	PM_Shutdown ();
	IN_Shutdown ();
	VW_Shutdown ();
	CA_Shutdown ();
	MM_Shutdown ();
}

const float radtoint=(float)FINEANGLES/2/PI;

void BuildTables (void)
{
	int i;
	float angle,anglestep;
	double tang;
	fixed value;
		for (i=0;i<FINEANGLES/8;i++)
		{
			tang = tan( (i+0.5)/radtoint);
			finetangent[i] = tang*TILEGLOBAL;
			finetangent[FINEANGLES/4-1-i] = 1/tang*TILEGLOBAL;
		}
		angle = 0;
		anglestep = PI/2/ANGLEQUAD;
		for (i=0;i<=ANGLEQUAD;i++)
		{
			value=GLOBAL1*sin(angle);
			sintable[i]=sintable[i+ANGLES]=sintable[ANGLES/2-i] = value;
			sintable[ANGLES-i]=sintable[ANGLES/2+i] = value | 0x80000000l;
			angle += anglestep;
		}
}

void CalcProjection (long focal)
{
	int i;
	long intang;
	float angle;
	double tang;
	double planedist;
	double globinhalf;
	int halfview;
	double halfangle,facedist;
	focallength = focal;
	facedist = focal+MINDIST;
	halfview = viewwidth/2;
	scale = halfview*facedist/(VIEWGLOBAL/2);
	heightnumerator = (TILEGLOBAL*scale)>>6;
	minheightdiv = heightnumerator/0x7fff +1;
	for (i=0;i<halfview;i++)
	{
		tang = (long)i*VIEWGLOBAL/viewwidth/facedist;
		angle = atan(tang);
		intang = angle*radtoint;
		pixelangle[halfview-1-i] = intang;
		pixelangle[halfview+i] = -intang;
	}
	maxslope = finetangent[pixelangle[0]];
	maxslope >>= 8;
}

void SetupWalls (void)
{
	int i;
	for (i=1;i<MAXWALLTILES;i++)
	{
		horizwall[i]=(i-1)*2;
		vertwall[i]=(i-1)*2+1;
	}
}

void CheckStart (void)
{
	_AX=3;
	geninterrupt (0x10);
	printf("Starting: DOOM: Legions of Hell v"STR_VERSION"\n\nCheckForEpisodes: checking...\n");
	CheckForEpisodes();
	VW_WaitVBL (1*35);
	printf("**FULL VERSION**\n\n\n");
	printf("===============================================================================\n");
	printf("=                                             				      =\n");
	printf("=     	  		This is the FULL version of DOOM:LOH    	      =\n");
	printf("=         	    	    **You have all 4 episodes**         	      =\n");
	printf("=					      				      =\n");
	printf("===============================================================================\n");
		VW_WaitVBL (1*35);
}

void SignonScreen (void)
{
	unsigned        segstart,seglength;

#ifdef WITH_VGA
	VL_SetVGAPlaneMode ();
	VL_TestPaletteSet ();
	VL_SetPalette (&gamepal);
#else
	VL_SetCGAMode();
#endif

	if (!virtualreality)
	{
#ifdef WITH_VGA
		VW_SetScreen(0x8000,0);
		VL_MungePic (&introscn,320,200);
		VL_MemToScreen (&introscn,320,200,0,0);
		VW_SetScreen(0,0);
#else
		switch(cgamode)
		{
			case CGA_MODE4:
			case CGA_MODE5:
			VL_MemToScreen (&introscn,320,200,0,0);
			break;
			case CGA_COMPOSITE_MODE:
			VL_MemToScreen (&introscn + 16000,320,200,0,0);
			break;
			case TANDY_MODE:
			VL_MemToScreen (&introscn + 32000,320,200,0,0);
			break;
			case CGA_INVERSE_MONO:
			VL_MemToScreen (&introscn + 48000,320,200,0,0);
			break;
			case HERCULES720_MODE:
			case HERCULES640_MODE:
			VL_MemToScreen (&introscn,320,200,0,0);
			break;
		}
		VW_UpdateScreen();
#endif
	}
	segstart = FP_SEG(&introscn);
	seglength = 64000/16;
	if (FP_OFF(&introscn))
	{
		segstart++;
		seglength--;
	}
	MML_UseSpace (segstart,seglength);
}

void FinishSignon (void)
{
	VW_Bar (0,189,320,11,peekb(0xa000,0));
	WindowX = 0;
	WindowW = 320;
	PrintY = 190;
	SETFONTCOLOR(0x7,0x7);
	US_CPrint ("Press a key");

	#ifndef WITH_VGA
	VW_UpdateScreen();
	#endif

	if (!NoWait && !timedemo)
		IN_Ack ();
	VW_Bar (0,189,320,11,peekb(0xa000,0));
	PrintY = 190;
	SETFONTCOLOR(0x7,0x7);
	US_CPrint ("Working...");

	#ifndef WITH_VGA
	VW_UpdateScreen();
	#endif


	SETFONTCOLOR(0,15);
	if (!NoWait)
		VW_WaitVBL(3*70);
}

//===========================================================================

/*
=================
=
= MS_CheckParm
=
=================
*/

boolean MS_CheckParm (char far *check)
{
	int i;
	char *parm; for (i = 1;i<_argc;i++)
	{
		parm = _argv[i];
		while (!isalpha(*parm))
			if (!*parm++)
				break;
			if (!_fstricmp(check,parm))
			return true;
		}
	return false;
}

static int wolfdigimap[]={
		BARRELBOOMSND,		0,
		ATKBFGSND,		1,
		GROWLSND,		2,
		MUTDIE1SND,		3,
		MUTDIE2SND,		4,
		MUTSEE1SND,		5,
		MUTSEE2SND,		6,
		BOSSDIESND,		7,
		BOSSSEESND,		8,
		CACODIESND,		9,
		CACOSEESND,		10,
		CYBERDIESND,		11,
		CYBERSEESND,		12,
		PAINSND,		13,
		DOORCLOSESND,		14,
		DOOROPENSND,		15,
		IMPFIRESND,		16,
		IMPFIREBOOMSND,		17,
		BONUSITEMUPSND,	      	18,
		ITEMUPSND,		19,
		NOWAYSND,		20,
		PISTOLFIRESND,		21,
		PLASMAGUNFIRESND,	22,
		PLAYERDIESND,		23,
		PLAYERPAINSND,		24,
		SHOTGUYDIE1SND,		25,
		SHOTGUYDIE2SND,		26,
		SHOTGUYDIE3SND,		27,
		SHOTGUYPAINSND,		28,
		SHOTGUYSEE1SND,		29,
		SHOTGUYSEE2SND,		30,
		SHOTGUYSEE3SND,		31,
		FISTPUNCHSND,		32,
		RLAUNCHERFIRESND,	33,
		RLAUNCHERBOOM1SND,	34,
		CHAINSAWFIRESND,	35,
		CHAINSAWIDLESND,	36,
		DEMONFIRESND,		37,
		DEMONDIESND,		38,
		DEMONSEESND,		39,
		SHOTGUNFIRESND,		40,
		SPIDERDIESND,		41,
		SPIDERSEESND,		42,
		SWITCH1SND,		43,
		SWITCH2SND,		44,
		TELEPORTSND,		45,
		WEAPONPICKUPSND,	46,
		DSHOTGUNFIRESND,	47,
		DSOPENSND,		48,
		DSLOADSND,		49,
		DSCLOSESND,		50,
		MOVEUPDOWNSND,		51,
		SLOPSND,		52,
		DMACTSND,		53,
		RISINGSND,		54,
		DTHKNTSEESND,           55,
		DTHKNTDIESND,           56,
		MANATKSND,              57,
		MANDIESND,              58,
		MANSEESND,              59,
		MANPAINSND,             60,
		PAINDIESND,             61,
		PAINPAINSND,            62,
		PAINSEESND,             63,
		REVATKSND,              64,
		REVDIESND,              65,
		REVPUNCHSND,            66,
		REVSEESND,              67,
		REVSWINGSND,            68,
		VILEATKSND,             69,
		VILEDIESND,             70,
		VILESEESND,             71,
		VILEPAINSND,            72,
		SOULATKSND,             73,
		TELE2SND,               74,
		LASTSOUND
};

void InitDigiMap (void)
{
	int *map;
	for (map = wolfdigimap;*map != LASTSOUND;map += 2)
	{
		DigiMap[map[0]] = map[1];
		PM_CheckMainMem();
		ClearMemory();
	}
}

void InitGame (void)
{
	int i,x,y;
	unsigned *blockstart;
	if (MS_CheckParm ("virtual"))
		virtualreality = true;
	else
		virtualreality = false;

	if (MS_CheckParm ("timedemo"))
	{
		timedemo = true;
	}

	if (MS_CheckParm ("composite"))
	{
		cgamode = CGA_COMPOSITE_MODE;
	}
	if (MS_CheckParm ("classic"))
	{
		cgamode = CGA_MODE4;
	}
	if (MS_CheckParm ("tandy"))
	{
		cgamode = TANDY_MODE;
	}
	if (MS_CheckParm ("lcd"))
	{
		cgamode = CGA_INVERSE_MONO;
	}
	if (MS_CheckParm ("hercules720"))
	{
		cgamode = HERCULES720_MODE;
		//adjustherculesaspect = true;
	}
	if (MS_CheckParm ("hercules"))
	{
		cgamode = HERCULES640_MODE;
		//adjustherculesaspect = true;
	}
	if (MS_CheckParm("wide"))
	{
		usewiderendering = true;
	}


	MM_Startup ();                  // so the signon screen can be freed

	SignonScreen ();

	VW_Startup ();
	IN_Startup ();
	PM_Startup ();
	PM_UnlockMainMem ();
	SD_Startup ();
	CA_Startup ();
	US_Startup ();


#ifndef SPEAR
	if (mminfo.mainmem < 235000L)
#else
	if (mminfo.mainmem < 257000L && !MS_CheckParm("debugmode"))
#endif
	// {
	// 	memptr screen;

	// 	CA_CacheGrChunk (ERRORSCREEN);
	// 	screen = grsegs[ERRORSCREEN];
	// 	ShutdownId();
	// 	movedata ((unsigned)screen,7+7*160,0xb800,0,17*160);
	// 	gotoxy (1,23);
	// 	exit(1);
	// }


//
// build some tables
//
	InitDigiMap ();

	for (i=0;i<MAPSIZE;i++)
	{
		nearmapylookup[i] = &tilemap[0][0]+MAPSIZE*i;
		farmapylookup[i] = i*64;
	}

	for (i=0;i<PORTTILESHIGH;i++)
		uwidthtable[i] = UPDATEWIDE*i;

	blockstart = &blockstarts[0];
	for (y=0;y<UPDATEHIGH;y++)
		for (x=0;x<UPDATEWIDE;x++)
			*blockstart++ = SCREENWIDTH*16*y+x*TILEWIDTH;

	updateptr = &update[0];

#ifdef WITH_VGA
	bufferofs = 0;
	displayofs = 0;
#endif
	ReadConfig ();


//
// draw intro screen stuff
//
	if (!virtualreality)
		IntroScreen ();

//
// load in and lock down some basic chunks
//

	CA_CacheGrChunk(STARTFONT);
	MM_SetLock (&grsegs[STARTFONT],true);

	LoadLatchMem ();
	BuildTables ();          // trig tables
	SetupWalls ();

#if 0
{
	int temp,
	i;
	temp = viewsize;
	profilehandle = open("SCALERS.TXT", O_CREAT | O_WRONLY | O_TEXT);
	for (i=1;i<20;i++)
	NewViewSize(i);
	viewsize = temp;
	close(profilehandle);}
#endif
	NewViewSize (viewsize);
	InitRedShifts ();
	if (!virtualreality)
		FinishSignon();

#ifdef WITH_VGA
	displayofs = PAGE1START;
	bufferofs = PAGE2START;
#else
	VW_UpdateScreen();
#endif

	if (virtualreality)
	{
		NoWait = true;
		geninterrupt(0x60);
	}
}

boolean SetViewSize (unsigned width, unsigned height)
{
	if(usewiderendering)
	{
		weaponviewheight = ((height >> 1)&~1)+1;
	}
	else
	{
		weaponviewheight = (height&~1)+1;
	}

	if(cgamode == HERCULES720_MODE && adjustherculesaspect)
	{
		height += height >> 1;
		height = height&~7;
	}

	viewwidth = width&~15;                  // must be divisable by 16
	viewheight = height&~1;                 // must be even
	centerx = viewwidth/2-1;
	shootdelta = viewwidth/10;
	screenofs = ((200-STATUSLINES-viewheight)/2*SCREENWIDTH+(320-viewwidth)/8);
	cgascreenofs = ((200-STATUSLINES-viewheight)/4*SCREENWIDTH+(320-viewwidth)/8);

	if(cgamode == HERCULES720_MODE)
	{
		screenofs = ((348-STATUSLINES-viewheight)/8*linewidth+(360-viewwidth)/8);
	}
	else if(cgamode == HERCULES640_MODE)
	{
		screenofs = ((200-STATUSLINES-viewheight)/4*linewidth+(320-viewwidth)/8);
	}
//
// calculate trace angles and projection constants
//
	CalcProjection (FOCALLENGTH);

//
// build all needed compiled scalers
//
//	MM_BombOnError (false);


	SetupScaling (viewwidth*1.5);
#if 0
MM_BombOnError (true);
if (mmerror)
{
	Quit ("Can't build scalers!");
	mmerror = false;
	return false;
}
#endif
	return true;
}

void ShowViewSize (int width)
{
	int oldwidth,oldheight;
	oldwidth = viewwidth;
	oldheight = viewheight;
	viewwidth = width*16;
	viewheight = width*16*HEIGHTRATIO;
	DrawPlayBorder ();
	viewheight = oldheight;
	viewwidth = oldwidth;
}

void NewViewSize (int width)
{
	CA_UpLevel ();
	MM_SortMem ();
	viewsize = width;
	SetViewSize (width*16,width*16*HEIGHTRATIO);
	CA_DownLevel ();
}

void Quit (char *error)
{
	unsigned finscreen;
	memptr screen;

	if (virtualreality)
		geninterrupt(0x61);

	ClearMemory ();

	// TODO : Fix me
	if(cgamode == HERCULES720_MODE || cgamode == HERCULES640_MODE)
	{
		ShutdownId ();
		clrscr();
		if(error && *error)
		{
			printf("Error: %s\n", error);
		}
		exit(0);
	}


	if (!*error)
	{
		WriteConfig();
		SD_MusicOff();
		_AX=3;
		geninterrupt(0x10);
	}
	else
	{}
	ShutdownId ();

	if (error && *error)
	{
		printf("DOOM: Legions of Hell  ERROR!\nPlease contact me and report the error\n\n");
		gotoxy(2,4);
		puts(error);
		printf("\n");
		gotoxy (1,8);
		exit(1);
	}
	else if (!error || !(*error))
	{
		clrscr();
		printf("-= DOOM: Legions of Hell =\n(C)2007 Crack Software\n Thanks for playing\n"); printf("\n");} exit(0);}

static  char *ParmStrings[] = {"baby","easy","normal","hard",""};
void    DemoLoop (void)
{
	static int LastDemo;
	int i,level;
	long nsize;
	memptr nullblock;

	StartCPMusic(INTROSONG);

	if (!NoWait)
		CSLOGOAnim ();
		VW_FadeOut ();
	while (1)
	{
		while (!NoWait)
		{
			MM_SortMem ();
			CA_CacheScreen (TITLEPIC);
			VW_UpdateScreen ();
			VW_FadeIn ();
			if (IN_UserInput(TickBase*10))
			break;
		VW_FadeOut();
		CA_CacheScreen (CREDITSPIC);
		VW_UpdateScreen();
		VW_FadeIn ();
		if (IN_UserInput(TickBase*10))
		break;
			VW_FadeOut ();
			DrawHighScores ();
			VW_UpdateScreen ();
			VW_FadeIn ();
		if (IN_UserInput(TickBase*10))
			break;
		PlayDemo (LastDemo++%4);
		if (playstate == ex_abort)
			break;
		StartCPMusic(INTROSONG);
	}
	VW_FadeOut ();
	if (Keyboard[sc_Tab] && MS_CheckParm(PARMCHEAT))
		RecordDemo ();
	else
		US_ControlPanel (0);
	if (startgame || loadedgame){GameLoop ();
		VW_FadeOut();
		StartCPMusic(INTROSONG);
		}
	}
}

char *nosprtxt[]={"nospr",nil};

void main (void)
{
	int i;

	CheckForEpisodes();
	Patch386 ();
	InitGame ();
	DemoLoop();
	CA_LoadAllSounds();
	ClearMemory();
	PM_CheckMainMem();
	Quit("Demo loop exited???");
}
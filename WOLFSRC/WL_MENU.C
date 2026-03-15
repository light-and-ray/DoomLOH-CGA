#include "wl_def.h"
#pragma hdrstop

CP_iteminfo
	MainItems={MENU_X,MENU_Y+5,8,readthis,24},
	SndItems={SM_X+26,SM_Y1+40,3,0,52},
	LSItems={LSM_X,LSM_Y-3,6,0,24},
	CtlItems={CTL_X+16,CTL_Y-28,8,-1,24},
	CusItems={8,CTL_Y+4,7,-1,0},
	NewEitems={NE_X+8,NE_Y+47,4,0,88},
	NewItems={NM_X-8,NM_Y-37,4,0,40},
	OptItems={MENU_X,MENU_Y-3,3,0,24};

#pragma warn -sus
CP_itemtype far MainMenu[]={
	{1,"",CP_NewGame},
	{1,"",CP_Options},
	{1,"",CP_LoadGame},
	{0,"",CP_SaveGame},
	{2,"",CP_ReadThis},
	{1,"",CP_ViewScores},
	{1,"",0},
	{1,"",0}
},
OptMenu[]={
	{1,"",CP_Sound},
	{1,"",CP_Control},
	{1,"",CP_ChangeView}
},
far SndMenu[]={
	{1,"",0},
	{1,"",0},
	{1,"",0}
},
far CtlMenu[]={
	{0,"",0},
	{0,"",0},
	{0,"",0},
	{0,"",0},
	{0,"",MouseSensitivity},
	{1,"",CustomControls}
},
#pragma warn +sus
far NewEmenu[]={
	{1,"",0},
	{1,"",0},
	{1,"",0},
	{1,"",0}
},
far NewMenu[]={
	{1,"",0},
	{1,"",0},
	{1,"",0},
	{1,"",0}
},
far LSMenu[]={
	{1,"",0},
	{1,"",0},
	{1,"",0},
	{1,"",0},
	{1,"",0},
	{1,"",0}
},
far CusMenu[]={
	{1,"",0},
	{0,"",0},
	{1,"",0},
	{0,"",0},
	{0,"",0},
	{1,"",0},
	{1,"",0}
};

int color_hlite[]={DEACTIVE,HIGHLIGHT,READHCOLOR,0x67},
color_norml[]={DEACTIVE,TEXTCOLOR,READCOLOR,0x6b};

int EpisodeSelect[4]={1};

int SaveGamesAvail[6],StartGame,SoundStatus=1,pickquick;

char SaveGameNames[6][32],SaveName[13]="SAVEGAM?.";

	static byte*ScanNames[] ={
"?","?","1","2","3","4","5","6","7","8","9","0","-","+","?","?","Q","W","E","R","T","Y","U","I","O","P","[","]","|","?","A","S",
"D","F","G","H","J","K","L",";","\"","?","?","?","Z","X","C","V","B","N","M",",",".","/","?","?","?","?","?","?","?","?","?","?",
"?","?","?","?","?","?","?","?","\xf","?","-","\x15","5","\x11","+","?","\x13","?","?","?","?","?","?","?","?","?","?","?","?","?","?","?",
"?","?","?","?","?","?","?","?","?","?","?","?","?","?","?","?","?","?","?","?","?","?","?","?","?","?","?","?","?","?","?","?"
},
far ExtScanCodes[] ={1,0xe,0xf,0x1d,0x2a,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,0x40,0x41,0x42,0x43,0x44,0x57,0x59,0x46,0x1c,0x36,
0x37,0x38,0x47,0x49,0x4f,0x51,0x52,0x53,0x45,0x48,0x50,0x4b,0x4d,0x00},*ExtScanNames[] =
{"Esc","BkSp","Tab","Ctrl","LShft","Space","CapsLk","F1","F2","F3","F4","F5","F6","F7","F8","F9","F10","F11","F12","ScrlLk","Enter","RShft",
"PrtSc","Alt","Home","PgUp","End","PgDn","Ins","Del","NumLk","Up","Down","Left","Right",""};

void US_ControlPanel(byte scancode)
{
	int which,i,start;
	if (ingame)
	if (CP_CheckQuick(scancode))
	return;
		StartCPMusic(MENUSONG);
		SetupControlPanel();
		switch(scancode)
		{
			case sc_F1:
				HelpScreens();
				goto finishup;
			case sc_F2:
				CP_SaveGame(0);
				goto finishup;
			case sc_F3:
				CP_LoadGame(0);
				goto finishup;
			case sc_F4:
				CP_Sound();
				goto finishup;
			case sc_F5:
				CP_ChangeView();
				goto finishup;
			case sc_F6:
				CP_Control();
				goto finishup;

			finishup:
			fontnumber=0;
			return;
		}
		DrawMainMenu();
		MenuFadeIn();
		StartGame=0;
		do{
			DrawMainMenu();
			which=HandleMenu(&MainItems,&MainMenu[0],NULL);
			switch(which)
			{
				case viewscores:
					if (MainMenu[viewscores].routine == NULL)
					if (CP_EndGame())
						StartGame=1;
						DrawMainMenu();
						MenuFadeIn();
					break;
				case backtodemo:
					MM_SortMem();
					StartGame=1;
					if (!ingame)
						StartCPMusic(INTROSONG);
						VL_FadeOut(0,255,0,0,0,10);
					break;
				case -1:
				case quit:
					CP_Quit();
					break;
				default:
					if (!StartGame)
					{
						DrawMainMenu();
						MenuFadeIn();
					}
				}
			} while(!StartGame);
			fontnumber=0;
			if (startgame || loadedgame)
			{
			#pragma warn -sus
			MainMenu[viewscores].routine = NULL;
			#pragma warn +sus
		}
}

void DrawMainMenu(void)
{
	ClearMScreen();
	CacheLump(MAINMENU_LUMP_START,MAINMENU_LUMP_END);
	VWB_DrawPic(98,0,M_DOOMPIC);
	VWB_DrawPic(100,60,M_NEWGAMEPIC);
	VWB_DrawPic(100,76,M_OPTIONSPIC);
	VWB_DrawPic(100,92,M_LOADGAMEPIC);
	VWB_DrawPic(100,108,M_SAVEGAMEPIC);
	VWB_DrawPic(100,124,M_READTHISPIC);
	VWB_DrawPic(100,156,M_BACKTOPIC);
	if (ingame)
	{
		VWB_DrawPic(100,140,M_ENDGAMEPIC);
		VWB_DrawPic(196,156,M_GAMEPIC);
	}
	else
	{
		VWB_DrawPic(100,140,M_VIEWSCORESPIC);
		VWB_DrawPic(196,156,M_DEMOPIC);
	}
	VWB_DrawPic(100,172,M_QUITGAMEPIC);
	DrawWindow (70,56,23,134,BKGDCOLOR);
	if (ingame)
		MainMenu[backtodemo].active=2;
	else
		MainMenu[backtodemo].active=1;
	DrawMenu(&MainItems,&MainMenu[0]);
	UnCacheLump(MAINMENU_LUMP_START,MAINMENU_LUMP_END);
	VW_UpdateScreen();
}

void CP_Options(void)
{
	int which;
	DrawOptionsMenu();
	MenuFadeIn();
	WaitKeyUp();
	do
	{
		which=HandleMenu(&OptItems,&OptMenu[0],NULL);
		DrawOptionsMenu();
		MenuFadeIn();
		WaitKeyUp();
	} while(which>=0);
	MenuFadeOut();
}

void DrawOptionsMenu(void)
{
	ClearMScreen();
	CA_CacheGrChunk(M_OPTIONSPIC);
	CacheLump (OPTIONS_LUMP_START,OPTIONS_LUMP_END);
	VWB_DrawPic(114,28,M_OPTIONSPIC);
	VWB_DrawPic(100,52,O_SOUNDPIC);
	VWB_DrawPic(100,68,O_CONTROLPIC);
	VWB_DrawPic(100,84,O_SCREENSIZEPIC);
	DrawWindow (70,48,23,54,BKGDCOLOR);
	DrawMenu(&OptItems,&OptMenu[0]);
	DrawMenuGun(&OptItems);
	UNCACHEGRCHUNK (M_OPTIONSPIC);
	UnCacheLump (OPTIONS_LUMP_START,OPTIONS_LUMP_END);
	VW_UpdateScreen();
}

void CP_ReadThis(void)
{
	HelpScreens();
	StartCPMusic(MENUSONG);
}

int CP_CheckQuick(unsigned scancode)
{
	switch(scancode)
	{
		case sc_F7:
			CA_CacheGrChunk(STARTFONT+1);
			WindowH=160;
			if (GetYorN(7,10,ENDSTRPIC))
			{
				playstate = ex_died;
				pickquick = gamestate.lives = 0;
			}
			DrawAllPlayBorder();
			WindowH=200;
			fontnumber=0;
			MainMenu[savegame].active = 0;
			return 1;
	   /*	case sc_F8:
			if (SaveGamesAvail[LSItems.curpos] && pickquick)
			{
				CA_CacheGrChunk(STARTFONT+1);
				fontnumber = 1;
				Message(STR_SAVING"...");
				CP_SaveGame(1);
				fontnumber=0;
			}
			else
			{
			CA_CacheGrChunk(STARTFONT+1);
			CacheLump (LOADSAVE_LUMP_START,LOADSAVE_LUMP_END);
			VW_FadeOut ();
			StartCPMusic(MENUSONG);
			pickquick=CP_SaveGame(0);
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
			UNCACHEGRCHUNK(C_DISKLOADING1PIC);
			UNCACHEGRCHUNK(C_DISKLOADING2PIC);
			UnCacheLump (LOADSAVE_LUMP_START,LOADSAVE_LUMP_END);
			UNCACHEGRCHUNK (STARTFONT+1);
		}
		return 1;
	case sc_F9:
		if (SaveGamesAvail[LSItems.curpos] && pickquick)
		{
			char string[100]=STR_LGC;
			CA_CacheGrChunk(STARTFONT+1);
			fontnumber = 1;
			strcat(string,SaveGameNames[LSItems.curpos]);
			strcat(string,"\"?");
			if (Confirm(string))
				CP_LoadGame(1);
			DrawAllPlayBorder();
			fontnumber=0;
		}
		else
		{
			CA_CacheGrChunk(STARTFONT+1);
			CacheLump (LOADSAVE_LUMP_START,LOADSAVE_LUMP_END);
			VW_FadeOut ();
			StartCPMusic(MENUSONG);
			pickquick=CP_LoadGame(0);
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
			UNCACHEGRCHUNK(C_DISKLOADING1PIC);
			UNCACHEGRCHUNK(C_DISKLOADING2PIC);
		}
		return 1;   */
	case sc_F10:
		CA_CacheGrChunk(STARTFONT+1);
		WindowX=WindowY=0;
		WindowW=320;WindowH=160;
		if (GetYorN(9,10,END1PIC))
		{
			int i;
			VW_UpdateScreen();
			SD_MusicOff();
			SD_StopSound();
			MenuFadeOut();
			for (i=1;i<=0xf5;i++)
				alOut(i,0);
			Quit(NULL);
		}
		DrawAllPlayBorder();
		WindowH=200;
		fontnumber=0;
		return 1;
	}
	return 0;
}

int CP_EndGame(void)
{
	if (!GetYorN(7,12,ENDSTRPIC))
		return 0;
	pickquick = gamestate.lives = 0;
	playstate = ex_died;
		#pragma warn -sus
		MainMenu[savegame].active = 0;
		MainMenu[viewscores].routine=CP_ViewScores;
		_fstrcpy(MainMenu[viewscores].string,"");
		#pragma warn +sus
	return 1;
}

void CP_ViewScores(void)
{
	fontnumber=0;
	DrawHighScores ();
	VW_UpdateScreen ();
	MenuFadeIn();
	fontnumber=1;
	IN_Ack();
	MenuFadeOut();
}

void CP_NewGame(void)
{
	int which,episode;
	firstpart:
		DrawNewEpisode();
		do
		{
			which=HandleMenu(&NewEitems,&NewEmenu[0],NULL);
			switch(which)
			{
				case -1:
				MenuFadeOut();
					return;
				default:
				if (!EpisodeSelect[which])
				{
					SD_PlaySound (WTFSND);
					Message("This episode is locked!");
					IN_ClearKeysDown();
					IN_Ack();
					DrawNewEpisode();
					which = 0;
				}
				else
				{
					episode = which;
					which = 1;
				}
				break;
			}
		} while (!which);
		ShootSnd();
		if (ingame)
			if (!GetYorN(7,11,CURGAMEPIC))
			{
				MenuFadeOut();
				return;
			}
			MenuFadeOut();
			DrawNewGame();
		which=HandleMenu(&NewItems,&NewMenu[0],0);
		if (which<0)
		{
			MenuFadeOut();
			goto firstpart;
		}
		ShootSnd();
		NewGame(which,episode);
		StartGame=1;
		MenuFadeOut();
		MainMenu[readthis].active=1;
		pickquick = 0;
}

void DrawNewEpisode(void)
{
	int i;
	ClearMScreen();
	CacheLump (EPISODE_LUMP_START,EPISODE_LUMP_END);
	SETFONTCOLOR(READHCOLOR,BKGDCOLOR);
	PrintY=2;WindowX=0;
	VWB_DrawPic (84,52,E_WHICHEPISODEPIC);
	DrawWindow (14,66,23,70,BKGDCOLOR);
	SETFONTCOLOR(TEXTCOLOR,BKGDCOLOR);
	DrawMenu(&NewEitems,&NewEmenu[0]);
		for (i=0;i<4;i++)
			VWB_DrawPic(NE_X+32,NE_Y+48+i*16,E_EPISODE1PIC+i);
		UnCacheLump (EPISODE_LUMP_START,EPISODE_LUMP_END);

	VW_UpdateScreen();
	MenuFadeIn();
	WaitKeyUp();
}

void DrawNewGame(void)
{
	ClearMScreen();

	CacheLump (SKILL_LUMP_START,SKILL_LUMP_END);

	SETFONTCOLOR(READHCOLOR,BKGDCOLOR);

	VWB_DrawPic (70,36,S_SKILLLEVELPIC);
	VWB_DrawPic (70,63,S_SKILL1PIC);
	VWB_DrawPic (70,79,S_SKILL2PIC);
	VWB_DrawPic (70,95,S_SKILL3PIC);
	VWB_DrawPic (70,111,S_SKILL4PIC);
	DrawWindow (38,59,23,70,BKGDCOLOR);
	DrawMenu(&NewItems,&NewMenu[0]);
	UnCacheLump (SKILL_LUMP_START,SKILL_LUMP_END);
	VW_UpdateScreen();
	MenuFadeIn();
	WaitKeyUp();
}
void CP_Sound(void)
{
	enum {SOUNDS,SFX,MUSIC};
	int i,which;

	DrawSoundMenu();
	MenuFadeIn();
	WaitKeyUp();

	do
	{
		which=HandleMenu(&SndItems,&SndMenu[0],NULL);
		switch(which)
		{
			case SOUNDS:
				if (SoundMode==sdm_Off)
				{
					SD_WaitSoundDone();
					SD_SetSoundMode(sdm_PC);
					CA_LoadAllSounds();
					DrawSoundMenu();
					ShootSnd();
				}
				else
				if (SoundMode==sdm_PC)
				{
					SD_WaitSoundDone();
					SD_SetSoundMode(sdm_AdLib);
					CA_LoadAllSounds();
					DrawSoundMenu();
					ShootSnd();
				}
				else
				if (SoundMode==sdm_AdLib)
				{
					SD_WaitSoundDone();
					SD_SetSoundMode(sdm_Off);
					DrawSoundMenu();
				}
				break;
			case SFX:
				if (DigiMode==sds_Off)
				{
					SD_SetDigiDevice(sds_SoundBlaster);
					DrawSoundMenu();
					ShootSnd();
				}
				else
				if (DigiMode==sds_SoundBlaster)
				{
					SD_SetDigiDevice(sds_Off);
					DrawSoundMenu();
				}
				break;
			case MUSIC:
				if (MusicMode!=smm_Off)
				{
					SD_SetMusicMode(smm_Off);
					DrawSoundMenu();
					ShootSnd();
				}
				else
				if (MusicMode!=smm_AdLib)
				{
					SD_SetMusicMode(smm_AdLib);
					DrawSoundMenu();
					ShootSnd();
					StartCPMusic(MENUSONG);
				}
			break;
		}
	} while(which>=0);

	MenuFadeOut();
}

void DrawSoundMenu(void)
{
	int i,on;
	CacheLump (OPTIONS_LUMP_START,OPTIONS_LUMP_END);

	ClearMScreen();

	VWB_DrawPic(99,SM_Y1+40,O_SOUNDPIC);
	VWB_DrawPic(176,SM_Y1+40,O_COLONPIC);
	VWB_DrawPic(99,SM_Y1+56,O_DIGITIZEDPIC);
	VWB_DrawPic(99,SM_Y1+72,O_MUSICPIC);
	DrawWindow(70,57,23,54,BKGDCOLOR);
	DrawMenu(&SndItems,&SndMenu[0]);

				 if (SoundMode==sdm_Off)
					VWB_DrawPic (184,64,O_NOTSELECTEDPIC);
				 else if (SoundMode==sdm_PC)
					VWB_DrawPic (184,64,O_PCSOUNDPIC);
				 else if (SoundMode==sdm_AdLib)
					VWB_DrawPic (184,64,O_SELECTEDPIC);

				if (DigiMode==sds_Off)
					VWB_DrawPic(152,80,O_NOTSELECTEDPIC);
				else if (DigiMode==sds_SoundSource)
					VWB_DrawPic(152,77,O_SOUNDPIC);
				else if (DigiMode==sds_SoundBlaster)
					VWB_DrawPic(152,80,O_SELECTEDPIC);

				if (MusicMode==smm_Off)
					VWB_DrawPic (172,96,O_NOTSELECTEDPIC);
				else if (MusicMode==smm_AdLib)
					VWB_DrawPic (172,96,O_SELECTEDPIC);

	DrawMenuGun(&SndItems);
	UnCacheLump (OPTIONS_LUMP_START,OPTIONS_LUMP_END);
	VW_UpdateScreen();
}

void DrawLSAction(int which)
{
	#define LSA_X	96
	#define LSA_Y	80
	#define LSA_W	130
	#define LSA_H	42
	CA_CacheGrChunk (C_DISKLOADING1PIC);
	CA_CacheGrChunk (C_DISKLOADING2PIC);

	DrawWindow(LSA_X,LSA_Y,LSA_W,LSA_H,TEXTCOLOR);
	DrawOutline(LSA_X,LSA_Y,LSA_W,LSA_H,0,HIGHLIGHT);
	VWB_DrawPic(LSA_X+8,LSA_Y+5,C_DISKLOADING1PIC);

	fontnumber=1;
	SETFONTCOLOR(0,TEXTCOLOR);
	PrintX=LSA_X+46;
	PrintY=LSA_Y+13;

	if (!which)
		US_Print(STR_LOADING"...");
	else
		US_Print(STR_SAVING"...");

	VW_UpdateScreen();
}

int CP_LoadGame(int quick)
{
	int handle,which,exit=0;
	char name[13];

	strcpy(name,SaveName);

	if (quick)
	{
		which=LSItems.curpos;

		if (SaveGamesAvail[which])
		{
			name[7]=which+'0';
			handle=open(name,O_BINARY);
			lseek(handle,32,SEEK_SET);
			loadedgame=true;
			LoadTheGame(handle,0,0);
			loadedgame=false;
			close(handle);

			DrawFace ();
			DrawHealth ();
			DrawAmmo ();
			DrawKeys ();
			DrawWeapon ();
			DrawArmor ();
			if (!godflag)
			{
				if (!gamestate.godmode)
					godmode = gamestate.godmodecount = 0;
			}
			return 1;
		}
	}
	DrawLoadSaveScreen(0);

	do
	{
		which=HandleMenu(&LSItems,&LSMenu[0],TrackWhichGame);
		if (which>=0 && SaveGamesAvail[which])
		{
			ShootSnd();
			name[7]=which+'0';

			handle=open(name,O_BINARY);
			lseek(handle,32,SEEK_SET);

			DrawLSAction(0);
			loadedgame=true;

			LoadTheGame(handle,LSA_X+8,LSA_Y+5);
			close(handle);

			StartGame=1;
			ShootSnd();
			MainMenu[readthis].active=1;

			exit=1;
			break;
		}

	} while(which>=0);

	MenuFadeOut();
	return exit;
}

void TrackWhichGame(int w)
{
	static int lastgameon=0;

	PrintLSEntry(lastgameon,TEXTCOLOR);
	PrintLSEntry(w,HIGHLIGHT);
	lastgameon=w;
}

void DrawLoadSaveScreen(int loadsave)
{
	#define DISKX	100
	#define DISKY	0

	int i;

	CacheLump (LOADSAVE_LUMP_START,LOADSAVE_LUMP_END);
	CA_CacheGrChunk (M_LOADGAMEPIC);
	CA_CacheGrChunk (M_SAVEGAMEPIC);
	ClearMScreen();
	fontnumber=1;

	if (!loadsave)
		VWB_DrawPic(98,8,M_LOADGAMEPIC);
	else
		VWB_DrawPic(98,8,M_SAVEGAMEPIC);

	VWB_DrawPic(88,54,I_LSENTRYPIC);
	VWB_DrawPic(88,70,I_LSENTRYPIC);
	VWB_DrawPic(88,86,I_LSENTRYPIC);
	VWB_DrawPic(88,102,I_LSENTRYPIC);
	VWB_DrawPic(88,118,I_LSENTRYPIC);
	VWB_DrawPic(88,134,I_LSENTRYPIC);

	for (i=0;i<6;i++)
		PrintLSEntry(i,TEXTCOLOR);

	DrawWindow (62,48,23,102,BKGDCOLOR);

	DrawMenu(&LSItems,&LSMenu[0]);
	UNCACHEGRCHUNK(M_LOADGAMEPIC);
	UNCACHEGRCHUNK(M_SAVEGAMEPIC);
	UnCacheLump (LOADSAVE_LUMP_START,LOADSAVE_LUMP_END);
	VW_UpdateScreen();
	MenuFadeIn();
	WaitKeyUp();
}

void PrintLSEntry(int w,int color)
{
	SETFONTCOLOR(color,BKGDCOLOR);
	PrintX=LSM_X+LSItems.indent+2;
	PrintY=LSM_Y+w*16+1;
	fontnumber=0;

	if (SaveGamesAvail[w])
		US_Print(SaveGameNames[w]);
	else
		US_Print("      - "STR_EMPTY" -");

	fontnumber=1;
}

int CP_SaveGame(int quick)
{
	int handle,which,exit=0;
	unsigned nwritten;
	char name[13],input[32];

	strcpy(name,SaveName);

	if (quick)
	{
		which=LSItems.curpos;

		if (SaveGamesAvail[which])
		{
			name[7]=which+'0';
			unlink(name);
			handle=creat(name,S_IREAD|S_IWRITE);

			strcpy(input,&SaveGameNames[which][0]);

			_dos_write(handle,(void far *)input,32,&nwritten);
			lseek(handle,32,SEEK_SET);
			SaveTheGame(handle,0,0);
			close(handle);

			return 1;
		}
	}
	DrawLoadSaveScreen(1);

	do
	{
		which=HandleMenu(&LSItems,&LSMenu[0],TrackWhichGame);
		if (which>=0)
		{
			if (SaveGamesAvail[which])
				if  (!GetYorN(7,11,GAMESVDPIC))
				{
					DrawLoadSaveScreen(1);
					continue;
				}
				else
				{
					DrawLoadSaveScreen(1);
					PrintLSEntry(which,HIGHLIGHT);
					VW_UpdateScreen();
				}

			ShootSnd();

			strcpy(input,&SaveGameNames[which][0]);
			name[7]=which+'0';

			fontnumber=0;
			if (!SaveGamesAvail[which])
				VWB_Bar(LSM_X+LSItems.indent+1,LSM_Y+which*16+1,LSM_W-LSItems.indent-16,10,BKGDCOLOR);
			VW_UpdateScreen();

			if (US_LineInput(LSM_X+LSItems.indent+2,LSM_Y+which*16+1,input,input,true,31,LSM_W-LSItems.indent-30))
			{
				SaveGamesAvail[which]=1;
				strcpy(&SaveGameNames[which][0],input);

				unlink(name);
				handle=creat(name,S_IREAD|S_IWRITE);
				_dos_write(handle,(void far *)input,32,&nwritten);
				lseek(handle,32,SEEK_SET);

				DrawLSAction(1);
				SaveTheGame(handle,LSA_X+8,LSA_Y+5);

				close(handle);

				ShootSnd();
				exit=1;
			}
			else
			{
				VWB_Bar(LSM_X+LSItems.indent+1,LSM_Y+which*16+1,LSM_W-LSItems.indent-16,10,BKGDCOLOR);
				PrintLSEntry(which,HIGHLIGHT);
				VW_UpdateScreen();
				SD_PlaySound(WTFSND);
				continue;
			}

			fontnumber=1;
			break;
		}

	} while(which>=0);

	MenuFadeOut();
	return exit;
}

int CalibrateJoystick(void)
{
	unsigned xmin,ymin,xmax,ymax,jb;
	CacheLump (JOYSTICK_LUMP_START,JOYSTICK_LUMP_END);
	VWB_DrawPic(102,46,C_JOY1PIC);
	VW_UpdateScreen();

	do
	{
		jb=IN_JoyButtons();
		if (Keyboard[sc_Escape])
			return 0;
		if (Keyboard[sc_Tab] && Keyboard[sc_P] && MS_CheckParm(PARMCHEAT))
			PicturePause();

	} while(!(jb&1));

	SD_PlaySound(WTFSND);
	IN_GetJoyAbs(joystickport,&xmin,&ymin);
	VWB_DrawPic(102,46,C_JOY2PIC);
	VW_UpdateScreen();

	do
	{
		jb=IN_JoyButtons();
		if (Keyboard[sc_Escape])
			return 0;
		if (Keyboard[sc_Tab] && Keyboard[sc_P] && MS_CheckParm(PARMCHEAT))
			PicturePause();
	} while(!(jb&2));

	IN_GetJoyAbs(joystickport,&xmax,&ymax);
	SD_PlaySound(WTFSND);
	UnCacheLump (JOYSTICK_LUMP_START,JOYSTICK_LUMP_END);

	while (IN_JoyButtons());

	if ((xmin != xmax) && (ymin != ymax))
		IN_SetupJoy(joystickport,xmin,xmax,ymin,ymax);
	else
		return 0;

	return 1;
}

void CP_Control(void)
{
	#define CTL_SPC	70
	enum {MOUSEENABLE,JOYENABLE,USEPORT2,PADENABLE,MOUSESENS,CUSTOMIZE};
	int i,which;

	DrawCtlScreen();
	MenuFadeIn();
	WaitKeyUp();

	do
	{
		which=HandleMenu(&CtlItems,&CtlMenu[0],NULL);
		switch(which)
		{
			case MOUSEENABLE:
				mouseenabled^=1;
				_CX=_DX=CENTER;
				Mouse(4);
				DrawCtlScreen();
				CusItems.curpos=-1;
				ShootSnd();
				break;

			case JOYENABLE:
				joystickenabled^=1;
				if (joystickenabled)
					if (!CalibrateJoystick())
						joystickenabled = 0;
				DrawCtlScreen();
				CusItems.curpos=-1;
				ShootSnd();
				break;

			case USEPORT2:
				joystickport^=1;
				DrawCtlScreen();
				ShootSnd();
				break;

			case PADENABLE:
				joypadenabled^=1;
				DrawCtlScreen();
				ShootSnd();
				break;

			case MOUSESENS:
			case CUSTOMIZE:
				DrawCtlScreen();
				MenuFadeIn();
				WaitKeyUp();
				break;
		}
	} while(which>=0);

	MenuFadeOut();
}

void DrawMouseSens(void)
{
	ClearMScreen();
	CA_CacheGrChunk (O_MOUSESENSPIC);
	CA_CacheGrChunk (BARPIC);
	CA_CacheGrChunk (METERPIC);

	VWB_DrawPic (58,72,O_MOUSESENSPIC);
	VWB_DrawPic (114,96,BARPIC);
	VWB_DrawPic (120+8*mouseadjustment,97,METERPIC);

	VW_UpdateScreen();
	UNCACHEGRCHUNK (O_MOUSESENSPIC);
	MenuFadeIn();
}

void MouseSensitivity(void)
{
	ControlInfo ci;
	int exit=0,oldMA;

	oldMA=mouseadjustment;
	DrawMouseSens();
	do
	{
		ReadAnyControl(&ci);
		switch(ci.dir)
		{
			case dir_North:
			case dir_West:
				if (mouseadjustment)
				{
					mouseadjustment--;
					VWB_DrawPic (114,96,BARPIC);
					VWB_DrawPic (120+8*mouseadjustment,97,METERPIC);
					VW_UpdateScreen();
					SD_PlaySound(WTFSND);
					while(Keyboard[sc_LeftArrow]);
					WaitKeyUp();
				}
				break;

			case dir_South:
			case dir_East:
				if (mouseadjustment<9)
				{
					mouseadjustment++;
					VWB_DrawPic (114,96,BARPIC);
					VWB_DrawPic (120+8*mouseadjustment,97,METERPIC);
					VW_UpdateScreen();
					SD_PlaySound(WTFSND);
					while(Keyboard[sc_RightArrow]);
					WaitKeyUp();
				}
				break;
		}
		if (Keyboard[sc_Tab] && Keyboard[sc_P] && MS_CheckParm(PARMCHEAT))
			PicturePause();

		if (ci.button0 || Keyboard[sc_Space] || Keyboard[sc_Enter])
			exit=1;
		else
		if (ci.button1 || Keyboard[sc_Escape])
			exit=2;

	} while(!exit);

	if (exit==2)
	{
		mouseadjustment=oldMA;
		SD_PlaySound(WTFSND);
	}
	else
		SD_PlaySound(WTFSND);

	WaitKeyUp();
	MenuFadeOut();
}

void DrawCtlScreen(void)
{
 int i,x,y;

 CacheLump (OPTIONS_LUMP_START,OPTIONS_LUMP_END);
 ClearMScreen();
 VWB_DrawPic(110,16,O_CONTROLPIC);
 VWB_DrawPic(68,42,O_MOUSEENABLEPIC);
 VWB_DrawPic(68,58,O_JOYSTICKENABLEPIC);
 VWB_DrawPic(68,74,O_JOYPORT2PIC);
 VWB_DrawPic(68,90,O_GRAVISENABLEPIC);
 VWB_DrawPic(68,106,O_MOUSESENSPIC);
 VWB_DrawPic(68,122,O_CUSTOMIZEPIC);
 DrawWindow(38,38,23,102,BKGDCOLOR);
 WindowX=0;
 WindowW=320;
 SETFONTCOLOR(TEXTCOLOR,BKGDCOLOR);

 if (JoysPresent[0])
   CtlMenu[1].active=
   CtlMenu[2].active=
   CtlMenu[3].active=1;

 CtlMenu[2].active=CtlMenu[3].active=joystickenabled;

 if (MousePresent)
 {
  CtlMenu[4].active=
  CtlMenu[0].active=1;
 }

 CtlMenu[4].active=mouseenabled;

 DrawMenu(&CtlItems,&CtlMenu[0]);

 if (mouseenabled)
   VWB_DrawPic(151,45,O_SELECTEDPIC);
 else
   VWB_DrawPic(151,45,O_NOTSELECTEDPIC);

 if (joystickenabled)
   VWB_DrawPic(183,61,O_SELECTEDPIC);
 else
   VWB_DrawPic(183,61,O_NOTSELECTEDPIC);

 if (joystickport)
   VWB_DrawPic(194,77,O_SELECTEDPIC);
 else
   VWB_DrawPic(194,77,O_NOTSELECTEDPIC);

 if (joypadenabled)
   VWB_DrawPic(156,93,O_SELECTEDPIC);
 else
   VWB_DrawPic(156,93,O_NOTSELECTEDPIC);

 if (CtlItems.curpos<0 || !CtlMenu[CtlItems.curpos].active)
   for (i=0;i<6;i++)
	 if (CtlMenu[i].active)
	 {
	  CtlItems.curpos=i;
	  break;
	 }

 DrawMenuGun(&CtlItems);
 UnCacheLump(OPTIONS_LUMP_START,OPTIONS_LUMP_END);
 VW_UpdateScreen();
}

enum {FIRE,STRAFE,RUN,OPEN};
char mbarray[4][3]={"b0","b1","b2","b3"},
	   order[4]={RUN,OPEN,FIRE,STRAFE};

void CustomControls(void)
{
 int which;

 DrawCustomScreen();
 do
 {
  which=HandleMenu(&CusItems,&CusMenu[0],FixupCustom);
  switch(which)
  {
   case 0:
	 DefineMouseBtns();
	 DrawCustMouse(1);
	 break;
   case 2:
	DefineJoyBtns();
	DrawCustJoy(0);
	break;
   case 5:
	DefineKeyBtns();
	DrawCustKeybd(0);
	break;
   case 6:
	DefineKeyMove();
	DrawCustKeys(0);
  }
 } while(which>=0);

 MenuFadeOut();
}
void DefineMouseBtns(void)
{
 CustomCtrls mouseallowed={0,1,1,1};
 EnterCtrlData(1,&mouseallowed,DrawCustMouse,PrintCustMouse,MOUSE);
}
void DefineJoyBtns(void)
{
 CustomCtrls joyallowed={1,1,1,1};
 EnterCtrlData(5,&joyallowed,DrawCustJoy,PrintCustJoy,JOYSTICK);
}

void DefineKeyBtns(void)
{
 CustomCtrls keyallowed={1,1,1,1};
 EnterCtrlData(8,&keyallowed,DrawCustKeybd,PrintCustKeybd,KEYBOARDBTNS);
}

void DefineKeyMove(void)
{
	CustomCtrls keyallowed={1,1,1,1};
	EnterCtrlData(10,&keyallowed,DrawCustKeys,PrintCustKeys,KEYBOARDMOVE);
}

enum {FWRD,RIGHT,BKWD,LEFT};
int moveorder[4]={LEFT,RIGHT,FWRD,BKWD};
void EnterCtrlData(int index,CustomCtrls *cust,void (*DrawRtn)(int),void (*PrintRtn)(int),int type)
{
 int j,exit,tick,redraw,which,x,picked;
 ControlInfo ci;

 ShootSnd();
 PrintY=CST_Y+13*index;
 IN_ClearKeysDown();
 exit=0;
 redraw=1;
 for (j=0;j<4;j++)
   if (cust->allowed[j])
   {
	which=j;
	break;
   }

 do
 {
  if (redraw)
  {
   x=CST_START+CST_SPC*which;
   DrawWindow(40,PrintY-1,275,13,BKGDCOLOR);

   DrawRtn(1);
   DrawWindow(x-2,PrintY,CST_SPC,11,TEXTCOLOR);
   DrawOutline(x-2,PrintY,CST_SPC,11,0,HIGHLIGHT);
   SETFONTCOLOR(0,TEXTCOLOR);
   PrintRtn(which);
   PrintX=x;
   SETFONTCOLOR(TEXTCOLOR,BKGDCOLOR);
   VW_UpdateScreen();
   WaitKeyUp();
   redraw=0;
  }

  ReadAnyControl(&ci);

  if (type==MOUSE || type==JOYSTICK)
	if (IN_KeyDown(sc_Enter)||IN_KeyDown(sc_Control)||IN_KeyDown(sc_Alt))
	{
	 IN_ClearKeysDown();
	 ci.button0=ci.button1=false;
	}
  if ((ci.button0|ci.button1|ci.button2|ci.button3)||
	  ((type==KEYBOARDBTNS||type==KEYBOARDMOVE) && LastScan==sc_Enter))
  {
   tick=TimeCount=picked=0;
   SETFONTCOLOR(0,TEXTCOLOR);

   do
   {
	int button,result=0;


	if (type==KEYBOARDBTNS||type==KEYBOARDMOVE)
	  IN_ClearKeysDown();

	if (TimeCount>10)
	{
	 switch(tick)
	 {
	  case 0:
	VWB_Bar(x,PrintY+1,CST_SPC-2,10,TEXTCOLOR);
	break;
	  case 1:
	PrintX=x;
	US_Print("?");
	 }
	 tick^=1;
	 TimeCount=0;
	 VW_UpdateScreen();
	}

	switch(type)
	{
	 case MOUSE:
	   Mouse(3);
	   button=_BX;
	   switch(button)
	   {
	case 1: result=1; break;
	case 2: result=2; break;
	case 4: result=3; break;
	   }

	   if (result)
	   {
	int z;


	for (z=0;z<4;z++)
	  if (order[which]==buttonmouse[z])
	  {
	   buttonmouse[z]=bt_nobutton;
	   break;
	  }

	buttonmouse[result-1]=order[which];
	picked=1;
	SD_PlaySound(WTFSND);
	   }
	   break;

	 case JOYSTICK:
	   if (ci.button0) result=1;
	   else
	   if (ci.button1) result=2;
	   else
	   if (ci.button2) result=3;
	   else
	   if (ci.button3) result=4;

	   if (result)
	   {
	int z;


	for (z=0;z<4;z++)
	  if (order[which]==buttonjoy[z])
	  {
	   buttonjoy[z]=bt_nobutton;
	   break;
	  }

	buttonjoy[result-1]=order[which];
	picked=1;
	SD_PlaySound(WTFSND);
	   }
	   break;

	 case KEYBOARDBTNS:
	   if (LastScan)
	   {
	buttonscan[order[which]]=LastScan;
	picked=1;
	ShootSnd();
	IN_ClearKeysDown();
	   }
	   break;

	 case KEYBOARDMOVE:
	   if (LastScan)
	   {
	dirscan[moveorder[which]]=LastScan;
	picked=1;
	ShootSnd();
	IN_ClearKeysDown();
	   }
	   break;
	}
	if (IN_KeyDown(sc_Escape))
	{
	 picked=1;
	 continue;
	}

   } while(!picked);

   SETFONTCOLOR(TEXTCOLOR,BKGDCOLOR);
   redraw=1;
   WaitKeyUp();
   continue;
  }

  if (ci.button1 || IN_KeyDown(sc_Escape))
	exit=1;

  switch(ci.dir)
  {
   case dir_West:
	 do
	 {
	  which--;
	  if (which<0)
	which=3;
	 } while(!cust->allowed[which]);
	 redraw=1;
	 SD_PlaySound(WTFSND);
	 while(ReadAnyControl(&ci),ci.dir!=dir_None);
	 IN_ClearKeysDown();
	 break;

   case dir_East:
	 do
	 {
	  which++;
	  if (which>3)
	which=0;
	 } while(!cust->allowed[which]);
	 redraw=1;
	 SD_PlaySound(WTFSND);
	 while(ReadAnyControl(&ci),ci.dir!=dir_None);
	 IN_ClearKeysDown();
	 break;
   case dir_North:
   case dir_South:
	 exit=1;
  }
 } while(!exit);

 SD_PlaySound(WTFSND);
 WaitKeyUp();
 DrawWindow(40,PrintY-1,275,13,BKGDCOLOR);
}
void FixupCustom(int w)
{
	static int lastwhich=-1;

	switch(w)
	{
		case 0: DrawCustMouse(1); break;
		case 2: DrawCustJoy(1); break;
		case 5: DrawCustKeybd(1); break;
		case 6: DrawCustKeys(1);
	}


	if (lastwhich>=0)
	{
		if (lastwhich!=w)
			switch(lastwhich)
			{
				case 0: DrawCustMouse(0); break;
				case 2: DrawCustJoy(0); break;
				case 5: DrawCustKeybd(0); break;
				case 6: DrawCustKeys(0);
			}
	}
	lastwhich=w;
}

void DrawCustomScreen(void)
{
	int i;

	ClearMScreen();
	CA_CacheGrChunk(O_CUSTOMIZEPIC);
	CacheLump(CUSTOMIZE_LUMP_START,CUSTOMIZE_LUMP_END);
	fontnumber=0;
	VWB_DrawPic(98,16,O_CUSTOMIZEPIC);
	DrawWindow (6,70,23,118,BKGDCOLOR);

	//
	// MOUSE
	//
	VWB_DrawPic(122,48,Q_MOUSEPIC);
	VWB_DrawPic(60,61,Q_RUNPIC);
	VWB_DrawPic(120,61,Q_OPENPIC);
	VWB_DrawPic(180,61,Q_FIREPIC);
	VWB_DrawPic(240,61,Q_STRAFEPIC);

	DrawWindow(40,73,275,13,BKGDCOLOR);
	DrawCustMouse(0);


	//
	// JOYSTICK/PAD
	//
	VWB_DrawPic(64,88,Q_JOYGRAVISPIC);

	VWB_DrawPic(60,103,Q_RUNPIC);
	VWB_DrawPic(120,103,Q_OPENPIC);
	VWB_DrawPic(180,103,Q_FIREPIC);
	VWB_DrawPic(240,103,Q_STRAFEPIC);
	DrawWindow(40,112,275,13,BKGDCOLOR);
	DrawCustJoy(0);


	//
	// KEYBOARD
	//
	VWB_DrawPic(102,128,Q_KEYBOARDPIC);
	VWB_DrawPic(60,143,Q_RUNPIC);
	VWB_DrawPic(120,143,Q_OPENPIC);
	VWB_DrawPic(180,143,Q_FIREPIC);
	VWB_DrawPic(240,143,Q_STRAFEPIC);

	DrawWindow(40,151,275,13,BKGDCOLOR);
	DrawCustKeybd(0);


	//
	// KEYBOARD MOVE KEYS
	//
	VWB_DrawPic(60,166,Q_LEFTPIC);
	VWB_DrawPic(120,166,Q_RIGHTPIC);
	VWB_DrawPic(180,166,Q_FRWDPIC);
	VWB_DrawPic(240,166,Q_BKWRDPIC);
	DrawWindow(40,177,275,13,BKGDCOLOR);
	DrawCustKeys(0);

	if (CusItems.curpos<0)
		for (i=0;i<CusItems.amount;i++)
			if (CusMenu[i].active)
			{
				CusItems.curpos=i;
				break;
			}


	UNCACHEGRCHUNK (O_CUSTOMIZEPIC);
	UnCacheLump (CUSTOMIZE_LUMP_START,CUSTOMIZE_LUMP_END);
	VW_UpdateScreen();
	MenuFadeIn();
}
void PrintCustMouse(int i)
{
	int j;

	for (j=0;j<4;j++)
		if (order[i]==buttonmouse[j])
		{
			PrintX=CST_START+CST_SPC*i;
			US_Print(mbarray[j]);
			break;
		}
}
void DrawCustMouse(int hilight)
{
	int i,color;

	color=TEXTCOLOR;
	if (hilight)
		color=HIGHLIGHT;
	SETFONTCOLOR(color,BKGDCOLOR);

	if (!mouseenabled)
	{
		SETFONTCOLOR(DEACTIVE,BKGDCOLOR);
		CusMenu[0].active=0;
	}
	else
		CusMenu[0].active=1;

	PrintY=CST_Y+13*2;
	for (i=0;i<4;i++)
		PrintCustMouse(i);
}

void PrintCustJoy(int i)
{
	int j;

	for (j=0;j<4;j++)
		if (order[i]==buttonjoy[j])
		{
			PrintX=CST_START+CST_SPC*i;
			US_Print(mbarray[j]);
			break;
		}
}

void DrawCustJoy(int hilight)
{
	int i,color;


	color=TEXTCOLOR;
	if (hilight)
		color=HIGHLIGHT;
	SETFONTCOLOR(color,BKGDCOLOR);

	if (!joystickenabled)
	{
		SETFONTCOLOR(DEACTIVE,BKGDCOLOR);
		CusMenu[2].active=0;
	}
	else
		CusMenu[2].active=1;

	PrintY=CST_Y+13*5;
	for (i=0;i<4;i++)
		PrintCustJoy(i);
}

void PrintCustKeybd(int i)
{
	PrintX=CST_START+CST_SPC*i;
	US_Print(IN_GetScanName(buttonscan[order[i]]));
}

void DrawCustKeybd(int hilight)
{
	int i,color;

	color=TEXTCOLOR;
	if (hilight)
		color=HIGHLIGHT;
	SETFONTCOLOR(color,BKGDCOLOR);

	PrintY=CST_Y+13*8;
	for (i=0;i<4;i++)
		PrintCustKeybd(i);
}

void PrintCustKeys(int i)
{
	PrintX=CST_START+CST_SPC*i;
	US_Print(IN_GetScanName(dirscan[moveorder[i]]));
}

void DrawCustKeys(int hilight)
{
	int i,color;

	color=TEXTCOLOR;
	if (hilight)
		color=HIGHLIGHT;
	SETFONTCOLOR(color,BKGDCOLOR);

	PrintY=CST_Y+13*10;
	for (i=0;i<4;i++)
		PrintCustKeys(i);
}

void CP_ChangeView(void)
{
	int exit=0,oldview,newview;
	ControlInfo ci;

	WindowX=WindowY=0;
	WindowW=320;
	WindowH=200;
	newview=oldview=viewwidth/16;
	DrawChangeView(oldview);

	do
	{
		CheckPause();
		ReadAnyControl(&ci);
		switch(ci.dir)
		{
		case dir_South:
		case dir_West:
			newview--;
			if (newview<10)
				newview=10;
			ShowViewSize(newview);
			VW_UpdateScreen();
			TicDelay(10);
			break;

		case dir_North:
		case dir_East:
			newview++;
			if (newview>=20)
			newview=20;
			ShowViewSize(newview);
			VW_UpdateScreen();
			TicDelay(10);
			break;
		}
		if (Keyboard[sc_Tab] && Keyboard[sc_P] && MS_CheckParm(PARMCHEAT))
			PicturePause();

		if (ci.button0 || Keyboard[sc_Enter])
			exit=1;
		else
		if (ci.button1 || Keyboard[sc_Escape])
		{
			viewwidth=oldview*16;
			SD_PlaySound(WTFSND);
			MenuFadeOut();
			return;
		}

	} while(!exit);

	if (oldview!=newview)
	{
		SD_PlaySound (WTFSND);
		Message(STR_THINK"...");
		NewViewSize(newview);
	}
	ShootSnd();
	MenuFadeOut();
}

void DrawChangeView(int view)
{
	VWB_Bar(0,160,320,40,VIEWCOLOR);
	ShowViewSize(view);

	PrintY=161;
	WindowX=0;
	WindowY=320;
	SETFONTCOLOR(HIGHLIGHT,BKGDCOLOR);

	US_CPrint(STR_SIZE1"\n");
	US_CPrint(STR_SIZE2"\n");
	US_CPrint(STR_SIZE3);
	VW_UpdateScreen();

	MenuFadeIn();
}

void CP_Quit(void)
{
	int i;

	if (GetYorN(9,12,END1PIC))
	{
		VW_UpdateScreen();
		SD_MusicOff();
		SD_StopSound();
		MenuFadeOut();
		for (i=1;i<=0xf5;i++)
			alOut(i,0);
		Quit(NULL);
	}
	DrawMainMenu();
}

void IntroScreen(void)
{
#define MAINCOLOR	0x7a
#define EMSCOLOR	0xba
#define XMSCOLOR	0xcf
#define FILLCOLOR	231

	long memory,emshere,xmshere;
	int i,num,ems[10]={100,200,300,400,500,600,700,800,900,1000},
		xms[10]={100,200,300,400,500,600,700,800,900,1000},
		main[10]={32,64,96,128,160,192,224,256,288,320};

	memory=(1023l+mminfo.nearheap+mminfo.farheap)/1024l;
	for (i=0;i<10;i++)
		if (memory>=main[i])
			VWB_Bar(49,163-8*i,6,5,MAINCOLOR-i);

	if (EMSPresent)
	{
		emshere=4l*EMSPagesAvail;
		for (i=0;i<10;i++)
			if (emshere>=ems[i])
				VWB_Bar(89,163-8*i,6,5,EMSCOLOR-i);
	}

	if (XMSPresent)
	{
		xmshere=4l*XMSPagesAvail;
		for (i=0;i<10;i++)
			if (xmshere>=xms[i])
				VWB_Bar(129,163-8*i,6,5,XMSCOLOR-i);
	}
	if (MousePresent)
		VWB_Bar(164,82,12,2,FILLCOLOR);

	if (JoysPresent[0] || JoysPresent[1])
		VWB_Bar(164,105,12,2,FILLCOLOR);

	if (AdLibPresent && !SoundBlasterPresent)
		VWB_Bar(164,128,12,2,FILLCOLOR);

	if (SoundBlasterPresent)
		VWB_Bar(164,151,12,2,FILLCOLOR);

	if (SoundSourcePresent)
		VWB_Bar(164,174,12,2,FILLCOLOR);
}

void ClearMScreen(void)
{
CA_CacheGrChunk (BACKDROPPIC);
VWB_DrawPic (0,0,BACKDROPPIC);
UNCACHEGRCHUNK (BACKDROPPIC);
}

void CacheLump(int lumpstart,int lumpend)
{
 int i;

 for (i=lumpstart;i<=lumpend;i++)
   CA_CacheGrChunk(i);
}

void UnCacheLump(int lumpstart,int lumpend)
{
 int i;

 for (i=lumpstart;i<=lumpend;i++)
	if (grsegs[i])
		UNCACHEGRCHUNK(i);
}

void DrawWindow(int x,int y,int w,int h,int wcolor)
{
	VWB_Bar(x,y,w,h,wcolor);
	DrawOutline(x,y,w,h,BORD2COLOR,DEACTIVE);
}

void DrawOutline(int x,int y,int w,int h,int color1,int color2)
{
	VWB_Hlin(x,x+w,y,color2);
	VWB_Vlin(y,y+h,x,color2);
	VWB_Hlin(x,x+w,y+h,color1);
	VWB_Vlin(y,y+h,x+w,color1);
}

void SetupControlPanel(void)
{
	struct ffblk f;
	char name[13];
	int which,i;

	CA_CacheGrChunk(STARTFONT+1);

	SETFONTCOLOR(TEXTCOLOR,BKGDCOLOR);
	fontnumber=1;
	WindowH=200;

	if (!ingame)
		CA_LoadAllSounds();
	else
		MainMenu[savegame].active=1;

	strcpy(name,SaveName);
	if (!findfirst(name,&f,0))
		do
		{
			which=f.ff_name[7]-'0';
			if (which<6)
			{
				int handle;
				char temp[32];

				SaveGamesAvail[which]=1;
				handle=open(f.ff_name,O_BINARY);
				read(handle,temp,32);
				close(handle);
				strcpy(&SaveGameNames[which][0],temp);
			}
		} while(!findnext(&f));
	_CX=_DX=CENTER;
	Mouse(4);
}

int HandleMenu(CP_iteminfo *item_i,CP_itemtype far *items,void (*routine)(int w))
{
	char key;
	static int redrawitem=1,lastitem=-1;
	int i,x,y,basey,exit,which,shape,timer;
	ControlInfo ci;

	CacheLump (CURSOR_LUMP_START,CURSOR_LUMP_END);

	which=item_i->curpos;
	x=item_i->x&-8;
	basey=item_i->y-2;
	y=basey+which*16;

	VWB_DrawPic(x,y,C_CURSOR1PIC);
	SetTextColor(items+which,1);
	if (redrawitem)
	{
		PrintX=item_i->x+item_i->indent;
		PrintY=item_i->y+which*16;
		US_Print((items+which)->string);
	}
	if (routine)
		routine(which);
	VW_UpdateScreen();

	shape=C_CURSOR1PIC;
	timer=8;
	exit=0;
	TimeCount=0;
	IN_ClearKeysDown();
	do
	{
		if (TimeCount>timer)
		{
			TimeCount=0;
			if (shape==C_CURSOR1PIC)
			{
				shape=C_CURSOR2PIC;
				timer=8;
			}
			else
			{
				shape=C_CURSOR1PIC;
				timer=70;
			}
			VWB_DrawPic(x,y,shape);
			if (routine)
				routine(which);
			VW_UpdateScreen();
		}

		CheckPause();
		key=LastASCII;
		if (key)
		{
			int ok=0;

			if (Keyboard[sc_Tab] && Keyboard[sc_P] && MS_CheckParm(PARMCHEAT))
				PicturePause();

			if (key>='a')
				key-='a'-'A';

			for (i=which+1;i<item_i->amount;i++)
				if ((items+i)->active && (items+i)->string[0]==key)
				{
					EraseGun(item_i,items,x,y,which);
					which=i;
					DrawGun(item_i,items,x,&y,which,basey,routine);
					ok=1;
					IN_ClearKeysDown();
					break;
				}

			if (!ok)
			{
				for (i=0;i<which;i++)
					if ((items+i)->active && (items+i)->string[0]==key)
					{
						EraseGun(item_i,items,x,y,which);
						which=i;
						DrawGun(item_i,items,x,&y,which,basey,routine);
						IN_ClearKeysDown();
						break;
					}
			}
		}
		ReadAnyControl(&ci);
		switch(ci.dir)
		{
			case dir_North:

			EraseGun(item_i,items,x,y,which);
			if (which && (items+which-1)->active)
				y-=6;
			do
			{
				if (!which)
					which=item_i->amount-1;
				else
					which--;
			} while(!(items+which)->active);

			DrawGun(item_i,items,x,&y,which,basey,routine);
			TicDelay(20);
			break;
			case dir_South:

			EraseGun(item_i,items,x,y,which);
			if (which!=item_i->amount-1 && (items+which+1)->active)
			y+=6;

			do
			{
				if (which==item_i->amount-1)
					which=0;
				else
					which++;
			} while(!(items+which)->active);

			DrawGun(item_i,items,x,&y,which,basey,routine);

			TicDelay(20);
			break;
		}

		if (ci.button0 ||
			Keyboard[sc_Space] ||
			Keyboard[sc_Enter])
				exit=1;

		if (ci.button1 ||
			Keyboard[sc_Escape])
				exit=2;

	} while(!exit);

	IN_ClearKeysDown();
	if (lastitem!=which)
	{
	//	VWB_Bar(x-1,y,21,19,BKGDCOLOR);
		PrintX=item_i->x+item_i->indent;
		PrintY=item_i->y+which*16;
		US_Print((items+which)->string);
		redrawitem=1;
	}
	else
		redrawitem=0;

	if (routine)
		routine(which);
	VW_UpdateScreen();

	item_i->curpos=which;

	lastitem=which;
	switch(exit)
	{
		case 1:
			if ((items+which)->routine!=NULL)
			{
				ShootSnd();
				MenuFadeOut();
				(items+which)->routine(0);
			}
			return which;

		case 2:
			SD_PlaySound(WTFSND);
			return -1;
	}
	SD_Poll();
	UnCacheLump (CURSOR_LUMP_START,CURSOR_LUMP_END);

	return 0;
}

void EraseGun(CP_iteminfo *item_i,CP_itemtype far *items,int x,int y,int which)
{
	VWB_Bar(x-1,y,21,20,BKGDCOLOR);
	SetTextColor(items+which,0);
	PrintX=item_i->x+item_i->indent;
	PrintY=item_i->y+which*16;
	US_Print((items+which)->string);
	VW_UpdateScreen();
}

void DrawGun(CP_iteminfo *item_i,CP_itemtype far *items,int x,int *y,int which,int basey,void (*routine)(int w))
{
	CacheLump (CURSOR_LUMP_START,CURSOR_LUMP_END);
	VWB_Bar(x-1,*y,21,20,BKGDCOLOR);
	*y=basey+which*16;
	VWB_DrawPic(x,*y,C_CURSOR1PIC);
	SetTextColor(items+which,1);

	PrintX=item_i->x+item_i->indent;
	PrintY=item_i->y+which*16;
	US_Print((items+which)->string);

	if (routine)
		routine(which);
	VW_UpdateScreen();
	SD_PlaySound(WTFSND);
      //	UnCacheLump (CURSOR_LUMP_START,CURSOR_LUMP_END);
}

void TicDelay(int count)
{
	ControlInfo ci;

	TimeCount=0;
	do
	{
		ReadAnyControl(&ci);
	} while(TimeCount<count && ci.dir!=dir_None);
}

void DrawMenu(CP_iteminfo *item_i,CP_itemtype far *items)
{
	int i,which=item_i->curpos;

	WindowX=PrintX=item_i->x+item_i->indent;
	WindowY=PrintY=item_i->y;
	WindowW=320;
	WindowH=200;
	for (i=0;i<item_i->amount;i++)
	{
		SetTextColor(items+i,which==i);

		PrintY=item_i->y+i*16;
		if ((items+i)->active)
			US_Print((items+i)->string);
		else
		{
			SETFONTCOLOR(DEACTIVE,BKGDCOLOR);
			US_Print((items+i)->string);
			SETFONTCOLOR(TEXTCOLOR,BKGDCOLOR);
		}

		US_Print("\n");
	}
}

void SetTextColor(CP_itemtype far *items,int hlight)
{
	if (hlight)
		{SETFONTCOLOR(color_hlite[items->active],BKGDCOLOR);}
	else
		{SETFONTCOLOR(color_norml[items->active],BKGDCOLOR);}
}

void WaitKeyUp(void)
{
	ControlInfo ci;
	while(ReadAnyControl(&ci),	ci.button0|
								ci.button1|
								ci.button2|
								ci.button3|
								Keyboard[sc_Space]|
								Keyboard[sc_Enter]|
								Keyboard[sc_Escape]);
}

void ReadAnyControl(ControlInfo *ci)
{
	int mouseactive=0;

	IN_ReadControl(0,ci);

	if (mouseenabled)
	{
		int mousey,mousex;
		Mouse(3);
		mousex=_CX;
		mousey=_DX;

		if (mousey<CENTER-SENSITIVE)
		{
			ci->dir=dir_North;
			_CX=_DX=CENTER;
			Mouse(4);
			mouseactive=1;
		}
		else
		if (mousey>CENTER+SENSITIVE)
		{
			ci->dir=dir_South;
			_CX=_DX=CENTER;
			Mouse(4);
			mouseactive=1;
		}

		if (mousex<CENTER-SENSITIVE)
		{
			ci->dir=dir_West;
			_CX=_DX=CENTER;
			Mouse(4);
			mouseactive=1;
		}
		else
		if (mousex>CENTER+SENSITIVE)
		{
			ci->dir=dir_East;
			_CX=_DX=CENTER;
			Mouse(4);
			mouseactive=1;
		}

		if (IN_MouseButtons())
		{
			ci->button0=IN_MouseButtons()&1;
			ci->button1=IN_MouseButtons()&2;
			ci->button2=IN_MouseButtons()&4;
			ci->button3=false;
			mouseactive=1;
		}
	}

	if (joystickenabled && !mouseactive)
	{
		int jx,jy,jb;


		INL_GetJoyDelta(joystickport,&jx,&jy);
		if (jy<-SENSITIVE)
			ci->dir=dir_North;
		else
		if (jy>SENSITIVE)
			ci->dir=dir_South;

		if (jx<-SENSITIVE)
			ci->dir=dir_West;
		else
		if (jx>SENSITIVE)
			ci->dir=dir_East;

		jb=IN_JoyButtons();
		if (jb)
		{
			ci->button0=jb&1;
			ci->button1=jb&2;
			if (joypadenabled)
			{
				ci->button2=jb&4;
				ci->button3=jb&8;
			}
			else
				ci->button2=ci->button3=false;
		}
	}
}

int Confirm(char far *string)
{
	int xit=0,i,x,y,tick=0,time,whichsnd[2]={WTFSND,WTFSND};

	Message(string);
	IN_ClearKeysDown();

	x=PrintX;
	y=PrintY;
	TimeCount=0;

	do
	{
		if (TimeCount>=10)
		{
			switch(tick)
			{
				case 0:
					VWB_Bar(x,y,8,13,TEXTCOLOR);
					break;
				case 1:
					PrintX=x;
					PrintY=y;
					US_Print("_");
			}
			VW_UpdateScreen();
			tick^=1;
			TimeCount=0;
		}

		if (Keyboard[sc_Tab] && Keyboard[sc_P] && MS_CheckParm(PARMCHEAT))
			PicturePause();

	} while(!Keyboard[sc_Y] && !Keyboard[sc_N] && !Keyboard[sc_Escape]);

	if (Keyboard[sc_Y])
	{
		xit=1;
		ShootSnd();
	}

	while(Keyboard[sc_Y] || Keyboard[sc_N] || Keyboard[sc_Escape]);

	IN_ClearKeysDown();
	SD_PlaySound(whichsnd[xit]);
	return xit;
}
int GetYorN(int x,int y,int pic)
{
	int xit=0,whichsnd[2]={WTFSND,WTFSND};

	CA_CacheGrChunk(pic);
	VWB_DrawPic(x * 8,y * 8,pic);
	UNCACHEGRCHUNK(pic);
	VW_UpdateScreen();
	IN_ClearKeysDown();

	do
	{
		if (Keyboard[sc_Tab] && Keyboard[sc_P] && MS_CheckParm("goobers"))
			PicturePause();

	} while(!Keyboard[sc_Y] && !Keyboard[sc_N] && !Keyboard[sc_Escape]);

	if (Keyboard[sc_Y])
	{
		xit=1;
		ShootSnd();
	}

	while(Keyboard[sc_Y] || Keyboard[sc_N] || Keyboard[sc_Escape]);

	IN_ClearKeysDown();
	SD_PlaySound(whichsnd[xit]);
	return xit;
}
void Message(char far *string)
{
	int h=0,w=0,mw=0,i,x,y,time;
	fontstruct _seg *font;

	CA_CacheGrChunk (STARTFONT+1);
	fontnumber=1;
	font=grsegs[STARTFONT+fontnumber];
	h=font->height;
	for (i=0;i<_fstrlen(string);i++)
		if (string[i]=='\n')
		{
			if (w>mw)
				mw=w;
			w=0;
			h+=font->height;
		}
		else
			w+=font->width[string[i]];

	if (w+10>mw)
		mw=w+10;

	PrintY=(WindowH/2)-h/2;
	PrintX=WindowX=160-mw/2;

	DrawWindow(WindowX-5,PrintY-5,mw+10,h+10,TEXTCOLOR);
	DrawOutline(WindowX-5,PrintY-5,mw+10,h+10,0,HIGHLIGHT);
	SETFONTCOLOR(0,TEXTCOLOR);
	US_Print(string);
	VW_UpdateScreen();
}

static	int	lastmusic;

void StartCPMusic(int song)
{
	musicnames	chunk;

	if (audiosegs[STARTMUSIC + lastmusic])	// JDC
		MM_FreePtr ((memptr *)&audiosegs[STARTMUSIC + lastmusic]);
	lastmusic = song;

	SD_MusicOff();
	chunk =	song;

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

void FreeMusic (void)
{
	if (audiosegs[STARTMUSIC + lastmusic])
		MM_FreePtr ((memptr *)&audiosegs[STARTMUSIC + lastmusic]);
}

byte *
IN_GetScanName(ScanCode scan)
{
	byte		**p;
	ScanCode	far *s;

	for (s = ExtScanCodes,p = ExtScanNames;*s;p++,s++)
		if (*s == scan)
			return(*p);

	return(ScanNames[scan]);
}

void CheckPause(void)
{
	if (Paused)
	{
		switch(SoundStatus)
		{
			case 0: SD_MusicOn(); break;
			case 1: SD_MusicOff(); break;
		}

		SoundStatus^=1;
		VW_WaitVBL(3);
		IN_ClearKeysDown();
		Paused=false;
 }
}

void DrawMenuGun(CP_iteminfo *iteminfo)
{
	int x,y;

	CacheLump (CURSOR_LUMP_START,CURSOR_LUMP_END);
	x=iteminfo->x;
	y=iteminfo->y+iteminfo->curpos*16-2;
	VWB_DrawPic(x,y,C_CURSOR1PIC);
	UnCacheLump (CURSOR_LUMP_START,CURSOR_LUMP_END);
}

void ShootSnd(void)
{
	SD_PlaySound(WTFSND);
}

void CheckForEpisodes(void)
{
	struct ffblk f;

	if (!findfirst("*.DLH",&f,FA_ARCH))
	{
		strcpy(extension,"DLH");
		NewEmenu[1].active =
		NewEmenu[2].active =
		NewEmenu[3].active =
		EpisodeSelect[1] =
		EpisodeSelect[2] =
		EpisodeSelect[3] = 1;
	}
	else

	strcat(configname,extension);
	strcat(SaveName,extension);
	strcat(PageFileName,extension);
	strcat(audioname,extension);
	strcat(demoname,extension);
}
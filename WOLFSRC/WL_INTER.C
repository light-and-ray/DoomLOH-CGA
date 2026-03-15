// WL_INTER.C

#include "WL_DEF.H"
#pragma hdrstop

void ClearSplitVWB (void)
{
	memset (update,0,sizeof(update));
	WindowX = 0;
	WindowY = 0;
	WindowW = 320;
	WindowH = 160;
}

void Victory (void)
{
	long	sec;
	int i,min,kr,sr,tr,x;
	char tempstr[8];

#define RATIOX 48

	StartCPMusic (D_VICTOR_MUS);
	ClearSplitVWB ();
	CacheLump(LEVELEND_LUMP_START,LEVELEND_LUMP_END);
	CacheLump(INTERMISSION_LUMP_START,INTERMISSION_LUMP_END);
	CA_CacheGrChunk(STARTFONT);

	VWB_Bar (0,0,320,200-STATUSLINES,0);
      //	DrawWindow (111,126,74,18,0); // PAR
     //   DrawWindow (183,66,74,54,0); // STATS

	VWB_DrawPic(120,32,YOUWINPIC);
	VWB_DrawPic(64,128,TIMEPIC);
	VWB_DrawPic(144,16,AVERAGEPIC);
	VWB_DrawPic(50,68,KILLSPIC);
	VWB_DrawPic(50,104,ITEMSPIC);
	VWB_DrawPic(50,86,SECRETPIC);
	VWB_DrawPic(240,68,L_PERCENTPIC);
	VWB_DrawPic(240,86,L_PERCENTPIC);
	VWB_DrawPic(240,104,L_PERCENTPIC);

	for (kr = sr = tr = sec = i = 0;i < 8;i++)
	{
		sec += LevelRatios[i].time;
		kr += LevelRatios[i].kill;
		sr += LevelRatios[i].secret;
		tr += LevelRatios[i].treasure;
	}

	kr /= 8;
	sr /= 8;
	tr /= 8;

	min = sec/60;
	sec %= 60;

	if (min > 99)
		min = sec = 99;

	VWB_DrawPic(112,128,L_NUM0PIC+(min/10));
	VWB_DrawPic(128,128,L_NUM0PIC+(min%10));
	VWB_DrawPic(144,128,L_COLONPIC);
	VWB_DrawPic(152,128,L_NUM0PIC+(sec/10));
	VWB_DrawPic(168,128,L_NUM0PIC+(sec%10));
	VW_UpdateScreen ();

	itoa(kr,tempstr,10);
	x=RATIOX+168-strlen(tempstr)*2;
	Write(x,68,tempstr);

	itoa(sr,tempstr,10);
	x=RATIOX+168-strlen(tempstr)*2;
	Write(x,86,tempstr);

	itoa(tr,tempstr,10);
	x=RATIOX+168-strlen(tempstr)*2;
	Write(x,104,tempstr);

	fontnumber = 1;

	VW_UpdateScreen ();
	VW_FadeIn ();

	IN_Ack();

	if (Keyboard[sc_P] && MS_CheckParm(PARMCHEAT))
		PicturePause();

	VW_FadeOut ();

	UnCacheLump(LEVELEND_LUMP_START,LEVELEND_LUMP_END);
	UnCacheLump(INTERMISSION_LUMP_START,INTERMISSION_LUMP_END);
}
#define IMAGEDELAY 10

boolean DoAnimation(void)
{
   char AnimName[13] = "EPxANIM.";
   int file;
   char chunktype[] = "  ";
   long chunknum,chunksize;
   int chunkvalue;
   byte bgcolor,readbyte,p;
   unsigned hasstrand,pixelpos,datalength;
   memptr buffer,strand;
   byte far *screen;
   byte far *bufferpos;
   unsigned x,y,temp;
   SDSMode PrevDigiMode = DigiMode;

   // Build the file name.
   AnimName[2]= '1'+gamestate.episode;
   strcat(AnimName,extension);
   // try to open the file.
   file = open(AnimName, O_BINARY | O_RDONLY, S_IREAD);
   if (file == -1)
      // file does not exist... abort.
      return false;
   // force it to start as faded out
   VW_FadeOut();
   // Force the output to NOT be digital (bombs with "PML_GetPageBuffer: Purged main block").
   DigiMode = sds_Off;
   // prepare the draw buffer
   MM_GetPtr (&buffer,64000);
   MM_SetLock (&buffer,true);
   IN_ClearKeysDown();
   do
   {
      // read the chunk information.
      read(file,&chunktype,2);
      read(file,&chunknum,sizeof(chunknum));
      read(file,&chunksize,sizeof(chunksize));
      if (strcmp(chunktype,"XX") == 0)   // end
      {
	 break;
      }
      if (strcmp(chunktype,"GR") == 0)   // graphic
      {
	 if (chunknum==1)
	 {
	    // read the background color.
	    read(file,&bgcolor,sizeof(bgcolor));
	    // clear the screen for the first chunk
	    VWB_Bar(0,0,320,200,bgcolor);
	    // set our buffer to the bgcolor
	    bufferpos = (byte far *)buffer+pixelpos;
	    for (x=0;x<64000;x++)
	       *bufferpos++ = bgcolor;
	 }
         // Process the draw data
         do
         {
            read(file,&hasstrand,sizeof(hasstrand));
	    read(file,&pixelpos,sizeof(pixelpos));
            read(file,&datalength,sizeof(datalength));
            if (!hasstrand)
               // no more draw strands, exit loop.
               break;
            bufferpos = (byte far *)buffer+pixelpos;
            // NOTE: This is slow because we are reading one byte at a time.
            //   TODO: Find out how to read a chunk from the file at once.
            for (x=0;x<datalength;x++)
            {
               read(file,&readbyte,sizeof(readbyte));
               *bufferpos++ = readbyte;
            }
	 }
         while (true);
         // update the backbuffer.
         for (p=0; p<4; p++)
         {
            byte mask=1<<p;
            VGAMAPMASK(mask);
            for (x=p;x<320;x+=4)
            {
               screen = MK_FP(SCREENSEG,bufferofs+(x>>2));
               bufferpos=(byte far *)buffer+x;
               for (y=0;y<200;y++,screen+=80,bufferpos+=320)
               {
		  // write to the screen.
                  *screen=*bufferpos;
               }
            }
         }
         // Mark the entire screen for updating.
         VW_MarkUpdateBlock(0,0,319,199);
         VW_UpdateScreen();
         // wait a bit
         TimeCount = 0;
         while (TimeCount < IMAGEDELAY)
         ;
      }
      else if (strcmp(chunktype,"FI") == 0)   // fade in
      {
         VW_FadeIn();
      }
      else if (strcmp(chunktype,"FO") == 0)   // fade out
      {
         VW_FadeOut();
      }
      else if (strcmp(chunktype,"SD") == 0)   // sound
      {
         read(file,&chunkvalue,sizeof(chunkvalue));
         SD_PlaySound(chunkvalue);
      }
      else if (strcmp(chunktype,"PA") == 0)   // pause
      {
         read(file,&chunkvalue,sizeof(chunkvalue));
         TimeCount = 0;
         while (TimeCount < chunkvalue)
         ;            
      }
      else
      {
         sprintf(str,"Unknown animation command: %s",chunktype);
         Quit (str);
      }
   }
   // allow the user to interrupt the animation sequence.
   while (!LastScan);

   // close up
   close(file);
   MM_FreePtr(&buffer);
   // Change back to the previous digi mode.
   DigiMode =PrevDigiMode;
   // return true to redraw the borders/statusbar
   return true;
}
#define ANIMDELAY 5

boolean CSLOGOAnim(void)
{
   char AnimName[13] = "ANIMCS.";
   int file;
   char chunktype[] = "  ";
   long chunknum,chunksize;
   int chunkvalue;
   byte bgcolor,readbyte,p;
   unsigned hasstrand,pixelpos,datalength;
   memptr buffer,strand;
   byte far *screen;
   byte far *bufferpos;
   unsigned x,y,temp;
   SDSMode PrevDigiMode = DigiMode;

 
   strcat(AnimName,extension);
   // try to open the file.
   file = open(AnimName, O_BINARY | O_RDONLY, S_IREAD);
   // force it to start as faded out
   VW_FadeOut();
   // Force the output to NOT be digital (bombs with "PML_GetPageBuffer: Purged main block").
   DigiMode = sds_Off;
   // prepare the draw buffer
   MM_GetPtr (&buffer,64000);
   MM_SetLock (&buffer,true);
   IN_ClearKeysDown();
   do
   {
      // read the chunk information.
      read(file,&chunktype,2);
      read(file,&chunknum,sizeof(chunknum));
      read(file,&chunksize,sizeof(chunksize));
      if (strcmp(chunktype,"XX") == 0)   // end
      {
         break;
      }
      if (strcmp(chunktype,"GR") == 0)   // graphic
      {
	 if (chunknum==1)
         {
            // read the background color.
            read(file,&bgcolor,sizeof(bgcolor));
            // clear the screen for the first chunk
            VWB_Bar(0,0,320,200,bgcolor);
            // set our buffer to the bgcolor
            bufferpos = (byte far *)buffer+pixelpos;
            for (x=0;x<64000;x++)
               *bufferpos++ = bgcolor;
         }
         // Process the draw data
         do
	 {
            read(file,&hasstrand,sizeof(hasstrand));
            read(file,&pixelpos,sizeof(pixelpos));
            read(file,&datalength,sizeof(datalength));
            if (!hasstrand)
               // no more draw strands, exit loop.
               break;
            bufferpos = (byte far *)buffer+pixelpos;
            // NOTE: This is slow because we are reading one byte at a time.
            //   TODO: Find out how to read a chunk from the file at once.
            for (x=0;x<datalength;x++)
            {
               read(file,&readbyte,sizeof(readbyte));
	       *bufferpos++ = readbyte;
            }
         }
         while (true);
         // update the backbuffer.
         for (p=0; p<4; p++)
         {
            byte mask=1<<p;
            VGAMAPMASK(mask);
            for (x=p;x<320;x+=4)
            {
               screen = MK_FP(SCREENSEG,bufferofs+(x>>2));
               bufferpos=(byte far *)buffer+x;
	       for (y=0;y<200;y++,screen+=80,bufferpos+=320)
               {
                  // write to the screen.
                  *screen=*bufferpos;
               }
            }
         }
         // Mark the entire screen for updating.
         VW_MarkUpdateBlock(0,0,319,199);
         VW_UpdateScreen();
         // wait a bit
         TimeCount = 0;
         while (TimeCount < ANIMDELAY)
	 ;
      }
      else if (strcmp(chunktype,"FI") == 0)   // fade in
      {
         VW_FadeIn();
      }
      else if (strcmp(chunktype,"FO") == 0)   // fade out
      {
         VW_FadeOut();
      }
      else if (strcmp(chunktype,"SD") == 0)   // sound
      {
         read(file,&chunkvalue,sizeof(chunkvalue));
	 SD_PlaySound(chunkvalue);
      }
      else if (strcmp(chunktype,"PA") == 0)   // pause
      {
         read(file,&chunkvalue,sizeof(chunkvalue));
         TimeCount = 0;
         while (TimeCount < chunkvalue)
         ;            
      }
      else
      {
         sprintf(str,"Unknown animation command: %s",chunktype);
         Quit (str);
      }
   }
   // allow the user to interrupt the animation sequence.
   while (!LastScan);

   // close up
   close(file);
   MM_FreePtr(&buffer);
   // Change back to the previous digi mode.
   DigiMode =PrevDigiMode;
   // return true to redraw the borders/statusbar
   return true;
}

void Write(int x,int y,char *string)
{
 int alpha[]={L_NUM0PIC,L_NUM1PIC,L_NUM2PIC,L_NUM3PIC,L_NUM4PIC,L_NUM5PIC,
	L_NUM6PIC,L_NUM7PIC,L_NUM8PIC,L_NUM9PIC,L_COLONPIC};

 int i,ox,nx,ny;
 char ch;

 ox=nx=x;
 ny=y;
 for (i=0;i<strlen(string);i++)
   if (string[i]=='\n')
   {
	nx=ox;
	ny+=16;
   }
   else
   {
	ch=string[i];
	if (ch>='a')
	  ch-=('a'-'A');
	ch-='0';

	   VWB_DrawPic(nx,ny,alpha[ch]);
	if (string[i]==':')
	nx+=8;
	else
	nx+=16;
   }
}

void BJ_Breathe (void)
{
	static int which=0,max=10;
	int pics[2]={PLAYER1PIC,PLAYER2PIC};
	if (TimeCount>max)
	{
		which^=1;
		VWB_DrawPic (0,0,pics[which]);
		VW_UpdateScreen();
		TimeCount=0;
		max=35;
	}
}

LRstruct LevelRatios[8];

void LevelCompleted (void)
{
	#define VBLWAIT	30
	#define PAR_AMOUNT	500
	#define PERCENT100AMT	10000
	typedef struct {
			float time;
			char timestr[6];
			} times;

	int	x,i,min,sec,ratio,kr,sr,tr;
	unsigned	temp;
	char tempstr[10];
	long bonus,timeleft=0;
	times parTimes[]=
	{
	 {1,	" 1:00"},
	 {2,	" 2:00"},
	 {2.75,	" 2:45"},
	 {5.5,	" 5:30"},
	 {3,	" 3:00"},
	 {3,	" 3:00"},
	 {2.5,	" 2:30"},
	 {1.5,	" 1:30"},
	 {0,	"??:??"},
	 {0,	" 2:30"},

	 {1.5,	"01:30"},
	 {3.5,	"03:30"},
	 {3,	"03:00"},
	 {2,	"02:00"},
	 {4,	"04:00"},
	 {6,	"06:00"},
	 {1,	"01:00"},
	 {3,	"03:00"},
	 {0,	"??:??"},
	 {0,	"??:??"},

	 {2,	" 2:00"}, // timed 3-1
	 {1.5,	"01:30"},
	 {2.5,	"02:30"},
	 {5,	" 5:00"}, // timed 3-4
	 {3.5,	"03:30"},
	 {2.5,	"02:30"},
	 {2,	"02:00"},
	 {6,	"06:00"},
	 {0,	"??:??"},
	 {0,	"??:??"},

	 {4,	" 4:00"}, // timed 4-1
	 {2,	"02:00"},
	 {1.5,	"01:30"},
	 {1,	"01:00"},
	 {4.5,	"04:30"},
	 {3.5,	"03:30"},
	 {2,	"02:00"},
	 {4.5,	"04:30"},
	 {0,	"??:??"},
	 {0,	"??:??"},
	 {0,	"??:??"}
	};

	CacheLump(LEVELEND_LUMP_START,LEVELEND_LUMP_END);
	CacheLump(INTERMISSION_LUMP_START,INTERMISSION_LUMP_END);
	CA_CacheGrChunk (FINISHEDPIC);
	ClearSplitVWB ();
	VWB_Bar (0,0,320,200-STATUSLINES,0);
     //	DrawWindow (220,48,69,54,0);  //STAT
     //	DrawWindow (79,142,73,18,0);  //T
     //	DrawWindow (224,142,64,18,0); //P
     //	DrawWindow (184,106,98,18,0); //B
	StartCPMusic(D_MMENU_MUS);
	if (!godflag)
		godmode = gamestate.godmode = false;
	IN_ClearKeysDown();
	IN_StartAck();
	switch (gamestate.mapon+gamestate.episode*10)
	{
	case 0:
		CA_CacheGrChunk (E1M1PIC);
		VWB_DrawPic (76,8,E1M1PIC);
		UNCACHEGRCHUNK (E1M1PIC);
		break;
	case 1:
		CA_CacheGrChunk (E1M2PIC);
		VWB_DrawPic (108,8,E1M2PIC);
		UNCACHEGRCHUNK (E1M2PIC);
		break;
	case 2:
		CA_CacheGrChunk (E1M3PIC);
		VWB_DrawPic (76,8,E1M3PIC);
		UNCACHEGRCHUNK (E1M3PIC);
		break;
	case 3:
		CA_CacheGrChunk (E1M4PIC);
		VWB_DrawPic (44,8,E1M4PIC);
		UNCACHEGRCHUNK (E1M4PIC);
		break;
	case 4:
		CA_CacheGrChunk (E1M5PIC);
		VWB_DrawPic (78,8,E1M5PIC);
		UNCACHEGRCHUNK (E1M5PIC);
		break;
	case 5:
		CA_CacheGrChunk (E1M6PIC);
		VWB_DrawPic (76,8,E1M6PIC);
		UNCACHEGRCHUNK (E1M6PIC);
		break;
	case 6:
		CA_CacheGrChunk (E1M7PIC);
		VWB_DrawPic (80,8,E1M7PIC);
		UNCACHEGRCHUNK (E1M7PIC);
		break;
	case 7:
		CA_CacheGrChunk (E1M8PIC);
		VWB_DrawPic (32,8,E1M8PIC);
		UNCACHEGRCHUNK (E1M8PIC);
		break;
	case 8:
		CA_CacheGrChunk (E1M9PIC);
		VWB_DrawPic (92,8,E1M9PIC);
		UNCACHEGRCHUNK (E1M9PIC);
		break;
	case 9:
		CA_CacheGrChunk (E1M10PIC);
		VWB_DrawPic (86,8,E1M10PIC);
		UNCACHEGRCHUNK (E1M10PIC);
		break;
	case 10:
		CA_CacheGrChunk (E2M1PIC);
		VWB_DrawPic (70,8,E2M1PIC);
		UNCACHEGRCHUNK (E2M1PIC);
		break;
	case 11:
		CA_CacheGrChunk (E2M2PIC);
		VWB_DrawPic (92,8,E2M2PIC);
		UNCACHEGRCHUNK (E2M2PIC);
		break;
	case 12:
		CA_CacheGrChunk (E2M3PIC);
		VWB_DrawPic (86,8,E2M3PIC);
		UNCACHEGRCHUNK (E2M3PIC);
		break;
	case 13:
		CA_CacheGrChunk (E2M4PIC);
		VWB_DrawPic (60,8,E2M4PIC);
		UNCACHEGRCHUNK (E2M4PIC);
		break;
	case 14:
		CA_CacheGrChunk (E2M5PIC);
		VWB_DrawPic (112,8,E2M5PIC);
		UNCACHEGRCHUNK (E2M5PIC);
		break;
	case 15:
		CA_CacheGrChunk (E2M6PIC);
		VWB_DrawPic (80,8,E2M6PIC);
		UNCACHEGRCHUNK (E2M6PIC);
		break;
	case 16:
		CA_CacheGrChunk (E2M7PIC);
		VWB_DrawPic (76,8,E2M7PIC);
		UNCACHEGRCHUNK (E2M7PIC);
		break;
	case 17:
		CA_CacheGrChunk (E2M8PIC);
		VWB_DrawPic (60,8,E2M8PIC);
		UNCACHEGRCHUNK (E2M8PIC);
		break;
	case 18:
		CA_CacheGrChunk (E2M9PIC);
		VWB_DrawPic (94,8,E2M9PIC);
		UNCACHEGRCHUNK (E2M9PIC);
		break;
	case 19:
		CA_CacheGrChunk (E2M10PIC);
		VWB_DrawPic (60,8,E2M10PIC);
		UNCACHEGRCHUNK (E2M10PIC);
		break;
	case 20:
		CA_CacheGrChunk (E3M1PIC);
		VWB_DrawPic (66,8,E3M1PIC);
		UNCACHEGRCHUNK (E3M1PIC);
		break;
	case 21:
		CA_CacheGrChunk (E3M2PIC);
		VWB_DrawPic (90,8,E3M2PIC);
		UNCACHEGRCHUNK (E3M2PIC);
		break;
	case 22:
		CA_CacheGrChunk (E3M3PIC);
		VWB_DrawPic (122,8,E3M3PIC);
		UNCACHEGRCHUNK (E3M3PIC);
		break;
	case 23:
		CA_CacheGrChunk (E3M4PIC);
		VWB_DrawPic (102,8,E3M4PIC);
		UNCACHEGRCHUNK (E3M4PIC);
		break;
	case 24:
		CA_CacheGrChunk (E3M5PIC);
		VWB_DrawPic (128,8,E3M5PIC);
		UNCACHEGRCHUNK (E3M5PIC);
		break;
	case 25:
		CA_CacheGrChunk (E3M6PIC);
		VWB_DrawPic (96,8,E3M6PIC);
		UNCACHEGRCHUNK (E3M6PIC);
		break;
	case 26:
		CA_CacheGrChunk (E3M7PIC);
		VWB_DrawPic (64,8,E3M7PIC);
		UNCACHEGRCHUNK (E3M7PIC);
		break;
	case 27:
		CA_CacheGrChunk (E3M8PIC);
		VWB_DrawPic (94,8,E3M8PIC);
		UNCACHEGRCHUNK (E3M8PIC);
		break;
	case 28:
		CA_CacheGrChunk (E3M9PIC);
		VWB_DrawPic (108,8,E3M9PIC);
		UNCACHEGRCHUNK (E3M9PIC);
		break;
	case 29:
		CA_CacheGrChunk (E3M10PIC);
		VWB_DrawPic (74,8,E3M10PIC);
		UNCACHEGRCHUNK (E3M10PIC);
		break;
	case 30:
		CA_CacheGrChunk (E4M1PIC);
		VWB_DrawPic (54,8,E4M1PIC);
		UNCACHEGRCHUNK (E4M1PIC);
		break;
	case 31:
		CA_CacheGrChunk (E4M2PIC);
		VWB_DrawPic (80,8,E4M2PIC);
		UNCACHEGRCHUNK (E4M2PIC);
		break;
	case 32:
		CA_CacheGrChunk (E4M3PIC);
		VWB_DrawPic (84,8,E4M3PIC);
		UNCACHEGRCHUNK (E4M3PIC);
		break;
	case 33:
		CA_CacheGrChunk (E4M4PIC);
		VWB_DrawPic (88,8,E4M4PIC);
		UNCACHEGRCHUNK (E4M4PIC);
		break;
	case 34:
		CA_CacheGrChunk (E4M5PIC);
		VWB_DrawPic (104,8,E4M5PIC);
		UNCACHEGRCHUNK (E4M5PIC);
		break;
	case 35:
		CA_CacheGrChunk (E4M6PIC);
		VWB_DrawPic (84,8,E4M6PIC);
		UNCACHEGRCHUNK (E4M6PIC);
		break;
	case 36:
		CA_CacheGrChunk (E4M7PIC);
		VWB_DrawPic (84,8,E4M7PIC);
		UNCACHEGRCHUNK (E4M7PIC);
		break;
	case 37:
		CA_CacheGrChunk (E4M8PIC);
		VWB_DrawPic (66,8,E4M8PIC);
		UNCACHEGRCHUNK (E4M8PIC);
		break;
	case 38:
		CA_CacheGrChunk (E4M9PIC);
		VWB_DrawPic (34,8,E4M9PIC);
		UNCACHEGRCHUNK (E4M9PIC);
		break;
	case 39:
		CA_CacheGrChunk (E4M10PIC);
		VWB_DrawPic (106,8,E4M10PIC);
		UNCACHEGRCHUNK (E4M10PIC);
		break;
	}

	 VWB_DrawPic(114,23,FINISHEDPIC);
	 VWB_DrawPic(50,108,BONUSPIC);
	 VWB_DrawPic(15,146,TIMEPIC);
	 VWB_DrawPic(176,146,PARPIC);
	 VWB_DrawPic(50,50,KILLSPIC);
	 VWB_DrawPic(50,68,SECRETPIC);
	 VWB_DrawPic(50,86,ITEMSPIC);
	 VWB_DrawPic(276,50,L_PERCENTPIC);
	 VWB_DrawPic(276,68,L_PERCENTPIC);
	 VWB_DrawPic(276,86,L_PERCENTPIC);
	 UNCACHEGRCHUNK (FINISHEDPIC);

	 Write(223,144,parTimes[gamestate.episode*10+mapon].timestr);
	 sec=gamestate.TimeCount/70;

	 if (sec > 99*60)
	   sec = 99*60;

	 if (gamestate.TimeCount<parTimes[gamestate.episode*10+mapon].time*4200)
		timeleft=(parTimes[gamestate.episode*10+mapon].time*4200)/70-sec;

	 min=sec/60;
	 sec%=60;
	 VWB_DrawPic(80,144,L_NUM0PIC+(min/10));
	 VWB_DrawPic(96,144,L_NUM0PIC+(min%10));
	 VWB_DrawPic(112,144,L_COLONPIC);
	 VWB_DrawPic(120,144,L_NUM0PIC+(sec/10));
	 VWB_DrawPic(136,144,L_NUM0PIC+(sec%10));

	 VW_UpdateScreen ();
	 VW_FadeIn ();

	 kr = sr = tr = 0;
	 if (gamestate.killtotal)
		kr=(gamestate.killcount*100)/gamestate.killtotal;
	 if (gamestate.secrettotal)
		sr=(gamestate.secretcount*100)/gamestate.secrettotal;
	 if (gamestate.treasuretotal)
		tr=(gamestate.treasurecount*100)/gamestate.treasuretotal;

	 bonus=timeleft*PAR_AMOUNT;
	 if (bonus)
	 {
	  for (i=0;i<=timeleft;i++)
	  {
	   ltoa((long)i*PAR_AMOUNT,tempstr,10);
	   x=216-strlen(tempstr)*2;
	   Write(x,108,tempstr);
	   if (!(i%(PAR_AMOUNT/10)))
		 SD_PlaySound(WTFSND);
	   VW_UpdateScreen();
	   while(SD_SoundPlaying());
		BJ_Breathe();
	   if (IN_CheckAck())
		 goto done;
	  }

	  VW_UpdateScreen();
	  SD_PlaySound(WTFSND);
	  while(SD_SoundPlaying())
		BJ_Breathe();
	 }

	 #define RATIOXX		37
	 //
	 // KILL RATIO
	 //
	 ratio=kr;
	 for (i=0;i<=ratio;i++)
	 {
	  itoa(i,tempstr,10);
	  x=237-strlen(tempstr)*2;
	  Write(x,50,tempstr);
	  if (!(i%10))
		SD_PlaySound(WTFSND);
	  VW_UpdateScreen ();
	  while(SD_SoundPlaying())
		BJ_Breathe();

	  if (IN_CheckAck())
		goto done;
	 }
	 if (ratio==100)
	 {
	   VW_WaitVBL(VBLWAIT);
	   SD_StopSound();
	   bonus+=PERCENT100AMT;
	   ltoa(bonus,tempstr,10);
	   x=216-strlen(tempstr)*2;
	   Write(x,108,tempstr);
	   VW_UpdateScreen();
	   SD_PlaySound(WTFSND);
	 }
	 else
	 if (!ratio)
	 {
	   VW_WaitVBL(VBLWAIT);
	   SD_StopSound();
	   SD_PlaySound(WTFSND);
	 }
	 else
	 SD_PlaySound(WTFSND);

	 VW_UpdateScreen();
	 while(SD_SoundPlaying())
		BJ_Breathe();

	 ratio=sr;
	 for (i=0;i<=ratio;i++)
	 {
	  itoa(i,tempstr,10);
	  x=237-strlen(tempstr)*2;
	  Write(x,68,tempstr);
	  if (!(i%10))
		SD_PlaySound(WTFSND);
	  VW_UpdateScreen ();
	  while(SD_SoundPlaying())
		BJ_Breathe();
	  BJ_Breathe();

	  if (IN_CheckAck())
		goto done;
	 }
	 if (ratio==100)
	 {
	   VW_WaitVBL(VBLWAIT);
	   SD_StopSound();
	   bonus+=PERCENT100AMT;
	   ltoa(bonus,tempstr,10);
	   x=216-strlen(tempstr)*2;
	   Write(x,108,tempstr);
	   VW_UpdateScreen();
	   SD_PlaySound(WTFSND);
	 }
	 else
	 if (!ratio)
	 {
	   VW_WaitVBL(VBLWAIT);
	   SD_StopSound();
	   SD_PlaySound(WTFSND);
	 }
	 else
	   SD_PlaySound(WTFSND);
	 VW_UpdateScreen();
	 while(SD_SoundPlaying())
		BJ_Breathe();

	 ratio=tr;
	 for (i=0;i<=ratio;i++)
	 {
	  itoa(i,tempstr,10);
	  x=237-strlen(tempstr)*2;
	  Write(x,86,tempstr);
	  if (!(i%10))
		SD_PlaySound(WTFSND);
	  VW_UpdateScreen ();
	  while(SD_SoundPlaying())
		BJ_Breathe();
	  if (IN_CheckAck())
		goto done;
	 }
	 if (ratio==100)
	 {
	   VW_WaitVBL(VBLWAIT);
	   SD_StopSound();
	   bonus+=PERCENT100AMT;
	   ltoa(bonus,tempstr,10);
	   x=216-strlen(tempstr)*2;
	   Write(x,108,tempstr);
	   VW_UpdateScreen();
	   SD_PlaySound(WTFSND);
	 }
	 else
	 if (!ratio)
	 {
	   VW_WaitVBL(VBLWAIT);
	   SD_StopSound();
	   SD_PlaySound(WTFSND);
	 }
	 else
	 SD_PlaySound(WTFSND);
	 VW_UpdateScreen();
	 while(SD_SoundPlaying())
		BJ_Breathe();

	 done:

	 itoa(kr,tempstr,10);
	 x=237-strlen(tempstr)*2;
	 Write(x,50,tempstr);

	 itoa(sr,tempstr,10);
	 x=237-strlen(tempstr)*2;
	 Write(x,68,tempstr);

	 itoa(tr,tempstr,10);
	 x=237-strlen(tempstr)*2;
	 Write(x,86,tempstr);

	 bonus=(long)timeleft*PAR_AMOUNT+
		   (PERCENT100AMT*(kr==100))+
		   (PERCENT100AMT*(sr==100))+
		   (PERCENT100AMT*(tr==100));

	 GivePoints(bonus);
	 ltoa(bonus,tempstr,10);
	 x=216-strlen(tempstr)*2;
	 Write(x,108,tempstr);

	 LevelRatios[mapon].kill=kr;
	 LevelRatios[mapon].secret=sr;
	 LevelRatios[mapon].treasure=tr;
	 LevelRatios[mapon].time=min*60+sec;

	VW_UpdateScreen();

	TimeCount=0;
	IN_StartAck();
	while(!IN_CheckAck())
		BJ_Breathe();

	if (Keyboard[sc_P] && MS_CheckParm(PARMCHEAT))
		PicturePause();

	VW_FadeOut ();
	temp = bufferofs;
	for (i=0;i<3;i++)
	{
		bufferofs = screenloc[i];
		DrawPlayBorder ();
	}
	bufferofs = temp;

	UnCacheLump(LEVELEND_LUMP_START,LEVELEND_LUMP_END);
	UnCacheLump(INTERMISSION_LUMP_START,INTERMISSION_LUMP_END);
}

boolean PreloadUpdate(unsigned current, unsigned total)
{
	unsigned w=100;
	unsigned x = 320;

	w = ((long)w * current) / total;
	x = ((long)x * current) / total;
	if (x)
	VWB_Bar(0,167,x,1,0xf9);
	VWB_Bar (120,85,92,12,37);
	SETFONTCOLOR(0,0);
	PrintX=122; PrintY=86;
	US_PrintUnsigned(w);
	US_Print("% complete");
	VW_WaitVBL(1);
	VW_UpdateScreen();
		return (false);
}
void PreloadGraphics(void)
{
	ClearSplitVWB ();
	VWB_Bar (0,0,320,200-STATUSLINES,108);
	CA_CacheGrChunk (ENTERINGPIC);
	VWB_DrawPic (108,2,ENTERINGPIC);
	switch (gamestate.mapon+gamestate.episode*10)
	{
	case 0:
		CA_CacheGrChunk (E1M1PIC);
		VWB_DrawPic (76,16,E1M1PIC);
		UNCACHEGRCHUNK (E1M1PIC);
		break;
	case 1:
		CA_CacheGrChunk (E1M2PIC);
		VWB_DrawPic (108,16,E1M2PIC);
		UNCACHEGRCHUNK (E1M2PIC);
		break;
	case 2:
		CA_CacheGrChunk (E1M3PIC);
		VWB_DrawPic (76,16,E1M3PIC);
		UNCACHEGRCHUNK (E1M3PIC);
		break;
	case 3:
		CA_CacheGrChunk (E1M4PIC);
		VWB_DrawPic (44,16,E1M4PIC);
		UNCACHEGRCHUNK (E1M4PIC);
		break;
	case 4:
		CA_CacheGrChunk (E1M5PIC);
		VWB_DrawPic (78,16,E1M5PIC);
		UNCACHEGRCHUNK (E1M5PIC);
		break;
	case 5:
		CA_CacheGrChunk (E1M6PIC);
		VWB_DrawPic (76,16,E1M6PIC);
		UNCACHEGRCHUNK (E1M6PIC);
		break;
	case 6:
		CA_CacheGrChunk (E1M7PIC);
		VWB_DrawPic (80,16,E1M7PIC);
		UNCACHEGRCHUNK (E1M7PIC);
		break;
	case 7:
		CA_CacheGrChunk (E1M8PIC);
		VWB_DrawPic (32,16,E1M8PIC);
		UNCACHEGRCHUNK (E1M8PIC);
		break;
	case 8:
		CA_CacheGrChunk (E1M9PIC);
		VWB_DrawPic (92,16,E1M9PIC);
		UNCACHEGRCHUNK (E1M9PIC);
		break;
	case 9:
		CA_CacheGrChunk (E1M10PIC);
		VWB_DrawPic (86,16,E1M10PIC);
		UNCACHEGRCHUNK (E1M10PIC);
		break;
	case 10:
		CA_CacheGrChunk (E2M1PIC);
		VWB_DrawPic (70,16,E2M1PIC);
		UNCACHEGRCHUNK (E2M1PIC);
		break;
	case 11:
		CA_CacheGrChunk (E2M2PIC);
		VWB_DrawPic (92,16,E2M2PIC);
		UNCACHEGRCHUNK (E2M2PIC);
		break;
	case 12:
		CA_CacheGrChunk (E2M3PIC);
		VWB_DrawPic (86,16,E2M3PIC);
		UNCACHEGRCHUNK (E2M3PIC);
		break;
	case 13:
		CA_CacheGrChunk (E2M4PIC);
		VWB_DrawPic (60,16,E2M4PIC);
		UNCACHEGRCHUNK (E2M4PIC);
		break;
	case 14:
		CA_CacheGrChunk (E2M5PIC);
		VWB_DrawPic (112,16,E2M5PIC);
		UNCACHEGRCHUNK (E2M5PIC);
		break;
	case 15:
		CA_CacheGrChunk (E2M6PIC);
		VWB_DrawPic (80,16,E2M6PIC);
		UNCACHEGRCHUNK (E2M6PIC);
		break;
	case 16:
		CA_CacheGrChunk (E2M7PIC);
		VWB_DrawPic (76,16,E2M7PIC);
		UNCACHEGRCHUNK (E2M7PIC);
		break;
	case 17:
		CA_CacheGrChunk (E2M8PIC);
		VWB_DrawPic (60,16,E2M8PIC);
		UNCACHEGRCHUNK (E2M8PIC);
		break;
	case 18:
		CA_CacheGrChunk (E2M9PIC);
		VWB_DrawPic (94,16,E2M9PIC);
		UNCACHEGRCHUNK (E2M9PIC);
		break;
	case 19:
		CA_CacheGrChunk (E2M10PIC);
		VWB_DrawPic (60,16,E2M10PIC);
		UNCACHEGRCHUNK (E2M10PIC);
		break;
	case 20:
		CA_CacheGrChunk (E3M1PIC);
		VWB_DrawPic (66,16,E3M1PIC);
		UNCACHEGRCHUNK (E3M1PIC);
		break;
	case 21:
		CA_CacheGrChunk (E3M2PIC);
		VWB_DrawPic (90,16,E3M2PIC);
		UNCACHEGRCHUNK (E3M2PIC);
		break;
	case 22:
		CA_CacheGrChunk (E3M3PIC);
		VWB_DrawPic (122,16,E3M3PIC);
		UNCACHEGRCHUNK (E3M3PIC);
		break;
	case 23:
		CA_CacheGrChunk (E3M4PIC);
		VWB_DrawPic (102,16,E3M4PIC);
		UNCACHEGRCHUNK (E3M4PIC);
		break;
	case 24:
		CA_CacheGrChunk (E3M5PIC);
		VWB_DrawPic (128,16,E3M5PIC);
		UNCACHEGRCHUNK (E3M5PIC);
		break;
	case 25:
		CA_CacheGrChunk (E3M6PIC);
		VWB_DrawPic (96,16,E3M6PIC);
		UNCACHEGRCHUNK (E3M6PIC);
		break;
	case 26:
		CA_CacheGrChunk (E3M7PIC);
		VWB_DrawPic (64,16,E3M7PIC);
		UNCACHEGRCHUNK (E3M7PIC);
		break;
	case 27:
		CA_CacheGrChunk (E3M8PIC);
		VWB_DrawPic (94,16,E3M8PIC);
		UNCACHEGRCHUNK (E3M8PIC);
		break;
	case 28:
		CA_CacheGrChunk (E3M9PIC);
		VWB_DrawPic (108,16,E3M9PIC);
		UNCACHEGRCHUNK (E3M9PIC);
		break;
	case 29:
		CA_CacheGrChunk (E3M10PIC);
		VWB_DrawPic (74,16,E3M10PIC);
		UNCACHEGRCHUNK (E3M10PIC);
		break;
	case 30:
		CA_CacheGrChunk (E4M1PIC);
		VWB_DrawPic (54,16,E4M1PIC);
		UNCACHEGRCHUNK (E4M1PIC);
		break;
	case 31:
		CA_CacheGrChunk (E4M2PIC);
		VWB_DrawPic (80,16,E4M2PIC);
		UNCACHEGRCHUNK (E4M2PIC);
		break;
	case 32:
		CA_CacheGrChunk (E4M3PIC);
		VWB_DrawPic (84,16,E4M3PIC);
		UNCACHEGRCHUNK (E4M3PIC);
		break;
	case 33:
		CA_CacheGrChunk (E4M4PIC);
		VWB_DrawPic (88,16,E4M4PIC);
		UNCACHEGRCHUNK (E4M4PIC);
		break;
	case 34:
		CA_CacheGrChunk (E4M5PIC);
		VWB_DrawPic (104,16,E4M5PIC);
		UNCACHEGRCHUNK (E4M5PIC);
		break;
	case 35:
		CA_CacheGrChunk (E4M6PIC);
		VWB_DrawPic (84,16,E4M6PIC);
		UNCACHEGRCHUNK (E4M6PIC);
		break;
	case 36:
		CA_CacheGrChunk (E4M7PIC);
		VWB_DrawPic (84,16,E4M7PIC);
		UNCACHEGRCHUNK (E4M7PIC);
		break;
	case 37:
		CA_CacheGrChunk (E4M8PIC);
		VWB_DrawPic (66,16,E4M8PIC);
		UNCACHEGRCHUNK (E4M8PIC);
		break;
	case 38:
		CA_CacheGrChunk (E4M9PIC);
		VWB_DrawPic (34,16,E4M9PIC);
		UNCACHEGRCHUNK (E4M9PIC);
		break;
	case 39:
		CA_CacheGrChunk (E4M10PIC);
		VWB_DrawPic (106,16,E4M10PIC);
		UNCACHEGRCHUNK (E4M10PIC);
		break;
	}
	UNCACHEGRCHUNK (ENTERINGPIC);
	VW_UpdateScreen();
	VW_FadeIn ();

	PM_Preload (PreloadUpdate);
	IN_UserInput (70);
	VW_FadeOut ();

	DrawPlayBorder ();
	VW_UpdateScreen ();
}

void	DrawHighScores(void)
{
	char		buffer[16],*str,buffer1[5];
	byte		temp,temp1,temp2,temp3;
	word		i,j,
				w,h,
				x,y;
	HighScore	*s;

	MM_SortMem ();

	CA_CacheGrChunk (HIGHSCORESPIC);
	CA_CacheGrChunk (STARTFONT);
	CA_CacheGrChunk (LEVELPIC);
	CA_CacheGrChunk (SCOREPIC);
	CA_CacheGrChunk (NAMEPIC);

	ClearMScreen();

	VWB_DrawPic(76,16,HIGHSCORESPIC);
	VWB_DrawPic (32,68,NAMEPIC);
	VWB_DrawPic (160,68,LEVELPIC);
	VWB_DrawPic (224,68,SCOREPIC);
	UNCACHEGRCHUNK (HIGHSCORESPIC);
	UNCACHEGRCHUNK (NAMEPIC);
	UNCACHEGRCHUNK (LEVELPIC);
	UNCACHEGRCHUNK (SCOREPIC);

	fontnumber=0;
	SETFONTCOLOR(0xb6,0xb6);

	for (i = 0,s = Scores;i < MaxScores;i++,s++)
	{
		PrintY = 76 + (16 * i);
		PrintX = 4*8;
		US_Print(s->name);

		ultoa(s->completed,buffer,10);
		for (str = buffer;*str;str++)
			*str = *str + (129 - '0');
		USL_MeasureString(buffer,&w,&h);
		PrintX = (22 * 8)-w;

		PrintX -= 6;
		itoa(s->episode+1,buffer1,10);
		US_Print("E");
		US_Print(buffer1);
		US_Print("/L");
		US_Print(buffer);

		ultoa(s->score,buffer,10);
		for (str = buffer;*str;str++)
			*str = *str + (129 - '0');
		USL_MeasureString(buffer,&w,&h);
		PrintX = (34 * 8) - 8 - w;
		US_Print(buffer);
	}
	VW_UpdateScreen ();
}

void	CheckHighScore (long score,word other)
{
	word		i,j;
	int			n;
	HighScore	myscore;

	strcpy(myscore.name,"");
	myscore.score = score;
	myscore.episode = gamestate.episode;
	myscore.completed = other;

	for (i = 0,n = -1;i < MaxScores;i++)
	{
		if
		(
			(myscore.score > Scores[i].score)
		||	(
				(myscore.score == Scores[i].score)
			&& 	(myscore.completed > Scores[i].completed)
			)
		)
		{
			for (j = MaxScores;--j > i;)
				Scores[j] = Scores[j - 1];
			Scores[i] = myscore;
			n = i;
			break;
		}
	}
	DrawHighScores ();
	VW_FadeIn ();
	if (n != -1)
	{
		PrintY = 76 + (16 * n);
		PrintX = 4*8;
		backcolor = BORDCOLOR;
		fontcolor = 15;
		US_LineInput(PrintX,PrintY,Scores[n].name,nil,true,MaxHighName,100);
	}
	else
	{
		IN_ClearKeysDown ();
		IN_UserInput(500);
	}

}
// WL_GAME.C

#include "WL_DEF.H"
#pragma hdrstop

#ifdef MYPROFILE
#include <TIME.H>
#endif

boolean		ingame,fizzlein;
unsigned	latchpics[NUMLATCHPICS];
gametype	gamestate;

long		spearx,speary;
unsigned        spearangle;
boolean		spearflag;

extern	objtype *LastAttacker;

long		spearx,speary;
unsigned	spearangle;
boolean		spearflag;
int	warpwx,warpwy,warpex,warpey;
byte	pwx,pwy,pw2x,pw2y,pw3x,pw3y,pw4x,pw4y;

int ElevatorBackTo[]={2,4,8,4};

void ScanInfoPlane (void);
void SetupGameLevel (void);
void DrawPlayScreen (void);
void LoadLatchMem (void);
void GameLoop (void);

	fixed	globalsoundx,globalsoundy;
	int		leftchannel,rightchannel;
#define ATABLEMAX 15
byte righttable[ATABLEMAX][ATABLEMAX * 2] = {
{ 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 7, 7, 7, 6, 0, 0, 0, 0, 0, 1, 3, 5, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 7, 7, 6, 4, 0, 0, 0, 0, 0, 2, 4, 6, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 7, 6, 6, 4, 1, 0, 0, 0, 1, 2, 4, 6, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 7, 6, 5, 4, 2, 1, 0, 1, 2, 3, 5, 7, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 6, 5, 4, 3, 2, 2, 3, 3, 5, 6, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 6, 6, 5, 4, 4, 4, 4, 5, 6, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 6, 6, 5, 5, 5, 6, 6, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 6, 6, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8}
};
byte lefttable[ATABLEMAX][ATABLEMAX * 2] = {
{ 8, 8, 8, 8, 8, 8, 8, 8, 5, 3, 1, 0, 0, 0, 0, 0, 6, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 6, 4, 2, 0, 0, 0, 0, 0, 4, 6, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 6, 4, 2, 1, 0, 0, 0, 1, 4, 6, 6, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 7, 5, 3, 2, 1, 0, 1, 2, 4, 5, 6, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 6, 5, 3, 3, 2, 2, 3, 4, 5, 6, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 7, 6, 5, 4, 4, 4, 4, 5, 6, 6, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 7, 6, 6, 5, 5, 5, 6, 6, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 7, 7, 6, 6, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8}
};

void
SetSoundLoc(fixed gx,fixed gy)
{
	fixed	xt,yt;
	int		x,y;

	gx -= viewx;
	gy -= viewy;

	xt = FixedByFrac(gx,viewcos);
	yt = FixedByFrac(gy,viewsin);
	x = (xt - yt) >> TILESHIFT;

	xt = FixedByFrac(gx,viewsin);
	yt = FixedByFrac(gy,viewcos);
	y = (yt + xt) >> TILESHIFT;

	if (y >= ATABLEMAX)
		y = ATABLEMAX - 1;
	else if (y <= -ATABLEMAX)
		y = -ATABLEMAX;
	if (x < 0)
		x = -x;
	if (x >= ATABLEMAX)
		x = ATABLEMAX - 1;
	leftchannel  =  lefttable[x][y + ATABLEMAX];
	rightchannel = righttable[x][y + ATABLEMAX];

#if 0
	CenterWindow(8,1);
	US_PrintSigned(leftchannel);
	US_Print(",");
	US_PrintSigned(rightchannel);
	VW_UpdateScreen();
#endif
}

void PlaySoundLocGlobal(word s,fixed gx,fixed gy)
{
	SetSoundLoc(gx,gy);
	SD_PositionSound(leftchannel,rightchannel);
	if (SD_PlaySound(s))
	{
		globalsoundx = gx;
		globalsoundy = gy;
	}
}

void UpdateSoundLoc(void)
{
	if (SoundPositioned)
	{
		SetSoundLoc(globalsoundx,globalsoundy);
		SD_SetPosition(leftchannel,rightchannel);
	}
}

void ClearMemory (void)
{
	PM_UnlockMainMem();
	SD_StopDigitized();
	MM_SortMem ();
}

void ScanInfoPlane (void)
{
	unsigned	x,y,i,j;
	int			tile;
	unsigned	far	*start;

	start = mapsegs[1];
	for (y=0;y<mapheight;y++)
		for (x=0;x<mapwidth;x++)
		{
			tile = *start++;
			if (!tile)
				continue;

			switch (tile)
			{
			case 19:
			case 20:
			case 21:
			case 22:
				SpawnPlayer(x,y,NORTH+tile-19);
				break;
			case 11:
				break;
			case 12:
				warpex = x;
				warpey = y;
				break;
			case 13:
				break;
			case 14:
				warpwx = x;
				warpwy = y;
				break;

		
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
			case 35:
			case 36:
			case 37:
			case 38:
			case 39:
			case 40:
			case 41:
			case 42:
			case 43:
			case 44:
			case 45:
			case 46:
			case 47:
			case 48:
			case 49:
			case 50:
			case 51:
			case 52:
			case 53:
			case 54:
			case 55:
			case 56:
			case 57:
			case 58:
			case 59:
			case 60:
			case 61:
			case 62:
			case 63:
			case 64:
			case 65:
			case 66:
			case 67:
			case 68:
			case 69:
			case 70:
			case 71:
			case 72:
			case 73:						// TRUCK AND SPEAR!
			case 74:
			case 75:
			case 76:
			case 77:
			case 78:
			case 79:
			case 80:
			case 81:
			case 82:
			case 83:
			case 84:
			case 85:
			case 86:
			case 87:
			case 88:
			case 89:
			case 90:
			case 91:
			case 92:
			case 93:
			case 94:
			case 95:
			case 96:
			case 97:
			case 98:
			case 99:
			case 100:
			case 101:
			case 102:
			case 103:
			case 104:
			case 105:
				SpawnStatic(x,y,tile-23);
				break;

			case 107:
				(unsigned)actorat[x][y] = 0xffff;
				if(!loadedgame)
					gamestate.secrettotal++;
				break;

			case 180:
			case 181:
			case 182:
			case 183:
				if (gamestate.difficulty<gd_hard)
					break;
				tile -= 36;
			case 144:
			case 145:
			case 146:
			case 147:
				if (gamestate.difficulty<gd_medium)
					break;
				tile -= 36;
			case 108:
			case 109:
			case 110:
			case 111:
				SpawnStand(en_mutant,x,y,tile-108);
				break;


			case 184:
			case 185:
			case 186:
			case 187:
				if (gamestate.difficulty<gd_hard)
					break;
				tile -= 36;
			case 148:
			case 149:
			case 150:
			case 151:
				if (gamestate.difficulty<gd_medium)
					break;
				tile -= 36;
			case 112:
			case 113:
			case 114:
			case 115:
				SpawnPatrol(en_mutant,x,y,tile-112);
				break;

			case 124:
				SpawnDeadGuard (x,y);
				break;

			case 188:
			case 189:
			case 190:
			case 191:
				if (gamestate.difficulty<gd_hard)
					break;
				tile -= 36;
			case 152:
			case 153:
			case 154:
			case 155:
				if (gamestate.difficulty<gd_medium)
					break;
				tile -= 36;
			case 116:
			case 117:
			case 118:
			case 119:
				SpawnStand(en_chaingunguy,x,y,tile-116);
				break;


			case 192:
			case 193:
			case 194:
			case 195:
				if (gamestate.difficulty<gd_hard)
					break;
				tile -= 36;
			case 156:
			case 157:
			case 158:
			case 159:
				if (gamestate.difficulty<gd_medium)
					break;
				tile -= 36;
			case 120:
			case 121:
			case 122:
			case 123:
				SpawnPatrol(en_chaingunguy,x,y,tile-120);
				break;

			case 198:
			case 199:
			case 200:
			case 201:
				if (gamestate.difficulty<gd_hard)
					break;
				tile -= 36;
			case 162:
			case 163:
			case 164:
			case 165:
				if (gamestate.difficulty<gd_medium)
					break;
				tile -= 36;
			case 126:
			case 127:
			case 128:
			case 129:
				SpawnStand(en_cacodemon,x,y,tile-126);
				break;

			case 202:
			case 203:
			case 204:
			case 205:
				if (gamestate.difficulty<gd_hard)
					break;
				tile -= 36;
			case 166:
			case 167:
			case 168:
			case 169:
				if (gamestate.difficulty<gd_medium)
					break;
				tile -= 36;
			case 130:
			case 131:
			case 132:
			case 133:
				SpawnPatrol(en_cacodemon,x,y,tile-130);
				break;

			case 206:
			case 207:
			case 208:
			case 209:
				if (gamestate.difficulty<gd_hard)
					break;
				tile -= 36;
			case 170:
			case 171:
			case 172:
			case 173:
				if (gamestate.difficulty<gd_medium)
					break;
				tile -= 36;
			case 134:
			case 135:
			case 136:
			case 137:
				SpawnStand(en_demon,x,y,tile-134);
				break;


			case 210:
			case 211:
			case 212:
			case 213:
				if (gamestate.difficulty<gd_hard)
					break;
				tile -= 36;
			case 174:
			case 175:
			case 176:
			case 177:
				if (gamestate.difficulty<gd_medium)
					break;
				tile -= 36;
			case 138:
			case 139:
			case 140:
			case 141:
				SpawnPatrol(en_demon,x,y,tile-138);
				break;

			case 214:
				SpawnCyber (x,y);
				break;
			case 215:
				SpawnMancubus (x,y);
				break;
			case 332:
				SpawnVile (x,y);
				break;
			case 196:
				SpawnSpider (x,y);
				break;
			case 178:
				SpawnRevenant (x,y);
				break;
			case 179:
				SpawnBoss (x,y);
				break;
			case 197:
				SpawnBoss2 (x,y);
				break;

			case 252:
			case 253:
			case 254:
			case 255:
				if (gamestate.difficulty<gd_hard)
					break;
				tile -= 18;
			case 234:
			case 235:
			case 236:
			case 237:
				if (gamestate.difficulty<gd_medium)
					break;
				tile -= 18;
			case 216:
			case 217:
			case 218:
			case 219:
				SpawnStand(en_shotgunguy,x,y,tile-216);
				break;

			case 256:
			case 257:
			case 258:
			case 259:
				if (gamestate.difficulty<gd_hard)
					break;
				tile -= 18;
			case 238:
			case 239:
			case 240:
			case 241:
				if (gamestate.difficulty<gd_medium)
					break;
				tile -= 18;
			case 220:
			case 221:
			case 222:
			case 223:
				SpawnPatrol(en_shotgunguy,x,y,tile-220);
				break;

			case 276:
			case 277:
			case 278:
			case 279:
				if (gamestate.difficulty<gd_hard)
				break;
				tile-=8;
			case 268:
			case 269:
			case 270:
			case 271:
				if (gamestate.difficulty<gd_medium)
				break;
				tile -= 8;
			case 260:
			case 261:
			case 262:
			case 263:
				SpawnStand(en_imp,x,y,tile-260);
				break;

			case 280:
			case 281:
			case 282:
			case 283:
				if (gamestate.difficulty<gd_hard)
				break;
				tile -= 8;
			case 272:
			case 273:
			case 274:
			case 275:
				if (gamestate.difficulty<gd_medium)
				break;
				tile -= 8;
			case 264:
			case 265:
			case 266:
			case 267:
				SpawnPatrol (en_imp,x,y,tile-264);
				break;

			case 300:
			case 301:
			case 302:
			case 303:
				if (gamestate.difficulty<gd_hard)
				break;
				tile -= 8;
			case 292:
			case 293:
			case 294:
			case 295:
				if (gamestate.difficulty<gd_medium)
				break;
				tile -= 8;
			case 284:
			case 285:
			case 286:
			case 287:
				SpawnStand(en_painelemental,x,y,tile-284);
				break;

			case 304:
			case 305:
			case 306:
			case 307:
				if (gamestate.difficulty<gd_hard)
				break;
				tile -= 8;
			case 296:
			case 297:
			case 298:
			case 299:
				if (gamestate.difficulty<gd_medium)
				break;
				tile -= 8;
			case 288:
			case 289:
			case 290:
			case 291:
				SpawnPatrol(en_painelemental,x,y,tile-288);
				break;

			case 324:
			case 325:
			case 326:
			case 327:
				if (gamestate.difficulty<gd_hard)
				break;
				tile -= 8;
			case 316:
			case 317:
			case 318:
			case 319:
				if (gamestate.difficulty<gd_medium)
				break;
				tile -= 8;
			case 308:
			case 309:
			case 310:
			case 311:
				SpawnStand (en_lostsoul,x,y,tile-308);
				break;

			case 328:
			case 329:
			case 330:
			case 331:
				if (gamestate.difficulty<gd_hard)
				break;
				tile -= 8;
			case 320:
			case 321:
			case 322:
			case 323:
				if (gamestate.difficulty<gd_medium)
				break;
				tile -= 8;
			case 312:
			case 313:
			case 314:
			case 315:
				SpawnPatrol (en_lostsoul,x,y,tile-312);
				break;

			case 15:
				SpawnBarrel (x,y);
				break;
			case 704:
			case 705:
			case 706:
			case 707:
				pwx = x; pwy = y;
				break;
			case 712:
			case 713:
			case 714:
			case 715:
				pw2x = x; pw2y = y;
				break;
			case 720:
			case 721:
			case 722:
			case 723:
				pw3x = x; pw3y = y;
				break;
			case 728:
			case 729:
			case 730:
			case 731:
				pw4x = x; pw4y = y;
				break;
			}

		}
}

void SetupGameLevel (void)
{
	int	x,y,i;
	unsigned	far *map,tile,spot;

	if (!loadedgame)
	{
	 gamestate.TimeCount=
	 gamestate.secrettotal=
	 gamestate.killtotal=
	 gamestate.treasuretotal=
	 gamestate.secretcount=
	 gamestate.killcount=
	 gamestate.treasurecount=0;
	 if (gamestate.mapon==10)
	 {
         	gamestate.bestweapon= 512;
		gamestate.nextweapon = gamestate.weapon = gamestate.chosenweapon = wp_hellorb;
		gamestate.ammo = gamestate.ammo2 =
		gamestate.ammo3 = gamestate.ammo4 = 0;
		DrawAmmo ();
		DrawWeapon();
	 }
	}

	if (demoplayback || demorecord)
		US_InitRndT (false);
	else
		US_InitRndT (true);

	CA_CacheMap (gamestate.mapon+10*gamestate.episode);
	mapon-=gamestate.episode*10;

	mapwidth = mapheaderseg[mapon]->width;
	mapheight = mapheaderseg[mapon]->height;

	if (mapwidth != 64 || mapheight != 64)
		Quit ("Map not 64*64!");

	memset (tilemap,0,sizeof(tilemap));
	memset (actorat,0,sizeof(actorat));
	map = mapsegs[0];
	for (y=0;y<mapheight;y++)
		for (x=0;x<mapwidth;x++)
		{
			tile = *map++;
			if (tile<AREATILE)
			{
				tilemap[x][y] = tile;
				(unsigned)actorat[x][y] = tile;
			}
			else
			{
				tilemap[x][y] = 0;
				(unsigned)actorat[x][y] = 0;
			}
		}
	InitActorList ();
	InitDoorList ();
	InitStaticList ();

	map = mapsegs[0];
	for (y=0;y<mapheight;y++)
		for (x=0;x<mapwidth;x++)
		{
			tile = *map++;
			if (tile >= 90 && tile <= 105)
			{
				switch (tile)
				{
				case 90:
				case 92:
				case 94:
				case 96:
				case 98:
				case 100:
				case 102:
				case 104:
					SpawnDoor (x,y,1,(tile-90)/2);
					break;
				case 91:
				case 93:
				case 95:
				case 97:
				case 99:
				case 101:
				case 103:
				case 105:
					SpawnDoor (x,y,0,(tile-90)/2);
					break;
				}
			}
		}
	ScanInfoPlane ();

	map = mapsegs[0];
	for (y=0;y<mapheight;y++)
		for (x=0;x<mapwidth;x++)
		{
			tile = *map++;
			if (tile == AMBUSHTILE)
			{
				tilemap[x][y] = 0;
				if ( (unsigned)actorat[x][y] == AMBUSHTILE)
					actorat[x][y] = NULL;

				if (*map >= AREATILE)
					tile = *map;
				if (*(map-1-mapwidth) >= AREATILE)
					tile = *(map-1-mapwidth);
				if (*(map-1+mapwidth) >= AREATILE)
					tile = *(map-1+mapwidth);
				if ( *(map-2) >= AREATILE)
					tile = *(map-2);

				*(map-1) = tile;
			}
		}
	CA_LoadAllSounds ();
}

void DrawPlayBorderSides (void)
{
	int	xl,yl;

	xl = 160-viewwidth/2;
	yl = (200-STATUSLINES-viewheight)/2;

	VWB_Bar (0,0,xl-1,200-STATUSLINES,108);
	VWB_Bar (xl+viewwidth+1,0,xl-2,200-STATUSLINES,108);

	VWB_Vlin (yl-1,yl+viewheight,xl-1,0);
	VWB_Vlin (yl-1,yl+viewheight,xl+viewwidth,106);
}

void DrawAllPlayBorderSides (void)
{
	unsigned	i,temp;

	temp = bufferofs;
	for (i=0;i<3;i++)
	{
		bufferofs = screenloc[i];
		DrawPlayBorderSides ();
	}
	bufferofs = temp;
}

void DrawAllPlayBorder (void)
{
	unsigned	i,temp;

	temp = bufferofs;
	for (i=0;i<3;i++)
	{
		bufferofs = screenloc[i];
		DrawPlayBorder ();
	}
	bufferofs = temp;
}

void DrawPlayBorder (void)
{
	int	xl,yl;

	VWB_Bar (0,0,320,200-STATUSLINES,108);

	xl = 160-viewwidth/2;
	yl = (200-STATUSLINES-viewheight)/2;
	VWB_Bar (xl,yl,viewwidth,viewheight,0);

	VWB_Hlin (xl-1,xl+viewwidth,yl-1,0);
	VWB_Hlin (xl-1,xl+viewwidth,yl+viewheight,106);
	VWB_Vlin (yl-1,yl+viewheight,xl-1,0);
	VWB_Vlin (yl-1,yl+viewheight,xl+viewwidth,106);
	VWB_Plot (xl-1,yl+viewheight,105);
}
void DrawPlayScreen (void)
{
	int	i,j,p,m;
	unsigned	temp;

	VW_FadeOut ();

	temp = bufferofs;

	CA_CacheGrChunk (STATUSBARPIC);

	for (i=0;i<3;i++)
	{
		bufferofs = screenloc[i];
		DrawPlayBorder ();
		VWB_DrawPic (0,200-STATUSLINES,STATUSBARPIC);
	}
	bufferofs = temp;

	UNCACHEGRCHUNK (STATUSBARPIC);

	DrawFace ();
	DrawHealth ();
	DrawAmmo ();
	DrawKeys ();
	DrawWeapon ();
	DrawArmor ();
}

#define MAXDEMOSIZE	8192

void StartDemoRecord (int levelnumber)
{
	MM_GetPtr (&demobuffer,MAXDEMOSIZE);
	MM_SetLock (&demobuffer,true);
	demoptr = (char far *)demobuffer;
	lastdemoptr = demoptr+MAXDEMOSIZE;

	*demoptr = levelnumber;
	demoptr += 4;
	demorecord = true;
}

char	demoname[13] = "DEMO?.";

void FinishDemoRecord (void)
{
	long	length,level;

	demorecord = false;

	length = demoptr - (char far *)demobuffer;

	demoptr = ((char far *)demobuffer)+1;
	*(unsigned far *)demoptr = length;

	CenterWindow(24,3);
	PrintY+=6;
	US_Print(" Demo number (0-9):");
	VW_UpdateScreen();

	if (US_LineInput (px,py,str,NULL,true,2,0))
	{
		level = atoi (str);
		if (level>=0 && level<=9)
		{
			demoname[4] = '0'+level;
			CA_WriteFile (demoname,(void far *)demobuffer,length);
		}
	}
	MM_FreePtr (&demobuffer);
}

void RecordDemo (void)
{
	int level,esc;

	CenterWindow(26,3);
	PrintY+=6;
	CA_CacheGrChunk(STARTFONT);
	fontnumber=0;
	US_Print("  Demo which level(1-40):");
	VW_UpdateScreen();
	VW_FadeIn ();
	esc = !US_LineInput (px,py,str,NULL,true,2,0);
	if (esc)
		return;

	level = atoi (str);
	level--;

	SETFONTCOLOR(0,15);
	VW_FadeOut ();

	NewGame (gd_hard,level/10);
	gamestate.mapon = level%10;

	StartDemoRecord (level);

	DrawPlayScreen ();
	VW_FadeIn ();

	startgame = false;
	demorecord = true;

	SetupGameLevel ();
	StartMusic ();
	PM_CheckMainMem ();
	fizzlein = true;

	PlayLoop ();

	demoplayback = false;

	StopMusic ();
	VW_FadeOut ();
	ClearMemory ();

	FinishDemoRecord ();
}

void PlayDemo (int demonumber)
{
	int length;

	demoname[4] = '0'+demonumber;
	CA_LoadFile (demoname,&demobuffer);
	MM_SetLock (&demobuffer,true);
	demoptr = (char far *)demobuffer;
	NewGame (1,0);
	gamestate.mapon = *demoptr++;
	gamestate.difficulty = gd_hard;
	length = *((unsigned far *)demoptr)++;
	demoptr++;
	lastdemoptr = demoptr-4+length;

	VW_FadeOut ();

	SETFONTCOLOR(0,15);
	DrawPlayScreen ();
	VW_FadeIn ();

	startgame = false;
	demoplayback = true;

	SetupGameLevel ();
	StartMusic ();
	PM_CheckMainMem ();
	fizzlein = true;

	PlayLoop ();

	MM_FreePtr (&demobuffer);
	demoplayback = false;

	StopMusic ();
	VW_FadeOut ();
	ClearMemory ();
}

#define DEATHROTATE 2

void Died (void)
{
	float	fangle;
	long	dx,dy;
	int		iangle,curangle,clockwise,counter,change;

	gamestate.weapon = -1;

	dx = killerobj->x - player->x;
	dy = player->y - killerobj->y;

	fangle = atan2(dy,dx);
	if (fangle<0)
		fangle = M_PI*2+fangle;

	iangle = fangle/(M_PI*2)*ANGLES;

	if (player->angle > iangle)
	{
		counter = player->angle - iangle;
		clockwise = ANGLES-player->angle + iangle;
	}
	else
	{
		clockwise = iangle - player->angle;
		counter = player->angle + ANGLES-iangle;
	}

	curangle = player->angle;

	if (clockwise<counter)
	{
		if (curangle>iangle)
			curangle -= ANGLES;
		do
		{
			change = tics*DEATHROTATE;
			if (curangle + change > iangle)
				change = iangle-curangle;

			curangle += change;
			player->angle += change;
			if (player->angle >= ANGLES)
				player->angle -= ANGLES;

			ThreeDRefresh ();
			CalcTics ();
		} while (curangle != iangle);
	}
	else
	{
		if (curangle<iangle)
			curangle += ANGLES;
		do
		{
			change = -tics*DEATHROTATE;
			if (curangle + change < iangle)
				change = iangle-curangle;

			curangle += change;
			player->angle += change;
			if (player->angle < 0)
				player->angle += ANGLES;

			ThreeDRefresh ();
			CalcTics ();
		} while (curangle != iangle);
	}
	SD_PlaySound (PLAYERDIESND);
	FinishPaletteShifts ();
	bufferofs += screenofs;
	VW_Bar (0,0,viewwidth,viewheight,108);
	IN_ClearKeysDown ();
	FizzleFade(bufferofs,displayofs+screenofs,viewwidth,viewheight,70,false);
	bufferofs -= screenofs;
	IN_UserInput(100);
	SD_WaitSoundDone ();
	gamestate.lives--;
	if (gamestate.lives > -1)
	{
		gamestate.health = 100;
		gamestate.armor = 0;
      gamestate.bestweapon = 3;
      gamestate.nextweapon = gamestate.weapon = gamestate.chosenweapon = wp_pistol;
		gamestate.ammo = 10;
		gamestate.ammomax = 200;
		gamestate.ammo2 = 0;
		gamestate.ammo2max = 50;
		gamestate.ammo3 = 0;
		gamestate.ammo3max = 50;
		gamestate.ammo4 = 0;
		gamestate.ammo4max = 300;
		gamestate.keys = 0;
		gamestate.attackframe = gamestate.attackcount =
		gamestate.weaponframe = 0;

		if (!godflag)
			godmode = gamestate.godmode = false;


		DrawKeys ();
		DrawWeapon ();
		DrawAmmo ();
		DrawHealth ();
		DrawFace ();
		DrawArmor ();
      }
}

void GameLoop (void)
{
	int i,xl,yl,xh,yh;
	char num[20];
	boolean	died;
#ifdef MYPROFILE
	clock_t start,end;
#endif

restartgame:
	ClearMemory ();
	SETFONTCOLOR(0,15);
	DrawPlayScreen ();
	died = false;
restart:
	do
	{
		if (!loadedgame)
		  gamestate.score = gamestate.oldscore;

		if (!godflag)
		{
			if (!gamestate.godmode)
				godmode = gamestate.godmodecount = 0;
		}
			if (!gamestate.berserk)
				gamestate.berserkcount = 0;
			if (!gamestate.goggles)
				gamestate.gogglescount = 0;
			if (!gamestate.biosuit)
				biosuit = gamestate.biosuitcount = 0;
			if (!gamestate.invisibility)
				invisibility = gamestate.invisibilitycount = 0;

		startgame = false;
		if (loadedgame)
			loadedgame = false;
		else
			SetupGameLevel ();

		if (gamestate.episode==3 && gamestate.mapon==10)
		{
			gamestate.keys |= 6;
			DrawKeys ();
		}
		ingame = true;
		StartMusic ();
		PM_CheckMainMem ();
		if (!died)
			PreloadGraphics ();
		else
			died = false;
			fizzlein = true;

startplayloop:
		PlayLoop ();

		if (spearflag)
		{
			SD_StopSound();
			//SD_PlaySound (GETSPEARSND);
			if (DigiMode != sds_Off)
			{
				long lasttimecount = TimeCount;

				while(TimeCount < lasttimecount+150)
				SD_Poll();
			}
			else
				SD_WaitSoundDone();
			ClearMemory ();
			gamestate.oldscore = gamestate.score;
			gamestate.mapon=10;
			SetupGameLevel ();
			StartMusic ();
			PM_CheckMainMem ();
			player->x = spearx;
			player->y = speary;
			player->angle = spearangle;
			spearflag = false;
			Thrust (0,0);
			goto startplayloop;
		}

		StopMusic ();
		ingame = false;

		if (demorecord && playstate != ex_warped)
			FinishDemoRecord ();

		if (startgame || loadedgame)
			goto restartgame;

		switch (playstate)
		{
		case ex_completed:
		case ex_secretlevel:
			gamestate.keys = 0;
			DrawKeys ();
			VW_FadeOut ();

			ClearMemory ();

			LevelCompleted ();
			if (!godflag)
				godmode = gamestate.godmodecount = 0;
				berserk = gamestate.berserkcount = 0;
				gamestate.gogglescount = 0;
				biosuit = gamestate.biosuit = 0;
				invisibility = gamestate.invisibilitycount=0;
			gamestate.oldscore = gamestate.score;

			if (gamestate.mapon == 9)
				gamestate.mapon = ElevatorBackTo[gamestate.episode];	// back from secret
			else

			if (playstate == ex_secretlevel)
				gamestate.mapon = 9;

			else
				gamestate.mapon++;

			break;

		case ex_died:
			Died ();
			died = true;

			if (gamestate.lives > -1)
				break;

			VW_FadeOut ();
			ClearMemory ();
			CheckHighScore (gamestate.score,gamestate.mapon+1);

			#pragma warn -sus
			MainMenu[viewscores].routine = CP_ViewScores;
			#pragma warn +sus

			return;

		case ex_victorious:
			VL_FadeOut (0,255,16,16,16,300);
			ClearMemory ();
			Victory ();
			ClearMemory ();
			DoAnimation();
			CheckHighScore (gamestate.score,gamestate.mapon+1);

			#pragma warn -sus
			MainMenu[viewscores].routine = CP_ViewScores;
			#pragma warn +sus
			return;

		default:
			ClearMemory ();
			break;
		}
	} while (1);
}
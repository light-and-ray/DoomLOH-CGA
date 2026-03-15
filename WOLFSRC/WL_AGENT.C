// WL_AGENT.C

#include "WL_DEF.H"
#pragma hdrstop

#define MAXMOUSETURN	10

#define MOVESCALE		150l
#define BACKMOVESCALE	100l
#define ANGLESCALE		20

boolean		running;
long		thrustspeed;
extern	statetype s_rocket;
extern	statetype s_plasma;
extern	statetype s_bfg;
extern	statetype s_hellorb1;

unsigned	plux,pluy;

int			anglefrac;
int			gotgatgun;

objtype		*LastAttacker;

void	T_Player (objtype *ob);
void	T_Attack (objtype *ob);
void	WarpPlayer (int facedir);
void	CheckWarpDest (int x, int y, objtype *check);

statetype s_player = {false,0,0,T_Player,NULL,NULL};
statetype s_attack = {false,0,0,T_Attack,NULL,NULL};

long	playerxmove,playerymove;

struct atkinf
{
	char	tics,attack,frame;
} attackinfo[10][14] =

{
{ {6,0,1},{6,2,2},{6,0,3},{6,-1,4} },
{ {6,0,1},{6,1,2},{6,0,3},{6,-1,4} },
{ {6,0,1},{6,1,2},{6,0,3},{6,0,4},{8,0,5},{8,0,6},{16,0,7},{8,0,6},{8,-1,5} },
{ {10,0,1},{10,1,2},{10,0,3},{10,0,4},{12,0,5},{12,5,6},{12,0,7},{12,6,8},{12,0,9},{12,7,10},{12,-1,11} },
{ {6,0,1},{6,1,2},{6,4,3},{12,-1,4} },
{ {6,0,1},{6,1,2},{6,0,3},{20,0,4},{60,-1,0} },
{ {6,0,1},{6,1,2},{6,4,3},{20,-1,4} },
{ {60,8,0},{12,0,1},{12,1,2},{12,0,3},{9,0,4},{9,-1,4} },
{ {6,0,2},{6,2,2},{6,3,3},{6,-1,3} },
{ {6,1,3},{6,0,4},{6,0,5},{6,0,6},{6,0,7},{6,0,8},{6,0,9},{6,0,10},{6,-1,11} },
};

int	strafeangle[9] = {0,90,180,270,45,135,225,315,0};

void DrawWeapon (void);
void GiveWeapon (int weapon);
void	GiveAmmo (int ammo, int ammotype);
void	GiveBackPack (void);

boolean TryMove (objtype *ob);
void T_Player (objtype *ob);

void ClipMove (objtype *ob, long xmove, long ymove);

int ChkAtkAmmo (int weapon)
{
   switch (weapon)
   {
      case wp_fists:
      case wp_chainsaw:
      case wp_hellorb: return 1;
      case wp_pistol:
      case wp_chaingun:
	   if (gamestate.ammo > 0 ) return 1;
	 break;
      case wp_shotgun:
	   if (gamestate.ammo2 > 0) return 1;
	   break;
      case wp_supershotgun:
	   if (gamestate.ammo2 > 1) return 1;
	   break;
      case wp_launcher:
	   if (gamestate.ammo3 > 0) return 1;
	   break;
      case wp_plasmagun:
      case wp_bfggun:
	   if (gamestate.ammo4 > 0) return 1;
	   break;
   }
   return 0;
}

void CheckWeaponChange (void)
{
   int   i,kbuttons[9]={sc_1,sc_2,sc_3,sc_3,sc_4,sc_5,sc_6,sc_7,sc_8};

   for (i=0;i<9;i++)
      if ((Keyboard[kbuttons[i]]) && (i != gamestate.nextweapon))
	{
	   if (gamestate.bestweapon & (1<<i) && ChkAtkAmmo(i))
	   {
	     gamestate.nextweapon = gamestate.chosenweapon = i;
	     gamestate.changingweapon = gamestate.goingdown = true;
	     IN_ClearKeysDown();
	      DrawWeapon ();
	      DrawAmmo ();
	      return;
	  }
      }
}

void ControlMovement (objtype *ob)
{
	long	oldx,oldy;
	int		angle,maxxmove;
	int		angleunits;
	long	speed;

	thrustspeed = 0;

	oldx = player->x;
	oldy = player->y;

	if (Keyboard[sc_A])
	{
		if (strafe > 0)
		{
			angle = ob->angle - ANGLES/4;
			if (angle < 0)
				angle += ANGLES;
			Thrust (angle,strafe*MOVESCALE);
		}
		else if (strafe < 0)
		{
			angle = ob->angle + ANGLES/4;
			if (angle >= ANGLES)
				angle -= ANGLES;
			Thrust (angle,-strafe*MOVESCALE);
		}
	}
	if (Keyboard[sc_S])
	{
		if (strafe > 0)
		{
			angle = ob->angle - ANGLES/4;
			if (angle < 0)
				angle += ANGLES;
			Thrust (angle,strafe*MOVESCALE);
		}
		else if (strafe < 0)
		{
			angle = ob->angle + ANGLES/4;
			if (angle >= ANGLES)
				angle -= ANGLES;
			Thrust (angle,-strafe*MOVESCALE);
		}
}
	if (buttonstate[bt_strafe])
	{
		if (controlx > 0)
		{
			angle = ob->angle - ANGLES/4;
			if (angle < 0)
				angle += ANGLES;
			Thrust (angle,controlx*MOVESCALE);
		}
		else if (controlx < 0)
		{
			angle = ob->angle + ANGLES/4;
			if (angle >= ANGLES)
				angle -= ANGLES;
			Thrust (angle,-controlx*MOVESCALE);
		}
	}
	else
	{
		anglefrac += controlx;
		angleunits = anglefrac/ANGLESCALE;
		anglefrac -= angleunits*ANGLESCALE;
		ob->angle -= angleunits;

		if (ob->angle >= ANGLES)
			ob->angle -= ANGLES;
		if (ob->angle < 0)
			ob->angle += ANGLES;

	}
	if (controly < 0)
	{
		Thrust (ob->angle,-controly*MOVESCALE);
	}
	else if (controly > 0)
	{
		angle = ob->angle + ANGLES/2;
		if (angle >= ANGLES)
			angle -= ANGLES;
		Thrust (angle,controly*BACKMOVESCALE);
	}

	if (gamestate.victoryflag)
		return;

	playerxmove = player->x - oldx;
	playerymove = player->y - oldy;
}

void StatusDrawPic (unsigned x, unsigned y, unsigned picnum)
{
#ifdef WITH_VGA
	unsigned	temp;

	temp = bufferofs;
	bufferofs = 0;

	bufferofs = PAGE1START+(200-STATUSLINES)*SCREENWIDTH;
	LatchDrawPic (x,y,picnum);
	bufferofs = PAGE2START+(200-STATUSLINES)*SCREENWIDTH;
	LatchDrawPic (x,y,picnum);
	bufferofs = PAGE3START+(200-STATUSLINES)*SCREENWIDTH;
	LatchDrawPic (x,y,picnum);

	bufferofs = temp;
#else
	VWB_DrawPicDirectToScreen(x << 1, y + 200 - STATUSLINES, picnum);

	if(cgamode == HERCULES720_MODE && adjustherculesaspect)
	{
		VWB_DrawPicDirectToScreen(x << 1, y + 240 - STATUSLINES, picnum);
	}
	else
	{
		VWB_DrawPicDirectToScreen(x << 1, y + 200 - STATUSLINES, picnum);
	}

#endif
}


/*
==================
=
= DrawFace
=
==================
*/

void DrawFace (void)
{
	if (gamestate.health)
	{
		if (godmode)
			StatusDrawPic (144,2,GODFACEPIC);
		else
		if (gamestate.health > 100)
		StatusDrawPic (144,2,FACE1APIC+gamestate.faceframe);
		else
		if (gamestate.invisibility)
		StatusDrawPic (144,2,INVISFACE1PIC+gamestate.faceframe);
		else
		StatusDrawPic (144,2,FACE1APIC+3*((100-gamestate.health)/20)+gamestate.faceframe);
	}
	else
	{
	   StatusDrawPic (144,2,FACE8APIC);
	}
}

#define FACETICS	70

int	facecount;

void	UpdateFace (void)
{

	if (SD_SoundPlaying() == WEAPONPICKUPSND)
	  return;

	facecount += tics;
	if (facecount > 100)
	{
		gamestate.faceframe = (US_RndT()>>6);
		if (gamestate.faceframe==3)
			gamestate.faceframe = 1;

		facecount = 0;
		DrawFace ();
	}
	if (!biosuit)
	{
		if (MAPSPOT(player->tilex,player->tiley,3)==238)
		{
			if (gamestate.TimeCount%70==1)
				TakeDamage (16,NULL);
		}
		if (MAPSPOT(player->tilex,player->tiley,3)==244)
		{
			if (gamestate.TimeCount%35==1)
				TakeDamage (16,NULL);
		}
	}
}
void EvilFace (void)
{
	if (gamestate.health>80)
		StatusDrawPic (144,2,EVIL0FACE);
	if (gamestate.health>60 && gamestate.health<79)
		StatusDrawPic (144,2,EVIL1FACE);
	if (gamestate.health>40 && gamestate.health<59)
		StatusDrawPic (144,2,EVIL2FACE);
	if (gamestate.health>20 && gamestate.health<39)
		StatusDrawPic (144,2,EVIL3FACE);
	if (gamestate.health>0 && gamestate.health<19)
		StatusDrawPic (144,2,EVIL4FACE);
}
void	LatchNumber (int x, int y, int width, long number)
{
	unsigned	length,c;
	char	str[20];

	ltoa (number,str,10);

	length = strlen (str);

	while (length<width)
	{
		StatusDrawPic (x,y,N_BLANKPIC);
		x+=16;
		width--;
	}

	c= length <= width ? 0 : length-width;

	while (c<length)
	{
		StatusDrawPic (x,y,str[c]-'0'+ N_0PIC);
		x+=16;
		c++;
	}
}

void	LatchSNumber (int x, int y, int width, long number)
{
	unsigned	length,c;
	char	str[20];

	ltoa (number,str,10);

	length = strlen (str);

	while (length<width)
	{
		StatusDrawPic (x,y,A_BLANKPIC);
		x+=4;
		width--;
	}

	c= length <= width ? 0 : length-width;

	while (c<length)
	{
		StatusDrawPic (x,y,str[c]-'0'+ A_0PIC);
		x+=4;
		c++;
	}
}

void	DrawHealth (void)
{
	LatchNumber (52,3,3,gamestate.health);
}

void	TakeDamage (int points,objtype *attacker)
{
	LastAttacker = attacker;

	if (gamestate.victoryflag)
		return;
	if (gamestate.difficulty==gd_baby)
	  points>>=2;

	  if (!godmode)
	  {
		if (gamestate.armor)
		{
			if ((gamestate.health -= points/3) >= 0)
			{
				gamestate.health -= ((int)points/3);
				if (gamestate.health > 200)
					gamestate.health = 200;
				}
				gamestate.armor -= points/2+2;
			}
			else
				gamestate.health -= points;
		}
		if (gamestate.armor<=0)
		gamestate.armor=0;
		DrawArmor ();
		DrawWeapon ();

	if (gamestate.health<=0)
	{
		gamestate.health = 0;
		playstate = ex_died;
		killerobj = attacker;
	}
	if (!godmode)
	StartDamageFlash (points);

	gotgatgun=0;

	DrawHealth ();
	DrawFace ();

	if (points > 20 && gamestate.health!=0 && !godmode)
	{
		if (gamestate.health >80)
		StatusDrawPic (144,2,BJOUCH1PIC);
		else if (gamestate.health>60 && gamestate.health<81)
		StatusDrawPic (144,2,BJOUCH2PIC);
		else if (gamestate.health>40 && gamestate.health<61)
		StatusDrawPic (144,2,BJOUCH3PIC);
		else if (gamestate.health>20 && gamestate.health<41)
		StatusDrawPic (144,2,BJOUCH4PIC);
		else if (gamestate.health<21)
		StatusDrawPic (144,2,BJOUCH5PIC);

		facecount = 0;
	}
	SD_PlaySound (PLAYERPAINSND);

}

void	HealSelf (int points)
{
	gamestate.health += points;
	if (gamestate.health>100)
		gamestate.health = 100;

	DrawHealth ();
	gotgatgun = 0;
	DrawFace ();
}
void HealBonus (int points)
{
	gamestate.health += points;
	if (gamestate.health > 200)
		gamestate.health = 200;

	DrawHealth ();
	gotgatgun = 0;
	DrawFace ();
}
void DrawArmor (void)
{
	LatchNumber (184,3,3,gamestate.armor);
}
void GiveArmor (int points)
{
	gamestate.armor += points;
	if (gamestate.armor>100)
	gamestate.armor=100;

	DrawArmor ();
}
void ArmorBonus (int points)
{
	gamestate.armor += points;
	if (gamestate.armor>200)
	gamestate.armor=200;

	DrawArmor ();
	gotgatgun = 0;
}

void	GiveExtraMan (void)
{
	if (gamestate.lives<9)
		gamestate.lives++;
	SD_PlaySound (BONUSITEMUPSND);
}

void	GivePoints (long points)
{
	if (!DebugOk)
	{
	gamestate.score += points;
	while (gamestate.score >= gamestate.nextextra)
	{
		gamestate.nextextra += EXTRAPOINTS;
		GiveExtraMan ();
	}
	}
}

void DrawWeapon (void)
{
	if (gamestate.bestweapon & 2)
	StatusDrawPic (104,2,WEAP2PIC);
	else
	StatusDrawPic (104,2,NOWEAP2PIC);

	if (gamestate.bestweapon & 4 || gamestate.bestweapon & 8)
	StatusDrawPic (120,2,WEAP3PIC);
	else
	StatusDrawPic (120,2,NOWEAP3PIC);

	if (gamestate.bestweapon & 16)
	StatusDrawPic (128,2,WEAP4PIC);
	else
	StatusDrawPic (128,2,NOWEAP4PIC);

	if (gamestate.bestweapon & 32)
	StatusDrawPic (104,12,WEAP5PIC);
	else
	StatusDrawPic (104,12,NOWEAP5PIC);

	if (gamestate.bestweapon & 64)
	StatusDrawPic (120,12,WEAP6PIC);
	else
	StatusDrawPic (120,12,NOWEAP6PIC);

	if (gamestate.bestweapon & 128)
	StatusDrawPic (128,12,WEAP7PIC);
	else
	StatusDrawPic (128,12,NOWEAP7PIC);
}

void DrawKeys (void)
{
	if (gamestate.keys & 1)
		StatusDrawPic (236,2,GOLDKEYPIC);
	else
		StatusDrawPic (236,2,NOKEYPIC);

	if (gamestate.keys & 2)
		StatusDrawPic (236,12,SILVERKEYPIC);
	else
		StatusDrawPic (236,12,NOKEYPIC);

	if (gamestate.keys & 4)
		StatusDrawPic (236,22,REDKEYPIC);
	else
		StatusDrawPic (236,22,NOKEYPIC);
}

void GiveWeapon (int weapon)
{
	switch (weapon)
	{
		case wp_shotgun:
			GiveAmmo(4,ammo2);
			break;
		case wp_supershotgun:
			GiveAmmo(8,ammo2);
			break;
		case wp_chaingun:
			GiveAmmo(20,ammo1);
			break;
		case wp_launcher:
			GiveAmmo(2,ammo3);
			break;
		case wp_plasmagun:
			GiveAmmo(20,ammo4);
			break;
		case wp_bfggun:
			GiveAmmo(40,ammo4);
			break;
		}
	if (gamestate.chosenweapon == weapon)
	return;
   gamestate.bestweapon |= (1<<weapon);
   gamestate.nextweapon = gamestate.chosenweapon = weapon;
   gamestate.goingdown = gamestate.changingweapon = true;
   DrawWeapon ();
   DrawAmmo ();
}

void	DrawAmmo (void)
{
	switch (gamestate.nextweapon)
	{
	case wp_fists:
	case wp_chainsaw:
	LatchNumber (2,3,3,0);
	break;
	case wp_pistol:
	case wp_chaingun:
	LatchNumber (2,3,3,gamestate.ammo);
	break;
	case wp_shotgun:
	case wp_supershotgun:
	LatchNumber (2,3,3,gamestate.ammo2);
	break;
	case wp_launcher:
	LatchNumber (2,3,3,gamestate.ammo3);
	break;
	case wp_plasmagun:
	case wp_bfggun:
	LatchNumber (2,3,3,gamestate.ammo4);
	break;
}
	LatchSNumber (276,5,3,gamestate.ammo);
	LatchSNumber (276,11,3,gamestate.ammo2);
	LatchSNumber (276,17,3,gamestate.ammo3);
	LatchSNumber (276,23,3,gamestate.ammo4);
	LatchSNumber (302,5,3,gamestate.ammomax);
	LatchSNumber (302,11,3,gamestate.ammo2max);
	LatchSNumber (302,17,3,gamestate.ammo3max);
	LatchSNumber (302,23,3,gamestate.ammo4max);
}

void	GiveAmmo (int ammo, int ammotype)
{
	switch (ammotype)
	{
		case ammo1:
		{
			gamestate.ammo += ammo;
			if (gamestate.ammo > gamestate.ammomax)
			gamestate.ammo = gamestate.ammomax;
		}
		break;
		case ammo2:
		{
			gamestate.ammo2 += ammo;
			if (gamestate.ammo2 > gamestate.ammo2max)
			gamestate.ammo2 = gamestate.ammo2max;
		}
		break;
		case ammo3:
		{
			gamestate.ammo3 += ammo;
			if (gamestate.ammo3 > gamestate.ammo3max)
			gamestate.ammo3 = gamestate.ammo3max;
		}
		break;
		case ammo4:
		{
			gamestate.ammo4 += ammo;
			if (gamestate.ammo4 > gamestate.ammo4max)
			gamestate.ammo4 = gamestate.ammo4max;
		}
		break;
	}
	DrawAmmo ();
}
void GiveBackPack (void)
{
	if (gamestate.ammomax < 400)
		gamestate.ammomax += MAXAMMO;
	else
		gamestate.ammomax = 400;

	if (gamestate.ammo2max < 100)
		gamestate.ammo2max += MAXAMMO2;
	else
		gamestate.ammo2max = 100;

	if (gamestate.ammo3max < 100)
		gamestate.ammo3max += MAXAMMO3;
	else
		gamestate.ammo3max = 100;

	if (gamestate.ammo4max < 600)
		gamestate.ammo4max += MAXAMMO4;
	else
		gamestate.ammo4max = 600;

	GiveAmmo (10,ammo1);
	GiveAmmo (4,ammo2);
	GiveAmmo (1,ammo3);
	GiveAmmo (20,ammo4);
}

void GiveKey (int key)
{
	gamestate.keys |= (1<<key);
	DrawKeys ();
}

void GetBonus (statobj_t *check)
{
	switch (check->itemnumber)
	{
	case	bo_key1:
		GetMessage ("You found the yellow key.");
		GiveKey (check->itemnumber - bo_key1);
		SD_PlaySound (ITEMUPSND);
		break;
	case	bo_key2:
		GetMessage ("You found the blue key.");
		GiveKey (check->itemnumber - bo_key1);
		SD_PlaySound (ITEMUPSND);
		break;
	case	bo_key3:
		GetMessage ("You found the red key.");
		GiveKey (check->itemnumber - bo_key1);
		SD_PlaySound (ITEMUPSND);
		break;
	case	bo_ammo1:
		if (gamestate.ammo==gamestate.ammomax)
		return;

		SD_PlaySound (ITEMUPSND);
		GiveAmmo (10,ammo1);
		GetMessage("Picked up a clip.");
		break;
	case	bo_ammoused:
		if (gamestate.ammo==gamestate.ammomax)
		return;

		SD_PlaySound (ITEMUPSND);
		GiveAmmo (5,ammo1);
		GetMessage ("Picked up a clip.");
		break;
	case	bo_ammo1box:
		if (gamestate.ammo==gamestate.ammomax)
		return;

		SD_PlaySound (ITEMUPSND);
		GiveAmmo (25,ammo1);
		GetMessage ("Picked up a box of ammo.");
		break;
	case	bo_ammo2:
		if (gamestate.ammo2==gamestate.ammo2max)
		return;

		SD_PlaySound (ITEMUPSND);
		GiveAmmo (4,ammo2);
		GetMessage ("Picked up some shells.");
		break;
	case	bo_ammo2box:
		if (gamestate.ammo2==gamestate.ammo2max)
		return;

		SD_PlaySound (ITEMUPSND);
		GiveAmmo (16,ammo2);
		GetMessage("Picked up a box of shells.");
		break;
	case	bo_ammo3:
		if (gamestate.ammo3==gamestate.ammo3max)
		return;

		SD_PlaySound (ITEMUPSND);
		GiveAmmo (1,ammo3);
		GetMessage ("Picked up a rocket.");
		break;
	case	bo_ammo3box:
		if (gamestate.ammo3==gamestate.ammo3max)
		return;

		SD_PlaySound (ITEMUPSND);
		GiveAmmo (4,ammo3);
		GetMessage ("Picked up a box of rockets.");
		break;
	case	bo_ammo4:
		if (gamestate.ammo4==gamestate.ammo4max)
		return;

		SD_PlaySound (ITEMUPSND);
		GiveAmmo (30,ammo4);
		GetMessage("Picked up a cell.");
		break;
	case	bo_ammo4box:
		if (gamestate.ammo4==gamestate.ammo4max)
		return;

		SD_PlaySound (ITEMUPSND);
		GiveAmmo (100,ammo4);
		GetMessage ("Picked up a cell pack.");
		break;
	case	bo_shotgun:
		if (gamestate.chosenweapon == wp_shotgun)
		GiveAmmo (4,ammo2);
		else
		GiveWeapon (wp_shotgun);
		SD_PlaySound (WEAPONPICKUPSND);
		GetMessage ("Picked up a shotgun.");
		break;
	case	bo_supershotgun:
		GiveWeapon (wp_supershotgun);
		EvilFace();
		facecount=0;
		SD_PlaySound (WEAPONPICKUPSND);
		GetMessage("You found a Super Shotgun!");
		break;
	case	bo_chaingun:
		GiveWeapon (wp_chaingun);
		EvilFace();
		facecount = 0;
		SD_PlaySound (WEAPONPICKUPSND);
		GetMessage("Picked up a chaingun.");
		break;
	case	bo_launcher:
		GiveWeapon (wp_launcher);
		EvilFace();
		facecount = 0;
		GetMessage("You found the Rocket Launcher!");
		SD_PlaySound (WEAPONPICKUPSND);
		break;
	case	bo_plasmagun:
		GiveWeapon (wp_plasmagun);
		GetMessage ("Picked up the plasma gun.");
		SD_PlaySound (WEAPONPICKUPSND);
		break;
	case	bo_bfggun:
		GiveWeapon (wp_bfggun);
		EvilFace();
		facecount = 0;
		GetMessage("You found the BFG 9000! Oh yes!");
		SD_PlaySound (WEAPONPICKUPSND);
		break;
	case	bo_chainsaw:
		SD_PlaySound (WEAPONPICKUPSND);
		GetMessage ("You found the chainsaw! Go find some meat!");
		GiveWeapon (wp_chainsaw);

		EvilFace();
		facecount = 0;
		break;
	case	bo_megaarmor:
		SD_PlaySound (ITEMUPSND);
		ArmorBonus (200);
		GetMessage ("Picked up the MegaArmor!");
		break;
	case	bo_armor:
		if (gamestate.armor>=100)
		return;

		SD_PlaySound (ITEMUPSND);
		GiveArmor (100);
		GetMessage ("Picked up the armor.");
		break;
	case	bo_armor1:
		SD_PlaySound (ITEMUPSND);
		ArmorBonus (1);
		GetMessage ("Picked up an armor bonus.");
		gamestate.treasurecount++;
		break;
	case	bo_health1:
		SD_PlaySound (ITEMUPSND);
		HealBonus (1);
		GetMessage ("Picked up a health bonus.");
		gamestate.treasurecount++;
		break;
	case	bo_medpack:
		if (gamestate.health>=100)
		return;

		SD_PlaySound (ITEMUPSND);
		GetMessage("Picked up a stimpack.");
		HealSelf (10);
		break;
	case	bo_healthkit:
		if (gamestate.health>=100)
		return;

		SD_PlaySound (ITEMUPSND);
		GetMessage("Picked up a health pack.");
		HealSelf (25);
		break;
	case	bo_berserk:
		berserk=1;
		gamestate.berserk=true;
		gamestate.berserkcount++;
		gamestate.nextweapon = gamestate.chosenweapon = wp_fists;
		SD_PlaySound (ITEMUPSND);
		GetMessage("You found a berserk pack.");
		gamestate.treasurecount++;
		break;
	case	bo_backpack:
		SD_PlaySound (ITEMUPSND);
		GiveBackPack ();
		DrawAmmo ();
		GetMessage ("You found a backpack.");
		gamestate.treasurecount++;
		break;
	case	bo_godmode:
		SD_PlaySound (BONUSITEMUPSND);
		godmode=1;
		gamestate.godmode=true;
		gamestate.godmodecount++;
		gamestate.treasurecount++;
		GetMessage ("GODMODE!");
		break;
	case	bo_megasphere:
		SD_PlaySound (ITEMUPSND);
		HealBonus (199);
		ArmorBonus (200);
		gamestate.treasurecount++;
		GetMessage("MEGASPHERE!");
		break;
	case	bo_goggles:
		gamestate.goggles=true;
		gamestate.gogglescount++;
		SD_PlaySound (ITEMUPSND);
		gamestate.treasurecount++;
		GetMessage("Picked up amplification goggles.");
		break;
	case	bo_cloak:
		SD_PlaySound (BONUSITEMUPSND);
		gamestate.treasurecount++;
		invisibility = 1;
		gamestate.invisibility = true;
		gamestate.invisibilitycount++;
		GetMessage("INVISIBILITY!");
		break;
	case	bo_soul:
		if (gamestate.health<100)
		HealSelf (99);
		if (gamestate.armor<100)
		GiveArmor (100);
		SD_PlaySound (ITEMUPSND);
		gamestate.treasurecount++;
		GetMessage("SOULSPHERE!");
		break;
	case	bo_biosuit:
		SD_PlaySound (ITEMUPSND);
		gamestate.treasurecount++;
		biosuit=1;
		gamestate.biosuit=true;
		gamestate.biosuitcount++;
		gamestate.treasurecount++;
		GetMessage("You found a biosuit.");
		break;
	case	bo_hellorb:
		spearflag=true;
		spearx = player->x;
		speary = player->y;
		spearangle=player->angle;
		playstate = ex_completed;
		break;
	}

	StartBonusFlash ();
	check->shapenum = -1;
}

boolean TryMove (objtype *ob)
{
	int			xl,yl,xh,yh,x,y;
	objtype		*check;
	long		deltax,deltay;

	xl = (ob->x-PLAYERSIZE) >>TILESHIFT;
	yl = (ob->y-PLAYERSIZE) >>TILESHIFT;

	xh = (ob->x+PLAYERSIZE) >>TILESHIFT;
	yh = (ob->y+PLAYERSIZE) >>TILESHIFT;

	for (y=yl;y<=yh;y++)
		for (x=xl;x<=xh;x++)
		{
			check = actorat[x][y];
			if (check && check<objlist)
				return false;
		}
	if (yl>0)
		yl--;
	if (yh<MAPSIZE-1)
		yh++;
	if (xl>0)
		xl--;
	if (xh<MAPSIZE-1)
		xh++;

	for (y=yl;y<=yh;y++)
		for (x=xl;x<=xh;x++)
		{
			check = actorat[x][y];
			if (check > objlist
			&& (check->flags & FL_SHOOTABLE) )
			{
				deltax = ob->x - check->x;
				if (deltax < -MINACTORDIST || deltax > MINACTORDIST)
					continue;
				deltay = ob->y - check->y;
				if (deltay < -MINACTORDIST || deltay > MINACTORDIST)
					continue;

				return false;
			}
		}

	return true;
}

void ClipMove (objtype *ob, long xmove, long ymove)
{
	long	basex,basey;

	basex = ob->x;
	basey = ob->y;

	ob->x = basex+xmove;
	ob->y = basey+ymove;
	if (TryMove (ob))
		return;

      /*	if (noclip && ob->x > 2*TILEGLOBAL && ob->y > 2*TILEGLOBAL &&
	ob->x < (((long)(mapwidth-1))<<TILESHIFT)
	&& ob->y < (((long)(mapheight-1))<<TILESHIFT) )
		return;
	*/
	ob->x = basex+xmove;
	ob->y = basey;
	if (TryMove (ob))
		return;

	ob->x = basex;
	ob->y = basey+ymove;
	if (TryMove (ob))
		return;

	ob->x = basex;
	ob->y = basey;
}

void Thrust (int angle, long speed)
{
	long xmove,ymove;
	long	slowmax;
	unsigned	offset;

	if (gamestate.bobdir != 2)
	{gamestate.bobber++;}
	if (gamestate.bobdir==2)
	{gamestate.bobber--;}
	if (gamestate.bobber==20)
	{gamestate.bobdir=2;}
	if (gamestate.bobber==0)
	{gamestate.bobdir=1;}
	if (gamestate.bobdir2 != 2)
	{gamestate.bobber2++;}
	if (gamestate.bobdir2 == 2)
	{gamestate.bobber2--;}
	if (gamestate.bobber2==40)
	{gamestate.bobdir2=2;}
	if (gamestate.bobber2==0)
	{gamestate.bobdir2=1;}

	thrustspeed += speed;

	if (speed >= MINDIST*2)
		speed = MINDIST*2-1;

	xmove = FixedByFrac(speed,costable[angle]);
	ymove = -FixedByFrac(speed,sintable[angle]);

	ClipMove(player,xmove,ymove);

	player->tilex = player->x >> TILESHIFT;
	player->tiley = player->y >> TILESHIFT;

	offset = farmapylookup[player->tiley]+player->tilex;
	player->areanumber = (*(mapsegs[0] + offset) -AREATILE)&63;

	if (*(mapsegs[1] + offset) == EXITTILE)
		playstate = ex_victorious;
	if (*(mapsegs[1] + offset) == WARPEASTTILE)
		WarpPlayer(EAST);
	if (*(mapsegs[1] + offset) == WARPWESTTILE)
		WarpPlayer(WEST);
	if (*(mapsegs[1] + offset) == ENDLEVELTILE)
		playstate = ex_completed;
	if (*(mapsegs[1] + offset) == 107)
		if ((unsigned)actorat[player->tilex][player->tiley] == 0xffff)
		{
			gamestate.secretcount++;
			actorat[player->tilex][player->tiley] = NULL;
			GetMessage("You found a secret area");
		}
}

void Cmd_Fire (void)
{
	if(gamestate.changingweapon == true)
	return;

	buttonheld[bt_attack] = true;

	gamestate.weaponframe = 0;

	player->state = &s_attack;

	gamestate.attackframe = 0;
	gamestate.attackcount =
		attackinfo[gamestate.nextweapon][gamestate.attackframe].tics;
	gamestate.weaponframe =
		attackinfo[gamestate.nextweapon][gamestate.attackframe].frame;
}

void Cmd_Use (void)
{
	objtype 	*check;
	int			checkx,checky,doornum,dir;

	if (player->angle < ANGLES/8 || player->angle > 7*ANGLES/8)
	{
		checkx = player->tilex + 1;
		checky = player->tiley;
		dir = di_east;
	}
	else if (player->angle < 3*ANGLES/8)
	{
		checkx = player->tilex;
		checky = player->tiley-1;
		dir = di_north;
	}
	else if (player->angle < 5*ANGLES/8)
	{
		checkx = player->tilex - 1;
		checky = player->tiley;
		dir = di_west;
	}
	else
	{
		checkx = player->tilex;
		checky = player->tiley + 1;
		dir = di_south;
	}

	doornum = tilemap[checkx][checky];
	if (*(mapsegs[1]+farmapylookup[checky]+checkx) == PUSHABLETILE)
	{

		PushWall (checkx,checky,dir);
		return;
	}
	if (!buttonheld[bt_use] && MAPSPOT(checkx,checky,2) == ELEVATORTILE)
	{
		buttonheld[bt_use] = true;

		MAPSPOT(checkx,checky,2)++;
		if (*(mapsegs[0]+farmapylookup[player->tiley]+player->tilex) == ALTELEVATORTILE)
			playstate = ex_secretlevel;
		else
			playstate = ex_completed;
		SD_PlaySound (SWITCH1SND);
		SD_WaitSoundDone();
	}
	else if (!buttonheld[bt_use] && doornum & 0x80)
	{
		buttonheld[bt_use] = true;
		OperateDoor (doornum & ~0x80);
	}
	else if (!buttonheld[bt_use] && MAPSPOT(checkx,checky,2) >= SWITCHTILE)
	{
		buttonheld[bt_use] = true;
		if (MAPSPOT(checkx,checky,2) >= SWITCHTILE && MAPSPOT(checkx,checky,2) <=SWITCHTILE+19)
		{
			switch (MAPSPOT(checkx,checky,2))
			{
				case SWITCHTILE:
				case SWITCHTILE+6:
					MAPSPOT(checkx,checky,2)++;
					SD_PlaySound (SWITCH2SND);
					break;
				case SWITCHTILE+2:
				case SWITCHTILE+4:
				case SWITCHTILE+12:
				case SWITCHTILE+14:
				case SWITCHTILE+16:
				case SWITCHTILE+18:
					MAPSPOT(checkx,checky,2)++;
					SD_PlaySound (SWITCH1SND);
					break;
			}
			switch (MAPSPOT(checkx,checky,1))
			{
				case 700:
					PushWall (pwx,pwy,di_north);
					break;
				case 701:
					PushWall (pwx,pwy,di_south);
					break;
				case 702:
					PushWall (pwx,pwy,di_east);
					break;
				case 703:
					PushWall (pwx,pwy,di_west);
					break;
				case 708: // Pushwall2 North
					PushWall (pw2x,pw2y,di_north);
					break;
	    case 709: // Pushwall2 South
	       PushWall (pw2x,pw2y,di_south);
	       break;
	    case 710: // Pushwall2 East
	       PushWall (pw2x,pw2y,di_east);
	       break;
	    case 711: // Pushwall2 West
	       PushWall (pw2x,pw2y,di_west);
	       break;
	    case 716: // Pushwall3 North
	       PushWall (pw3x,pw3y,di_north);
	       break;
	    case 717: // Pushwall3 South
	       PushWall (pw3x,pw3y,di_south);
	       break;
	    case 718: // Pushwall3 East
	       PushWall (pw3x,pw3y,di_east);
	       break;
	    case 719: // Pushwall3 West
	       PushWall (pw3x,pw3y,di_west);
	       break;
	    case 724: // Pushwall4 North
	       PushWall (pw4x,pw4y,di_north);
	       break;
	    case 725: // Pushwall4 South
	       PushWall (pw4x,pw4y,di_south);
	       break;
	    case 726: // Pushwall4 East
	       PushWall (pw4x,pw4y,di_east);
	       break;
	    case 727: // Pushwall4 West
	       PushWall (pw4x,pw4y,di_west);
	       break;
	 }
      }
   }
	else
	{}
	  //	SD_PlaySound (NOWAYSND);

}

void SpawnPlayer (int tilex, int tiley, int dir)
{
	player->obclass = playerobj;
	player->active = true;
	player->tilex = tilex;
	player->tiley = tiley;
	player->areanumber =
		*(mapsegs[0] + farmapylookup[player->tiley]+player->tilex);
	player->x = ((long)tilex<<TILESHIFT)+TILEGLOBAL/2;
	player->y = ((long)tiley<<TILESHIFT)+TILEGLOBAL/2;
	player->state = &s_player;
	player->angle = (1-dir)*90;
	if (player->angle<0)
		player->angle += ANGLES;
	player->flags = FL_NEVERMARK;
	Thrust (0,0);

	InitAreas ();
}

void RocketLauncherAttack (objtype *ob)
{
	if (gamestate.ammo3<1)
	{
		return;
	}
	SD_PlaySound (RLAUNCHERFIRESND);

	GetNewActor ();

	new->state = &s_rocket;
	new->ticcount = 1;

	new->x = ob->x;
	new->y = ob->y;
	new->tilex = new->x >> TILESHIFT;
	new->tiley = new->y >> TILESHIFT;

	new->obclass = rocketobj;
	new->dir = nodir;
	new->angle = ob->angle;
	new->speed = 0x4200l;
	new->flags = FL_NONMARK | FL_BONUS;
	new->active = true;
}
void PlasmaAttack (objtype *ob)
{
	if (gamestate.ammo4<1)
	{
		return;
	}
	SD_PlaySound (PLASMAGUNFIRESND);

	GetNewActor ();

	new->state = &s_plasma;
	new->ticcount = 1;

	new->x = ob->x;
	new->y = ob->y;
	new->tilex = new->x >> TILESHIFT;
	new->tiley = new->y >> TILESHIFT;

	new->obclass = plasmaobj;
	new->dir = nodir;
	new->angle = ob->angle;
	new->speed = 0x4200l;
	new->flags = FL_NONMARK | FL_BONUS;
	new->active = true;
}
void BFGAttack (objtype *ob)
{
	if (gamestate.ammo4<20)
	{
		return;
	}

	GetNewActor ();

	new->state = &s_bfg;
	new->ticcount = 1;

	new->x = ob->x;
	new->y = ob->y;
	new->tilex = new->x >> TILESHIFT;
	new->tiley = new->y >> TILESHIFT;

	new->obclass = bfgobj;
	new->dir = nodir;
	new->angle = ob->angle;
	new->speed = 0x1200l;
	new->flags = FL_NONMARK | FL_BONUS;
	new->active = true;
}
void HellOrbAttack (objtype *ob)
{
	SD_PlaySound (TELE2SND);

	GetNewActor ();

	new->state = &s_hellorb1;
	new->ticcount = 1;
	new->x = ob->x;
	new->y = ob->y;
	new->tilex = new->x >> TILESHIFT;
	new->tiley = new->y >> TILESHIFT;

	new->obclass = hellorbobj;
	new->dir = nodir;
	new->angle = ob->angle;
	new->speed = 0x4200l;
	new->flags = FL_NONMARK | FL_BONUS;
	new->active = true;
}

void	KnifeAttack (objtype *ob)
{
	objtype *check,*closest;
	long	dist;
	int	damage;

	if (gamestate.nextweapon == wp_chainsaw)
		SD_PlaySound (CHAINSAWFIRESND);

	dist = 0x7fffffff;
	closest = NULL;
	for (check=ob->next ; check ; check=check->next)
		if ( (check->flags & FL_SHOOTABLE)
		&& (check->flags & FL_VISABLE)
		&& abs (check->viewx-centerx) < shootdelta)
		{
			if (check->transx < dist)
			{
				dist = check->transx;
				closest = check;
			}
		}

	if (!closest || dist> 0x18000l)
	{
		return;
	}
	else
	{
		if (gamestate.nextweapon == wp_fists)
			SD_PlaySound (FISTPUNCHSND);
	}
	if (berserk)
		DamageActor (closest,1000);
	else
	DamageActor (closest,US_RndT() >> 4);
}

void	GunAttack (objtype *ob)
{
	objtype *check,*closest,*oldclosest;
	int		damage;
	int		dx,dy,dist;
	int	count=0,ratio=1;
	long	viewdist;

	if (gamestate.nextweapon==wp_launcher)
	{
		RocketLauncherAttack (ob);
		return;}
	if (gamestate.nextweapon==wp_plasmagun)
	{
		PlasmaAttack (ob);
		return;}
	if (gamestate.nextweapon==wp_bfggun)
	{
		BFGAttack (ob);
		return;}
	if (gamestate.nextweapon==wp_hellorb)
	{
		HellOrbAttack (ob);
		return;}

	switch (gamestate.nextweapon)
	{
	case wp_pistol:
	case wp_chaingun:
		SD_PlaySound (PISTOLFIRESND);
		break;
	case wp_shotgun:
		SD_PlaySound (SHOTGUNFIRESND);
		ratio=2;
		break;
	case wp_supershotgun:
		SD_PlaySound (DSHOTGUNFIRESND);
		ratio=4;
		break;
	}

	madenoise = true;

	TOP:
	viewdist = 0x7fffffffl;
	closest = NULL;

	while (1)
	{
		oldclosest = closest;

		for (check=ob->next ; check ; check=check->next)
			if ( (check->flags & FL_SHOOTABLE)
			&& (check->flags & FL_VISABLE)
			&& abs (check->viewx-centerx) < shootdelta*ratio
			)
			{
				if (check->transx < viewdist)
				{
					viewdist = check->transx;
					closest = check;
				}
			}

		if (closest == oldclosest)
			return;

		if (CheckLine(closest))
			break;

	}
	dx = abs(closest->tilex - player->tilex);
	dy = abs(closest->tiley - player->tiley);
	dist = dx>dy ? dx:dy;

	if (dist<2)
		damage = US_RndT() / 4;
	else if (dist<4)
		damage = US_RndT() / 6;
	else
	{
		if ( (US_RndT() / 12) < dist)
			return;
		damage = US_RndT() / 6;
	}
	if ((gamestate.nextweapon==wp_shotgun) && count <3)
	{
		DamageActor (closest,damage);
		count++;
		goto TOP;
	}
	else if ((gamestate.nextweapon==wp_supershotgun) && count < 6)
	{
		DamageActor (closest,damage);
		count++;
		goto TOP;
	}
	else

	DamageActor (closest,damage);
}

void VictorySpin (void)
{
	long	desty;

	if (player->angle > 270)
	{
		player->angle -= tics * 3;
		if (player->angle < 270)
			player->angle = 270;
	}
	else if (player->angle < 270)
	{
		player->angle += tics * 3;
		if (player->angle > 270)
			player->angle = 270;
	}

	desty = (((long)player->tiley-5)<<TILESHIFT)-0x3000;

	if (player->y > desty)
	{
		player->y -= tics*4096;
		if (player->y < desty)
			player->y = desty;
	}
}

void	T_Attack (objtype *ob)
{
	struct	atkinf	*cur;
	int i;

	UpdateFace ();

	if ( buttonstate[bt_use] && !buttonheld[bt_use] )
		buttonstate[bt_use] = false;

	if ( buttonstate[bt_attack] && !buttonheld[bt_attack])
		buttonstate[bt_attack] = false;

	ControlMovement (ob);

	plux = player->x >> UNSIGNEDSHIFT;
	pluy = player->y >> UNSIGNEDSHIFT;
	player->tilex = player->x >> TILESHIFT;
	player->tiley = player->y >> TILESHIFT;

	gamestate.attackcount -= tics;
	while (gamestate.attackcount <= 0)
	{
		cur = &attackinfo[gamestate.nextweapon][gamestate.attackframe];
		switch (cur->attack)
		{
	    case -1:
	    ob->state = &s_player;
	    if (gamestate.ammo2<2 && gamestate.nextweapon == wp_supershotgun)
			{gamestate.nextweapon = gamestate.chosenweapon = wp_shotgun;
			gamestate.changingweapon = gamestate.goingdown = true;}
		else if (gamestate.ammo4<40 && gamestate.nextweapon == wp_bfggun)
			{gamestate.nextweapon = gamestate.chosenweapon = wp_plasmagun;
			gamestate.changingweapon = gamestate.goingdown = true;}
	    if (!ChkAtkAmmo (gamestate.nextweapon) && gamestate.chosenweapon != wp_fists)
	    {
	       gamestate.nextweapon = wp_fists;
	       gamestate.goingdown = gamestate.changingweapon = true;
	       for (i=wp_pistol; i<9; i++)
	       {
		  if (ChkAtkAmmo (i) &&  gamestate.bestweapon & (1<<i))
		  {
		     gamestate.nextweapon = i;
		     gamestate.goingdown = gamestate.changingweapon = true;
		  }
	       }
	       DrawWeapon ();
	       DrawAmmo ();
	    }
	    else
	    {
	       if (gamestate.nextweapon != gamestate.chosenweapon)
	       {
		  if (ChkAtkAmmo(gamestate.chosenweapon))
		     gamestate.nextweapon = gamestate.chosenweapon;
		  DrawWeapon ();
	       }
	    }
	    gamestate.attackframe = gamestate.weaponframe = 0;
	    return;

	 case 4: // Chaingun - Fall back 2 frames and fire again if button Pressed
	    if (!ChkAtkAmmo(gamestate.nextweapon)) break;
	    if (buttonstate[bt_attack]) gamestate.attackframe -= 2;

	 case 1: // Fire the Weapon

	  // can only happen with chain gun
	    if (!ChkAtkAmmo(wp_chaingun) && gamestate.nextweapon == wp_chaingun)
	       {   gamestate.attackframe++; break; }

	    if (ChkAtkAmmo(gamestate.nextweapon))
	    {
	       GunAttack (ob);
	       switch (gamestate.nextweapon)
			{
				case wp_pistol:
				case wp_chaingun:
					gamestate.ammo--;
					break;
				case wp_shotgun:
					gamestate.ammo2--;
					break;
				case wp_supershotgun:
					gamestate.ammo2-=2;
					break;
				case wp_launcher:
					gamestate.ammo3--;
					break;
				case wp_plasmagun:
					gamestate.ammo4--;
					break;
				case wp_bfggun:
					gamestate.ammo4-=40;
					break;
			}
	       DrawAmmo ();
	       break;
	    }
	    else
	    { return; }
	 case 2:
	    KnifeAttack (ob);
	    break;
	 case 3:
	    if (ChkAtkAmmo(gamestate.nextweapon) && buttonstate[bt_attack])
	       gamestate.attackframe -= 2;
	    break;
		case 5:
			SD_PlaySound (DSOPENSND);
			break;
		case 6:
			SD_PlaySound (DSLOADSND);
			break;
		case 7:
			SD_PlaySound (DSCLOSESND);
			break;
		case 8:
			SD_PlaySound (ATKBFGSND);
			break;
		}

		gamestate.attackcount += cur->tics;
		gamestate.attackframe++;
		gamestate.weaponframe =
			attackinfo[gamestate.nextweapon][gamestate.attackframe].frame;
	}

}

void	T_Player (objtype *ob)
{

	UpdateFace ();
	CheckWeaponChange ();

	if ( buttonstate[bt_use] )
		Cmd_Use ();

	if ( buttonstate[bt_attack] && !buttonheld[bt_attack])
		Cmd_Fire ();

	ControlMovement (ob);

	plux = player->x >> UNSIGNEDSHIFT;
	pluy = player->y >> UNSIGNEDSHIFT;
	player->tilex = player->x >> TILESHIFT;
	player->tiley = player->y >> TILESHIFT;
}
void WarpPlayer (int facedir)
{
objtype  *check;
int  x,y;

buttonstate[bt_attack] = false;
buttonheld[bt_attack] = false;
gamestate.attackframe = 0;
gamestate.weaponframe = 0;
DrawPlayerWeapon();
ThreeDRefresh();
switch (facedir)
  {
/*  case EAST:
   check = actorat[warpex][warpey];
   if (check > objlist && (check->flags & FL_SHOOTABLE))
    KillActor(check);
   SpawnPlayer(warpex,warpey,facedir);
   break;   */
  case EAST:
	CheckWarpDest(warpex,warpey,check);
	SpawnPlayer(warpex,warpey,facedir);
	break;
  case WEST:
   check = actorat[warpwx][warpwy];
   if (check > objlist && (check->flags & FL_SHOOTABLE))
    KillActor(check);
   SpawnPlayer(warpwx,warpwy,facedir);
   break;
  }

if (gamestate.mapon==10)
SD_PlaySound (TELE2SND);
else
SD_PlaySound (TELEPORTSND);
/*if (DigiMode != sds_Off)
  {
  long lasttimecount = TimeCount;
  while(TimeCount < lasttimecount+35)
  SD_Poll();
  }
else
  VW_WaitVBL(1*50);    */

buttonstate[bt_attack] = false;
buttonheld[bt_attack] = false;
gamestate.attackframe = 0;
//gamestate.weaponframe = 0;
DrawPlayerWeapon();
ThreeDRefresh();
}
void CheckWarpDest (int x, int y, objtype *check)
{
         int xl,yl,xh,yh;

         xl = x-1;
         yl = y-1;
         xh = x+1;
         yh = y+1;

            for (y=yl;y<=yh;y++)
            {
               for (x=xl;x<=xh;x++)
               {
                  check = actorat[x][y];
                  if (check > objlist && check->flags & FL_SHOOTABLE)
                     KillActor(check);
               }
            }
      }
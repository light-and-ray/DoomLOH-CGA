// WL_ACT2.C

#include <math.h>
#include "WL_DEF.H"
#pragma hdrstop

#define PROJECTILESIZE	0xc000l

#define BJROCKETSIZE	0x6000l
#define BARRELEXPLSIZE 0x26000l
#define BFGSPRAYSIZE   0x60000l

dirtype dirtable[9] = {northwest,north,northeast,west,nodir,east,
	southwest,south,southeast};

int	starthitpoints[4][NUMENEMIES] =
	 //
	 // SKILL 1
	 //
	 {
	 {40,	// mutant
	  140,	// chaingun guy
	  210,	// cacodemon
	  180,	// demons
	  4000,	// Cyber Demon
	  900,	// Spider Demon
	  1000,	// Revenant
	  70,	// shotgun guy
	  80,   // imp
	  200,	// pain elemental
	  100,	// lost soul
	  600,	// Mancubus
	  80,	// Barrel
	  1500,	// Boss
	  650,	// Boss2
	  1500, // Arch-Vile
	  },
	 //
	 // SKILL 2
	 //
	 {40,	// mutant
	  140,	// chaingun guy
	  210,	// cacodemon
	  180,	// demons
	  4000,	// Cyber Demon
	  900,	// Spider Demon
	  1000,	// Revenant
	  70,	// shotgun guy
	  80,   // imp
	  200,	// pain elemental
	  100,	// lost soul
	  600,	// Mancubus
	  80,	// Barrel
	  1500,	// Boss
	  650,	// Boss2
	  1500, // Arch-Vile
	  },
	 //
	 // SKILL 3
	 //
	 {40,	// mutant
	  140,	// chaingun guy
	  210,	// cacodemon
	  180,	// demons
	  4000,	// Cyber Demon
	  900,	// Spider Demon
	  1000,	// Revenant
	  70,	// shotgun guy
	  80,   // imp
	  200,	// pain elemental
	  100,	// lost soul
	  600,	// Mancubus
	  80,	// Barrel
	  1500,	// Boss
	  650,	// Boss2
	  1500, // Arch-Vile
	  },
	 //
	 // SKILL 4
	 //
	 {40,	// mutant
	  140,	// chaingun guy
	  210,	// cacodemon
	  180,	// demons
	  4000,	// Cyber Demon
	  900,	// Spider Demon
	  1000,	// Revenant
	  70,	// shotgun guy
	  80,   // imp
	  200,	// pain elemental
	  100,	// lost soul
	  600,	// Mancubus
	  80,	// Barrel
	  1500,	// Boss
	  650,	// Boss2
	  1500, // Arch-Vile
	  }};

void	T_Path (objtype *ob);
void	T_Shoot (objtype *ob);
void	T_Bite (objtype *ob);
void	T_DemonChase (objtype *ob);
void	T_Chase (objtype *ob);
void	T_Projectile (objtype *ob);
void	T_Stand (objtype *ob);

void A_DeathScream (objtype *ob);
void A_BarrelAttack (objtype *ob);
void A_BFGSpray (objtype *ob);

extern	statetype s_rocket;
extern	statetype s_boom2;
extern	statetype s_boom3;

statetype s_rocket	 	= {true,SPR_ROCKET_1,3,T_Projectile,NULL,&s_rocket};

statetype s_boom1	 	= {false,SPR_BOOM_1,6,NULL,A_BarrelAttack,&s_boom2};
statetype s_boom2	 	= {false,SPR_BOOM_2,6,NULL,NULL,&s_boom3};
statetype s_boom3	 	= {false,SPR_BOOM_3,6,NULL,NULL,NULL};

extern	statetype s_plasma;
extern	statetype s_plasma2;
extern	statetype s_plasmaboom1;
extern	statetype s_plasmaboom2;
extern	statetype s_plasmaboom3;
extern	statetype s_plasmaboom4;

statetype s_plasma	= {false,SPR_PLASMA_1,3,T_Projectile,NULL,&s_plasma2};
statetype s_plasma2	= {false,SPR_PLASMA_2,3,T_Projectile,NULL,&s_plasma};

statetype s_plasmaboom1	= {false,SPR_PLASMA_BOOM1,6,NULL,NULL,&s_plasmaboom2};
statetype s_plasmaboom2 = {false,SPR_PLASMA_BOOM2,6,NULL,NULL,&s_plasmaboom3};
statetype s_plasmaboom3	= {false,SPR_PLASMA_BOOM3,6,NULL,NULL,&s_plasmaboom4};
statetype s_plasmaboom4 = {false,SPR_PLASMA_BOOM4,6,NULL,NULL,NULL};

extern	statetype s_bfg;
extern	statetype s_bfg2;
extern	statetype s_bfgboom1;
extern	statetype s_bfgboom2;
extern	statetype s_bfgboom3;
extern	statetype s_bfgboom4;
extern	statetype s_bfgboom5;
extern	statetype s_bfgboom6;

statetype s_bfg	= {false,SPR_BFG_1,3,T_Projectile,NULL,&s_bfg2};
statetype s_bfg2	= {false,SPR_BFG_2,3,T_Projectile,NULL,&s_bfg};

statetype s_bfgboom1	= {false,SPR_BFG_BOOM1,6,NULL,A_BFGSpray,&s_bfgboom2};
statetype s_bfgboom2	= {false,SPR_BFG_BOOM2,6,NULL,NULL,&s_bfgboom3};
statetype s_bfgboom3	= {false,SPR_BFG_BOOM3,6,NULL,NULL,&s_bfgboom4};
statetype s_bfgboom4	= {false,SPR_BFG_BOOM4,6,NULL,NULL,&s_bfgboom5};
statetype s_bfgboom5	= {false,SPR_BFG_BOOM5,6,NULL,NULL,&s_bfgboom6};
statetype s_bfgboom6	= {false,SPR_BFG_BOOM6,6,NULL,NULL,NULL};

extern	statetype s_splat1;
extern	statetype s_splat2;
extern	statetype s_splat3;
extern	statetype s_splat4;
extern	statetype s_splat5;
extern	statetype s_splat6;
extern	statetype s_splat7;
extern	statetype s_splat8;
extern	statetype s_splat9;

statetype s_splat1      = {false,SPR_SPLAT1,6,NULL,NULL,&s_splat2};
statetype s_splat2      = {false,SPR_SPLAT2,6,NULL,NULL,&s_splat3};
statetype s_splat3	= {false,SPR_SPLAT3,6,NULL,NULL,&s_splat4};
statetype s_splat4	= {false,SPR_SPLAT4,6,NULL,NULL,&s_splat5};
statetype s_splat5	= {false,SPR_SPLAT5,6,NULL,NULL,&s_splat6};
statetype s_splat6	= {false,SPR_SPLAT6,6,NULL,NULL,&s_splat7};
statetype s_splat7	= {false,SPR_SPLAT7,6,NULL,NULL,&s_splat8};
statetype s_splat8	= {false,SPR_SPLAT8,6,NULL,NULL,&s_splat9};
statetype s_splat9	= {false,SPR_SPLAT9,6,NULL,NULL,NULL};

extern	statetype s_puff1;
extern	statetype s_puff2;
extern	statetype s_puff3;
extern	statetype s_puff4;

statetype s_puff1	= {false,SPR_PUFF1,6,NULL,NULL,&s_puff2};
statetype s_puff2	= {false,SPR_PUFF2,6,NULL,NULL,&s_puff3};
statetype s_puff3	= {false,SPR_PUFF3,6,NULL,NULL,&s_puff4};
statetype s_puff4	= {false,SPR_PUFF4,6,NULL,NULL,NULL};

extern	statetype s_bfgsplat1;
extern	statetype s_bfgsplat2;
extern	statetype s_bfgsplat3;
extern	statetype s_bfgsplat4;

statetype s_bfgsplat1	= {false,SPR_BFGSPLAT1,6,NULL,NULL,&s_bfgsplat2};
statetype s_bfgsplat2	= {false,SPR_BFGSPLAT2,6,NULL,NULL,&s_bfgsplat3};
statetype s_bfgsplat3	= {false,SPR_BFGSPLAT3,6,NULL,NULL,&s_bfgsplat4};
statetype s_bfgsplat4	= {false,SPR_BFGSPLAT4,6,NULL,NULL,NULL};

extern	statetype s_hellorb1;
extern	statetype s_hellorb2;
extern	statetype s_hellorb3;
extern	statetype s_hellorb4;

statetype s_hellorb1	= {false,SPR_HELLBALL1,6,T_Projectile,NULL,&s_hellorb2};
statetype s_hellorb2	= {false,SPR_HELLBALL2,6,T_Projectile,NULL,&s_hellorb3};
statetype s_hellorb3	= {false,SPR_HELLBALL3,6,T_Projectile,NULL,&s_hellorb4};
statetype s_hellorb4	= {false,SPR_HELLBALL4,6,T_Projectile,NULL,&s_hellorb1};


extern	statetype s_revball1;
extern	statetype s_revball2;

extern	statetype s_revballboom1;
extern	statetype s_revballboom2;
extern	statetype s_revballboom3;

statetype s_revball1	= {true,SPR_REVBALL_1,6,T_Projectile,NULL,&s_revball2};
statetype s_revball2	= {true,SPR_REVBALL2_1,6,T_Projectile,NULL,&s_revball1};

statetype s_revballboom1	= {false,SPR_REVBALL_BOOM1,6,NULL,NULL,&s_revballboom2};
statetype s_revballboom2	= {false,SPR_REVBALL_BOOM2,6,NULL,NULL,&s_revballboom3};
statetype s_revballboom3	= {false,SPR_REVBALL_BOOM3,6,NULL,NULL,NULL};

void T_Spider (objtype *ob);
void A_Slurpie (objtype *ob);
void A_ShootLostSoul (objtype *ob);
void A_MechaSound (objtype *ob);
void A_Barrel (objtype *ob);

#define PROJSIZE	0x2000

boolean ProjectileTryMove (objtype *ob)
{
	int			xl,yl,xh,yh,x,y;
	objtype		*check;
	long		deltax,deltay;

	xl = (ob->x-PROJSIZE) >>TILESHIFT;
	yl = (ob->y-PROJSIZE) >>TILESHIFT;

	xh = (ob->x+PROJSIZE) >>TILESHIFT;
	yh = (ob->y+PROJSIZE) >>TILESHIFT;

	for (y=yl;y<=yh;y++)
		for (x=xl;x<=xh;x++)
		{
			check = actorat[x][y];
			if (check && check<objlist)
				return false;
		}

	return true;
}

void T_Projectile (objtype *ob)
{
	long	deltax,deltay;
	int		damage;
	long	speed;
	objtype *check, *extracheck;

	speed = (long)ob->speed*tics;

	deltax = FixedByFrac(speed,costable[ob->angle]);
	deltay = -FixedByFrac(speed,sintable[ob->angle]);

	if (deltax>0x10000l)
		deltax = 0x10000l;
	if (deltay>0x10000l)
		deltay = 0x10000l;

	ob->x += deltax;
	ob->y += deltay;

	deltax = LABS(ob->x - player->x);
	deltay = LABS(ob->y - player->y);

	if (!ProjectileTryMove (ob))
	{
		switch (ob->obclass)
		{
			case rocketobj:
				ob->state = &s_boom1;
				break;
			case plasmaobj:
				ob->state = &s_plasmaboom1;
				break;
			case bfgobj:
				ob->state = &s_bfgboom1;
				break;
			case impballobj:
				ob->state = &s_impballboom1;
				break;
			case bossballobj:
				ob->state = &s_bossballboom1;
				break;
			case cacoballobj:
				ob->state = &s_cacoballboom1;
				break;
			case fatballobj:
				ob->state = &s_fatballboom1;
				break;
			case splasmaobj:
				ob->state = &s_splasmaboom1;
				break;
			case revballobj:
				ob->state = &s_revballboom1;
				break;
			default:
				ob->state = NULL;
				break;
		}
		return;
	}
	if (ob->obclass == rocketobj && ob->flags & FL_NONMARK && ob->flags & FL_BONUS)
	{
		check=objlist;
		while (check)
		{
			if (check->flags & FL_SHOOTABLE)
			{
				deltax = LABS(ob->x - check->x);
				deltay = LABS(ob->y - check->y);

				if (deltax <BJROCKETSIZE && deltay < BJROCKETSIZE)
				{
					if (check->obclass != hellorbobj)
					ob->state = &s_boom1;

					DamageActor (check,150);
					}
				}
				check = check->next;
			}
		}
		else if (ob->obclass == plasmaobj && ob->flags & FL_NONMARK && ob->flags & FL_BONUS)
		{
			check = objlist;
			while (check)
			{
				if (check->flags & FL_SHOOTABLE)
				{
					deltax = LABS(ob->x - check->x);
					deltay = LABS(ob->y - check->y);

					if (deltax < BJROCKETSIZE && deltay < BJROCKETSIZE)
					{
						if (check->obclass != bfgobj)
						ob->state = &s_plasmaboom1;

						DamageActor (check,25);
					}
				}
				check = check->next;
			}
		}
		else if (ob->obclass == bfgobj && ob->flags & FL_NONMARK && ob->flags & FL_BONUS)
		{
			check = objlist;
			while (check)
			{
				if (check->flags & FL_SHOOTABLE)
				{
					deltax = LABS(ob->x - check->x);
					deltay = LABS(ob->y - check->y);

					if (deltax < BJROCKETSIZE && deltay < BJROCKETSIZE)
					{
						if (check->obclass != bfgobj)
						ob->state = &s_bfgboom1;

						DamageActor (check,500);
					}
				}
				check = check->next;
			}
		}
		if (ob->obclass == hellorbobj && ob->flags & FL_NONMARK && ob->flags & FL_BONUS)
		{
			check = objlist;
			while (check)
			{
				if (check->flags & FL_SHOOTABLE)
				{
					deltax = LABS(ob->x - check->x);
					deltay = LABS(ob->y - check->y);

					if (deltax < BJROCKETSIZE && deltay < BJROCKETSIZE)
					{
						if (check->obclass != bfgobj)
						ob->state = NULL;
						DamageActor (check,25);
					}
				}
				check = check->next;
			}
		}
		else
		{
			{
	if (deltax < PROJECTILESIZE && deltay < PROJECTILESIZE)
	{
		switch (ob->obclass)
		{
		case rocketobj:
			damage = (US_RndT() >>3)+20;
			break;
		case bossballobj:
			damage = (US_RndT() >>5);
			break;
		case fatballobj:
			damage = (US_RndT() >>4)+10;
			break;
		case splasmaobj:
		case impballobj:
		case cacoballobj:
			damage = (US_RndT() >>5)+10;
			break;
		case vilefireobj:
			damage = (US_RndT() >>5)+40;
			break;
		case revballobj:
			damage = (US_RndT() >>7)+12;
			break;
		}

		TakeDamage (damage,ob);
		switch (ob->obclass)
		{
			case rocketobj:
				ob->state = &s_boom1;
				break;
			case plasmaobj:
				ob->state = &s_plasmaboom1;
				break;
			case bfgobj:
				ob->state = &s_bfgboom1;
				break;
			case impballobj:
				ob->state = &s_impballboom1;
				break;
			case bossballobj:
				ob->state = &s_bossballboom1;
				break;
			case cacoballobj:
				ob->state = &s_cacoballboom1;
				break;
			case fatballobj:
				ob->state = &s_fatballboom1;
				break;
			case splasmaobj:
				ob->state = &s_splasmaboom1;
				break;
			case revballobj:
				ob->state = &s_revballboom1;
				break;
			default:
				ob->state = NULL;
				break;
		}
		return;
		}
	}

	ob->tilex = ob->x >> TILESHIFT;
	ob->tiley = ob->y >> TILESHIFT;
	}
}
//
// barrel
//

extern	statetype s_barrelstand;

extern	statetype s_barreldie1;
extern	statetype s_barreldie2;
extern	statetype s_barreldie3;
extern	statetype s_barreldie4;

statetype s_barrelstand = {false,SPR_BARREL,0,NULL,NULL,&s_barrelstand};

statetype s_barreldie1 = {false,SPR_BARREL2,10,NULL,NULL,&s_barreldie2};
statetype s_barreldie2 = {false,SPR_BARRELBOOM1,10,NULL,A_Barrel,&s_barreldie3};
statetype s_barreldie3 	= {false,SPR_BARRELBOOM2,10,NULL,A_BarrelAttack,&s_barreldie4};
statetype s_barreldie4	= {false,SPR_BARRELBOOM3,10,NULL,NULL,NULL};

#pragma argsused
void A_Barrel (objtype *ob)
{
	if (!SD_SoundPlaying)
	PlaySoundLocActor (BARRELBOOMSND,ob);
}
void SpawnBarrel (int tilex, int tiley)
{
	unsigned far *map,tile;

	SpawnNewObj (tilex,tiley,&s_barrelstand);
	new->obclass = barrelobj;
	new->hitpoints = starthitpoints[gamestate.difficulty][en_barrel];
	new->flags |= FL_SHOOTABLE|FL_AMBUSH;

}
int Damage (long dx,long dy,long range,int max)
{
	double ddx = (double) dx;
	double ddy = (double) dy;
	double drange = (double) range;
	double dmax = (double) max;
	double distance = sqrt (ddx*ddx+ddy*ddy);

	if (distance>drange)
	return 0;
	else
	return (int)(dmax-distance/drange*dmax);
}
void A_BarrelAttack(objtype *ob)
{
	long deltax, deltay;
	objtype *extracheck;
	int damage;

	extracheck = objlist;
	while (extracheck)
	{
		if (extracheck != ob && (extracheck->flags & FL_SHOOTABLE))
		{
			deltax = LABS(ob->x - extracheck->x);
			deltay = LABS(ob->y - extracheck->y);
			if (deltax < BARRELEXPLSIZE*2 && deltay < BARRELEXPLSIZE*2) {
			damage = Damage (deltax, deltay, BARRELEXPLSIZE,300);
			if (damage>0)
			DamageActor (extracheck,damage);
		}
	}
	extracheck = extracheck->next;
}
deltax=LABS(ob->x - player->x);
deltay=LABS(ob->y - player->y);
if (deltax < BARRELEXPLSIZE *2 && deltay < BARRELEXPLSIZE * 2) {
damage = Damage (deltax, deltay, BARRELEXPLSIZE,300);
if (damage > 0)
TakeDamage (damage,ob);
}
}
void A_BFGSpray(objtype *ob)
{
	long deltax, deltay;
	objtype *extracheck;
	int damage;

	extracheck = objlist;
	while (extracheck)
	{
		if (extracheck != ob && (extracheck->flags & FL_SHOOTABLE))
		{
			deltax = LABS(ob->x - extracheck->x);
			deltay = LABS(ob->y - extracheck->y);
			if (deltax < BARRELEXPLSIZE*2 && deltay < BFGSPRAYSIZE*2) {
			damage = Damage (deltax, deltay, BFGSPRAYSIZE,300);
			if (damage>0)
			DamageActor (extracheck,damage);
		}
	}
	extracheck = extracheck->next;
	}
}

extern	statetype s_mutantstand;

extern	statetype s_mutantpath1;
extern	statetype s_mutantpath1s;
extern	statetype s_mutantpath2;
extern	statetype s_mutantpath3;
extern	statetype s_mutantpath3s;
extern	statetype s_mutantpath4;

extern	statetype s_mutantpain;

extern	statetype s_mutantshoot1;
extern	statetype s_mutantshoot2;
extern	statetype s_mutantshoot3;

extern	statetype s_mutantchase1;
extern	statetype s_mutantchase1s;
extern	statetype s_mutantchase2;
extern	statetype s_mutantchase3;
extern	statetype s_mutantchase3s;
extern	statetype s_mutantchase4;

extern	statetype s_mutantdie1;
extern	statetype s_mutantdie2;
extern	statetype s_mutantdie3;
extern	statetype s_mutantdie4;
extern	statetype s_mutantdie5;

extern	statetype s_mutantexpl1;
extern	statetype s_mutantexpl2;
extern	statetype s_mutantexpl3;
extern	statetype s_mutantexpl4;
extern	statetype s_mutantexpl5;
extern	statetype s_mutantexpl6;
extern	statetype s_mutantexpl7;
extern	statetype s_mutantexpl8;
extern	statetype s_mutantexpl9;

statetype s_mutantstand	= {true,SPR_MUTANT_W1_1,0,T_Stand,NULL,&s_mutantstand};

statetype s_mutantpath1 	= {true,SPR_MUTANT_W1_1,20,T_Path,NULL,&s_mutantpath1s};
statetype s_mutantpath1s 	= {true,SPR_MUTANT_W1_1,5,NULL,NULL,&s_mutantpath2};
statetype s_mutantpath2 	= {true,SPR_MUTANT_W2_1,15,T_Path,NULL,&s_mutantpath3};
statetype s_mutantpath3 	= {true,SPR_MUTANT_W3_1,20,T_Path,NULL,&s_mutantpath3s};
statetype s_mutantpath3s 	= {true,SPR_MUTANT_W3_1,5,NULL,NULL,&s_mutantpath4};
statetype s_mutantpath4 	= {true,SPR_MUTANT_W4_1,15,T_Path,NULL,&s_mutantpath1};

statetype s_mutantpain 	= {true,SPR_MUTANT_PAIN_1,10,NULL,NULL,&s_mutantchase1};

statetype s_mutantshoot1 	= {false,SPR_MUTANT_SHOOT_1,20,NULL,NULL,&s_mutantshoot2};
statetype s_mutantshoot2 	= {false,SPR_MUTANT_SHOOT2_1,20,NULL,T_Shoot,&s_mutantshoot3};
statetype s_mutantshoot3	= {false,SPR_MUTANT_SHOOT_1,20,NULL,NULL,&s_mutantchase1};

statetype s_mutantchase1 	= {true,SPR_MUTANT_W1_1,10,T_Chase,NULL,&s_mutantchase1s};
statetype s_mutantchase1s 	= {true,SPR_MUTANT_W1_1,3,NULL,NULL,&s_mutantchase2};
statetype s_mutantchase2 	= {true,SPR_MUTANT_W2_1,8,T_Chase,NULL,&s_mutantchase3};
statetype s_mutantchase3 	= {true,SPR_MUTANT_W3_1,10,T_Chase,NULL,&s_mutantchase3s};
statetype s_mutantchase3s 	= {true,SPR_MUTANT_W3_1,3,NULL,NULL,&s_mutantchase4};
statetype s_mutantchase4 	= {true,SPR_MUTANT_W4_1,8,T_Chase,NULL,&s_mutantchase1};

statetype s_mutantdie1		= {false,SPR_MUTANT_DIE_1,15,NULL,A_DeathScream,&s_mutantdie2};
statetype s_mutantdie2		= {false,SPR_MUTANT_DIE_2,15,NULL,NULL,&s_mutantdie3};
statetype s_mutantdie3		= {false,SPR_MUTANT_DIE_3,15,NULL,NULL,&s_mutantdie4};
statetype s_mutantdie4		= {false,SPR_MUTANT_DIE_4,15,NULL,NULL,&s_mutantdie5};
statetype s_mutantdie5		= {false,SPR_MUTANT_DEAD,0,NULL,NULL,&s_mutantdie5};

statetype s_mutantexpl1		= {false,SPR_MUTANT_DIE2_1,6,NULL,A_Slurpie,&s_mutantexpl2};
statetype s_mutantexpl2		= {false,SPR_MUTANT_DIE2_2,6,NULL,NULL,&s_mutantexpl3};
statetype s_mutantexpl3		= {false,SPR_MUTANT_DIE2_3,6,NULL,NULL,&s_mutantexpl4};
statetype s_mutantexpl4		= {false,SPR_MUTANT_DIE2_4,6,NULL,NULL,&s_mutantexpl5};
statetype s_mutantexpl5		= {false,SPR_MUTANT_DIE2_5,6,NULL,NULL,&s_mutantexpl6};
statetype s_mutantexpl6		= {false,SPR_MUTANT_DIE2_6,6,NULL,NULL,&s_mutantexpl7};
statetype s_mutantexpl7		= {false,SPR_MUTANT_DIE2_7,6,NULL,NULL,&s_mutantexpl8};
statetype s_mutantexpl8		= {false,SPR_MUTANT_DIE2_8,6,NULL,NULL,&s_mutantexpl9};
statetype s_mutantexpl9		= {false,SPR_MUTANT_DIE2_9,6,NULL,NULL,&s_mutantexpl9};


//
// imp
//
void T_ImpBallThrow (objtype *ob);

extern	statetype s_impstand;

extern	statetype s_imppath1;
extern	statetype s_imppath1s;
extern	statetype s_imppath2;
extern	statetype s_imppath3;
extern	statetype s_imppath3s;
extern	statetype s_imppath4;

extern	statetype s_imppain;

extern	statetype s_impshoot1;
extern	statetype s_impshoot2;
extern	statetype s_impshoot3;

extern	statetype s_impchase1;
extern	statetype s_impchase1s;
extern	statetype s_impchase2;
extern	statetype s_impchase3;
extern	statetype s_impchase3s;
extern	statetype s_impchase4;

extern	statetype s_impdie1;
extern	statetype s_impdie2;
extern	statetype s_impdie3;
extern	statetype s_impdie4;
extern	statetype s_impdie5;

extern	statetype s_impexpl1;
extern	statetype s_impexpl2;
extern	statetype s_impexpl3;
extern	statetype s_impexpl4;
extern	statetype s_impexpl5;
extern	statetype s_impexpl6;
extern	statetype s_impexpl7;
extern	statetype s_impexpl8;

statetype s_impstand	= {true,SPR_IMP_W1_1,0,T_Stand,NULL,&s_impstand};

statetype s_imppath1	= {true,SPR_IMP_W1_1,20,T_Path,NULL,&s_imppath1s};
statetype s_imppath1s	= {true,SPR_IMP_W1_1,5,NULL,NULL,&s_imppath2};
statetype s_imppath2	= {true,SPR_IMP_W2_1,15,T_Path,NULL,&s_imppath3};
statetype s_imppath3	= {true,SPR_IMP_W3_1,20,T_Path,NULL,&s_imppath3s};
statetype s_imppath3s	= {true,SPR_IMP_W3_1,5,NULL,NULL,&s_imppath4};
statetype s_imppath4	= {true,SPR_IMP_W4_1,15,T_Path,NULL,&s_imppath1};

statetype s_imppain	= {true,SPR_IMP_PAIN_1,10,NULL,NULL,&s_impchase1};

statetype s_impshoot1	= {false,SPR_IMP_SHOOT_1,20,NULL,NULL,&s_impshoot2};
statetype s_impshoot2	= {false,SPR_IMP_SHOOT2_1,20,NULL,T_ImpBallThrow,&s_impshoot3};
statetype s_impshoot3	= {false,SPR_IMP_SHOOT3_1,20,NULL,NULL,&s_impchase1};

statetype s_impchase1	= {true,SPR_IMP_W1_1,10,T_Chase,NULL,&s_impchase1s};
statetype s_impchase1s	= {true,SPR_IMP_W1_1,3,NULL,NULL,&s_impchase2};
statetype s_impchase2	= {true,SPR_IMP_W2_1,8,T_Chase,NULL,&s_impchase3};
statetype s_impchase3	= {true,SPR_IMP_W3_1,10,T_Chase,NULL,&s_impchase3s};
statetype s_impchase3s	= {true,SPR_IMP_W3_1,3,NULL,NULL,&s_impchase4};
statetype s_impchase4	= {true,SPR_IMP_W4_1,8,T_Chase,NULL,&s_impchase1};

statetype s_impdie1	= {false,SPR_IMP_DIE_1,15,NULL,A_DeathScream,&s_impdie2};
statetype s_impdie2	= {false,SPR_IMP_DIE_2,15,NULL,NULL,&s_impdie3};
statetype s_impdie3	= {false,SPR_IMP_DIE_3,15,NULL,NULL,&s_impdie4};
statetype s_impdie4	= {false,SPR_IMP_DIE_4,15,NULL,NULL,&s_impdie5};
statetype s_impdie5	= {false,SPR_IMP_DEAD,0,NULL,NULL,&s_impdie5};

statetype s_impexpl1	= {false,SPR_IMP_DIE2_1,15,NULL,A_Slurpie,&s_impexpl2};
statetype s_impexpl2	= {false,SPR_IMP_DIE2_2,15,NULL,NULL,&s_impexpl3};
statetype s_impexpl3	= {false,SPR_IMP_DIE2_3,15,NULL,NULL,&s_impexpl4};
statetype s_impexpl4	= {false,SPR_IMP_DIE2_4,15,NULL,NULL,&s_impexpl5};
statetype s_impexpl5    = {false,SPR_IMP_DIE2_5,15,NULL,NULL,&s_impexpl6};
statetype s_impexpl6	= {false,SPR_IMP_DIE2_6,15,NULL,NULL,&s_impexpl7};
statetype s_impexpl7    = {false,SPR_IMP_DIE2_7,15,NULL,NULL,&s_impexpl8};
statetype s_impexpl8	= {false,SPR_IMP_DIE2_8,0,NULL,NULL,&s_impexpl8};

extern	statetype s_impball1;
extern	statetype s_impball2;
extern	statetype s_impballboom1;
extern	statetype s_impballboom2;
extern	statetype s_impballboom3;

statetype s_impball1	= {false,SPR_FIRE_1,6,T_Projectile,NULL,&s_impball2};
statetype s_impball2	= {false,SPR_FIRE_2,6,T_Projectile,NULL,&s_impball1};

statetype s_impballboom1	= {false,SPR_FIRE_BOOM1,6,NULL,NULL,&s_impballboom2};
statetype s_impballboom2	= {false,SPR_FIRE_BOOM2,6,NULL,NULL,&s_impballboom3};
statetype s_impballboom3	= {false,SPR_FIRE_BOOM3,6,NULL,NULL,NULL};

void T_ImpBallThrow (objtype *ob)
{
	long	deltax,deltay;
	float	angle;
	int	iangle;

	deltax = player->x - ob->x;
	deltay = ob->y - player->y;
	angle  = atan2 (deltay,deltax);
	if (angle<0)
		angle = M_PI*2+angle;
	iangle = angle/(M_PI*2)*ANGLES;

	GetNewActor();
	new->state = &s_impball1;
	new->ticcount = 1;

	new->tilex = ob->tilex;
	new->tiley = ob->tiley;
	new->x = ob->x;
	new->y = ob->y;
	new->obclass = impballobj;
	new->dir = nodir;
	new->angle = iangle;
	new->speed = 0x2000l;

	new->flags = FL_NONMARK;
	new->active = true;

	PlaySoundLocActor (IMPFIRESND,new);
}
//
// pain elemental
//

extern	statetype s_painstand;

extern	statetype s_painpath1;
extern	statetype s_painpath1s;
extern	statetype s_painpath2;
extern	statetype s_painpath3;
extern	statetype s_painpath3s;

extern	statetype s_painpain;

extern	statetype s_painshoot1;
extern	statetype s_painshoot2;
extern	statetype s_painshoot3;

extern	statetype s_painchase1;
extern	statetype s_painchase1s;
extern	statetype s_painchase2;
extern	statetype s_painchase3;
extern	statetype s_painchase3s;

extern	statetype s_paindie1;
extern	statetype s_paindie2;
extern	statetype s_paindie3;
extern	statetype s_paindie4;
extern	statetype s_paindie5;

statetype s_painstand	= {true,SPR_PAIN_W1_1,0,T_Stand,NULL,&s_painstand};

statetype s_painpath1  	= {true,SPR_PAIN_W1_1,20,T_Path,NULL,&s_painpath1s};
statetype s_painpath1s	= {true,SPR_PAIN_W1_1,5,NULL,NULL,&s_painpath2};
statetype s_painpath2	= {true,SPR_PAIN_W2_1,15,T_Path,NULL,&s_painpath3};
statetype s_painpath3	= {true,SPR_PAIN_W3_1,20,T_Path,NULL,&s_painpath3s};
statetype s_painpath3s	= {true,SPR_PAIN_W3_1,5,NULL,NULL,&s_painpath1};

statetype s_painpain	= {true,SPR_PAIN_PAIN_1,10,NULL,NULL,&s_painchase1};

statetype s_painshoot1	= {false,SPR_PAIN_SHOOT_1,20,NULL,NULL,&s_painshoot2};
statetype s_painshoot2	= {false,SPR_PAIN_SHOOT2_1,20,NULL,A_ShootLostSoul,&s_painshoot3};
statetype s_painshoot3	= {false,SPR_PAIN_SHOOT3_1,20,NULL,NULL,&s_painchase1};

statetype s_painchase1	= {true,SPR_PAIN_W1_1,10,T_Chase,NULL,&s_painchase1s};
statetype s_painchase1s	= {true,SPR_PAIN_W1_1,3,NULL,NULL,&s_painchase2};
statetype s_painchase2	= {true,SPR_PAIN_W2_1,8,T_Chase,NULL,&s_painchase3};
statetype s_painchase3	= {true,SPR_PAIN_W3_1,10,T_Chase,NULL,&s_painchase3s};
statetype s_painchase3s	= {true,SPR_PAIN_W3_1,3,NULL,NULL,&s_painchase1};

statetype s_paindie1	= {false,SPR_PAIN_DIE_1,15,NULL,A_DeathScream,&s_paindie2};
statetype s_paindie2	= {false,SPR_PAIN_DIE_2,15,NULL,NULL,&s_paindie3};
statetype s_paindie3	= {false,SPR_PAIN_DIE_3,15,NULL,A_ShootLostSoul,&s_paindie4};
statetype s_paindie4	= {false,SPR_PAIN_DIE_4,15,NULL,NULL,&s_paindie5};
statetype s_paindie5	= {false,SPR_PAIN_DIE_5,15,NULL,NULL,NULL};

void A_ShootLostSoul (objtype *ob)
{
	unsigned	far *map,tile,hitpoints[4]={100,130,160,190};

	SpawnNewObj (ob->tilex,ob->tiley,&s_soulchase1);
	new->speed = SPDPATROL*5;

	new->x = ob->x;
	new->y = ob->y;

	new->distance = ob->distance;
	new->dir = ob->dir;
	new->flags = ob->flags | FL_SHOOTABLE;

	new->obclass = soulobj;
	new->hitpoints = hitpoints[gamestate.difficulty];
}

//
// lost soul
//
extern	statetype s_soulstand;

extern	statetype s_soulpath1;
extern	statetype s_soulpath1s;
extern	statetype s_soulpath2;

extern	statetype s_soulpain;

extern	statetype s_soulshoot1;
extern	statetype s_soulshoot2;

extern	statetype s_soulchase1;
extern	statetype s_soulchase1s;
extern	statetype s_soulchase2;

extern	statetype s_souldie1;
extern	statetype s_souldie2;
extern	statetype s_souldie3;
extern	statetype s_souldie4;
extern	statetype s_souldie5;

statetype s_soulstand	= {true,SPR_SOUL_W1_1,0,T_Stand,NULL,&s_soulstand};

statetype s_soulpath1	= {true,SPR_SOUL_W1_1,20,T_Path,NULL,&s_soulpath1s};
statetype s_soulpath1s	= {true,SPR_SOUL_W1_1,5,NULL,NULL,&s_soulpath2};
statetype s_soulpath2	= {true,SPR_SOUL_W2_1,15,T_Path,NULL,&s_soulpath1};

statetype s_soulpain	= {true,SPR_SOUL_PAIN_1,10,NULL,NULL,&s_soulchase1};

statetype s_soulshoot1	= {false,SPR_SOUL_SHOOT_1,20,NULL,NULL,&s_soulshoot2};
statetype s_soulshoot2	= {false,SPR_SOUL_SHOOT2_1,20,NULL,T_Bite,&s_soulchase1};

statetype s_soulchase1	= {true,SPR_SOUL_W1_1,10,T_DemonChase,NULL,&s_soulchase1s};
statetype s_soulchase1s	= {true,SPR_SOUL_W1_1,3,NULL,NULL,&s_soulchase2};
statetype s_soulchase2	= {true,SPR_SOUL_W2_1,8,T_DemonChase,NULL,&s_soulchase1};

statetype s_souldie1	= {false,SPR_SOUL_DIE_1,15,NULL,A_DeathScream,&s_souldie2};
statetype s_souldie2	= {false,SPR_SOUL_DIE_2,15,NULL,NULL,&s_souldie3};
statetype s_souldie3	= {false,SPR_SOUL_DIE_3,15,NULL,NULL,&s_souldie4};
statetype s_souldie4	= {false,SPR_SOUL_DIE_4,15,NULL,NULL,&s_souldie5};
statetype s_souldie5	= {false,SPR_SOUL_DIE_5,15,NULL,NULL,NULL};

//
// demon
//

extern	statetype s_demonpath1;
extern	statetype s_demonpath1s;
extern	statetype s_demonpath2;
extern	statetype s_demonpath3;
extern	statetype s_demonpath3s;
extern	statetype s_demonpath4;

extern	statetype s_demonjump1;
extern	statetype s_demonjump2;
extern	statetype s_demonjump3;
extern	statetype s_demonjump4;
extern	statetype s_demonjump5;

extern	statetype s_demonpain;

extern	statetype s_demonchase1;
extern	statetype s_demonchase1s;
extern	statetype s_demonchase2;
extern	statetype s_demonchase3;
extern	statetype s_demonchase3s;
extern	statetype s_demonchase4;

extern	statetype s_demondie1;
extern	statetype s_demondie1d;
extern	statetype s_demondie2;
extern	statetype s_demondie3;
extern	statetype s_demondead;

statetype s_demonpath1 	= {true,SPR_DEMON_W1_1,20,T_Path,NULL,&s_demonpath1s};
statetype s_demonpath1s 	= {true,SPR_DEMON_W1_1,5,NULL,NULL,&s_demonpath2};
statetype s_demonpath2 	= {true,SPR_DEMON_W2_1,15,T_Path,NULL,&s_demonpath3};
statetype s_demonpath3 	= {true,SPR_DEMON_W3_1,20,T_Path,NULL,&s_demonpath3s};
statetype s_demonpath3s 	= {true,SPR_DEMON_W3_1,5,NULL,NULL,&s_demonpath4};
statetype s_demonpath4 	= {true,SPR_DEMON_W4_1,15,T_Path,NULL,&s_demonpath1};

statetype s_demonjump1 	= {false,SPR_DEMON_JUMP1,10,NULL,NULL,&s_demonjump2};
statetype s_demonjump2 	= {false,SPR_DEMON_JUMP2,10,NULL,T_Bite,&s_demonjump3};
statetype s_demonjump3 	= {false,SPR_DEMON_JUMP3,10,NULL,NULL,&s_demonjump4};
statetype s_demonjump4 	= {false,SPR_DEMON_JUMP1,10,NULL,NULL,&s_demonjump5};
statetype s_demonjump5 	= {false,SPR_DEMON_W1_1,10,NULL,NULL,&s_demonchase1};

statetype s_demonpain	= {false,SPR_DEMON_DIE_1,10,NULL,NULL,&s_demonchase1};

statetype s_demonchase1 	= {true,SPR_DEMON_W1_1,10,T_DemonChase,NULL,&s_demonchase1s};
statetype s_demonchase1s 	= {true,SPR_DEMON_W1_1,3,NULL,NULL,&s_demonchase2};
statetype s_demonchase2 	= {true,SPR_DEMON_W2_1,8,T_DemonChase,NULL,&s_demonchase3};
statetype s_demonchase3 	= {true,SPR_DEMON_W3_1,10,T_DemonChase,NULL,&s_demonchase3s};
statetype s_demonchase3s 	= {true,SPR_DEMON_W3_1,3,NULL,NULL,&s_demonchase4};
statetype s_demonchase4 	= {true,SPR_DEMON_W4_1,8,T_DemonChase,NULL,&s_demonchase1};

statetype s_demondie1		= {false,SPR_DEMON_DIE_1,15,NULL,A_DeathScream,&s_demondie2};
statetype s_demondie2		= {false,SPR_DEMON_DIE_2,15,NULL,NULL,&s_demondie3};
statetype s_demondie3		= {false,SPR_DEMON_DIE_3,15,NULL,NULL,&s_demondead};
statetype s_demondead		= {false,SPR_DEMON_DEAD,15,NULL,NULL,&s_demondead};


//
// chaingun guy
//

extern	statetype s_chainguystand;

extern	statetype s_chainguypath1;
extern	statetype s_chainguypath1s;
extern	statetype s_chainguypath2;
extern	statetype s_chainguypath3;
extern	statetype s_chainguypath3s;
extern	statetype s_chainguypath4;

extern	statetype s_chainguypain;

extern	statetype s_chainguyshoot1;
extern	statetype s_chainguyshoot2;
extern	statetype s_chainguyshoot3;

extern	statetype s_chainguychase1;
extern	statetype s_chainguychase1s;
extern	statetype s_chainguychase2;
extern	statetype s_chainguychase3;
extern	statetype s_chainguychase3s;
extern	statetype s_chainguychase4;

extern	statetype s_chainguydie1;
extern	statetype s_chainguydie2;
extern	statetype s_chainguydie3;
extern	statetype s_chainguydie4;
extern	statetype s_chainguydie5;

extern	statetype s_chainguyexpl1;
extern	statetype s_chainguyexpl2;
extern	statetype s_chainguyexpl3;
extern	statetype s_chainguyexpl4;
extern	statetype s_chainguyexpl5;
extern	statetype s_chainguyexpl6;

statetype s_chainguystand	= {true,SPR_CHAINGUY_W1_1,0,T_Stand,NULL,&s_chainguystand};

statetype s_chainguypath1 	= {true,SPR_CHAINGUY_W1_1,20,T_Path,NULL,&s_chainguypath1s};
statetype s_chainguypath1s 	= {true,SPR_CHAINGUY_W1_1,5,NULL,NULL,&s_chainguypath2};
statetype s_chainguypath2 	= {true,SPR_CHAINGUY_W2_1,15,T_Path,NULL,&s_chainguypath3};
statetype s_chainguypath3 	= {true,SPR_CHAINGUY_W3_1,20,T_Path,NULL,&s_chainguypath3s};
statetype s_chainguypath3s 	= {true,SPR_CHAINGUY_W3_1,5,NULL,NULL,&s_chainguypath4};
statetype s_chainguypath4 	= {true,SPR_CHAINGUY_W4_1,15,T_Path,NULL,&s_chainguypath1};

statetype s_chainguypain 	= {true,SPR_CHAINGUY_PAIN_1,10,NULL,NULL,&s_chainguychase1};

statetype s_chainguyshoot1 	= {false,SPR_CHAINGUY_SHOOT_1,6,NULL,T_Shoot,&s_chainguyshoot2};
statetype s_chainguyshoot2 	= {false,SPR_CHAINGUY_SHOOT2_1,20,NULL,T_Shoot,&s_chainguyshoot3};
statetype s_chainguyshoot3	= {false,SPR_CHAINGUY_SHOOT_1,6,NULL,T_Shoot,&s_chainguychase1};

statetype s_chainguychase1 	= {true,SPR_CHAINGUY_W1_1,10,T_Chase,NULL,&s_chainguychase1s};
statetype s_chainguychase1s 	= {true,SPR_CHAINGUY_W1_1,3,NULL,NULL,&s_chainguychase2};
statetype s_chainguychase2 	= {true,SPR_CHAINGUY_W2_1,8,T_Chase,NULL,&s_chainguychase3};
statetype s_chainguychase3 	= {true,SPR_CHAINGUY_W3_1,10,T_Chase,NULL,&s_chainguychase3s};
statetype s_chainguychase3s 	= {true,SPR_CHAINGUY_W3_1,3,NULL,NULL,&s_chainguychase4};
statetype s_chainguychase4 	= {true,SPR_CHAINGUY_W4_1,8,T_Chase,NULL,&s_chainguychase1};

statetype s_chainguydie1		= {false,SPR_CHAINGUY_DIE_1,11,NULL,A_DeathScream,&s_chainguydie2};
statetype s_chainguydie2		= {false,SPR_CHAINGUY_DIE_2,11,NULL,NULL,&s_chainguydie3};
statetype s_chainguydie3		= {false,SPR_CHAINGUY_DIE_3,11,NULL,NULL,&s_chainguydie4};
statetype s_chainguydie4		= {false,SPR_CHAINGUY_DIE_4,11,NULL,NULL,&s_chainguydie5};
statetype s_chainguydie5		= {false,SPR_CHAINGUY_DEAD,0,NULL,NULL,&s_chainguydie5};

statetype s_chainguyexpl1	= {false,SPR_CHAINGUY_DIE2_1,11,NULL,A_Slurpie,&s_chainguyexpl2};
statetype s_chainguyexpl2	= {false,SPR_CHAINGUY_DIE2_2,11,NULL,NULL,&s_chainguyexpl3};
statetype s_chainguyexpl3	= {false,SPR_CHAINGUY_DIE2_3,11,NULL,NULL,&s_chainguyexpl4};
statetype s_chainguyexpl4	= {false,SPR_CHAINGUY_DIE2_4,11,NULL,NULL,&s_chainguyexpl5};
statetype s_chainguyexpl5	= {false,SPR_CHAINGUY_DIE2_5,11,NULL,NULL,&s_chainguyexpl6};
statetype s_chainguyexpl6	= {false,SPR_CHAINGUY_DIE2_6,0,NULL,NULL,&s_chainguyexpl6};


//
// shotgun guy
//

extern	statetype s_shotguystand;

extern	statetype s_shotguypath1;
extern	statetype s_shotguypath1s;
extern	statetype s_shotguypath2;
extern	statetype s_shotguypath3;
extern	statetype s_shotguypath3s;
extern	statetype s_shotguypath4;

extern	statetype s_shotguypain;

extern	statetype s_shotguyshoot1;
extern	statetype s_shotguyshoot2;

extern	statetype s_shotguychase1;
extern	statetype s_shotguychase1s;
extern	statetype s_shotguychase2;
extern	statetype s_shotguychase3;
extern	statetype s_shotguychase3s;
extern	statetype s_shotguychase4;

extern	statetype s_shotguydie1;
extern	statetype s_shotguydie2;
extern	statetype s_shotguydie3;
extern	statetype s_shotguydie4;
extern	statetype s_shotguydie5;

extern	statetype s_shotguyexpl1;
extern	statetype s_shotguyexpl2;
extern	statetype s_shotguyexpl3;
extern	statetype s_shotguyexpl4;
extern	statetype s_shotguyexpl5;
extern	statetype s_shotguyexpl6;
extern	statetype s_shotguyexpl7;
extern	statetype s_shotguyexpl8;
extern	statetype s_shotguyexpl9;

statetype s_shotguystand	= {true,SPR_SHOTGUY_W1_1,0,T_Stand,NULL,&s_shotguystand};

statetype s_shotguypath1 	= {true,SPR_SHOTGUY_W1_1,20,T_Path,NULL,&s_shotguypath1s};
statetype s_shotguypath1s 	= {true,SPR_SHOTGUY_W1_1,5,NULL,NULL,&s_shotguypath2};
statetype s_shotguypath2 	= {true,SPR_SHOTGUY_W2_1,15,T_Path,NULL,&s_shotguypath3};
statetype s_shotguypath3 	= {true,SPR_SHOTGUY_W3_1,20,T_Path,NULL,&s_shotguypath3s};
statetype s_shotguypath3s 	= {true,SPR_SHOTGUY_W3_1,5,NULL,NULL,&s_shotguypath4};
statetype s_shotguypath4 	= {true,SPR_SHOTGUY_W4_1,15,T_Path,NULL,&s_shotguypath1};

statetype s_shotguypain 	= {true,SPR_SHOTGUY_PAIN_1,10,NULL,NULL,&s_shotguychase1};

statetype s_shotguyshoot1 	= {false,SPR_SHOTGUY_SHOOT_1,20,NULL,T_Shoot,&s_shotguyshoot2};
statetype s_shotguyshoot2 	= {false,SPR_SHOTGUY_SHOOT2_1,20,NULL,NULL,&s_shotguychase1};

statetype s_shotguychase1 	= {true,SPR_SHOTGUY_W1_1,10,T_Chase,NULL,&s_shotguychase1s};
statetype s_shotguychase1s 	= {true,SPR_SHOTGUY_W1_1,3,NULL,NULL,&s_shotguychase2};
statetype s_shotguychase2 	= {true,SPR_SHOTGUY_W2_1,8,T_Chase,NULL,&s_shotguychase3};
statetype s_shotguychase3 	= {true,SPR_SHOTGUY_W3_1,10,T_Chase,NULL,&s_shotguychase3s};
statetype s_shotguychase3s 	= {true,SPR_SHOTGUY_W3_1,3,NULL,NULL,&s_shotguychase4};
statetype s_shotguychase4 	= {true,SPR_SHOTGUY_W4_1,8,T_Chase,NULL,&s_shotguychase1};

statetype s_shotguydie1		= {false,SPR_SHOTGUY_DIE_1,7,NULL,A_DeathScream,&s_shotguydie2};
statetype s_shotguydie2		= {false,SPR_SHOTGUY_DIE_2,7,NULL,NULL,&s_shotguydie3};
statetype s_shotguydie3		= {false,SPR_SHOTGUY_DIE_3,7,NULL,NULL,&s_shotguydie4};
statetype s_shotguydie4		= {false,SPR_SHOTGUY_DIE_4,7,NULL,NULL,&s_shotguydie5};
statetype s_shotguydie5		= {false,SPR_SHOTGUY_DEAD,0,NULL,NULL,&s_shotguydie5};

statetype s_shotguyexpl1	= {false,SPR_SHOTGUY_DIE2_1,7,NULL,A_Slurpie,&s_shotguyexpl2};
statetype s_shotguyexpl2	= {false,SPR_SHOTGUY_DIE2_2,7,NULL,NULL,&s_shotguyexpl3};
statetype s_shotguyexpl3	= {false,SPR_SHOTGUY_DIE2_3,7,NULL,NULL,&s_shotguyexpl4};
statetype s_shotguyexpl4	= {false,SPR_SHOTGUY_DIE2_4,7,NULL,NULL,&s_shotguyexpl5};
statetype s_shotguyexpl5	= {false,SPR_SHOTGUY_DIE2_5,7,NULL,NULL,&s_shotguyexpl6};
statetype s_shotguyexpl6	= {false,SPR_SHOTGUY_DIE2_6,7,NULL,NULL,&s_shotguyexpl7};
statetype s_shotguyexpl7	= {false,SPR_SHOTGUY_DIE2_7,7,NULL,NULL,&s_shotguyexpl8};
statetype s_shotguyexpl8	= {false,SPR_SHOTGUY_DIE2_8,7,NULL,NULL,&s_shotguyexpl9};
statetype s_shotguyexpl9	= {false,SPR_SHOTGUY_DIE2_9,0,NULL,NULL,&s_shotguyexpl9};


//
// Cacodemon
//
void T_CacoBallThrow (objtype *ob);

extern	statetype s_cacostand;

extern	statetype s_cacopath1;
extern	statetype s_cacopath1s;

extern	statetype s_cacopain;

extern	statetype s_cacoshoot1;
extern	statetype s_cacoshoot2;
extern	statetype s_cacoshoot3;
extern	statetype s_cacoshoot4;

extern	statetype s_cacochase1;
extern	statetype s_cacochase1s;

extern	statetype s_cacodie1;
extern	statetype s_cacodie2;
extern	statetype s_cacodie3;
extern	statetype s_cacodie4;

statetype s_cacostand	= {true,SPR_CACO_W1_1,0,T_Stand,NULL,&s_cacostand};

statetype s_cacopath1 	= {true,SPR_CACO_W1_1,20,T_Path,NULL,&s_cacopath1s};
statetype s_cacopath1s 	= {true,SPR_CACO_W1_1,5,NULL,NULL,&s_cacopath1};

statetype s_cacopain 	= {true,SPR_CACO_PAIN_1,10,NULL,NULL,&s_cacochase1};

statetype s_cacoshoot1 	= {false,SPR_CACO_SHOOT1,20,NULL,NULL,&s_cacoshoot2};
statetype s_cacoshoot2 	= {false,SPR_CACO_SHOOT2,20,NULL,T_CacoBallThrow,&s_cacoshoot3};
statetype s_cacoshoot3 	= {false,SPR_CACO_SHOOT3,10,NULL,NULL,&s_cacoshoot4};
statetype s_cacoshoot4	= {false,SPR_CACO_SHOOT4,10,NULL,NULL,&s_cacochase1};

statetype s_cacochase1 	= {true,SPR_CACO_W1_1,10,T_Chase,NULL,&s_cacochase1s};
statetype s_cacochase1s 	= {true,SPR_CACO_W1_1,3,NULL,NULL,&s_cacochase1};

statetype s_cacodie1		= {false,SPR_CACO_DIE_1,15,NULL,A_DeathScream,&s_cacodie2};
statetype s_cacodie2		= {false,SPR_CACO_DIE_2,15,NULL,NULL,&s_cacodie3};
statetype s_cacodie3		= {false,SPR_CACO_DIE_3,15,NULL,NULL,&s_cacodie4};
statetype s_cacodie4		= {false,SPR_CACO_DEAD,0,NULL,NULL,&s_cacodie4};

extern	statetype s_cacoball1;
extern	statetype s_cacoball2;
extern	statetype s_cacoballboom1;
extern	statetype s_cacoballboom2;
extern	statetype s_cacoballboom3;

statetype s_cacoball1	= {false,SPR_FIRE2_1,6,T_Projectile,NULL,&s_cacoball2};
statetype s_cacoball2	= {false,SPR_FIRE2_2,6,T_Projectile,NULL,&s_cacoball1};

statetype s_cacoballboom1	= {false,SPR_FIRE2_BOOM1,6,NULL,NULL,&s_cacoballboom2};
statetype s_cacoballboom2	= {false,SPR_FIRE2_BOOM2,6,NULL,NULL,&s_cacoballboom3};
statetype s_cacoballboom3	= {false,SPR_FIRE2_BOOM3,6,NULL,NULL,NULL};

void T_CacoBallThrow (objtype *ob)
{
	long	deltax,deltay;
	float	angle;
	int	iangle;

	deltax = player->x - ob->x;
	deltay = ob->y - player->y;
	angle = atan2 (deltay,deltax);
	if (angle<0)
		angle = M_PI*2+angle;
	iangle = angle/(M_PI*2)*ANGLES;

	GetNewActor ();
	new->state = &s_cacoball1;
	new->ticcount = 1;

	new->tilex = ob->tilex;
	new->tiley = ob->tiley;
	new->x = ob->x;
	new->y = ob->y;
	new->obclass = cacoballobj;
	new->dir = nodir;
	new->angle = iangle;
	new->speed = 0x2000l;

	new->flags = FL_NONMARK;
	new->active = true;

	PlaySoundLocActor (IMPFIRESND,new);
}
//
// cyber-demon
//

void T_MissileThrow (objtype *ob);

extern	statetype s_cyberstand;

extern	statetype s_cyberchase1;
extern	statetype s_cyberchase1s;
extern	statetype s_cyberchase2;
extern	statetype s_cyberchase3;
extern	statetype s_cyberchase3s;
extern	statetype s_cyberchase4;

extern	statetype s_cyberdie1;
extern	statetype s_cyberdie2;
extern	statetype s_cyberdie3;
extern	statetype s_cyberdie4;
extern	statetype s_cyberdie5;
extern	statetype s_cyberdie6;
extern	statetype s_cyberdie7;
extern	statetype s_cyberdie8;
extern	statetype s_cyberdie9;
extern	statetype s_cyberdie10;

extern	statetype s_cybershoot1;
extern	statetype s_cybershoot2;


statetype s_cyberstand	= {true,SPR_CYBER_W1_1,0,T_Stand,NULL,&s_cyberstand};

statetype s_cyberchase1 	= {true,SPR_CYBER_W1_1,10,T_Chase,A_MechaSound,&s_cyberchase1s};
statetype s_cyberchase1s	= {true,SPR_CYBER_W1_1,3,NULL,NULL,&s_cyberchase2};
statetype s_cyberchase2 	= {true,SPR_CYBER_W2_1,8,T_Chase,NULL,&s_cyberchase3};
statetype s_cyberchase3 	= {true,SPR_CYBER_W3_1,10,T_Chase,A_MechaSound,&s_cyberchase3s};
statetype s_cyberchase3s	= {true,SPR_CYBER_W3_1,3,NULL,NULL,&s_cyberchase4};
statetype s_cyberchase4 	= {true,SPR_CYBER_W4_1,8,T_Chase,NULL,&s_cyberchase1};

statetype s_cyberdie1	= {true,SPR_CYBER_DIE_1,15,NULL,A_DeathScream,&s_cyberdie2};
statetype s_cyberdie2	= {false,SPR_CYBER_DIE2,15,NULL,NULL,&s_cyberdie3};
statetype s_cyberdie3	= {false,SPR_CYBER_DIE3,15,NULL,NULL,&s_cyberdie4};
statetype s_cyberdie4	= {false,SPR_CYBER_DIE4,15,NULL,NULL,&s_cyberdie5};
statetype s_cyberdie5	= {false,SPR_CYBER_DIE5,15,NULL,NULL,&s_cyberdie6};
statetype s_cyberdie6	= {false,SPR_CYBER_DIE6,15,NULL,NULL,&s_cyberdie7};
statetype s_cyberdie7	= {false,SPR_CYBER_DIE7,15,NULL,NULL,&s_cyberdie8};
statetype s_cyberdie8	= {false,SPR_CYBER_DIE8,15,NULL,NULL,&s_cyberdie9};
statetype s_cyberdie9	= {false,SPR_CYBER_DIE9,15,NULL,NULL,&s_cyberdie10};
statetype s_cyberdie10	= {false,SPR_CYBER_DEAD,0,NULL,NULL,&s_cyberdie10};

statetype s_cybershoot1 	= {false,SPR_CYBER_SHOOT_1,30,NULL,NULL,&s_cybershoot2};
statetype s_cybershoot2 	= {false,SPR_CYBER_SHOOT2_1,10,NULL,T_MissileThrow,&s_cyberchase1};

void T_MissileThrow (objtype *ob)
{
   long   deltax,deltay;
   float   angle;
   int      iangle;

   deltax = player->x - ob->x;
   deltay = ob->y - player->y;
   angle = atan2 (deltay,deltax);
   if (angle<0)
      angle = M_PI*2+angle;
   iangle = angle/(M_PI*2)*ANGLES;

   GetNewActor ();
   new->state = &s_rocket;
   new->ticcount = 1;

   new->tilex = ob->tilex;
   new->tiley = ob->tiley;
   new->x = ob->x;
   new->y = ob->y;
   new->obclass = rocketobj;
   new->dir = nodir;
   new->angle = iangle;
   new->speed = 0x2000l;

   new->flags = FL_NONMARK;
   new->active = true;

   PlaySoundLocActor (RLAUNCHERFIRESND,new);   //Change to the sound you want.
}

void SpawnStand (enemy_t which, int tilex, int tiley, int dir)
{
	unsigned	far *map,tile;

	switch (which)
	{
	case en_mutant:
		SpawnNewObj (tilex,tiley,&s_mutantstand);
		new->speed = SPDPATROL;
		if (!loadedgame)
		  gamestate.killtotal++;
		break;
	case en_imp:
		SpawnNewObj (tilex,tiley,&s_impstand);
		new->speed = SPDPATROL;
		if (!loadedgame)
			gamestate.killtotal++;
		break;
	case en_painelemental:
		SpawnNewObj (tilex,tiley,&s_painstand);
		new->speed = SPDPATROL;
		if (!loadedgame)
			gamestate.killtotal++;
		break;
	case en_lostsoul:
		SpawnNewObj (tilex,tiley,&s_soulstand);
		new->speed = SPDPATROL;
		if (!loadedgame)
			gamestate.killtotal++;
		break;

	case en_chaingunguy:
		SpawnNewObj (tilex,tiley,&s_chainguystand);
		new->speed = SPDPATROL;
		if (!loadedgame)
		  gamestate.killtotal++;
		break;

	case en_shotgunguy:
		SpawnNewObj (tilex,tiley,&s_shotguystand);
		new->speed = SPDPATROL;
		if (!loadedgame)
		  gamestate.killtotal++;
		break;

	case en_cacodemon:
		SpawnNewObj (tilex,tiley,&s_cacostand);
		new->speed = SPDPATROL;
		if (!loadedgame)
		  gamestate.killtotal++;
		break;
	}

	map = mapsegs[0]+farmapylookup[tiley]+tilex;
	if (*map == AMBUSHTILE)
	{
		tilemap[tilex][tiley] = 0;

		if (*(map+1) >= AREATILE)
			tile = *(map+1);
		if (*(map-mapwidth) >= AREATILE)
			tile = *(map-mapwidth);
		if (*(map+mapwidth) >= AREATILE)
			tile = *(map+mapwidth);
		if ( *(map-1) >= AREATILE)
			tile = *(map-1);

		*map = tile;
		new->areanumber = tile-AREATILE;

		new->flags |= FL_AMBUSH;
	}

	new->obclass = mutantobj+which;
	new->hitpoints = starthitpoints[gamestate.difficulty][which];
	new->dir = dir*2;
	new->flags |= FL_SHOOTABLE;
}

void SpawnDeadGuard (int tilex, int tiley)
{
	SpawnNewObj (tilex,tiley,&s_mutantdie4);
	new->obclass = inertobj;
}

void SpawnCyber (int tilex, int tiley)
{
	unsigned	far *map,tile;

	SpawnNewObj (tilex,tiley,&s_cyberstand);
	new->speed = SPDPATROL;

	new->obclass = cyberobj;
	new->hitpoints = starthitpoints[gamestate.difficulty][en_cyber];
	new->dir = nodir;
	new->flags |= FL_SHOOTABLE|FL_AMBUSH;
	if (!loadedgame)
	  gamestate.killtotal++;
}

void SpawnPatrol (enemy_t which, int tilex, int tiley, int dir)
{
	switch (which)
	{
	case en_mutant:
		SpawnNewObj (tilex,tiley,&s_mutantpath1);
		new->speed = SPDPATROL;
		if (!loadedgame)
		  gamestate.killtotal++;
		break;
	case en_imp:
		SpawnNewObj (tilex,tiley,&s_imppath1);
		new->speed = SPDPATROL;
		if (!loadedgame)
			gamestate.killtotal++;
		break;
	case en_painelemental:
		SpawnNewObj (tilex,tiley,&s_painpath1);
		new->speed = SPDPATROL;
		if (!loadedgame)
			gamestate.killtotal++;
		break;
	case en_lostsoul:
		SpawnNewObj (tilex,tiley,&s_soulpath1);
		new->speed = SPDPATROL;
		if (!loadedgame)
			gamestate.killtotal++;
		break;
	case en_chaingunguy:
		SpawnNewObj (tilex,tiley,&s_chainguypath1);
		new->speed = SPDPATROL;
		if (!loadedgame)
		  gamestate.killtotal++;
		break;

	case en_cacodemon:
		SpawnNewObj (tilex,tiley,&s_cacopath1);
		new->speed = SPDPATROL;
		if (!loadedgame)
		  gamestate.killtotal++;
		break;

	case en_shotgunguy:
		SpawnNewObj (tilex,tiley,&s_shotguypath1);
		new->speed = SPDPATROL;
		if (!loadedgame)
		  gamestate.killtotal++;
		break;

	case en_demon:
		SpawnNewObj (tilex,tiley,&s_demonpath1);
		new->speed = SPDDOG;
		if (!loadedgame)
		  gamestate.killtotal++;
		break;
	}

	new->obclass = mutantobj+which;
	new->dir = dir*2;
	new->hitpoints = starthitpoints[gamestate.difficulty][which];
	new->distance = tileglobal;
	new->flags |= FL_SHOOTABLE;
	new->active = true;

	actorat[new->tilex][new->tiley] = NULL;		// don't use original spot

	switch (dir)
	{
	case 0:
		new->tilex++;
		break;
	case 1:
		new->tiley--;
		break;
	case 2:
		new->tilex--;
		break;
	case 3:
		new->tiley++;
		break;
	}

	actorat[new->tilex][new->tiley] = new;
}

void A_DeathScream (objtype *ob)
{
	switch (ob->obclass)
	{
	case impobj:
	{
		int sounds[2]={	MUTDIE1SND,
				MUTDIE2SND
				};
		PlaySoundLocActor(sounds[US_RndT()%1],ob);
		}
		break;

	case shotguyobj:
	case mutantobj:
	case chainguyobj:
		{
		 int sounds[3]={ SHOTGUYDIE1SND,
				 SHOTGUYDIE2SND,
				 SHOTGUYDIE3SND
				 };
		 PlaySoundLocActor(sounds[US_RndT()%2],ob);
		}
		break;
	case soulobj:
		PlaySoundLocActor (IMPFIREBOOMSND,ob);
		break;
	case cacoobj:
		PlaySoundLocActor(CACODIESND,ob);	// JAB
		break;
	case painobj:
		PlaySoundLocActor (PAINDIESND,ob);
                break;
	case demonobj:
		PlaySoundLocActor(DEMONDIESND,ob);	// JAB
		break;
	case cyberobj:
		SD_PlaySound(CYBERDIESND);				// JAB
		break;
	case spiderobj:
		SD_PlaySound(SPIDERDIESND);
		break;
	case revenantobj:
		SD_PlaySound(REVDIESND);
		break;
	case bossobj:
		SD_PlaySound(BOSSDIESND);
		break;
	case boss2obj:
		SD_PlaySound(DTHKNTDIESND);
		break;
	case mancubusobj:
		SD_PlaySound(MANDIESND);
		break;
	case vileobj:
		SD_PlaySound(VILEDIESND);
		break;

	}
}

void	T_Mancubus (objtype *ob);
void T_PlasmaThrow (objtype *ob);

extern	statetype s_spiderstand;

extern	statetype s_spiderchase1;
extern	statetype s_spiderchase1s;
extern	statetype s_spiderchase2;
extern	statetype s_spiderchase3;
extern	statetype s_spiderchase3s;
extern	statetype s_spiderchase4;
extern	statetype s_spiderchase5;
extern	statetype s_spiderchase5s;
extern	statetype s_spiderchase6;

extern	statetype s_spiderdie1;
extern	statetype s_spiderdie2;
extern	statetype s_spiderdie3;
extern	statetype s_spiderdie4;
extern	statetype s_spiderdie5;
extern	statetype s_spiderdie6;
extern	statetype s_spiderdie7;
extern	statetype s_spiderdie8;

extern	statetype s_spidershoot1;
extern	statetype s_spidershoot2;
extern	statetype s_spidershoot3;
extern	statetype s_spidershoot4;
extern	statetype s_spidershoot5;
extern	statetype s_spidershoot6;

statetype s_spiderstand	= {true,SPR_SPIDER_W1_1,0,T_Stand,NULL,&s_spiderstand};

statetype s_spiderchase1 	= {true,SPR_SPIDER_W1_1,10,T_Spider,NULL,&s_spiderchase1s};
statetype s_spiderchase1s	= {true,SPR_SPIDER_W1_1,3,NULL,NULL,&s_spiderchase2};
statetype s_spiderchase2 	= {true,SPR_SPIDER_W2_1,8,T_Spider,NULL,&s_spiderchase3};
statetype s_spiderchase3 	= {true,SPR_SPIDER_W3_1,10,T_Spider,NULL,&s_spiderchase3s};
statetype s_spiderchase3s	= {true,SPR_SPIDER_W3_1,3,NULL,NULL,&s_spiderchase4};
statetype s_spiderchase4 	= {true,SPR_SPIDER_W4_1,8,T_Spider,NULL,&s_spiderchase5};
statetype s_spiderchase5	= {true,SPR_SPIDER_W5_1,10,T_Spider,NULL,&s_spiderchase5s};
statetype s_spiderchase5s	= {true,SPR_SPIDER_W5_1,3,NULL,NULL,&s_spiderchase6};
statetype s_spiderchase6	= {true,SPR_SPIDER_W6_1,8,T_Spider,NULL,&s_spiderchase1};

statetype s_spiderdie1	= {true,SPR_SPIDER_DIE1_1,10,NULL,A_DeathScream,&s_spiderdie2};
statetype s_spiderdie2	= {false,SPR_SPIDER_DIE2,10,NULL,NULL,&s_spiderdie3};
statetype s_spiderdie3	= {false,SPR_SPIDER_DIE3,10,NULL,NULL,&s_spiderdie4};
statetype s_spiderdie4	= {false,SPR_SPIDER_DIE4,10,NULL,NULL,&s_spiderdie5};
statetype s_spiderdie5	= {false,SPR_SPIDER_DIE5,10,NULL,NULL,&s_spiderdie6};
statetype s_spiderdie6	= {false,SPR_SPIDER_DIE6,10,NULL,NULL,&s_spiderdie7};
statetype s_spiderdie7	= {false,SPR_SPIDER_DIE7,10,NULL,NULL,&s_spiderdie8};
statetype s_spiderdie8	= {false,SPR_SPIDER_DEAD,20,NULL,NULL,&s_spiderdie8};

statetype s_spidershoot1 	= {false,SPR_SPIDER_SHOOT1_1,10,NULL,T_PlasmaThrow,&s_spidershoot2};
statetype s_spidershoot2 	= {false,SPR_SPIDER_SHOOT2_1,10,NULL,T_PlasmaThrow,&s_spidershoot3};
statetype s_spidershoot3	= {false,SPR_SPIDER_SHOOT1_1,10,NULL,T_PlasmaThrow,&s_spidershoot4};
statetype s_spidershoot4	= {false,SPR_SPIDER_SHOOT2_1,10,NULL,T_PlasmaThrow,&s_spidershoot5};
statetype s_spidershoot5	= {false,SPR_SPIDER_SHOOT1_1,10,NULL,T_PlasmaThrow,&s_spidershoot6};
statetype s_spidershoot6	= {false,SPR_SPIDER_SHOOT2_1,10,NULL,T_PlasmaThrow,&s_spiderchase1};

extern	statetype s_splasma1;
extern	statetype s_splasma2;
extern	statetype s_splasmaboom1;
extern	statetype s_splasmaboom2;
extern	statetype s_splasmaboom3;
extern	statetype s_splasmaboom4;
extern	statetype s_splasmaboom5;

statetype s_splasma1	= {false,SPR_SPLASMA_1,1,NULL,T_Projectile,&s_splasma2};
statetype s_splasma2	= {false,SPR_SPLASMA_2,1,NULL,T_Projectile,&s_splasma1};
statetype s_splasmaboom1	= {false,SPR_SPLASMA_BOOM1,10,NULL,NULL,&s_splasmaboom2};
statetype s_splasmaboom2	= {false,SPR_SPLASMA_BOOM2,10,NULL,NULL,&s_splasmaboom3};
statetype s_splasmaboom3	= {false,SPR_SPLASMA_BOOM3,10,NULL,NULL,&s_splasmaboom4};
statetype s_splasmaboom4	= {false,SPR_SPLASMA_BOOM4,10,NULL,NULL,&s_splasmaboom5};
statetype s_splasmaboom5	= {false,SPR_SPLASMA_BOOM5,1,NULL,NULL,NULL};

void T_PlasmaThrow (objtype *ob)
{
	long	deltax,deltay;
	float	angle;
	int		iangle;

	deltax = player->x - ob->x;
	deltay = ob->y - player->y;
	angle = atan2 (deltay,deltax);
	if (angle<0)
		angle = M_PI*2+angle;
	iangle = angle/(M_PI*2)*ANGLES;

	GetNewActor ();
	new->state = &s_splasma1;
	new->ticcount = 1;

	new->tilex = ob->tilex;
	new->tiley = ob->tiley;
	new->x = ob->x;
	new->y = ob->y;
	new->obclass = splasmaobj;
	new->dir = nodir;
	new->angle = iangle;
	new->speed = 0x2000l;
	new->flags = FL_NONMARK;
	new->active = true;

	PlaySoundLocActor (PLASMAGUNFIRESND,new);
}
void T_FatThrow (objtype *ob);
void A_FatAtkSnd (objtype *ob);

extern	statetype s_mancubusstand;

extern	statetype s_mancubuschase1;
extern	statetype s_mancubuschase1s;
extern	statetype s_mancubuschase2;
extern	statetype s_mancubuschase3;
extern	statetype s_mancubuschase3s;
extern	statetype s_mancubuschase4;

extern	statetype s_mancubusdie1;
extern	statetype s_mancubusdie2;
extern	statetype s_mancubusdie3;
extern	statetype s_mancubusdie4;
extern	statetype s_mancubusdie5;
extern	statetype s_mancubusdie6;
extern  statetype s_mancubusdie7;
extern	statetype s_mancubusdie8;
extern	statetype s_mancubusdie9;
extern	statetype s_mancubusdie10;
extern	statetype s_mancubusdie11;
extern	statetype s_mancubusdie12;

extern	statetype s_mancubusshoot1;
extern	statetype s_mancubusshoot2;
extern	statetype s_mancubusshoot3;

statetype s_mancubusstand	= {true,SPR_MANCUBUS_W1_1,0,T_Stand,NULL,&s_mancubusstand};

statetype s_mancubuschase1 	= {true,SPR_MANCUBUS_W1_1,10,T_Mancubus,NULL,&s_mancubuschase1s};
statetype s_mancubuschase1s	= {true,SPR_MANCUBUS_W2_1,3,NULL,NULL,&s_mancubuschase2};
statetype s_mancubuschase2 	= {true,SPR_MANCUBUS_W3_1,8,T_Mancubus,NULL,&s_mancubuschase3};
statetype s_mancubuschase3 	= {true,SPR_MANCUBUS_W4_1,10,T_Mancubus,NULL,&s_mancubuschase3s};
statetype s_mancubuschase3s	= {true,SPR_MANCUBUS_W5_1,3,NULL,NULL,&s_mancubuschase4};
statetype s_mancubuschase4 	= {true,SPR_MANCUBUS_W6_1,8,T_Mancubus,NULL,&s_mancubuschase1};

statetype s_mancubusdie1	= {true,SPR_MANCUBUS_DIE_1,1,NULL,A_DeathScream,&s_mancubusdie2};
statetype s_mancubusdie2	= {true,SPR_MANCUBUS_DIE_2,10,NULL,NULL,&s_mancubusdie3};
statetype s_mancubusdie3	= {false,SPR_MANCUBUS_DIE3,10,NULL,NULL,&s_mancubusdie4};
statetype s_mancubusdie4	= {false,SPR_MANCUBUS_DIE4,10,NULL,NULL,&s_mancubusdie5};
statetype s_mancubusdie5	= {false,SPR_MANCUBUS_DIE5,10,NULL,NULL,&s_mancubusdie6};
statetype s_mancubusdie6	= {false,SPR_MANCUBUS_DIE6,10,NULL,NULL,&s_mancubusdie7};
statetype s_mancubusdie7	= {false,SPR_MANCUBUS_DIE7,10,NULL,NULL,&s_mancubusdie8};
statetype s_mancubusdie8	= {false,SPR_MANCUBUS_DIE8,10,NULL,NULL,&s_mancubusdie9};
statetype s_mancubusdie9	= {false,SPR_MANCUBUS_DIE9,10,NULL,NULL,&s_mancubusdie10};
statetype s_mancubusdie10	= {false,SPR_MANCUBUS_DIE10,10,NULL,NULL,&s_mancubusdie11};
statetype s_mancubusdie11	= {false,SPR_MANCUBUS_DIE11,10,NULL,NULL,&s_mancubusdie12};
statetype s_mancubusdie12	= {false,SPR_MANCUBUS_DEAD,20,NULL,NULL,&s_mancubusdie12};

statetype s_mancubusshoot1 	= {false,SPR_MANCUBUS_SHOOT_1,30,NULL,A_FatAtkSnd,&s_mancubusshoot2};
statetype s_mancubusshoot2 	= {false,SPR_MANCUBUS_SHOOT2_1,10,NULL,T_FatThrow,&s_mancubusshoot3};
statetype s_mancubusshoot3	= {false,SPR_MANCUBUS_SHOOT2_1,10,NULL,T_FatThrow,&s_mancubuschase1};

extern	statetype s_fatball;
extern	statetype s_fatball2;

extern	statetype s_fatballboom1;
extern	statetype s_fatballboom2;
extern	statetype s_fatballboom3;

statetype s_fatball	= {true,SPR_FAT_1,6,T_Projectile,NULL,&s_fatball2};
statetype s_fatball2	= {true,SPR_FAT2_1,6,T_Projectile,NULL,&s_fatball};

statetype s_fatballboom1	= {false,SPR_BOOM_1,10,NULL,NULL,&s_fatballboom2};
statetype s_fatballboom2	= {false,SPR_BOOM_2,10,NULL,NULL,&s_fatballboom3};
statetype s_fatballboom3	= {false,SPR_BOOM_3,1,NULL,NULL,NULL};

void A_FatAtkSnd (objtype *ob)
{
	PlaySoundLocActor(MANATKSND,ob);
}
void T_FatThrow (objtype *ob)
{
	long	deltax,deltay;
	float	angle;
	int		iangle;

	deltax = player->x - ob->x;
	deltay = ob->y - player->y;
	angle = atan2 (deltay,deltax);
	if (angle<0)
		angle = M_PI*2+angle;
	iangle = angle/(M_PI*2)*ANGLES;
	if (ob->obclass == mancubusobj)
	{
		if (ob->state == &s_mancubusshoot2)
		{
			iangle-=4;
			if (iangle<0)
				iangle+=ANGLES;
		}
		else
		{
			iangle+=4;
			if (iangle>=ANGLES)
				iangle-=ANGLES;
		}
	}

	GetNewActor ();
	new->state = &s_fatball;
	new->ticcount = 1;

	new->tilex = ob->tilex;
	new->tiley = ob->tiley;
	new->x = ob->x;
	new->y = ob->y;
	new->obclass = fatballobj;
	PlaySoundLocActor (IMPFIRESND,new);

	new->dir = nodir;
	new->angle = iangle;
	new->speed = 0x2000l;
	new->flags = FL_NONMARK;
	new->active = true;
}
void SpawnSpider (int tilex, int tiley)
{
	unsigned	far *map,tile;

	SpawnNewObj (tilex,tiley,&s_spiderstand);
	new->speed = SPDPATROL;

	new->obclass = spiderobj;
	new->hitpoints = starthitpoints[gamestate.difficulty][en_spider];
	new->dir = nodir;
	new->flags |= FL_SHOOTABLE|FL_AMBUSH;
	if (!loadedgame)
	  gamestate.killtotal++;
}

void SpawnMancubus (int tilex, int tiley)
{
	unsigned	far *map,tile;

	SpawnNewObj (tilex,tiley,&s_mancubusstand);
	new->speed = SPDPATROL;

	new->obclass = mancubusobj;
	new->hitpoints = starthitpoints[gamestate.difficulty][en_mancubus];
	new->dir = nodir;
	new->flags |= FL_SHOOTABLE|FL_AMBUSH;
	if (!loadedgame)
	  gamestate.killtotal++;
}

void T_Spider (objtype *ob)
{
	long move;
	int	dx,dy,dist;
	boolean	dodge;

	dodge = false;
	dx = abs(ob->tilex - player->tilex);
	dy = abs(ob->tiley - player->tiley);
	dist = dx>dy ? dx : dy;

	if (CheckLine(ob))						// got a shot at player?
	{

		if ( US_RndT() < (tics<<3) )
		{
			NewState (ob,&s_spidershoot1);
			return;
		}
		dodge = true;
	}

	if (ob->dir == nodir)
	{
		if (dodge)
			SelectDodgeDir (ob);
		else
			SelectChaseDir (ob);
		if (ob->dir == nodir)
			return;							// object is blocked in
	}

	move = ob->speed*tics;

	while (move)
	{
		if (ob->distance < 0)
		{
			OpenDoor (-ob->distance-1);
			if (doorobjlist[-ob->distance-1].action != dr_open)
				return;
			ob->distance = TILEGLOBAL;	// go ahead, the door is now opoen
		}

		if (move < ob->distance)
		{
			MoveObj (ob,move);
			break;
		}
		ob->x = ((long)ob->tilex<<TILESHIFT)+TILEGLOBAL/2;
		ob->y = ((long)ob->tiley<<TILESHIFT)+TILEGLOBAL/2;

		move -= ob->distance;

		if (dist <4)
			SelectRunDir (ob);
		else if (dodge)
			SelectDodgeDir (ob);
		else
			SelectChaseDir (ob);

		if (ob->dir == nodir)
			return;							// object is blocked in
	}

}
void T_Mancubus (objtype *ob)
{
	long move;
	int	dx,dy,dist;
	boolean	dodge;

	dodge = false;
	dx = abs(ob->tilex - player->tilex);
	dy = abs(ob->tiley - player->tiley);
	dist = dx>dy ? dx : dy;

	if (CheckLine(ob))						// got a shot at player?
	{

		if ( US_RndT() < (tics<<3) )
		{
			NewState (ob,&s_mancubusshoot1);
			return;
		}
		dodge = true;
	}

	if (ob->dir == nodir)
	{
		if (dodge)
			SelectDodgeDir (ob);
		else
			SelectChaseDir (ob);
		if (ob->dir == nodir)
			return;							// object is blocked in
	}

	move = ob->speed*tics;

	while (move)
	{
		if (ob->distance < 0)
		{
			OpenDoor (-ob->distance-1);
			if (doorobjlist[-ob->distance-1].action != dr_open)
				return;
			ob->distance = TILEGLOBAL;	// go ahead, the door is now opoen
		}

		if (move < ob->distance)
		{
			MoveObj (ob,move);
			break;
		}
		ob->x = ((long)ob->tilex<<TILESHIFT)+TILEGLOBAL/2;
		ob->y = ((long)ob->tiley<<TILESHIFT)+TILEGLOBAL/2;

		move -= ob->distance;

		if (dist <4)
			SelectRunDir (ob);
		else if (dodge)
			SelectDodgeDir (ob);
		else
			SelectChaseDir (ob);

		if (ob->dir == nodir)
			return;							// object is blocked in
	}

}
void A_VileFire (objtype *ob);
void A_Victory (objtype *ob);
void A_VileFireSnd (objtype *ob);

extern  statetype s_vilechase1;
extern	statetype s_vilechase1s;
extern	statetype s_vilechase2;
extern	statetype s_vilechase3;
extern	statetype s_vilechase3s;
extern	statetype s_vilechase4;

extern	statetype s_viledie1;
extern	statetype s_viledie2;
extern	statetype s_viledie3;
extern	statetype s_viledie4;
extern	statetype s_viledie5;
extern	statetype s_viledie6;
extern	statetype s_viledie7;
extern	statetype s_viledie8;
extern	statetype s_viledie9;
extern	statetype s_viledie10;

extern	statetype s_vileshoot1;
extern	statetype s_vileshoot2;
extern	statetype s_vileshoot3;
extern	statetype s_vileshoot4;
extern	statetype s_vileshoot5;
extern	statetype s_vileshoot6;
extern	statetype s_vileshoot7;
extern	statetype s_vileshoot8;
extern	statetype s_vileshoot9;
extern	statetype s_vileshoot10;

statetype s_vilestand	= {true,SPR_VILE_W1_1,0,T_Stand,NULL,&s_vilestand};

statetype s_vilechase1	= {true,SPR_VILE_W1_1,10,T_Chase,NULL,&s_vilechase1s};
statetype s_vilechase1s	= {true,SPR_VILE_W2_1,6,NULL,NULL,&s_vilechase2};
statetype s_vilechase2	= {true,SPR_VILE_W3_1,8,T_Chase,NULL,&s_vilechase3};
statetype s_vilechase3	= {true,SPR_VILE_W4_1,10,T_Chase,NULL,&s_vilechase3s};
statetype s_vilechase3s	= {true,SPR_VILE_W5_1,6,NULL,NULL,&s_vilechase4};
statetype s_vilechase4	= {true,SPR_VILE_W6_1,8,T_Chase,NULL,&s_vilechase1};

statetype s_vileshoot1	= {false,SPR_VILE_SHOOT1_1,10,NULL,A_VileFireSnd,&s_vileshoot2};
statetype s_vileshoot2	= {false,SPR_VILE_SHOOT2_1,10,NULL,NULL,&s_vileshoot3};
statetype s_vileshoot3	= {false,SPR_VILE_SHOOT3_1,10,NULL,NULL,&s_vileshoot4};
statetype s_vileshoot4	= {false,SPR_VILE_SHOOT4_1,10,NULL,NULL,&s_vileshoot5};
statetype s_vileshoot5	= {false,SPR_VILE_SHOOT5_1,10,NULL,NULL,&s_vileshoot6};
statetype s_vileshoot6	= {false,SPR_VILE_SHOOT6_1,10,NULL,NULL,&s_vileshoot7};
statetype s_vileshoot7	= {false,SPR_VILE_SHOOT7_1,10,NULL,NULL,&s_vileshoot8};
statetype s_vileshoot8	= {false,SPR_VILE_SHOOT8_1,10,NULL,NULL,&s_vileshoot9};
statetype s_vileshoot9	= {false,SPR_VILE_SHOOT9_1,10,NULL,A_VileFire,&s_vileshoot10};
statetype s_vileshoot10	= {false,SPR_VILE_SHOOT10_1,10,NULL,NULL,&s_vilechase1};

statetype s_viledie1	= {true,SPR_VILE_DIE1_1,10,NULL,A_DeathScream,&s_viledie2};
statetype s_viledie2	= {false,SPR_VILE_DIE2,10,NULL,NULL,&s_viledie3};
statetype s_viledie3	= {false,SPR_VILE_DIE3,10,NULL,NULL,&s_viledie4};
statetype s_viledie4	= {false,SPR_VILE_DIE4,10,NULL,NULL,&s_viledie5};
statetype s_viledie5	= {false,SPR_VILE_DIE5,10,NULL,NULL,&s_viledie6};
statetype s_viledie6	= {false,SPR_VILE_DIE6,10,NULL,NULL,&s_viledie7};
statetype s_viledie7	= {false,SPR_VILE_DIE7,10,NULL,NULL,&s_viledie8};
statetype s_viledie8	= {false,SPR_VILE_DIE8,10,NULL,NULL,&s_viledie9};
statetype s_viledie9	= {false,SPR_VILE_DIE9,10,NULL,NULL,&s_viledie10};
statetype s_viledie10	= {false,SPR_VILE_DEAD,130,NULL,A_Victory,&s_viledie10};

void A_VileFireSnd (objtype *ob)
{
	PlaySoundLocActor (VILEATKSND,ob);
}

#pragma argsused
void A_Victory (objtype *ob)
{
	playstate = ex_victorious;
}

extern	statetype s_vilefire1;
extern	statetype s_vilefire2;
extern	statetype s_vilefire3;
extern	statetype s_vilefire4;
extern	statetype s_vilefire5;
extern	statetype s_vilefire6;
extern	statetype s_vilefire7;
extern	statetype s_vilefire8;

statetype s_vilefire1	= {false,SPR_VILE_FIRE1,10,T_Projectile,NULL,&s_vilefire2};
statetype s_vilefire2	= {false,SPR_VILE_FIRE2,10,T_Projectile,NULL,&s_vilefire3};
statetype s_vilefire3	= {false,SPR_VILE_FIRE3,10,T_Projectile,NULL,&s_vilefire4};
statetype s_vilefire4	= {false,SPR_VILE_FIRE4,10,T_Projectile,NULL,&s_vilefire5};
statetype s_vilefire5	= {false,SPR_VILE_FIRE5,10,T_Projectile,NULL,&s_vilefire6};
statetype s_vilefire6	= {false,SPR_VILE_FIRE6,10,T_Projectile,NULL,&s_vilefire7};
statetype s_vilefire7	= {false,SPR_VILE_FIRE7,10,T_Projectile,NULL,&s_vilefire8};
statetype s_vilefire8	= {false,SPR_VILE_FIRE8,10,T_Projectile,NULL,NULL};

void SpawnVile (int tilex, int tiley)
{
	unsigned	far *map,tile;

	SpawnNewObj (tilex,tiley,&s_vilestand);
	new->speed = SPDPATROL;

	new->obclass = vileobj;
	new->hitpoints = starthitpoints[gamestate.difficulty][en_vile];
	new->dir = nodir;
	new->flags |= FL_SHOOTABLE|FL_AMBUSH;
	if (!loadedgame)
	  gamestate.killtotal++;
}
void A_VileFire (objtype *ob)
{
	long deltax,deltay;
	float	angle;
	int	iangle;

	deltax = player->x - ob->x;
	deltay = ob->y - player->y;
	angle = atan2 (deltay,deltax);
	if (angle<0)
		angle = M_PI*2+angle;
	iangle = angle/(M_PI*2)*ANGLES;

	GetNewActor ();
	new->state = &s_vilefire1;
	new->ticcount = 1;

	new->tilex = ob->tilex;
	new->tiley = ob->tiley;
	new->x = ob->x;
	new->y = ob->y;
	new->obclass = vilefireobj;
	new->dir = nodir;
	new->angle = iangle;
	new->speed = 0x2000l;
}
void T_RevBallThrow (objtype *ob);

extern	statetype s_revenantchase1;
extern	statetype s_revenantchase1s;
extern	statetype s_revenantchase2;
extern	statetype s_revenantchase3;
extern	statetype s_revenantchase3s;
extern	statetype s_revenantchase4;

extern	statetype s_revenantdie1;
extern	statetype s_revenantdie2;
extern	statetype s_revenantdie3;
extern	statetype s_revenantdie4;
extern	statetype s_revenantdie5;
extern	statetype s_revenantdie6;
extern	statetype s_revenantdie7;

extern	statetype s_revenantshoot1;
extern	statetype s_revenantshoot2;
extern	statetype s_revenantshoot3;

statetype s_revenantstand	= {true,SPR_REVENANT_W1_1,0,T_Stand,NULL,&s_revenantstand};

statetype s_revenantchase1 	= {true,SPR_REVENANT_W1_1,10,T_Chase,NULL,&s_revenantchase1s};
statetype s_revenantchase1s	= {true,SPR_REVENANT_W2_1,6,NULL,NULL,&s_revenantchase2};
statetype s_revenantchase2 	= {true,SPR_REVENANT_W3_1,8,T_Chase,NULL,&s_revenantchase3};
statetype s_revenantchase3 	= {true,SPR_REVENANT_W4_1,10,T_Chase,NULL,&s_revenantchase3s};
statetype s_revenantchase3s	= {true,SPR_REVENANT_W5_1,6,NULL,NULL,&s_revenantchase4};
statetype s_revenantchase4 	= {true,SPR_REVENANT_W6_1,8,T_Chase,NULL,&s_revenantchase1};

statetype s_revenantdie1	= {true,SPR_REVENANT_DIE1_1,10,NULL,A_DeathScream,&s_revenantdie2};
statetype s_revenantdie2	= {true,SPR_REVENANT_DIE2_1,10,NULL,NULL,&s_revenantdie3};
statetype s_revenantdie3	= {false,SPR_REVENANT_DIE3,10,NULL,NULL,&s_revenantdie4};
statetype s_revenantdie4	= {false,SPR_REVENANT_DIE4,10,NULL,NULL,&s_revenantdie5};
statetype s_revenantdie5	= {false,SPR_REVENANT_DIE5,10,NULL,NULL,&s_revenantdie6};
statetype s_revenantdie6	= {false,SPR_REVENANT_DIE6,10,NULL,NULL,&s_revenantdie7};
statetype s_revenantdie7	= {false,SPR_REVENANT_DEAD,0,NULL,NULL,&s_revenantdie7};

statetype s_revenantshoot1 	= {false,SPR_REVENANT_SHOOT1_1,30,NULL,NULL,&s_revenantshoot2};
statetype s_revenantshoot2 	= {false,SPR_REVENANT_SHOOT4_1,10,NULL,T_RevBallThrow,&s_revenantshoot3};
statetype s_revenantshoot3	= {false,SPR_REVENANT_SHOOT4_1,10,NULL,T_RevBallThrow,&s_revenantchase1};

void T_RevBallThrow (objtype *ob)
{
   long   deltax,deltay;
   float   angle;
   int      iangle;

   deltax = player->x - ob->x;
   deltay = ob->y - player->y;
   angle = atan2 (deltay,deltax);
   if (angle<0)
      angle = M_PI*2+angle;
   iangle = angle/(M_PI*2)*ANGLES;

   GetNewActor ();
   new->state = &s_revball1;
   new->ticcount = 1;

   new->tilex = ob->tilex;
   new->tiley = ob->tiley;
   new->x = ob->x;
   new->y = ob->y;
   new->obclass = revballobj;
   new->dir = nodir;
   new->angle = iangle;
   new->speed = 0x2000l;

   new->flags = FL_NONMARK;
   new->active = true;

   PlaySoundLocActor (RLAUNCHERFIRESND,new);   //Change to the sound you want.
} 

void SpawnRevenant (int tilex, int tiley)
{
	unsigned	far *map,tile;

	SpawnNewObj (tilex,tiley,&s_revenantstand);
	new->speed = SPDPATROL;

	new->obclass = revenantobj;
	new->hitpoints = starthitpoints[gamestate.difficulty][en_revenant];
	new->dir = nodir;
	new->flags |= FL_SHOOTABLE|FL_AMBUSH;
	if (!loadedgame)
	  gamestate.killtotal++;
}

void A_MechaSound (objtype *ob)
{
	if (areabyplayer[ob->areanumber])
		PlaySoundLocActor (MOVEUPDOWNSND,ob);
}

//
// boss
//
void T_BossObjThrow (objtype *ob);

extern	statetype s_bossstand;

extern	statetype s_bosschase1;
extern	statetype s_bosschase1s;
extern	statetype s_bosschase2;
extern	statetype s_bosschase3;
extern	statetype s_bosschase3s;
extern	statetype s_bosschase4;

extern	statetype s_bossdie1;
extern	statetype s_bossdie2;
extern	statetype s_bossdie3;
extern	statetype s_bossdie4;
extern	statetype s_bossdie5;
extern	statetype s_bossdie6;
extern	statetype s_bossdie7;
extern	statetype s_bossdie8;
extern	statetype s_bossdie9;
extern	statetype s_bossdie10;

extern	statetype s_bossshoot1;
extern	statetype s_bossshoot2;
extern	statetype s_bossshoot3;

statetype s_bossstand	= {true,SPR_BOSS_W1_1,0,T_Stand,NULL,&s_bossstand};
statetype s_bosschase1 	= {true,SPR_BOSS_W1_1,6,T_Chase,NULL,&s_bosschase1s};
statetype s_bosschase1s	= {true,SPR_BOSS_W1_1,4,NULL,NULL,&s_bosschase2};
statetype s_bosschase2 	= {true,SPR_BOSS_W2_1,2,T_Chase,NULL,&s_bosschase3};
statetype s_bosschase3 	= {true,SPR_BOSS_W3_1,6,T_Chase,NULL,&s_bosschase3s};
statetype s_bosschase3s	= {true,SPR_BOSS_W3_1,4,NULL,NULL,&s_bosschase4};
statetype s_bosschase4 	= {true,SPR_BOSS_W4_1,2,T_Chase,NULL,&s_bosschase1};

statetype s_bossdie1	= {true,SPR_BOSS_W1_1,1,NULL,A_DeathScream,&s_bossdie2};
statetype s_bossdie2	= {false,SPR_BOSS_W1_1,10,NULL,NULL,&s_bossdie3};
statetype s_bossdie3	= {false,SPR_BOSS_DIE_1,10,NULL,A_Slurpie,&s_bossdie4};
statetype s_bossdie4	= {false,SPR_BOSS_DIE2,10,NULL,NULL,&s_bossdie5};
statetype s_bossdie5	= {false,SPR_BOSS_DIE3,10,NULL,NULL,&s_bossdie6};
statetype s_bossdie6	= {false,SPR_BOSS_DIE4,10,NULL,NULL,&s_bossdie7};
statetype s_bossdie7	= {false,SPR_BOSS_DIE5,10,NULL,NULL,&s_bossdie8};
statetype s_bossdie8	= {false,SPR_BOSS_DIE6,10,NULL,NULL,&s_bossdie9};
statetype s_bossdie9	= {false,SPR_BOSS_DIE7,10,NULL,NULL,&s_bossdie10};
statetype s_bossdie10	= {false,SPR_BOSS_DEAD,20,NULL,NULL,&s_bossdie10};

statetype s_bossshoot1 	= {false,SPR_BOSS_SHOOT_1,30,NULL,NULL,&s_bossshoot2};
statetype s_bossshoot2 	= {false,SPR_BOSS_SHOOT2_1,10,NULL,NULL,&s_bossshoot3};
statetype s_bossshoot3 	= {false,SPR_BOSS_SHOOT3_1,10,NULL,T_BossObjThrow,&s_bosschase1};

void SpawnBoss (int tilex, int tiley)
{
	unsigned	far *map,tile;

	SpawnNewObj (tilex,tiley,&s_bossstand);
	new->speed = SPDPATROL;

	new->obclass = bossobj;
	new->hitpoints = starthitpoints[gamestate.difficulty][en_boss];
	new->dir = nodir;
	new->flags |= FL_SHOOTABLE|FL_AMBUSH;
	if (!loadedgame)
	  gamestate.killtotal++;
}
//
// boss2
//
void T_BossObjThrow (objtype *ob);

extern	statetype s_boss2stand;

extern	statetype s_boss2chase1;
extern	statetype s_boss2chase1s;
extern	statetype s_boss2chase2;
extern	statetype s_boss2chase3;
extern	statetype s_boss2chase3s;
extern	statetype s_boss2chase4;

extern	statetype s_boss2die1;
extern	statetype s_boss2die2;
extern	statetype s_boss2die3;
extern	statetype s_boss2die4;
extern	statetype s_boss2die5;
extern	statetype s_boss2die6;
extern	statetype s_boss2die7;
extern	statetype s_boss2die8;
extern	statetype s_boss2die9;
extern	statetype s_boss2die10;

extern	statetype s_boss2shoot1;
extern	statetype s_boss2shoot2;
extern	statetype s_boss2shoot3;

statetype s_boss2stand	= {true,SPR_BOSS2_W1_1,0,T_Stand,NULL,&s_boss2stand};
statetype s_boss2chase1 	= {true,SPR_BOSS2_W1_1,6,T_Chase,NULL,&s_boss2chase1s};
statetype s_boss2chase1s	= {true,SPR_BOSS2_W1_1,4,NULL,NULL,&s_boss2chase2};
statetype s_boss2chase2 	= {true,SPR_BOSS2_W2_1,2,T_Chase,NULL,&s_boss2chase3};
statetype s_boss2chase3 	= {true,SPR_BOSS2_W3_1,6,T_Chase,NULL,&s_boss2chase3s};
statetype s_boss2chase3s	= {true,SPR_BOSS2_W3_1,4,NULL,NULL,&s_boss2chase4};
statetype s_boss2chase4 	= {true,SPR_BOSS2_W4_1,2,T_Chase,NULL,&s_boss2chase1};

statetype s_boss2die1	= {true,SPR_BOSS2_W1_1,1,NULL,A_DeathScream,&s_boss2die2};
statetype s_boss2die2	= {false,SPR_BOSS2_W1_1,10,NULL,NULL,&s_boss2die3};
statetype s_boss2die3	= {false,SPR_BOSS2_DIE_1,10,NULL,A_Slurpie,&s_boss2die4};
statetype s_boss2die4	= {false,SPR_BOSS2_DIE2,10,NULL,NULL,&s_boss2die5};
statetype s_boss2die5	= {false,SPR_BOSS2_DIE3,10,NULL,NULL,&s_boss2die6};
statetype s_boss2die6	= {false,SPR_BOSS2_DIE4,10,NULL,NULL,&s_boss2die7};
statetype s_boss2die7	= {false,SPR_BOSS2_DIE5,10,NULL,NULL,&s_boss2die8};
statetype s_boss2die8	= {false,SPR_BOSS2_DIE6,10,NULL,NULL,&s_boss2die9};
statetype s_boss2die9	= {false,SPR_BOSS2_DIE7,10,NULL,NULL,&s_boss2die10};
statetype s_boss2die10	= {false,SPR_BOSS2_DEAD,20,NULL,NULL,&s_boss2die10};

statetype s_boss2shoot1 	= {false,SPR_BOSS2_SHOOT_1,30,NULL,NULL,&s_boss2shoot2};
statetype s_boss2shoot2 	= {false,SPR_BOSS2_SHOOT2_1,10,NULL,NULL,&s_boss2shoot3};
statetype s_boss2shoot3 	= {false,SPR_BOSS2_SHOOT3_1,10,NULL,T_BossObjThrow,&s_boss2chase1};

void SpawnBoss2 (int tilex, int tiley)
{
	unsigned	far *map,tile;

	SpawnNewObj (tilex,tiley,&s_boss2stand);
	new->speed = SPDPATROL;

	new->obclass = boss2obj;
	new->hitpoints = starthitpoints[gamestate.difficulty][en_boss2];
	new->dir = nodir;
	new->flags |= FL_SHOOTABLE|FL_AMBUSH;
	if (!loadedgame)
	  gamestate.killtotal++;
}
//
// boss projectile
//
extern	statetype s_bossballobj;
extern	statetype s_bossballobj2;
extern	statetype s_bossballboom1;
extern	statetype s_bossballboom2;
extern	statetype s_bossballboom3;

statetype s_bossballobj	= {true,SPR_GREEN_1,6,T_Projectile,NULL,&s_bossballobj2};
statetype s_bossballobj2	= {true,SPR_GREEN2_1,6,T_Projectile,NULL,&s_bossballobj};

statetype s_bossballboom1	= {false,SPR_GREEN_BOOM_1,6,NULL,NULL,&s_bossballboom2};
statetype s_bossballboom2	= {false,SPR_GREEN_BOOM_2,6,NULL,NULL,&s_bossballboom3};
statetype s_bossballboom3	= {false,SPR_GREEN_BOOM_3,6,NULL,NULL,NULL};

void T_BossObjThrow (objtype *ob)
{
	long deltax,deltay;
	float	angle;
	int	iangle;

	deltax = player->x - ob->x;
	deltay = ob->y - player->y;
	angle = atan2 (deltay,deltax);
	if (angle<0)
		angle = M_PI*2+angle;
	iangle = angle/(M_PI*2)*ANGLES;

	GetNewActor ();
	new->state = &s_bossballobj;
	new->ticcount = 1;

	new->tilex = ob->tilex;
	new->tiley = ob->tiley;
	new->x = ob->x;
	new->y = ob->y;
	new->obclass = bossballobj;
	new->dir = nodir;
	new->angle = iangle;
	new->speed = 0x2000l;

	PlaySoundLocActor (IMPFIRESND,new);
}

#pragma argsused
void A_Slurpie (objtype *ob)
{
 SD_PlaySound(SLOPSND);
}

void T_Stand (objtype *ob)
{
	SightPlayer (ob);
}

void T_Chase (objtype *ob)
{
	long move;
	int	dx,dy,dist,chance;
	boolean	dodge;

	if (gamestate.victoryflag)
		return;

	dodge = false;
	if (CheckLine(ob))
	{
		dx = abs(ob->tilex - player->tilex);
		dy = abs(ob->tiley - player->tiley);
		dist = dx>dy ? dx : dy;
		if (!dist || (dist==1 && ob->distance<0x4000) )
			chance = 300;
		else
			chance = (tics<<4)/dist;
		if (gamestate.invisibility && dist>1) goto invisible;
		if ( US_RndT()<chance)
		{
			switch (ob->obclass)
			{
			case mutantobj:
				NewState (ob,&s_mutantshoot1);
				break;
			case chainguyobj:
				NewState (ob,&s_chainguyshoot1);
				break;
			case shotguyobj:
				NewState (ob,&s_shotguyshoot1);
				break;
			case cacoobj:
				NewState (ob,&s_cacoshoot1);
				break;
			case cyberobj:
				NewState (ob,&s_cybershoot1);
				break;
			case revenantobj:
				NewState (ob,&s_revenantshoot1);
				break;
			case bossobj:
				NewState (ob,&s_bossshoot1);
				break;
			case boss2obj:
				NewState (ob,&s_boss2shoot1);
				break;
			case impobj:
				NewState (ob,&s_impshoot1);
				break;
			case painobj:
				NewState (ob,&s_painshoot1);
				break;
			case soulobj:
				NewState (ob,&s_soulshoot1);
				break;
			case vileobj:
				NewState (ob,&s_vileshoot1);
				break;
			}
			return;
		}
		dodge = true;
	}
	invisible:
	if (ob->dir == nodir)
	{
		if (dodge)
			SelectDodgeDir (ob);
		else
			SelectChaseDir (ob);
		if (ob->dir == nodir)
			return;							// object is blocked in
	}

	move = ob->speed*tics;

	while (move)
	{
		if (ob->distance < 0)
		{
			OpenDoor (-ob->distance-1);
			if (doorobjlist[-ob->distance-1].action != dr_open)
				return;
			ob->distance = TILEGLOBAL;	// go ahead, the door is now opoen
		}

		if (move < ob->distance)
		{
			MoveObj (ob,move);
			break;
		}
		ob->x = ((long)ob->tilex<<TILESHIFT)+TILEGLOBAL/2;
		ob->y = ((long)ob->tiley<<TILESHIFT)+TILEGLOBAL/2;

		move -= ob->distance;

		if (dodge)
			SelectDodgeDir (ob);
		else
			SelectChaseDir (ob);

		if (ob->dir == nodir)
			return;							// object is blocked in
	}

}

void T_DemonChase (objtype *ob)
{
	long 	move;
	int		dist,chance;
	long	dx,dy;


	if (ob->dir == nodir)
	{
		SelectDodgeDir (ob);
		if (ob->dir == nodir)
			return;							// object is blocked in
	}

	move = ob->speed*tics;

	while (move)
	{
		dx = player->x - ob->x;
		if (dx<0)
			dx = -dx;
		dx -= move;
		if (dx <= MINACTORDIST)
		{
			dy = player->y - ob->y;
			if (dy<0)
				dy = -dy;
			dy -= move;
			if (dy <= MINACTORDIST)
			{
			switch (ob->obclass)
			{
			case demonobj:
				NewState (ob,&s_demonjump1);
				break;
			case soulobj:
				NewState (ob,&s_soulshoot1);
				break;
			}
			return;
			}
		}

		if (move < ob->distance)
		{
			MoveObj (ob,move);
			break;
		}
		ob->x = ((long)ob->tilex<<TILESHIFT)+TILEGLOBAL/2;
		ob->y = ((long)ob->tiley<<TILESHIFT)+TILEGLOBAL/2;

		move -= ob->distance;

		SelectDodgeDir (ob);

		if (ob->dir == nodir)
			return;							// object is blocked in
	}

}

void SelectPathDir (objtype *ob)
{
	unsigned spot;

	spot = MAPSPOT(ob->tilex,ob->tiley,1)-ICONARROWS;

	if (spot<8)
	{
	// new direction
		ob->dir = spot;
	}

	ob->distance = TILEGLOBAL;

	if (!TryWalk (ob))
		ob->dir = nodir;
}

void T_Path (objtype *ob)
{
	long 	move;
	long 	deltax,deltay,size;

	if (SightPlayer (ob))
		return;

	if (ob->dir == nodir)
	{
		SelectPathDir (ob);
		if (ob->dir == nodir)
			return;					// all movement is blocked
	}


	move = ob->speed*tics;

	while (move)
	{
		if (ob->distance < 0)
		{
			OpenDoor (-ob->distance-1);
			if (doorobjlist[-ob->distance-1].action != dr_open)
				return;
			ob->distance = TILEGLOBAL;	// go ahead, the door is now opoen
		}

		if (move < ob->distance)
		{
			MoveObj (ob,move);
			break;
		}

		if (ob->tilex>MAPSIZE || ob->tiley>MAPSIZE)
		{
			sprintf (str,"T_Path hit a wall at %u,%u, dir %u"
			,ob->tilex,ob->tiley,ob->dir);
			Quit (str);
		}
		ob->x = ((long)ob->tilex<<TILESHIFT)+TILEGLOBAL/2;
		ob->y = ((long)ob->tiley<<TILESHIFT)+TILEGLOBAL/2;
		move -= ob->distance;

		SelectPathDir (ob);

		if (ob->dir == nodir)
			return;					// all movement is blocked
	}
}

void T_Shoot (objtype *ob)
{
	int	dx,dy,dist;
	int	hitchance,damage;

	hitchance = 128;

	if (!areabyplayer[ob->areanumber])
		return;

	if (!CheckLine (ob))			// player is behind a wall
	  return;

	dx = abs(ob->tilex - player->tilex);
	dy = abs(ob->tiley - player->tiley);
	dist = dx>dy ? dx:dy;

	if (thrustspeed >= RUNSPEED)
	{
		if (ob->flags&FL_VISABLE)
			hitchance = 160-dist*16;		// player can see to dodge
		else
			hitchance = 160-dist*8;
	}
	else
	{
		if (ob->flags&FL_VISABLE)
			hitchance = 256-dist*16;		// player can see to dodge
		else
			hitchance = 256-dist*8;
	}

	if (US_RndT()<hitchance)
	{
		if (dist<2)
			damage = US_RndT()>>2;
		else if (dist<4)
			damage = US_RndT()>>3;
		else
			damage = US_RndT()>>4;

		TakeDamage (damage,ob);
	}

	switch(ob->obclass)
	{
	 case shotguyobj:
	 case chainguyobj:
	   PlaySoundLocActor(SHOTGUNFIRESND,ob);
	   break;
	 default:
	   PlaySoundLocActor(PISTOLFIRESND,ob);
	}

}

void T_Bite (objtype *ob)
{
	long	dx,dy;
	int	hitchance,damage;

	switch (ob->obclass)
	{
	case demonobj:
		PlaySoundLocActor(DEMONFIRESND,ob);	// JAB
		break;
	case soulobj:
		PlaySoundLocActor(SOULATKSND,ob);	// JAB
		break;
	}

	dx = player->x - ob->x;
	if (dx<0)
		dx = -dx;
	dx -= TILEGLOBAL;
	if (dx <= MINACTORDIST)
	{
		dy = player->y - ob->y;
		if (dy<0)
			dy = -dy;
		dy -= TILEGLOBAL;
		if (dy <= MINACTORDIST)
		{
		   if (US_RndT()<180)
		   {
			   TakeDamage (US_RndT()>>4,ob);
			   return;
		   }
		}
	}

	return;
}

boolean	CheckPosition (objtype *ob)
{
	int	x,y,xl,yl,xh,yh;
	objtype *check;

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

	return true;
}
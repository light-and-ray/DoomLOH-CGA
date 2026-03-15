#include "ID_HEADS.H"
#include <MATH.H>
#include <VALUES.H>

#include "WL_MENU.H"
#include "FOREIGN.H"

#define COLORBORDER(color)		asm{mov	dx,STATUS_REGISTER_1;in al,dx;\
	mov dx,ATR_INDEX;mov al,ATR_OVERSCAN;out dx,al;mov al,color;out	dx,al;\
	mov	al,32;out dx,al};

#define MAPSPOT(x,y,plane)		(*(mapsegs[plane]+farmapylookup[y]+x))

#define SIGN(x) 	((x)>0?1:-1)
#define ABS(x) 		((int)(x)>0?(x):-(x))
#define LABS(x) 	((long)(x)>0?(x):-(x))

#define SHADE_COUNT    32 // 32 Shades for each color
#define LSHADE_NORMAL 0
#define LSHADE_FOG 10

#define MAXACTORS		150				// max number of nazis, etc / map
#define MAXSTATS		400				// max number of lamps, bonus, etc
#define MAXDOORS		64				// max number of sliding doors
#define MAXWALLTILES	90				// max number of wall tiles

#define	ICONARROWS		1
#define PUSHABLETILE	9
#define EXITTILE		10				// at end of castle
#define ENDLEVELTILE	16
#define AREATILE		107				// first of NUMAREAS floor tiles
#define NUMAREAS		37
#define ELEVATORTILE	11
#define AMBUSHTILE		106
#define	ALTELEVATORTILE	107
#define WARPEASTTILE 11
#define WARPWESTTILE 13
#define SWITCHTILE	2
extern byte pwx,pwy,pw2x,pw2y,pw3x,pw3y,pw4x,pw4y;

#define NUMBERCHARS	9


//----------------

#define EXTRAPOINTS		40000

#define PLAYERSPEED		3000
#define RUNSPEED   		6000

#define	SCREENSEG		0xa000

#define SCREENBWIDE		80

#define HEIGHTRATIO		0.50		// also defined in id_mm.c

#define BORDERCOLOR	3
#define FLASHCOLOR	5
#define FLASHTICS	4


#define PLAYERSIZE		MINDIST			// player radius
#define MINACTORDIST	0x10000l		// minimum dist from player center
										// to any actor center

#define NUMLATCHPICS	100

#define MAXAMMO		200
#define MAXAMMO2        50
#define MAXAMMO3	50
#define MAXAMMO4	300

#define PI	3.141592657

#define GLOBAL1		(1l<<16)
#define TILEGLOBAL  GLOBAL1
#define PIXGLOBAL	(GLOBAL1/64)
#define TILESHIFT		16l
#define UNSIGNEDSHIFT	8

#define ANGLES		360					// must be divisable by 4
#define ANGLEQUAD	(ANGLES/4)
#define FINEANGLES	3600
#define ANG90		(FINEANGLES/4)
#define ANG180		(ANG90*2)
#define ANG270		(ANG90*3)
#define ANG360		(ANG90*4)
#define VANG90		(ANGLES/4)
#define VANG180		(VANG90*2)
#define VANG270		(VANG90*3)
#define VANG360		(VANG90*4)

#define MINDIST		(0x5800l)


#define	MAXSCALEHEIGHT	256				// largest scale on largest view

#define MAXVIEWWIDTH		320

#define MAPSIZE		64					// maps are 64*64 max
#define NORTH	0
#define EAST	1
#define SOUTH	2
#define WEST	3


#define STATUSLINES		32

#define SCREENSIZE		(SCREENBWIDE*208)
#define PAGE1START		0
#define PAGE2START		(SCREENSIZE)
#define PAGE3START		(SCREENSIZE*2u)
#define	FREESTART		(SCREENSIZE*3u)


#define PIXRADIUS		512

// object flag values

#define FL_SHOOTABLE	1
#define FL_BONUS		2
#define FL_NEVERMARK	4
#define FL_VISABLE		8
#define FL_ATTACKMODE	16
#define FL_FIRSTATTACK	32
#define FL_AMBUSH		64
#define FL_NONMARK		128
#define FL_DIRSOUTH 16
#define FL_DIREAST 32
#define FL_DIRFORWARD 64
#define FL_DIRBACKWARD 128


//
// sprite constants
//

enum	{
		SPR_DEMO,
		SPR_DEATHCAM,
//
// static sprites
//
		SPR_STAT_0,SPR_STAT_1,SPR_STAT_2,SPR_STAT_3,
		SPR_STAT_4,SPR_STAT_5,SPR_STAT_6,SPR_STAT_7,

		SPR_STAT_8,SPR_STAT_9,SPR_STAT_10,SPR_STAT_11,
		SPR_STAT_12,SPR_STAT_13,SPR_STAT_14,SPR_STAT_15,

		SPR_STAT_16,SPR_STAT_17,SPR_STAT_18,SPR_STAT_19,
		SPR_STAT_20,SPR_STAT_21,SPR_STAT_22,SPR_STAT_23,

		SPR_STAT_24,SPR_STAT_25,SPR_STAT_26,SPR_STAT_27,
		SPR_STAT_28,SPR_STAT_29,SPR_STAT_30,SPR_STAT_31,

		SPR_STAT_32,SPR_STAT_33,SPR_STAT_34,SPR_STAT_35,
		SPR_STAT_36,SPR_STAT_37,SPR_STAT_38,SPR_STAT_39,

		SPR_STAT_40,SPR_STAT_41,SPR_STAT_42,SPR_STAT_43,
		SPR_STAT_44,SPR_STAT_45,SPR_STAT_46,SPR_STAT_47,

		SPR_STAT_48,SPR_STAT_49,SPR_STAT_50,SPR_STAT_51,
		SPR_STAT_51A,SPR_STAT_52,SPR_STAT_53,SPR_STAT_54,

		SPR_STAT_55,SPR_STAT_56,SPR_STAT_57,SPR_STAT_58,
		SPR_STAT_59,SPR_STAT_60,SPR_STAT_61,

//
// Rocket, smoke and small explosion
//
		SPR_ROCKET_1,SPR_ROCKET_2,SPR_ROCKET_3,SPR_ROCKET_4,
		SPR_ROCKET_5,SPR_ROCKET_6,SPR_ROCKET_7,SPR_ROCKET_8,

		SPR_BOOM_1,SPR_BOOM_2,SPR_BOOM_3,

		SPR_BFG_1,SPR_BFG_2,SPR_BFG_BOOM1,SPR_BFG_BOOM2,
		SPR_BFG_BOOM3,SPR_BFG_BOOM4,SPR_BFG_BOOM5,SPR_BFG_BOOM6,

		SPR_PLASMA_1,SPR_PLASMA_2,SPR_PLASMA_BOOM1,SPR_PLASMA_BOOM2,SPR_PLASMA_BOOM3,
		SPR_PLASMA_BOOM4,

		SPR_FIRE_1,SPR_FIRE_2,SPR_FIRE_BOOM1,SPR_FIRE_BOOM2,
		SPR_FIRE_BOOM3,

		SPR_FIRE2_1,SPR_FIRE2_2,SPR_FIRE2_BOOM1,SPR_FIRE2_BOOM2,
		SPR_FIRE2_BOOM3,


//
// mutant
//
		SPR_MUTANT_W1_1,SPR_MUTANT_W1_2,SPR_MUTANT_W1_3,SPR_MUTANT_W1_4,
		SPR_MUTANT_W1_5,SPR_MUTANT_W1_6,SPR_MUTANT_W1_7,SPR_MUTANT_W1_8,

		SPR_MUTANT_W2_1,SPR_MUTANT_W2_2,SPR_MUTANT_W2_3,SPR_MUTANT_W2_4,
		SPR_MUTANT_W2_5,SPR_MUTANT_W2_6,SPR_MUTANT_W2_7,SPR_MUTANT_W2_8,

		SPR_MUTANT_W3_1,SPR_MUTANT_W3_2,SPR_MUTANT_W3_3,SPR_MUTANT_W3_4,
		SPR_MUTANT_W3_5,SPR_MUTANT_W3_6,SPR_MUTANT_W3_7,SPR_MUTANT_W3_8,

		SPR_MUTANT_W4_1,SPR_MUTANT_W4_2,SPR_MUTANT_W4_3,SPR_MUTANT_W4_4,
		SPR_MUTANT_W4_5,SPR_MUTANT_W4_6,SPR_MUTANT_W4_7,SPR_MUTANT_W4_8,

		SPR_MUTANT_PAIN_1,SPR_MUTANT_PAIN_2,SPR_MUTANT_PAIN_3,SPR_MUTANT_PAIN_4,
		SPR_MUTANT_PAIN_5,SPR_MUTANT_PAIN_6,SPR_MUTANT_PAIN_7,SPR_MUTANT_PAIN_8,

		SPR_MUTANT_DIE_1,SPR_MUTANT_DIE_2,SPR_MUTANT_DIE_3,SPR_MUTANT_DIE_4,
		SPR_MUTANT_DEAD,

		SPR_MUTANT_DIE2_1,SPR_MUTANT_DIE2_2,SPR_MUTANT_DIE2_3,SPR_MUTANT_DIE2_4,
		SPR_MUTANT_DIE2_5,SPR_MUTANT_DIE2_6,SPR_MUTANT_DIE2_7,SPR_MUTANT_DIE2_8,
		SPR_MUTANT_DIE2_9,

		SPR_MUTANT_SHOOT_1,SPR_MUTANT_SHOOT_2,SPR_MUTANT_SHOOT_3,SPR_MUTANT_SHOOT_4,
		SPR_MUTANT_SHOOT_5,SPR_MUTANT_SHOOT_6,SPR_MUTANT_SHOOT_7,SPR_MUTANT_SHOOT_8,

		SPR_MUTANT_SHOOT2_1,SPR_MUTANT_SHOOT2_2,SPR_MUTANT_SHOOT2_3,SPR_MUTANT_SHOOT2_4,
		SPR_MUTANT_SHOOT2_5,SPR_MUTANT_SHOOT2_6,SPR_MUTANT_SHOOT2_7,SPR_MUTANT_SHOOT2_8,

//
// demon
//
		SPR_DEMON_W1_1,SPR_DEMON_W1_2,SPR_DEMON_W1_3,SPR_DEMON_W1_4,
		SPR_DEMON_W1_5,SPR_DEMON_W1_6,SPR_DEMON_W1_7,SPR_DEMON_W1_8,

		SPR_DEMON_W2_1,SPR_DEMON_W2_2,SPR_DEMON_W2_3,SPR_DEMON_W2_4,
		SPR_DEMON_W2_5,SPR_DEMON_W2_6,SPR_DEMON_W2_7,SPR_DEMON_W2_8,

		SPR_DEMON_W3_1,SPR_DEMON_W3_2,SPR_DEMON_W3_3,SPR_DEMON_W3_4,
		SPR_DEMON_W3_5,SPR_DEMON_W3_6,SPR_DEMON_W3_7,SPR_DEMON_W3_8,

		SPR_DEMON_W4_1,SPR_DEMON_W4_2,SPR_DEMON_W4_3,SPR_DEMON_W4_4,
		SPR_DEMON_W4_5,SPR_DEMON_W4_6,SPR_DEMON_W4_7,SPR_DEMON_W4_8,

		SPR_DEMON_DIE_1,SPR_DEMON_DIE_2,SPR_DEMON_DIE_3,SPR_DEMON_DIE_4,
		SPR_DEMON_DIE_5,SPR_DEMON_DEAD,

		SPR_DEMON_JUMP1,SPR_DEMON_JUMP2,SPR_DEMON_JUMP3,



//
// cacodemon
//
		SPR_CACO_W1_1,SPR_CACO_W1_2,SPR_CACO_W1_3,SPR_CACO_W1_4,
		SPR_CACO_W1_5,SPR_CACO_W1_6,SPR_CACO_W1_7,SPR_CACO_W1_8,

		SPR_CACO_PAIN_1,SPR_CACO_PAIN_2,SPR_CACO_PAIN_3,SPR_CACO_PAIN_4,
		SPR_CACO_PAIN_5,SPR_CACO_PAIN_6,SPR_CACO_PAIN_7,SPR_CACO_PAIN_8,

		SPR_CACO_DIE_1,SPR_CACO_DIE_2,SPR_CACO_DIE_3,SPR_CACO_DIE_4,
		SPR_CACO_DIE_5,SPR_CACO_DEAD,

		SPR_CACO_SHOOT1,SPR_CACO_SHOOT2,SPR_CACO_SHOOT3,SPR_CACO_SHOOT4,

//
// shotgun guy
//
		SPR_SHOTGUY_W1_1,SPR_SHOTGUY_W1_2,SPR_SHOTGUY_W1_3,SPR_SHOTGUY_W1_4,
		SPR_SHOTGUY_W1_5,SPR_SHOTGUY_W1_6,SPR_SHOTGUY_W1_7,SPR_SHOTGUY_W1_8,

		SPR_SHOTGUY_W2_1,SPR_SHOTGUY_W2_2,SPR_SHOTGUY_W2_3,SPR_SHOTGUY_W2_4,
		SPR_SHOTGUY_W2_5,SPR_SHOTGUY_W2_6,SPR_SHOTGUY_W2_7,SPR_SHOTGUY_W2_8,

		SPR_SHOTGUY_W3_1,SPR_SHOTGUY_W3_2,SPR_SHOTGUY_W3_3,SPR_SHOTGUY_W3_4,
		SPR_SHOTGUY_W3_5,SPR_SHOTGUY_W3_6,SPR_SHOTGUY_W3_7,SPR_SHOTGUY_W3_8,

		SPR_SHOTGUY_W4_1,SPR_SHOTGUY_W4_2,SPR_SHOTGUY_W4_3,SPR_SHOTGUY_W4_4,
		SPR_SHOTGUY_W4_5,SPR_SHOTGUY_W4_6,SPR_SHOTGUY_W4_7,SPR_SHOTGUY_W4_8,

		SPR_SHOTGUY_PAIN_1,SPR_SHOTGUY_PAIN_2,SPR_SHOTGUY_PAIN_3,SPR_SHOTGUY_PAIN_4,
		SPR_SHOTGUY_PAIN_5,SPR_SHOTGUY_PAIN_6,SPR_SHOTGUY_PAIN_7,SPR_SHOTGUY_PAIN_8,

		SPR_SHOTGUY_DIE_1,SPR_SHOTGUY_DIE_2,SPR_SHOTGUY_DIE_3,SPR_SHOTGUY_DIE_4,
		SPR_SHOTGUY_DEAD,

		SPR_SHOTGUY_DIE2_1,SPR_SHOTGUY_DIE2_2,SPR_SHOTGUY_DIE2_3,SPR_SHOTGUY_DIE2_4,
		SPR_SHOTGUY_DIE2_5,SPR_SHOTGUY_DIE2_6,SPR_SHOTGUY_DIE2_7,SPR_SHOTGUY_DIE2_8,
		SPR_SHOTGUY_DIE2_9,

		SPR_SHOTGUY_SHOOT_1,SPR_SHOTGUY_SHOOT_2,SPR_SHOTGUY_SHOOT_3,SPR_SHOTGUY_SHOOT_4,
		SPR_SHOTGUY_SHOOT_5,SPR_SHOTGUY_SHOOT_6,SPR_SHOTGUY_SHOOT_7,SPR_SHOTGUY_SHOOT_8,

		SPR_SHOTGUY_SHOOT2_1,SPR_SHOTGUY_SHOOT2_2,SPR_SHOTGUY_SHOOT2_3,SPR_SHOTGUY_SHOOT2_4,
		SPR_SHOTGUY_SHOOT2_5,SPR_SHOTGUY_SHOOT2_6,SPR_SHOTGUY_SHOOT2_7,SPR_SHOTGUY_SHOOT2_8,

//
// chaingun guy
//
		SPR_CHAINGUY_W1_1,SPR_CHAINGUY_W1_2,SPR_CHAINGUY_W1_3,SPR_CHAINGUY_W1_4,
		SPR_CHAINGUY_W1_5,SPR_CHAINGUY_W1_6,SPR_CHAINGUY_W1_7,SPR_CHAINGUY_W1_8,

		SPR_CHAINGUY_W2_1,SPR_CHAINGUY_W2_2,SPR_CHAINGUY_W2_3,SPR_CHAINGUY_W2_4,
		SPR_CHAINGUY_W2_5,SPR_CHAINGUY_W2_6,SPR_CHAINGUY_W2_7,SPR_CHAINGUY_W2_8,

		SPR_CHAINGUY_W3_1,SPR_CHAINGUY_W3_2,SPR_CHAINGUY_W3_3,SPR_CHAINGUY_W3_4,
		SPR_CHAINGUY_W3_5,SPR_CHAINGUY_W3_6,SPR_CHAINGUY_W3_7,SPR_CHAINGUY_W3_8,

		SPR_CHAINGUY_W4_1,SPR_CHAINGUY_W4_2,SPR_CHAINGUY_W4_3,SPR_CHAINGUY_W4_4,
		SPR_CHAINGUY_W4_5,SPR_CHAINGUY_W4_6,SPR_CHAINGUY_W4_7,SPR_CHAINGUY_W4_8,

		SPR_CHAINGUY_PAIN_1,SPR_CHAINGUY_PAIN_2,SPR_CHAINGUY_PAIN_3,SPR_CHAINGUY_PAIN_4,
		SPR_CHAINGUY_PAIN_5,SPR_CHAINGUY_PAIN_6,SPR_CHAINGUY_PAIN_7,SPR_CHAINGUY_PAIN_8,

		SPR_CHAINGUY_DIE_1,SPR_CHAINGUY_DIE_2,SPR_CHAINGUY_DIE_3,SPR_CHAINGUY_DIE_4,
		SPR_CHAINGUY_DIE_5,SPR_CHAINGUY_DIE_6,SPR_CHAINGUY_DEAD,

		SPR_CHAINGUY_DIE2_1,SPR_CHAINGUY_DIE2_2,SPR_CHAINGUY_DIE2_3,SPR_CHAINGUY_DIE2_4,
		SPR_CHAINGUY_DIE2_5,SPR_CHAINGUY_DIE2_6,

		SPR_CHAINGUY_SHOOT_1,SPR_CHAINGUY_SHOOT_2,SPR_CHAINGUY_SHOOT_3,SPR_CHAINGUY_SHOOT_4,
		SPR_CHAINGUY_SHOOT_5,SPR_CHAINGUY_SHOOT_6,SPR_CHAINGUY_SHOOT_7,SPR_CHAINGUY_SHOOT_8,

		SPR_CHAINGUY_SHOOT2_1,SPR_CHAINGUY_SHOOT2_2,SPR_CHAINGUY_SHOOT2_3,SPR_CHAINGUY_SHOOT2_4,
		SPR_CHAINGUY_SHOOT2_5,SPR_CHAINGUY_SHOOT2_6,SPR_CHAINGUY_SHOOT2_7,SPR_CHAINGUY_SHOOT2_8,

//
// Imp
//
		SPR_IMP_W1_1,SPR_IMP_W1_2,SPR_IMP_W1_3,SPR_IMP_W1_4,
		SPR_IMP_W1_5,SPR_IMP_W1_6,SPR_IMP_W1_7,SPR_IMP_W1_8,

		SPR_IMP_W2_1,SPR_IMP_W2_2,SPR_IMP_W2_3,SPR_IMP_W2_4,
		SPR_IMP_W2_5,SPR_IMP_W2_6,SPR_IMP_W2_7,SPR_IMP_W2_8,

		SPR_IMP_W3_1,SPR_IMP_W3_2,SPR_IMP_W3_3,SPR_IMP_W3_4,
		SPR_IMP_W3_5,SPR_IMP_W3_6,SPR_IMP_W3_7,SPR_IMP_W3_8,

		SPR_IMP_W4_1,SPR_IMP_W4_2,SPR_IMP_W4_3,SPR_IMP_W4_4,
		SPR_IMP_W4_5,SPR_IMP_W4_6,SPR_IMP_W4_7,SPR_IMP_W4_8,

		SPR_IMP_PAIN_1,SPR_IMP_PAIN_2,SPR_IMP_PAIN_3,SPR_IMP_PAIN_4,
		SPR_IMP_PAIN_5,SPR_IMP_PAIN_6,SPR_IMP_PAIN_7,SPR_IMP_PAIN_8,

		SPR_IMP_DIE_1,SPR_IMP_DIE_2,SPR_IMP_DIE_3,SPR_IMP_DIE_4,
		SPR_IMP_DEAD,

		SPR_IMP_DIE2_1,SPR_IMP_DIE2_2,SPR_IMP_DIE2_3,SPR_IMP_DIE2_4,
		SPR_IMP_DIE2_5,SPR_IMP_DIE2_6,SPR_IMP_DIE2_7,SPR_IMP_DIE2_8,

		SPR_IMP_SHOOT_1,SPR_IMP_SHOOT_2,SPR_IMP_SHOOT_3,SPR_IMP_SHOOT_4,
		SPR_IMP_SHOOT_5,SPR_IMP_SHOOT_6,SPR_IMP_SHOOT_7,SPR_IMP_SHOOT_8,

		SPR_IMP_SHOOT2_1,SPR_IMP_SHOOT2_2,SPR_IMP_SHOOT2_3,SPR_IMP_SHOOT2_4,
		SPR_IMP_SHOOT2_5,SPR_IMP_SHOOT2_6,SPR_IMP_SHOOT2_7,SPR_IMP_SHOOT2_8,

		SPR_IMP_SHOOT3_1,SPR_IMP_SHOOT3_2,SPR_IMP_SHOOT3_3,SPR_IMP_SHOOT3_4,
		SPR_IMP_SHOOT3_5,SPR_IMP_SHOOT3_6,SPR_IMP_SHOOT3_7,SPR_IMP_SHOOT3_8,

//
// pain elemental
//
		SPR_PAIN_W1_1,SPR_PAIN_W1_2,SPR_PAIN_W1_3,SPR_PAIN_W1_4,
		SPR_PAIN_W1_5,SPR_PAIN_W1_6,SPR_PAIN_W1_7,SPR_PAIN_W1_8,

		SPR_PAIN_W2_1,SPR_PAIN_W2_2,SPR_PAIN_W2_3,SPR_PAIN_W2_4,
		SPR_PAIN_W2_5,SPR_PAIN_W2_6,SPR_PAIN_W2_7,SPR_PAIN_W2_8,

		SPR_PAIN_W3_1,SPR_PAIN_W3_2,SPR_PAIN_W3_3,SPR_PAIN_W3_4,
		SPR_PAIN_W3_5,SPR_PAIN_W3_6,SPR_PAIN_W3_7,SPR_PAIN_W3_8,

		SPR_PAIN_SHOOT_1,SPR_PAIN_SHOOT_2,SPR_PAIN_SHOOT_3,SPR_PAIN_SHOOT_4,
		SPR_PAIN_SHOOT_5,SPR_PAIN_SHOOT_6,SPR_PAIN_SHOOT_7,SPR_PAIN_SHOOT_8,

		SPR_PAIN_SHOOT2_1,SPR_PAIN_SHOOT2_2,SPR_PAIN_SHOOT2_3,SPR_PAIN_SHOOT2_4,
		SPR_PAIN_SHOOT2_5,SPR_PAIN_SHOOT2_6,SPR_PAIN_SHOOT2_7,SPR_PAIN_SHOOT2_8,

		SPR_PAIN_SHOOT3_1,SPR_PAIN_SHOOT3_2,SPR_PAIN_SHOOT3_3,SPR_PAIN_SHOOT3_4,
		SPR_PAIN_SHOOT3_5,SPR_PAIN_SHOOT3_6,SPR_PAIN_SHOOT3_7,SPR_PAIN_SHOOT3_8,

		SPR_PAIN_PAIN_1,SPR_PAIN_PAIN_2,SPR_PAIN_PAIN_3,SPR_PAIN_PAIN_4,
		SPR_PAIN_PAIN_5,SPR_PAIN_PAIN_6,SPR_PAIN_PAIN_7,SPR_PAIN_PAIN_8,

		SPR_PAIN_DIE_1,SPR_PAIN_DIE_2,SPR_PAIN_DIE_3,SPR_PAIN_DIE_4,
		SPR_PAIN_DIE_5,

//
// lost soul
//
		SPR_SOUL_W1_1,SPR_SOUL_W1_2,SPR_SOUL_W1_3,SPR_SOUL_W1_4,
		SPR_SOUL_W1_5,SPR_SOUL_W1_6,SPR_SOUL_W1_7,SPR_SOUL_W1_8,

		SPR_SOUL_W2_1,SPR_SOUL_W2_2,SPR_SOUL_W2_3,SPR_SOUL_W2_4,
		SPR_SOUL_W2_5,SPR_SOUL_W2_6,SPR_SOUL_W2_7,SPR_SOUL_W2_8,

		SPR_SOUL_PAIN_1,SPR_SOUL_PAIN_2,SPR_SOUL_PAIN_3,SPR_SOUL_PAIN_4,
		SPR_SOUL_PAIN_5,SPR_SOUL_PAIN_6,SPR_SOUL_PAIN_7,SPR_SOUL_PAIN_8,

		SPR_SOUL_DIE_1,SPR_SOUL_DIE_2,SPR_SOUL_DIE_3,SPR_SOUL_DIE_4,
		SPR_SOUL_DIE_5,

		SPR_SOUL_SHOOT_1,SPR_SOUL_SHOOT_2,SPR_SOUL_SHOOT_3,SPR_SOUL_SHOOT_4,
		SPR_SOUL_SHOOT_5,SPR_SOUL_SHOOT_6,SPR_SOUL_SHOOT_7,SPR_SOUL_SHOOT_8,

		SPR_SOUL_SHOOT2_1,SPR_SOUL_SHOOT2_2,SPR_SOUL_SHOOT2_3,SPR_SOUL_SHOOT2_4,
		SPR_SOUL_SHOOT2_5,SPR_SOUL_SHOOT2_6,SPR_SOUL_SHOOT2_7,SPR_SOUL_SHOOT2_8,
//
// cyberdemon
//
		SPR_CYBER_W1_1,SPR_CYBER_W1_2,SPR_CYBER_W1_3,SPR_CYBER_W1_4,
		SPR_CYBER_W1_5,SPR_CYBER_W1_6,SPR_CYBER_W1_7,SPR_CYBER_W1_8,

		SPR_CYBER_W2_1,SPR_CYBER_W2_2,SPR_CYBER_W2_3,SPR_CYBER_W2_4,
		SPR_CYBER_W2_5,SPR_CYBER_W2_6,SPR_CYBER_W2_7,SPR_CYBER_W2_8,

		SPR_CYBER_W3_1,SPR_CYBER_W3_2,SPR_CYBER_W3_3,SPR_CYBER_W3_4,
		SPR_CYBER_W3_5,SPR_CYBER_W3_6,SPR_CYBER_W3_7,SPR_CYBER_W3_8,

		SPR_CYBER_W4_1,SPR_CYBER_W4_2,SPR_CYBER_W4_3,SPR_CYBER_W4_4,
		SPR_CYBER_W4_5,SPR_CYBER_W4_6,SPR_CYBER_W4_7,SPR_CYBER_W4_8,

		SPR_CYBER_SHOOT_1,SPR_CYBER_SHOOT_2,SPR_CYBER_SHOOT_3,SPR_CYBER_SHOOT_4,
		SPR_CYBER_SHOOT_5,SPR_CYBER_SHOOT_6,SPR_CYBER_SHOOT_7,SPR_CYBER_SHOOT_8,

		SPR_CYBER_SHOOT2_1,SPR_CYBER_SHOOT2_2,SPR_CYBER_SHOOT2_3,SPR_CYBER_SHOOT2_4,
		SPR_CYBER_SHOOT2_5,SPR_CYBER_SHOOT2_6,SPR_CYBER_SHOOT2_7,SPR_CYBER_SHOOT2_8,

		SPR_CYBER_DEAD,

		SPR_CYBER_DIE_1,SPR_CYBER_DIE_2,SPR_CYBER_DIE_3,SPR_CYBER_DIE_4,
		SPR_CYBER_DIE_5,SPR_CYBER_DIE_6,SPR_CYBER_DIE_7,SPR_CYBER_DIE_8,

		SPR_CYBER_DIE2,SPR_CYBER_DIE3,SPR_CYBER_DIE4,SPR_CYBER_DIE5,
		SPR_CYBER_DIE6,SPR_CYBER_DIE7,SPR_CYBER_DIE8,SPR_CYBER_DIE9,

//
// spider demon
//
		SPR_SPIDER_W1_1,SPR_SPIDER_W1_2,SPR_SPIDER_W1_3,SPR_SPIDER_W1_4,
		SPR_SPIDER_W1_5,SPR_SPIDER_W1_6,SPR_SPIDER_W1_7,SPR_SPIDER_W1_8,

		SPR_SPIDER_W2_1,SPR_SPIDER_W2_2,SPR_SPIDER_W2_3,SPR_SPIDER_W2_4,
		SPR_SPIDER_W2_5,SPR_SPIDER_W2_6,SPR_SPIDER_W2_7,SPR_SPIDER_W2_8,

		SPR_SPIDER_W3_1,SPR_SPIDER_W3_2,SPR_SPIDER_W3_3,SPR_SPIDER_W3_4,
		SPR_SPIDER_W3_5,SPR_SPIDER_W3_6,SPR_SPIDER_W3_7,SPR_SPIDER_W3_8,

		SPR_SPIDER_W4_1,SPR_SPIDER_W4_2,SPR_SPIDER_W4_3,SPR_SPIDER_W4_4,
		SPR_SPIDER_W4_5,SPR_SPIDER_W4_6,SPR_SPIDER_W4_7,SPR_SPIDER_W4_8,

		SPR_SPIDER_W5_1,SPR_SPIDER_W5_2,SPR_SPIDER_W5_3,SPR_SPIDER_W5_4,
		SPR_SPIDER_W5_5,SPR_SPIDER_W5_6,SPR_SPIDER_W5_7,SPR_SPIDER_W5_8,

		SPR_SPIDER_W6_1,SPR_SPIDER_W6_2,SPR_SPIDER_W6_3,SPR_SPIDER_W6_4,
		SPR_SPIDER_W6_5,SPR_SPIDER_W6_6,SPR_SPIDER_W6_7,SPR_SPIDER_W6_8,

		SPR_SPIDER_SHOOT1_1,SPR_SPIDER_SHOOT1_2,SPR_SPIDER_SHOOT1_3,SPR_SPIDER_SHOOT1_4,
		SPR_SPIDER_SHOOT1_5,SPR_SPIDER_SHOOT1_6,SPR_SPIDER_SHOOT1_7,SPR_SPIDER_SHOOT1_8,

		SPR_SPIDER_SHOOT2_1,SPR_SPIDER_SHOOT2_2,SPR_SPIDER_SHOOT2_3,SPR_SPIDER_SHOOT2_4,
		SPR_SPIDER_SHOOT2_5,SPR_SPIDER_SHOOT2_6,SPR_SPIDER_SHOOT2_7,SPR_SPIDER_SHOOT2_8,

		SPR_SPIDER_DIE1_1,SPR_SPIDER_DIE1_2,SPR_SPIDER_DIE1_3,SPR_SPIDER_DIE1_4,
		SPR_SPIDER_DIE1_5,SPR_SPIDER_DIE1_6,SPR_SPIDER_DIE1_7,SPR_SPIDER_DIE1_8,

		SPR_SPIDER_DIE2,SPR_SPIDER_DIE3,SPR_SPIDER_DIE4,SPR_SPIDER_DIE5,
		SPR_SPIDER_DIE6,SPR_SPIDER_DIE7,SPR_SPIDER_DEAD,

//
// spider projectile
//
		SPR_SPLASMA_1,SPR_SPLASMA_2,SPR_SPLASMA_BOOM1,SPR_SPLASMA_BOOM2,
		SPR_SPLASMA_BOOM3,SPR_SPLASMA_BOOM4,SPR_SPLASMA_BOOM5,

//
// revenant
//
		SPR_REVENANT_W1_1,SPR_REVENANT_W1_2,SPR_REVENANT_W1_3,SPR_REVENANT_W1_4,
		SPR_REVENANT_W1_5,SPR_REVENANT_W1_6,SPR_REVENANT_W1_7,SPR_REVENANT_W1_8,

		SPR_REVENANT_W2_1,SPR_REVENANT_W2_2,SPR_REVENANT_W2_3,SPR_REVENANT_W2_4,
		SPR_REVENANT_W2_5,SPR_REVENANT_W2_6,SPR_REVENANT_W2_7,SPR_REVENANT_W2_8,

		SPR_REVENANT_W3_1,SPR_REVENANT_W3_2,SPR_REVENANT_W3_3,SPR_REVENANT_W3_4,
		SPR_REVENANT_W3_5,SPR_REVENANT_W3_6,SPR_REVENANT_W3_7,SPR_REVENANT_W3_8,

		SPR_REVENANT_W4_1,SPR_REVENANT_W4_2,SPR_REVENANT_W4_3,SPR_REVENANT_W4_4,
		SPR_REVENANT_W4_5,SPR_REVENANT_W4_6,SPR_REVENANT_W4_7,SPR_REVENANT_W4_8,

		SPR_REVENANT_W5_1,SPR_REVENANT_W5_2,SPR_REVENANT_W5_3,SPR_REVENANT_W5_4,
		SPR_REVENANT_W5_5,SPR_REVENANT_W5_6,SPR_REVENANT_W5_7,SPR_REVENANT_W5_8,

		SPR_REVENANT_W6_1,SPR_REVENANT_W6_2,SPR_REVENANT_W6_3,SPR_REVENANT_W6_4,
		SPR_REVENANT_W6_5,SPR_REVENANT_W6_6,SPR_REVENANT_W6_7,SPR_REVENANT_W6_8,

		SPR_REVENANT_SHOOT1_1,SPR_REVENANT_SHOOT1_2,SPR_REVENANT_SHOOT1_3,SPR_REVENANT_SHOOT1_4,
		SPR_REVENANT_SHOOT1_5,SPR_REVENANT_SHOOT1_6,SPR_REVENANT_SHOOT1_7,SPR_REVENANT_SHOOT1_8,

		SPR_REVENANT_SHOOT2_1,SPR_REVENANT_SHOOT2_2,SPR_REVENANT_SHOOT2_3,SPR_REVENANT_SHOOT2_4,
		SPR_REVENANT_SHOOT2_5,SPR_REVENANT_SHOOT2_6,SPR_REVENANT_SHOOT2_7,SPR_REVENANT_SHOOT2_8,

		SPR_REVENANT_SHOOT3_1,SPR_REVENANT_SHOOT3_2,SPR_REVENANT_SHOOT3_3,SPR_REVENANT_SHOOT3_4,
		SPR_REVENANT_SHOOT3_5,SPR_REVENANT_SHOOT3_6,SPR_REVENANT_SHOOT3_7,SPR_REVENANT_SHOOT3_8,

		SPR_REVENANT_SHOOT4_1,SPR_REVENANT_SHOOT4_2,SPR_REVENANT_SHOOT4_3,SPR_REVENANT_SHOOT4_4,
		SPR_REVENANT_SHOOT4_5,SPR_REVENANT_SHOOT4_6,SPR_REVENANT_SHOOT4_7,SPR_REVENANT_SHOOT4_8,

		SPR_REVENANT_DIE1_1,SPR_REVENANT_DIE1_2,SPR_REVENANT_DIE1_3,SPR_REVENANT_DIE1_4,
		SPR_REVENANT_DIE1_5,SPR_REVENANT_DIE1_6,SPR_REVENANT_DIE1_7,SPR_REVENANT_DIE1_8,

		SPR_REVENANT_DIE2_1,SPR_REVENANT_DIE2_2,SPR_REVENANT_DIE2_3,SPR_REVENANT_DIE2_4,
		SPR_REVENANT_DIE2_5,SPR_REVENANT_DIE2_6,SPR_REVENANT_DIE2_7,SPR_REVENANT_DIE2_8,

		SPR_REVENANT_DIE3,SPR_REVENANT_DIE4,SPR_REVENANT_DIE5,SPR_REVENANT_DIE6,
		SPR_REVENANT_DEAD,

//
// boss
//

		SPR_BOSS_W1_1,SPR_BOSS_W1_2,SPR_BOSS_W1_3,SPR_BOSS_W1_4,
		SPR_BOSS_W1_5,SPR_BOSS_W1_6,SPR_BOSS_W1_7,SPR_BOSS_W1_8,

		SPR_BOSS_W2_1,SPR_BOSS_W2_2,SPR_BOSS_W2_3,SPR_BOSS_W2_4,
		SPR_BOSS_W2_5,SPR_BOSS_W2_6,SPR_BOSS_W2_7,SPR_BOSS_W2_8,

		SPR_BOSS_W3_1,SPR_BOSS_W3_2,SPR_BOSS_W3_3,SPR_BOSS_W3_4,
		SPR_BOSS_W3_5,SPR_BOSS_W3_6,SPR_BOSS_W3_7,SPR_BOSS_W3_8,

		SPR_BOSS_W4_1,SPR_BOSS_W4_2,SPR_BOSS_W4_3,SPR_BOSS_W4_4,
		SPR_BOSS_W4_5,SPR_BOSS_W4_6,SPR_BOSS_W4_7,SPR_BOSS_W4_8,

		SPR_BOSS_SHOOT_1,SPR_BOSS_SHOOT_2,SPR_BOSS_SHOOT_3,SPR_BOSS_SHOOT_4,
		SPR_BOSS_SHOOT_5,SPR_BOSS_SHOOT_6,SPR_BOSS_SHOOT_7,SPR_BOSS_SHOOT_8,

		SPR_BOSS_SHOOT2_1,SPR_BOSS_SHOOT2_2,SPR_BOSS_SHOOT2_3,SPR_BOSS_SHOOT2_4,
		SPR_BOSS_SHOOT2_5,SPR_BOSS_SHOOT2_6,SPR_BOSS_SHOOT2_7,SPR_BOSS_SHOOT2_8,

		SPR_BOSS_SHOOT3_1,SPR_BOSS_SHOOT3_2,SPR_BOSS_SHOOT3_3,SPR_BOSS_SHOOT3_4,
		SPR_BOSS_SHOOT3_5,SPR_BOSS_SHOOT3_6,SPR_BOSS_SHOOT3_7,SPR_BOSS_SHOOT3_8,

		SPR_BOSS_DEAD,

		SPR_BOSS_DIE_1,SPR_BOSS_DIE_2,SPR_BOSS_DIE_3,SPR_BOSS_DIE_4,
		SPR_BOSS_DIE_5,SPR_BOSS_DIE_6,SPR_BOSS_DIE_7,SPR_BOSS_DIE_8,

		SPR_BOSS_DIE2,SPR_BOSS_DIE3,SPR_BOSS_DIE4,SPR_BOSS_DIE5,
		SPR_BOSS_DIE6,SPR_BOSS_DIE7,

//
// boss2
//

		SPR_BOSS2_W1_1,SPR_BOSS2_W1_2,SPR_BOSS2_W1_3,SPR_BOSS2_W1_4,
		SPR_BOSS2_W1_5,SPR_BOSS2_W1_6,SPR_BOSS2_W1_7,SPR_BOSS2_W1_8,

		SPR_BOSS2_W2_1,SPR_BOSS2_W2_2,SPR_BOSS2_W2_3,SPR_BOSS2_W2_4,
		SPR_BOSS2_W2_5,SPR_BOSS2_W2_6,SPR_BOSS2_W2_7,SPR_BOSS2_W2_8,

		SPR_BOSS2_W3_1,SPR_BOSS2_W3_2,SPR_BOSS2_W3_3,SPR_BOSS2_W3_4,
		SPR_BOSS2_W3_5,SPR_BOSS2_W3_6,SPR_BOSS2_W3_7,SPR_BOSS2_W3_8,

		SPR_BOSS2_W4_1,SPR_BOSS2_W4_2,SPR_BOSS2_W4_3,SPR_BOSS2_W4_4,
		SPR_BOSS2_W4_5,SPR_BOSS2_W4_6,SPR_BOSS2_W4_7,SPR_BOSS2_W4_8,

		SPR_BOSS2_SHOOT_1,SPR_BOSS2_SHOOT_2,SPR_BOSS2_SHOOT_3,SPR_BOSS2_SHOOT_4,
		SPR_BOSS2_SHOOT_5,SPR_BOSS2_SHOOT_6,SPR_BOSS2_SHOOT_7,SPR_BOSS2_SHOOT_8,

		SPR_BOSS2_SHOOT2_1,SPR_BOSS2_SHOOT2_2,SPR_BOSS2_SHOOT2_3,SPR_BOSS2_SHOOT2_4,
		SPR_BOSS2_SHOOT2_5,SPR_BOSS2_SHOOT2_6,SPR_BOSS2_SHOOT2_7,SPR_BOSS2_SHOOT2_8,

		SPR_BOSS2_SHOOT3_1,SPR_BOSS2_SHOOT3_2,SPR_BOSS2_SHOOT3_3,SPR_BOSS2_SHOOT3_4,
		SPR_BOSS2_SHOOT3_5,SPR_BOSS2_SHOOT3_6,SPR_BOSS2_SHOOT3_7,SPR_BOSS2_SHOOT3_8,

		SPR_BOSS2_DIE_1,SPR_BOSS2_DIE_2,SPR_BOSS2_DIE_3,SPR_BOSS2_DIE_4,
		SPR_BOSS2_DIE_5,SPR_BOSS2_DIE_6,SPR_BOSS2_DIE_7,SPR_BOSS2_DIE_8,

		SPR_BOSS2_DIE2,SPR_BOSS2_DIE3,SPR_BOSS2_DIE4,SPR_BOSS2_DIE5,
		SPR_BOSS2_DIE6,SPR_BOSS2_DIE7,SPR_BOSS2_DEAD,

//
// boss projectile
//

		SPR_GREEN_1,SPR_GREEN_2,SPR_GREEN_3,SPR_GREEN_4,
		SPR_GREEN_5,SPR_GREEN_6,SPR_GREEN_7,SPR_GREEN_8,

		SPR_GREEN2_1,SPR_GREEN2_2,SPR_GREEN2_3,SPR_GREEN2_4,
                SPR_GREEN2_5,SPR_GREEN2_6,SPR_GREEN2_7,SPR_GREEN2_8,

		SPR_GREEN_BOOM_1,SPR_GREEN_BOOM_2,SPR_GREEN_BOOM_3,

//
// fat mancubus
//
		SPR_MANCUBUS_W1_1,SPR_MANCUBUS_W1_2,SPR_MANCUBUS_W1_3,SPR_MANCUBUS_W1_4,
		SPR_MANCUBUS_W1_5,SPR_MANCUBUS_W1_6,SPR_MANCUBUS_W1_7,SPR_MANCUBUS_W1_8,

		SPR_MANCUBUS_W2_1,SPR_MANCUBUS_W2_2,SPR_MANCUBUS_W2_3,SPR_MANCUBUS_W2_4,
		SPR_MANCUBUS_W2_5,SPR_MANCUBUS_W2_6,SPR_MANCUBUS_W2_7,SPR_MANCUBUS_W2_8,

		SPR_MANCUBUS_W3_1,SPR_MANCUBUS_W3_2,SPR_MANCUBUS_W3_3,SPR_MANCUBUS_W3_4,
		SPR_MANCUBUS_W3_5,SPR_MANCUBUS_W3_6,SPR_MANCUBUS_W3_7,SPR_MANCUBUS_W3_8,

		SPR_MANCUBUS_W4_1,SPR_MANCUBUS_W4_2,SPR_MANCUBUS_W4_3,SPR_MANCUBUS_W4_4,
		SPR_MANCUBUS_W4_5,SPR_MANCUBUS_W4_6,SPR_MANCUBUS_W4_7,SPR_MANCUBUS_W4_8,

		SPR_MANCUBUS_W5_1,SPR_MANCUBUS_W5_2,SPR_MANCUBUS_W5_3,SPR_MANCUBUS_W5_4,
		SPR_MANCUBUS_W5_5,SPR_MANCUBUS_W5_6,SPR_MANCUBUS_W5_7,SPR_MANCUBUS_W5_8,

		SPR_MANCUBUS_W6_1,SPR_MANCUBUS_W6_2,SPR_MANCUBUS_W6_3,SPR_MANCUBUS_W6_4,
		SPR_MANCUBUS_W6_5,SPR_MANCUBUS_W6_6,SPR_MANCUBUS_W6_7,SPR_MANCUBUS_W6_8,

		SPR_MANCUBUS_SHOOT_1,SPR_MANCUBUS_SHOOT_2,SPR_MANCUBUS_SHOOT_3,SPR_MANCUBUS_SHOOT_4,
		SPR_MANCUBUS_SHOOT_5,SPR_MANCUBUS_SHOOT_6,SPR_MANCUBUS_SHOOT_7,SPR_MANCUBUS_SHOOT_8,

		SPR_MANCUBUS_SHOOT2_1,SPR_MANCUBUS_SHOOT2_2,SPR_MANCUBUS_SHOOT2_3,SPR_MANCUBUS_SHOOT2_4,
		SPR_MANCUBUS_SHOOT2_5,SPR_MANCUBUS_SHOOT2_6,SPR_MANCUBUS_SHOOT2_7,SPR_MANCUBUS_SHOOT2_8,

		SPR_MANCUBUS_DIE_1,SPR_MANCUBUS_DIE_2,SPR_MANCUBUS_DIE_3,SPR_MANCUBUS_DIE_4,
		SPR_MANCUBUS_DIE_5,SPR_MANCUBUS_DIE_6,SPR_MANCUBUS_DIE_7,SPR_MANCUBUS_DIE_8,

		SPR_MANCUBUS_DIE2_1,SPR_MANCUBUS_DIE2_2,SPR_MANCUBUS_DIE2_3,SPR_MANCUBUS_DIE2_4,
		SPR_MANCUBUS_DIE2_5,SPR_MANCUBUS_DIE2_6,SPR_MANCUBUS_DIE2_7,SPR_MANCUBUS_DIE2_8,

		SPR_MANCUBUS_DIE3,SPR_MANCUBUS_DIE4,SPR_MANCUBUS_DIE5,SPR_MANCUBUS_DIE6,
		SPR_MANCUBUS_DIE7,SPR_MANCUBUS_DIE8,SPR_MANCUBUS_DIE9,SPR_MANCUBUS_DIE10,

		SPR_MANCUBUS_DIE11,SPR_MANCUBUS_DEAD,

//
// fat projectile
//

		SPR_FAT_1,SPR_FAT_2,SPR_FAT_3,SPR_FAT_4,
		SPR_FAT_5,SPR_FAT_6,SPR_FAT_7,SPR_FAT_8,

		SPR_FAT2_1,SPR_FAT2_2,SPR_FAT2_3,SPR_FAT2_4,
		SPR_FAT2_5,SPR_FAT2_6,SPR_FAT2_7,SPR_FAT2_8,

//
// player
//
		SPR_BJ_W1_1,SPR_BJ_W1_2,SPR_BJ_W1_3,SPR_BJ_W1_4,
		SPR_BJ_W1_5,SPR_BJ_W1_6,SPR_BJ_W1_7,SPR_BJ_W1_8,

		SPR_BJ_W2_1,SPR_BJ_W2_2,SPR_BJ_W2_3,SPR_BJ_W2_4,
		SPR_BJ_W2_5,SPR_BJ_W2_6,SPR_BJ_W2_7,SPR_BJ_W2_8,

		SPR_BJ_W3_1,SPR_BJ_W3_2,SPR_BJ_W3_3,SPR_BJ_W3_4,
		SPR_BJ_W3_5,SPR_BJ_W3_6,SPR_BJ_W3_7,SPR_BJ_W3_8,

		SPR_BJ_W4_1,SPR_BJ_W4_2,SPR_BJ_W4_3,SPR_BJ_W4_4,
		SPR_BJ_W4_5,SPR_BJ_W4_6,SPR_BJ_W4_7,SPR_BJ_W4_8,

		SPR_BJ_PAIN_1,SPR_BJ_PAIN_2,SPR_BJ_PAIN_3,SPR_BJ_PAIN_4,
		SPR_BJ_PAIN_5,SPR_BJ_PAIN_6,SPR_BJ_PAIN_7,SPR_BJ_PAIN_8,

		SPR_BJ_DIE1,SPR_BJ_DIE2,SPR_BJ_DIE3,SPR_BJ_DIE4,
		SPR_BJ_DIE5,SPR_BJ_DIE6,SPR_BJ_DEAD,

//
// player attack frames
//
		SPR_FISTSREADY,SPR_FISTSATK1,SPR_FISTSATK2,SPR_FISTSATK3,
		SPR_FISTSATK4,

		SPR_PISTOLREADY,SPR_PISTOLATK1,SPR_PISTOLATK2,SPR_PISTOLATK3,
		SPR_PISTOLATK4,

		SPR_PLASMAGUNREADY,SPR_PLASMAGUNATK1,SPR_PLASMAGUNATK2,SPR_PLASMAGUNATK3,
		SPR_PLASMAGUNATK4,

		SPR_CHAINREADY,SPR_CHAINATK1,SPR_CHAINATK2,SPR_CHAINATK3,
		SPR_CHAINATK4,

		SPR_LAUNCHERREADY,SPR_LAUNCHERATK1,SPR_LAUNCHERATK2,SPR_LAUNCHERATK3,
		SPR_LAUNCHERATK4,

		SPR_SHOTGUNREADY,SPR_SHOTGUNATK1,SPR_SHOTGUNATK2,SPR_SHOTGUNATK3,
		SPR_SHOTGUNATK4,SPR_SHOTGUNRLD1,SPR_SHOTGUNRLD2,SPR_SHOTGUNRLD3,

		SPR_SUPERSHOTGUNREADY,SPR_SUPERSHOTGUNATK1,SPR_SUPERSHOTGUNATK2,SPR_SUPERSHOTGUNATK3,
		SPR_SUPERSHOTGUNATK4,SPR_SUPERSHOTGUNRLD1,SPR_SUPERSHOTGUNRLD2,SPR_SUPERSHOTGUNRLD3,
		SPR_SUPERSHOTGUNRLD4,SPR_SUPERSHOTGUNRLD5,SPR_SUPERSHOTGUNRLD6,SPR_SUPERSHOTGUNRLD7,

		SPR_BFGREADY,SPR_BFGATK1,SPR_BFGATK2,SPR_BFGATK3,
		SPR_BFGATK4,

		SPR_CHAINSAWREADY,SPR_CHAINSAWREADY2,SPR_CHAINSAWATK1,SPR_CHAINSAWATK2,

		SPR_STAT2_0,SPR_STAT2_1,SPR_STAT2_2,

		SPR_BARREL,SPR_BARREL2,SPR_BARRELBOOM1,SPR_BARRELBOOM2,
		SPR_BARRELBOOM3,

		SPR_SPLAT1,SPR_SPLAT2,SPR_SPLAT3,SPR_SPLAT4,
		SPR_SPLAT5,SPR_SPLAT6,SPR_SPLAT7,SPR_SPLAT8,
		SPR_SPLAT9,

		SPR_PUFF1,SPR_PUFF2,SPR_PUFF3,SPR_PUFF4,

		SPR_BFGSPLAT1,SPR_BFGSPLAT2,SPR_BFGSPLAT3,SPR_BFGSPLAT4,

		SPR_STAT3_0,SPR_STAT3_1,SPR_STAT3_2,SPR_STAT3_3,
		SPR_STAT3_4,SPR_STAT3_5,SPR_STAT3_6,SPR_STAT3_7,

		SPR_STAT3_8,SPR_STAT3_9,SPR_STAT3_10,SPR_STAT3_11,
		SPR_STAT3_12,SPR_STAT3_13,SPR_STAT3_14,SPR_STAT3_15,

		SPR_STAT3_16,SPR_STAT3_17,SPR_STAT3_18,SPR_STAT3_19,
		SPR_STAT3_20,SPR_STAT3_21,SPR_STAT3_22,SPR_STAT3_23,

		SPR_STAT3_24,SPR_STAT3_25,SPR_STAT3_26,SPR_STAT3_27,
		SPR_STAT3_28,SPR_STAT3_29,SPR_STAT3_30,SPR_STAT3_31,

		SPR_STAT3_32,SPR_STAT3_33,SPR_STAT3_34,SPR_STAT3_35,
		SPR_STAT3_36,SPR_STAT3_37,SPR_STAT3_38,SPR_STAT3_39,

		SPR_STAT3_40,SPR_STAT3_41,SPR_STAT3_42,SPR_STAT3_43,
		SPR_STAT3_44,SPR_STAT3_45,SPR_STAT3_46,SPR_STAT3_47,

		SPR_STAT3_48,SPR_STAT3_49,SPR_STAT3_50,SPR_STAT3_51,
		SPR_STAT3_52,SPR_STAT3_53,SPR_STAT3_54,SPR_STAT3_55,

		SPR_STAT3_56,SPR_STAT3_57,SPR_STAT3_58,SPR_STAT3_59,
		SPR_STAT3_60,SPR_STAT3_61,SPR_STAT3_62,SPR_STAT3_63,

		SPR_STAT3_64,SPR_TOILET,SPR_EXIT,SPR_BIOSUIT,
		SPR_HELLORB,SPR_HELLORB2,SPR_HELLORB3,SPR_HELLORB4,

		SPR_VILE_W1_1,SPR_VILE_W1_2,SPR_VILE_W1_3,SPR_VILE_W1_4,
		SPR_VILE_W1_5,SPR_VILE_W1_6,SPR_VILE_W1_7,SPR_VILE_W1_8,

		SPR_VILE_W2_1,SPR_VILE_W2_2,SPR_VILE_W2_3,SPR_VILE_W2_4,
		SPR_VILE_W2_5,SPR_VILE_W2_6,SPR_VILE_W2_7,SPR_VILE_W2_8,

		SPR_VILE_W3_1,SPR_VILE_W3_2,SPR_VILE_W3_3,SPR_VILE_W3_4,
		SPR_VILE_W3_5,SPR_VILE_W3_6,SPR_VILE_W3_7,SPR_VILE_W3_8,

		SPR_VILE_W4_1,SPR_VILE_W4_2,SPR_VILE_W4_3,SPR_VILE_W4_4,
		SPR_VILE_W4_5,SPR_VILE_W4_6,SPR_VILE_W4_7,SPR_VILE_W4_8,

		SPR_VILE_W5_1,SPR_VILE_W5_2,SPR_VILE_W5_3,SPR_VILE_W5_4,
		SPR_VILE_W5_5,SPR_VILE_W5_6,SPR_VILE_W5_7,SPR_VILE_W5_8,

		SPR_VILE_W6_1,SPR_VILE_W6_2,SPR_VILE_W6_3,SPR_VILE_W6_4,
		SPR_VILE_W6_5,SPR_VILE_W6_6,SPR_VILE_W6_7,SPR_VILE_W6_8,

		SPR_VILE_SHOOT1_1,SPR_VILE_SHOOT1_2,SPR_VILE_SHOOT1_3,SPR_VILE_SHOOT1_4,
		SPR_VILE_SHOOT1_5,SPR_VILE_SHOOT1_6,SPR_VILE_SHOOT1_7,SPR_VILE_SHOOT1_8,

		SPR_VILE_SHOOT2_1,SPR_VILE_SHOOT2_2,SPR_VILE_SHOOT2_3,SPR_VILE_SHOOT2_4,
		SPR_VILE_SHOOT2_5,SPR_VILE_SHOOT2_6,SPR_VILE_SHOOT2_7,SPR_VILE_SHOOT2_8,

		SPR_VILE_SHOOT3_1,SPR_VILE_SHOOT3_2,SPR_VILE_SHOOT3_3,SPR_VILE_SHOOT3_4,
		SPR_VILE_SHOOT3_5,SPR_VILE_SHOOT3_6,SPR_VILE_SHOOT3_7,SPR_VILE_SHOOT3_8,

		SPR_VILE_SHOOT4_1,SPR_VILE_SHOOT4_2,SPR_VILE_SHOOT4_3,SPR_VILE_SHOOT4_4,
		SPR_VILE_SHOOT4_5,SPR_VILE_SHOOT4_6,SPR_VILE_SHOOT4_7,SPR_VILE_SHOOT4_8,

		SPR_VILE_SHOOT5_1,SPR_VILE_SHOOT5_2,SPR_VILE_SHOOT5_3,SPR_VILE_SHOOT5_4,
		SPR_VILE_SHOOT5_5,SPR_VILE_SHOOT5_6,SPR_VILE_SHOOT5_7,SPR_VILE_SHOOT5_8,

		SPR_VILE_SHOOT6_1,SPR_VILE_SHOOT6_2,SPR_VILE_SHOOT6_3,SPR_VILE_SHOOT6_4,
		SPR_VILE_SHOOT6_5,SPR_VILE_SHOOT6_6,SPR_VILE_SHOOT6_7,SPR_VILE_SHOOT6_8,

		SPR_VILE_SHOOT7_1,SPR_VILE_SHOOT7_2,SPR_VILE_SHOOT7_3,SPR_VILE_SHOOT7_4,
		SPR_VILE_SHOOT7_5,SPR_VILE_SHOOT7_6,SPR_VILE_SHOOT7_7,SPR_VILE_SHOOT7_8,

		SPR_VILE_SHOOT8_1,SPR_VILE_SHOOT8_2,SPR_VILE_SHOOT8_3,SPR_VILE_SHOOT8_4,
		SPR_VILE_SHOOT8_5,SPR_VILE_SHOOT8_6,SPR_VILE_SHOOT8_7,SPR_VILE_SHOOT8_8,

		SPR_VILE_SHOOT9_1,SPR_VILE_SHOOT9_2,SPR_VILE_SHOOT9_3,SPR_VILE_SHOOT9_4,
		SPR_VILE_SHOOT9_5,SPR_VILE_SHOOT9_6,SPR_VILE_SHOOT9_7,SPR_VILE_SHOOT9_8,

		SPR_VILE_SHOOT10_1,SPR_VILE_SHOOT10_2,SPR_VILE_SHOOT10_3,SPR_VILE_SHOOT10_4,
		SPR_VILE_SHOOT10_5,SPR_VILE_SHOOT10_6,SPR_VILE_SHOOT10_7,SPR_VILE_SHOOT10_8,

		SPR_VILE_DIE1_1,SPR_VILE_DIE1_2,SPR_VILE_DIE1_3,SPR_VILE_DIE1_4,
		SPR_VILE_DIE1_5,SPR_VILE_DIE1_6,SPR_VILE_DIE1_7,SPR_VILE_DIE1_8,

		SPR_VILE_DIE2,SPR_VILE_DIE3,SPR_VILE_DIE4,SPR_VILE_DIE5,
		SPR_VILE_DIE6,SPR_VILE_DIE7,SPR_VILE_DIE8,SPR_VILE_DIE9,

		SPR_VILE_DEAD,

		SPR_VILE_FIRE1,SPR_VILE_FIRE2,SPR_VILE_FIRE3,SPR_VILE_FIRE4,
		SPR_VILE_FIRE5,SPR_VILE_FIRE6,SPR_VILE_FIRE7,SPR_VILE_FIRE8,

		SPR_REVBALL_1,SPR_REVBALL_2,SPR_REVBALL_3,SPR_REVBALL_4,
		SPR_REVBALL_5,SPR_REVBALL_6,SPR_REVBALL_7,SPR_REVBALL_8,

		SPR_REVBALL2_1,SPR_REVBALL2_2,SPR_REVBALL2_3,SPR_REVBALL2_4,
		SPR_REVBALL2_5,SPR_REVBALL2_6,SPR_REVBALL2_7,SPR_REVBALL2_8,

		SPR_REVBALL_BOOM1,SPR_REVBALL_BOOM2,SPR_REVBALL_BOOM3,

		SPR_HELLORB_READY,SPR_HELLORB_READY2,SPR_HELLORB_READY3,SPR_HELLORB_READY4,
		SPR_HELLORB_ATK1,SPR_HELLORB_ATK2,SPR_HELLORB_ATK3,SPR_HELLORB_ATK4,


		SPR_HELLORB_ATK5,SPR_HELLORB_ATK6,SPR_HELLORB_ATK7,SPR_HELLORB_ATK8,
		SPR_HELLBALL1,SPR_HELLBALL2,SPR_HELLBALL3,SPR_HELLBALL4,

		};


typedef long fixed;

typedef enum {
	di_north,
	di_east,
	di_south,
	di_west
} controldir_t;

typedef enum {
	dr_normal,
	dr_lock1,
	dr_lock2,
	dr_lock3,
	dr_lock4,
	dr_elevator,
	dr_green,
	dr_brown,
} door_t;

typedef enum {
	ac_badobject = -1,
	ac_no,
	ac_yes,
	ac_allways
} activetype;

typedef enum {
	nothing,
	playerobj,
	inertobj,
	mutantobj,
	chainguyobj,
	cacoobj,
	demonobj,
	cyberobj,
	spiderobj,
	revenantobj,
	shotguyobj,
	impobj,
	painobj,
	soulobj,
	bjobj,
	bossobj,
	boss2obj,
	mancubusobj,
	vileobj,
	rocketobj,
	plasmaobj,
	bossballobj,
	fatballobj,
	revballobj,
	bfgobj,
	splatobj,
	puffobj,
	barrelobj,
	impballobj,
	cacoballobj,
	splasmaobj,
	vilefireobj,
	hellorbobj
} classtype;

typedef enum {
	dressing,
	block,
	bo_key1,
	bo_key2,
	bo_key3,
	bo_ammo1,
	bo_ammoused,
	bo_ammo1box,
	bo_ammo2,
	bo_ammo2box,
	bo_ammo3,
	bo_ammo3box,
	bo_ammo4,
	bo_ammo4box,
	bo_shotgun,
	bo_supershotgun,
	bo_chaingun,
	bo_launcher,
	bo_plasmagun,
	bo_bfggun,
	bo_chainsaw,
	bo_megaarmor,
	bo_armor,
	bo_armor1,
	bo_health1,
	bo_medpack,
	bo_healthkit,
	bo_berserk,
	bo_backpack,
	bo_godmode,
	bo_megasphere,
	bo_goggles,
	bo_cloak,
	bo_soul,
	bo_biosuit,
	bo_hellorb,
   dir_south,
   dir_south_fw,
   dir_south_bw,
   dir_east,
   dir_east_fw,
   dir_east_bw,
   bl_dir_south,
   bl_dir_south_fw,
   bl_dir_south_bw,
   bl_dir_east,
   bl_dir_east_fw,
   bl_dir_east_bw
} stat_t;

typedef enum {
	east,
	northeast,
	north,
	northwest,
	west,
	southwest,
	south,
	southeast,
	nodir
} dirtype;


#define NUMENEMIES		16
typedef enum {
	en_mutant,
	en_chaingunguy,
	en_cacodemon,
	en_demon,
	en_cyber,
	en_spider,
	en_revenant,
	en_shotgunguy,
	en_imp,
	en_painelemental,
	en_lostsoul,
	en_mancubus,
	en_barrel,
	en_boss,
	en_boss2,
	en_vile
} enemy_t;


typedef struct	statestruct
{
	boolean	rotate;
	int		shapenum;			// a shapenum of -1 means get from ob->temp1
	int		tictime;
	void	(*think) (),(*action) ();
	struct	statestruct	*next;
} statetype;

typedef struct statstruct
{
	byte	tilex,tiley;
	byte	*visspot;
	int		shapenum;			// if shapenum == -1 the obj has been removed
	byte	flags;
	byte	itemnumber;
} statobj_t;


//---------------------
//
// door actor structure
//
//---------------------

typedef struct doorstruct
{
	byte	tilex,tiley;
	boolean	vertical;
	byte	lock;
	enum	{dr_open,dr_closed,dr_opening,dr_closing}	action;
	int		ticcount;
} doorobj_t;


//--------------------
//
// thinking actor structure
//
//--------------------

typedef struct objstruct
{
	activetype	active;
	int			ticcount;
	classtype	obclass;
	statetype	*state;

	byte		flags;				//	FL_SHOOTABLE, etc

	long		distance;			// if negative, wait for that door to open
	dirtype		dir;

	fixed 		x,y;
	unsigned	tilex,tiley;
	byte		areanumber;

	int	 		viewx;
	unsigned	viewheight;
	fixed		transx,transy;		// in global coord

	int 		angle;
	int			hitpoints;
	long		speed;

	int			temp1,temp2,temp3;
	struct		objstruct	*next,*prev;
} objtype;


#define NUMBUTTONS	8
enum	{
	bt_nobutton=-1,
	bt_attack=0,
	bt_strafe,
	bt_run,
	bt_use,
	bt_readyknife,
	bt_readypistol,
	bt_readymachinegun,
	bt_readychaingun
};


#define NUMWEAPONS	11
typedef enum	{
	wp_fists,         // 1
	wp_pistol,        // 2
	wp_shotgun,       // 4
	wp_supershotgun,  // 8
	wp_chaingun,      // 16
	wp_launcher,      // 32
	wp_plasmagun,     // 64
	wp_bfggun,        // 128
	wp_chainsaw,      // 256
	wp_hellorb        // 512
} weapontype;

typedef enum	{
	ammo1,
	ammo2,
	ammo3,
	ammo4
} ammotype;


typedef enum	{
	gd_baby,
	gd_easy,
	gd_medium,
	gd_hard
};

//---------------
//
// gamestate structure
//
//---------------

typedef	struct
{
	int			difficulty;
	int			mapon;
	long		oldscore,score,nextextra;
	int			lives;
	int			health;
	int			armor;
	int			ammo,ammomax;
	int			ammo2,ammo2max;
	int			ammo3,ammo3max;
	int			ammo4,ammo4max;
	int			keys;
	int			bobber;
	int			bobdir;
	int			bobber2;
	int			bobdir2;
	weapontype		bestweapon,weapon,chosenweapon,nextweapon;
	boolean			changingweapon;
	boolean			goingdown;
	int			weapchange;

	int			faceframe;

	boolean	godmode;
	long	godmodecount;
	boolean	berserk;
	long	berserkcount;
	boolean	goggles;
	long	gogglescount;
	boolean	biosuit;
	long	biosuitcount;
	boolean invisibility;
	long	invisibilitycount;
	int			attackframe,attackcount,weaponframe;

	int			episode,secretcount,treasurecount,killcount,
				secrettotal,treasuretotal,killtotal;
	long		TimeCount;
	long		killx,killy;
	boolean		victoryflag;		// set during victory animations
	char		message[40];
} gametype;

typedef struct  {
	boolean	messages;
	boolean floorsceilings;
	boolean paralaxsky;
} switchtype;

typedef	enum	{
	ex_stillplaying,
	ex_completed,
	ex_died,
	ex_warped,
	ex_resetgame,
	ex_loadedgame,
	ex_victorious,
	ex_abort,
	ex_demodone,
	ex_secretlevel
} exit_t;


/*
=============================================================================

						 WL_MAIN DEFINITIONS

=============================================================================
*/

extern	boolean		MS_CheckParm (char far *string);

extern	char		str[80],str2[20];
extern	boolean		nospr;
extern	boolean		IsA386;

extern	byte far	*scalermemory;

extern	fixed		focallength;
extern	unsigned	viewangles;
extern	unsigned	screenofs;
extern	unsigned	cgascreenofs;
extern	int		    viewwidth;
extern	int			viewheight;
extern	int			centerx;
extern	int			shootdelta;

extern	boolean		timedemo;

extern	int			dirangle[9];

extern	boolean         startgame,loadedgame,virtualreality;
extern	int		mouseadjustment;
//
// math tables
//
extern	int			pixelangle[MAXVIEWWIDTH];
extern	long		far finetangent[FINEANGLES/4];
extern	fixed 		far sintable[],far *costable;

//
// derived constants
//
extern	fixed 	scale,maxslope;
extern	long	heightnumerator;
extern	int		minheightdiv;

extern	char	configname[13];



void		HelpScreens (void);
void		OrderingInfo (void);
void		TEDDeath(void);
void		Quit (char *error);
void 		CalcProjection (long focal);
boolean		SetViewSize (unsigned width, unsigned height);
void		NewGame (int difficulty,int episode);
void 		NewViewSize (int width);
boolean 	LoadTheGame(int file,int x,int y);
boolean		SaveTheGame(int file,int x,int y);
void 		ShowViewSize (int width);
void		ShutdownId (void);


/*
=============================================================================

						 WL_GAME DEFINITIONS

=============================================================================
*/


extern	boolean		ingame,fizzlein;
extern	unsigned	latchpics[NUMLATCHPICS];
extern	gametype	gamestate;
switchtype	switches;
extern	int			doornum;

extern	char		demoname[13];

extern	long		spearx,speary;
extern	unsigned	spearangle;
extern	boolean		spearflag;


void 	DrawPlayBorder (void);
void 	ScanInfoPlane (void);
void	SetupGameLevel (void);
void 	NormalScreen (void);
void 	DrawPlayScreen (void);
void 	FizzleOut (void);
void 	GameLoop (void);
void ClearMemory (void);
void PlayDemo (int demonumber);
void RecordDemo (void);
void DrawAllPlayBorder (void);
void	DrawHighScores(void);
void DrawAllPlayBorderSides (void);


// JAB
#define	PlaySoundLocTile(s,tx,ty)	PlaySoundLocGlobal(s,(((long)(tx) << TILESHIFT) + (1L << (TILESHIFT - 1))),(((long)ty << TILESHIFT) + (1L << (TILESHIFT - 1))))
#define	PlaySoundLocActor(s,ob)		PlaySoundLocGlobal(s,(ob)->x,(ob)->y)
void	PlaySoundLocGlobal(word s,fixed gx,fixed gy);
void UpdateSoundLoc(void);


/*
=============================================================================

						 WL_PLAY DEFINITIONS

=============================================================================
*/

#ifdef SPEAR
extern	long		funnyticount;		// FOR FUNNY BJ FACE
#endif

extern	exit_t		playstate;

extern	boolean		madenoise;

extern	objtype 	objlist[MAXACTORS],*new,*obj,*player,*lastobj,
					*objfreelist,*killerobj;
extern	statobj_t	statobjlist[MAXSTATS],*laststatobj;
extern	doorobj_t	doorobjlist[MAXDOORS],*lastdoorobj;

extern	unsigned	farmapylookup[MAPSIZE];
extern	byte		*nearmapylookup[MAPSIZE];

extern	byte		tilemap[MAPSIZE][MAPSIZE];	// wall values only
extern	byte		spotvis[MAPSIZE][MAPSIZE];
extern	objtype		*actorat[MAPSIZE][MAPSIZE];

#define UPDATESIZE			(UPDATEWIDE*UPDATEHIGH)
extern	byte		update[UPDATESIZE];

extern	boolean		singlestep,godmode,invisibility,noclip,berserk,biosuit,godflag;
extern	int			extravbls;

//
// control info
//
extern	boolean		mouseenabled,joystickenabled,joypadenabled,joystickprogressive;
extern	int			joystickport;
extern	int			dirscan[4];
extern	int			buttonscan[NUMBUTTONS];
extern	int			buttonmouse[4];
extern	int			buttonjoy[4];

extern	boolean		buttonheld[NUMBUTTONS];

extern	int			viewsize;

//
// curent user input
//
extern	int			controlx,controly,strafe;		// range from -100 to 100
extern	boolean		buttonstate[NUMBUTTONS];

extern	boolean		demorecord,demoplayback;
extern	char		far *demoptr, far *lastdemoptr;
extern	memptr		demobuffer;



void	InitRedShifts (void);
void 	FinishPaletteShifts (void);

void	CenterWindow(word w,word h);
void 	InitActorList (void);
void 	GetNewActor (void);
void 	RemoveObj (objtype *gone);
void 	PollControls (void);
void 	StopMusic(void);
void 	StartMusic(void);
void	PlayLoop (void);
void StartDamageFlash (int damage);
void StartBonusFlash (void);
void StartBioFlash (void);

/*
=============================================================================

							WL_INTER

=============================================================================
*/

void IntroScreen (void);
void PreloadGraphics(void);
void LevelCompleted (void);
void	CheckHighScore (long score,word other);
void Victory (void);
void ClearSplitVWB (void);


/*
=============================================================================

							WL_DEBUG

=============================================================================
*/

int DebugKeys (void);
void PicturePause (void);


/*
=============================================================================

						 WL_DRAW DEFINITIONS

=============================================================================
*/

typedef struct
{
	unsigned floor1, floor2;
	unsigned ceiling1, ceiling2;
} t_floorcolors;

extern t_floorcolors floorcolors[];
extern unsigned weaponviewheight;

extern	unsigned screenloc[3];
extern	unsigned freelatch;

extern	long 	lasttimecount;
extern	long 	frameon;
int	finaltime;

extern	boolean	fizzlein;

extern	unsigned	wallheight[MAXVIEWWIDTH];

extern	fixed	tileglobal;
extern	fixed	focallength;
extern	fixed	mindist;

//
// math tables
//
extern	int			pixelangle[MAXVIEWWIDTH];
extern	long		far finetangent[FINEANGLES/4];
extern	fixed 		far sintable[],far *costable;

//
// derived constants
//
extern	fixed 	scale;
extern	long	heightnumerator,mindist;

//
// refresh variables
//
extern	fixed	viewx,viewy;			// the focal point
extern	int		viewangle;
extern	fixed	viewsin,viewcos;

extern	long		postsource;
extern	unsigned	postx;
extern	unsigned	postwidth;


extern	int		horizwall[],vertwall[];

extern	unsigned	pwallpos;


fixed	FixedByFrac (fixed a, fixed b);
void	TransformActor (objtype *ob);
void	BuildTables (void);
void	ClearScreen (void);
int		CalcRotate (objtype *ob);
void	DrawScaleds (void);
void	CalcTics (void);
void	FixOfs (void);
void	ThreeDRefresh (void);
void  FarScalePost (void);
void   GetMessage (char *lastmessage);
void   DrawMessage (void);
void	DrawGameScore (void);
void	BonusTimer (void);
void DrawBonusTimer (void);

/*
=============================================================================

						 WL_STATE DEFINITIONS

=============================================================================
*/
#define TURNTICS	10
#define SPDPATROL	512
#define SPDDOG		1500


extern	dirtype opposite[9];
extern	dirtype diagonal[9][9];


void	InitHitRect (objtype *ob, unsigned radius);
void	SpawnNewObj (unsigned tilex, unsigned tiley, statetype *state);
void	NewState (objtype *ob, statetype *state);

boolean TryWalk (objtype *ob);
void 	SelectChaseDir (objtype *ob);
void 	SelectDodgeDir (objtype *ob);
void	SelectRunDir (objtype *ob);
void	MoveObj (objtype *ob, long move);
boolean SightPlayer (objtype *ob);

void	KillActor (objtype *ob);
void	DamageActor (objtype *ob, unsigned damage);

boolean CheckLine (objtype *ob);
boolean	CheckSight (objtype *ob);


/*
=============================================================================

						 WL_SCALE DEFINITIONS

=============================================================================
*/


#define COMPSCALECODESTART	(65*4)		// offset to start of code in comp scaler

typedef struct
{
	unsigned	codeofs[65];
	unsigned	width[65];
	byte		code[];
}	t_compscale;

typedef struct
{
	unsigned	leftpix,rightpix;
	unsigned	dataofs[64];
// table data after dataofs[rightpix-leftpix+1]
}	t_compshape;


extern	t_compscale _seg *scaledirectory[MAXSCALEHEIGHT+1];
extern	long			fullscalefarcall[MAXSCALEHEIGHT+1];

extern	byte		bitmasks1[8][8];
extern	byte		bitmasks2[8][8];
extern	unsigned	wordmasks[8][8];

extern	byte		mapmasks1[4][8];
extern	byte		mapmasks2[4][8];
extern	byte		mapmasks3[4][8];

extern	int			maxscale,maxscaleshl2;

extern	boolean	insetupscaling;
extern	boolean		adjustherculesaspect;
extern	boolean		usewiderendering;

void SetupScaling (int maxscaleheight);
void ScaleShape (int xcenter, int shapenum, unsigned height);
void SimpleScaleShape (int xcenter, int shapenum, unsigned height);

/*
=============================================================================

						 WL_AGENT DEFINITIONS

=============================================================================
*/

//
// player state info
//
extern	boolean		running;
extern	long		thrustspeed;
extern	unsigned	plux,pluy;		// player coordinates scaled to unsigned
extern	int warpwx,warpwy,warpex,warpey;

extern	int			anglefrac;
extern	int			facecount;
extern	int			cheatcount,cheatpos;

void	SpawnPlayer (int tilex, int tiley, int dir);
void 	DrawFace (void);
void	DrawHealth (void);
void	TakeDamage (int points,objtype *attacker);
void	HealSelf (int points);
void	DrawArmor (void);
void	GiveArmor (int points);
void	DrawLives (void);
void	GiveExtraMan (void);
void	GivePoints (long points);
void	DrawWeapon (void);
void	DrawKeys (void);
void	GiveWeapon (int weapon);
void	DrawAmmo (void);
void	GiveAmmo (int ammo,int ammotype);
void	GiveKey (int key);
void	GetBonus (statobj_t *check);

void	Thrust (int angle, long speed);

/*
=============================================================================

						 WL_ACT1 DEFINITIONS

=============================================================================
*/

extern	doorobj_t	doorobjlist[MAXDOORS],*lastdoorobj;
extern	int			doornum;

extern	unsigned	doorposition[MAXDOORS],pwallstate;

extern	byte		far areaconnect[NUMAREAS][NUMAREAS];

extern	boolean		areabyplayer[NUMAREAS];

extern unsigned	pwallstate;
extern unsigned	pwallpos;			// amount a pushable wall has been moved (0-63)
extern unsigned	pwallx,pwally;
extern int			pwalldir;


void InitDoorList (void);
void InitStaticList (void);
void SpawnStatic (int tilex, int tiley, int type);
void SpawnDoor (int tilex, int tiley, boolean vertical, int lock);
void MoveDoors (void);
void MovePWalls (void);
void OpenDoor (int door);
void PlaceItemType (int itemtype, int tilex, int tiley);
void PushWall (int checkx, int checky, int dir);
void OperateDoor (int door);
void InitAreas (void);

#define s_nakedbody s_static10

extern	statetype s_mutantdie1;
extern	statetype s_mutantexpl1;
extern	statetype s_demondie1;
extern	statetype s_chainguydie1;
extern	statetype s_chainguyexpl1;
extern	statetype s_shotguydie1;
extern	statetype s_shotguyexpl1;
extern	statetype s_impdie1;
extern	statetype s_impexpl1;
extern	statetype s_paindie1;
extern	statetype s_souldie1;
extern	statetype s_cacodie1;
extern	statetype s_cyberdie1;
extern	statetype s_spiderdie1;
extern	statetype s_revenantdie1;
extern	statetype s_bossdie1;
extern	statetype s_boss2die1;
extern	statetype s_mancubusdie1;
extern	statetype s_viledie1;
extern	statetype s_barreldie1;

extern	statetype s_mutantchase1;
extern	statetype s_demonchase1;
extern	statetype s_chainguychase1;
extern	statetype s_cacochase1;
extern	statetype s_impchase1;
extern	statetype s_painchase1;
extern	statetype s_soulchase1;
extern	statetype s_shotguychase1;
extern	statetype s_cyberchase1;
extern	statetype s_spiderchase1;
extern	statetype s_revenantchase1;
extern	statetype s_mancubuschase1;
extern	statetype s_vilechase1;

extern	statetype s_bosschase1;
extern	statetype s_boss2chase1;

extern	statetype s_mutantpain;
extern	statetype s_demonpain;
extern	statetype s_chainguypain;
extern	statetype s_cacopain;
extern	statetype s_imppain;
extern	statetype s_painpain;
extern	statetype s_soulpain;
extern	statetype s_shotguypain;

extern	statetype s_impballboom1;
extern	statetype s_bossballboom1;
extern	statetype s_cacoballboom1;
extern	statetype s_fatballboom1;
extern	statetype s_splasmaboom1;
extern	statetype s_vilefire1;
extern	statetype s_revballboom1;

void SpawnStand (enemy_t which, int tilex, int tiley, int dir);
void SpawnPatrol (enemy_t which, int tilex, int tiley, int dir);
void KillActor (objtype *ob);

void	US_ControlPanel(byte);

void SpawnDeadGuard (int tilex, int tiley);
void SpawnCyber (int tilex, int tiley);
void SpawnSpider (int tilex, int tiley);
void SpawnMancubus (int tilex, int tiley);
void SpawnRevenant (int tilex, int tiley);
void SpawnVile (int tilex, int tiley);
void SpawnBoss (int tilex, int tiley);
void SpawnBoss2 (int tilex, int tiley);
void SpawnBarrel (int tilex, int tiley);

/*
=============================================================================

						 WL_TEXT DEFINITIONS

=============================================================================
*/

extern	void	HelpScreens(void);
extern	void	EndText(void);

boolean paralax;
int startpage;
boolean CSLOGOAnim(void);
boolean DoAnimation(void);

int			DebugOk;

/*
	Profiler system
*/
//#define WITH_PROFILER

enum
{
	PROF_THREEDREFRESH,
	PROF_WALLREFRESH,
	PROF_SCALEPOST,
	PROF_DRAWSCALED,
	PROF_DRAWWEAPON,
	PROF_CGABLIT,
	NUM_PROFILER_MARKERS
};

#ifdef WITH_PROFILER
typedef struct
{
	const char* name;
	unsigned long start;
	unsigned long total;
} profilermarker_t;

extern profilermarker_t profilermarkers[NUM_PROFILER_MARKERS];

#define BEGIN_PROFILE(x) profilermarkers[x].start = TimeCount;
#define END_PROFILE(x) profilermarkers[x].total += (TimeCount - profilermarkers[x].start);
#else
#define BEGIN_PROFILE(x)
#define END_PROFILE(x)
#endif

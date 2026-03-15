//
// Sound names & indexes
//
typedef enum {
	BARRELBOOMSND,			 // 0
	ATKBFGSND,			 // 1
	GROWLSND,			 // 2
	MUTDIE1SND,			 // 3
	MUTDIE2SND,			 // 4
	MUTSEE1SND,			 // 5
	MUTSEE2SND,			 // 6
	BOSSDIESND,			 // 7
	BOSSSEESND,			 // 8
	CACODIESND,			 // 9
	CACOSEESND,			 // 10
	CYBERDIESND,			 // 11
	CYBERSEESND,			 // 12
	PAINSND,			 // 13
	DOORCLOSESND,			 // 14
	DOOROPENSND,			 // 15
	IMPFIRESND,			 // 16
	IMPFIREBOOMSND,			 // 17
	BONUSITEMUPSND,			 // 18
	ITEMUPSND,			 // 19
	NOWAYSND,			 // 20
	PISTOLFIRESND,			 // 21
	PLASMAGUNFIRESND,			 // 22
	PLAYERDIESND,			 // 23
	PLAYERPAINSND,			 // 24
	SHOTGUYDIE1SND,			 // 25
	SHOTGUYDIE2SND,			 // 26
	SHOTGUYDIE3SND,			 // 27
	SHOTGUYPAINSND,			 // 28
	SHOTGUYSEE1SND,			 // 29
	SHOTGUYSEE2SND,			 // 30
	SHOTGUYSEE3SND,			 // 31
	FISTPUNCHSND,			 // 32
	RLAUNCHERFIRESND,			 // 33
	RLAUNCHERBOOM1SND,			 // 34
	CHAINSAWFIRESND,			 // 35
	CHAINSAWIDLESND,			 // 36
	DEMONFIRESND,			 // 37
	DEMONDIESND,			 // 38
	DEMONSEESND,			 // 39
	SHOTGUNFIRESND,			 // 40
	SPIDERDIESND,			 // 41
	SPIDERSEESND,			 // 42
	SWITCH1SND,			 // 43
	SWITCH2SND,			 // 44
	TELEPORTSND,			 // 45
	WEAPONPICKUPSND,			 // 46
	DSHOTGUNFIRESND,			 // 47
	DSOPENSND,			 // 48
	DSLOADSND,			 // 49
	DSCLOSESND,			 // 50
	MOVEUPDOWNSND,			 // 51
	WTFSND,				 // 52
	SLOPSND,		         // 53
	DMACTSND,		         // 54
	RISINGSND,			 // 55
	DTHKNTSEESND,                    // 56
	DTHKNTDIESND,                    // 57
	MANATKSND,                       // 58
	MANDIESND,                       // 59
	MANSEESND,                       // 60
	MANPAINSND,                      // 61
	PAINDIESND,                      // 62
	PAINPAINSND,                     // 63
	PAINSEESND,                      // 64
	REVATKSND,                       // 65
	REVDIESND,                       // 66
	REVPUNCHSND,                     // 67
	REVSEESND,                       // 68
	REVSWINGSND,                     // 69
	VILEATKSND,                      // 70
	VILEDIESND,                      // 71
	VILESEESND,                      // 72
	VILEPAINSND,                     // 73
	SOULATKSND,                      // 74
	TELE2SND,                        // 75
	LASTSOUND
	 } soundnames;

// Music names & indexes
typedef enum {
	D_TITLE_MUS,			 // 0
	D_BETWEEN_MUS,			 // 1
	D_DDTBLU_MUS,			 // 2
	D_DEAD_MUS,			 // 3
	D_E1M2_MUS,			 // 4
	D_E1M3_MUS,			 // 5
	D_E1M4_MUS,			 // 6
	D_E1M9_MUS,			 // 7
	D_E2M1_MUS,			 // 8
	D_E2M2_MUS,			 // 9
	D_E2M6_MUS,			 // 10
	D_E2M8_MUS,			 // 11
	D_E3M1_MUS,			 // 12
	D_E3M2_MUS,			 // 13
	D_E3M3_MUS,			 // 14
	D_IN_CIT_MUS,			 // 15
	D_READ_M_MUS,			 // 16
	D_RUNNIN_MUS,			 // 17
	D_SHAWN_MUS,			 // 18
	D_STALKS_MUS,			 // 19
	D_VICTOR_MUS,			 // 20
	DESCENT_MUS,			 // 21
	D_QUADM_MUS,			 // 22
	D_MMENU_MUS,			 // 23
	D_UNDERW_MUS,			 // 24
	LASTMUSIC
	} musicnames;

// Base offsets

#define NUMSOUNDS           LASTSOUND
#define NUMSNDCHUNKS        NUMSOUNDS*3+LASTMUSIC

#define STARTPCSOUNDS       0
#define STARTADLIBSOUNDS    NUMSOUNDS
#define STARTDIGISOUNDS     NUMSOUNDS*2
#define STARTMUSIC          NUMSOUNDS*3
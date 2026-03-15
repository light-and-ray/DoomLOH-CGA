// WL_DEBUG.C

#include "WL_DEF.H"
#pragma hdrstop
#include <BIOS.H>

int DebugKeys (void);

void PicturePause (void)
{
	int			i;
	byte		p;
	unsigned	x;
	byte		far	*dest,far *src;
	memptr		buffer;

	VW_ColorBorder (15);
	FinishPaletteShifts ();

	LastScan = 0;
	while (!LastScan)
	;
	if (LastScan != sc_Enter)
	{
		VW_ColorBorder (0);
		return;
	}

	VW_ColorBorder (1);
	VW_SetScreen (0,0);
//
// vga stuff...
//

	ClearMemory ();
	CA_SetAllPurge();
	MM_GetPtr (&buffer,64000);
	for (p=0;p<4;p++)
	{
	   src = MK_FP(0xa000,displayofs);
	   dest = (byte far *)buffer+p;
	   VGAREADMAP(p);
	   for (x=0;x<16000;x++,dest+=4)
		   *dest = *src++;
	}


#if 0
	for (p=0;p<4;p++)
	{
		src = MK_FP(0xa000,0);
		dest = (byte far *)buffer+51200+p;
		VGAREADMAP(p);
		for (x=0;x<3200;x++,dest+=4)
			*dest = *src++;
	}
#endif

	asm	mov	ax,0x13
	asm	int	0x10

	dest = MK_FP(0xa000,0);
	_fmemcpy (dest,buffer,64000);

	VL_SetPalette (&gamepal);


	IN_Shutdown ();

	VW_WaitVBL(70);
	bioskey(0);
	VW_WaitVBL(70);
	Quit (NULL);
}
int DebugKeys (void)
{
	boolean esc;
	int level,i;

	if (Keyboard[sc_G])		// G = god mode
	{
		CenterWindow (12,2);
		if (godmode)
		  US_PrintCentered ("God mode OFF");
		else
		  US_PrintCentered ("God mode ON");
		  VW_UpdateScreen();
		  IN_Ack();
		godmode ^= 1;
		godflag ^= 1;
		return 1;
	}
	else if (Keyboard[sc_I])
	{
		if (gamestate.mapon==10)
		{
			CenterWindow (32,2);
			US_PrintCentered ("Cheat disabled on final level.");
		}
		else
		{
		CenterWindow (12,2);
		US_PrintCentered ("Items Cheat");
		GiveBackPack();
		gamestate.bestweapon = 511;
		gamestate.ammo = 400;
		gamestate.ammo2 = gamestate.ammo3 = 100;
		gamestate.ammo4 = 600;
		gamestate.keys = 7;
		DrawAmmo ();
		DrawKeys ();
		DrawWeapon ();
                }
		VW_UpdateScreen ();
		IN_Ack ();
		return 1;
	}
	else if (Keyboard[sc_W])			// W = warp to level
	{
		CenterWindow(26,3);
		PrintY+=6;
		US_Print("  Warp to which level(1-10):");
		VW_UpdateScreen();
		esc = !US_LineInput (px,py,str,NULL,true,2,0);
		if (!esc)
		{
			level = atoi (str);

			if (level>0 && level<11)
			{
				gamestate.mapon = level-1;
				playstate = ex_warped;
			}
		}
		return 1;
	}
	return 0;
}
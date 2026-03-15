// WL_STATE.C

#include "WL_DEF.H"
#pragma hdrstop

extern	statetype s_splat1;
extern	statetype s_puff1;
extern	statetype s_bfgsplat1;

dirtype opposite[9] =
	{west,southwest,south,southeast,east,northeast,north,northwest,nodir};

dirtype diagonal[9][9] =
{
	{nodir,nodir,northeast,nodir,nodir,nodir,southeast,nodir,nodir},
			{nodir,nodir,nodir,nodir,nodir,nodir,nodir,nodir,nodir},
 {northeast,nodir,nodir,nodir,northwest,nodir,nodir,nodir,nodir},
			{nodir,nodir,nodir,nodir,nodir,nodir,nodir,nodir,nodir},
 {nodir,nodir,northwest,nodir,nodir,nodir,southwest,nodir,nodir},
			{nodir,nodir,nodir,nodir,nodir,nodir,nodir,nodir,nodir},
 {southeast,nodir,nodir,nodir,southwest,nodir,nodir,nodir,nodir},
			{nodir,nodir,nodir,nodir,nodir,nodir,nodir,nodir,nodir},
			{nodir,nodir,nodir,nodir,nodir,nodir,nodir,nodir,nodir}
};

void	SpawnNewObj (unsigned tilex, unsigned tiley, statetype *state);
void	NewState (objtype *ob, statetype *state);

boolean TryWalk (objtype *ob);
void	MoveObj (objtype *ob, long move);

void	KillActor (objtype *ob);
void	DamageActor (objtype *ob, unsigned damage);

boolean CheckLine (objtype *ob);
void FirstSighting (objtype *ob);
boolean	CheckSight (objtype *ob);

void SpawnNewObj (unsigned tilex, unsigned tiley, statetype *state)
{
	GetNewActor ();
	new->state = state;
	if (state->tictime)
		new->ticcount = US_RndT () % state->tictime;
	else
		new->ticcount = 0;

	new->tilex = tilex;
	new->tiley = tiley;
	new->x = ((long)tilex<<TILESHIFT)+TILEGLOBAL/2;
	new->y = ((long)tiley<<TILESHIFT)+TILEGLOBAL/2;
	new->dir = nodir;

	actorat[tilex][tiley] = new;
	new->areanumber =
		*(mapsegs[0] + farmapylookup[new->tiley]+new->tilex) - AREATILE;
}

void NewState (objtype *ob, statetype *state)
{
	ob->state = state;
	ob->ticcount = state->tictime;
}

#define CHECKDIAG(x,y)								\
{                                                   \
	temp=(unsigned)actorat[x][y];                   \
	if (temp)                                       \
	{                                               \
		if (temp<256)                               \
			return false;                           \
		if (((objtype *)temp)->flags&FL_SHOOTABLE)  \
			return false;                           \
	}                                               \
}

#define CHECKSIDE(x,y)								\
{                                                   \
	temp=(unsigned)actorat[x][y];                   \
	if (temp)                                       \
	{                                               \
		if (temp<128)                               \
			return false;                           \
		if (temp<256)                               \
			doornum = temp&63;                      \
		else if (((objtype *)temp)->flags&FL_SHOOTABLE)\
			return false;                           \
	}                                               \
}

boolean TryWalk (objtype *ob)
{
	int			doornum;
	unsigned	temp;

	doornum = -1;

	if (ob->obclass == inertobj)
	{
		switch (ob->dir)
		{
		case north:
			ob->tiley--;
			break;

		case northeast:
			ob->tilex++;
			ob->tiley--;
			break;

		case east:
			ob->tilex++;
			break;

		case southeast:
			ob->tilex++;
			ob->tiley++;
			break;

		case south:
			ob->tiley++;
			break;

		case southwest:
			ob->tilex--;
			ob->tiley++;
			break;

		case west:
			ob->tilex--;
			break;

		case northwest:
			ob->tilex--;
			ob->tiley--;
			break;
		}
	}
	else
		switch (ob->dir)
		{
		case north:
			if (ob->obclass == demonobj)
			{
				CHECKDIAG(ob->tilex,ob->tiley-1);
			}
			else
			{
				CHECKSIDE(ob->tilex,ob->tiley-1);
			}
			ob->tiley--;
			break;

		case northeast:
			CHECKDIAG(ob->tilex+1,ob->tiley-1);
			CHECKDIAG(ob->tilex+1,ob->tiley);
			CHECKDIAG(ob->tilex,ob->tiley-1);
			ob->tilex++;
			ob->tiley--;
			break;

		case east:
			if (ob->obclass == demonobj)
			{
				CHECKDIAG(ob->tilex+1,ob->tiley);
			}
			else
			{
				CHECKSIDE(ob->tilex+1,ob->tiley);
			}
			ob->tilex++;
			break;

		case southeast:
			CHECKDIAG(ob->tilex+1,ob->tiley+1);
			CHECKDIAG(ob->tilex+1,ob->tiley);
			CHECKDIAG(ob->tilex,ob->tiley+1);
			ob->tilex++;
			ob->tiley++;
			break;

		case south:
			if (ob->obclass == demonobj)
			{
				CHECKDIAG(ob->tilex,ob->tiley+1);
			}
			else
			{
				CHECKSIDE(ob->tilex,ob->tiley+1);
			}
			ob->tiley++;
			break;

		case southwest:
			CHECKDIAG(ob->tilex-1,ob->tiley+1);
			CHECKDIAG(ob->tilex-1,ob->tiley);
			CHECKDIAG(ob->tilex,ob->tiley+1);
			ob->tilex--;
			ob->tiley++;
			break;

		case west:
			if (ob->obclass == demonobj)
			{
				CHECKDIAG(ob->tilex-1,ob->tiley);
			}
			else
			{
				CHECKSIDE(ob->tilex-1,ob->tiley);
			}
			ob->tilex--;
			break;

		case northwest:
			CHECKDIAG(ob->tilex-1,ob->tiley-1);
			CHECKDIAG(ob->tilex-1,ob->tiley);
			CHECKDIAG(ob->tilex,ob->tiley-1);
			ob->tilex--;
			ob->tiley--;
			break;

		case nodir:
			return false;

		default:
			Quit ("Walk: Bad dir");
		}

	if (doornum != -1)
	{
		OpenDoor (doornum);
		ob->distance = -doornum-1;
		return true;
	}
	ob->areanumber =
		*(mapsegs[0] + farmapylookup[ob->tiley]+ob->tilex) - AREATILE;

	ob->distance = TILEGLOBAL;
	return true;
}

void SelectDodgeDir (objtype *ob)
{
	int 		deltax,deltay,i;
	unsigned	absdx,absdy;
	dirtype 	dirtry[5];
	dirtype 	turnaround,tdir;

	if (ob->flags & FL_FIRSTATTACK)
	{
		turnaround = nodir;
		ob->flags &= ~FL_FIRSTATTACK;
	}
	else
		turnaround=opposite[ob->dir];

	deltax = player->tilex - ob->tilex;
	deltay = player->tiley - ob->tiley;

	if (deltax>0)
	{
		dirtry[1]= east;
		dirtry[3]= west;
	}
	else
	{
		dirtry[1]= west;
		dirtry[3]= east;
	}

	if (deltay>0)
	{
		dirtry[2]= south;
		dirtry[4]= north;
	}
	else
	{
		dirtry[2]= north;
		dirtry[4]= south;
	}

	absdx = abs(deltax);
	absdy = abs(deltay);

	if (absdx > absdy)
	{
		tdir = dirtry[1];
		dirtry[1] = dirtry[2];
		dirtry[2] = tdir;
		tdir = dirtry[3];
		dirtry[3] = dirtry[4];
		dirtry[4] = tdir;
	}

	if (US_RndT() < 128)
	{
		tdir = dirtry[1];
		dirtry[1] = dirtry[2];
		dirtry[2] = tdir;
		tdir = dirtry[3];
		dirtry[3] = dirtry[4];
		dirtry[4] = tdir;
	}

	dirtry[0] = diagonal [ dirtry[1] ] [ dirtry[2] ];

	for (i=0;i<5;i++)
	{
		if ( dirtry[i] == nodir || dirtry[i] == turnaround)
			continue;

		ob->dir = dirtry[i];
		if (TryWalk(ob))
			return;
	}

	if (turnaround != nodir)
	{
		ob->dir = turnaround;

		if (TryWalk(ob))
			return;
	}

	ob->dir = nodir;
}

void SelectChaseDir (objtype *ob)
{
	int deltax,deltay,i;
	dirtype d[3];
	dirtype tdir, olddir, turnaround;
	olddir=ob->dir;
	turnaround=opposite[olddir];

	deltax=player->tilex - ob->tilex;
	deltay=player->tiley - ob->tiley;

	d[1]=nodir;
	d[2]=nodir;

	if (deltax>0)
		d[1]= east;
	else if (deltax<0)
		d[1]= west;
	if (deltay>0)
		d[2]=south;
	else if (deltay<0)
		d[2]=north;

	if (abs(deltay)>abs(deltax))
	{
		tdir=d[1];
		d[1]=d[2];
		d[2]=tdir;
	}

	if (d[1]==turnaround)
		d[1]=nodir;
	if (d[2]==turnaround)
		d[2]=nodir;


	if (d[1]!=nodir)
	{
		ob->dir=d[1];
		if (TryWalk(ob))
			return;
	}

	if (d[2]!=nodir)
	{
		ob->dir=d[2];
		if (TryWalk(ob))
			return;
	}

	if (olddir!=nodir)
	{
		ob->dir=olddir;
		if (TryWalk(ob))
			return;
	}

	if (US_RndT()>128)
	{
		for (tdir=north;tdir<=west;tdir++)
		{
			if (tdir!=turnaround)
			{
				ob->dir=tdir;
				if ( TryWalk(ob) )
					return;
			}
		}
	}
	else
	{
		for (tdir=west;tdir>=north;tdir--)
		{
			if (tdir!=turnaround)
			{
			  ob->dir=tdir;
			  if ( TryWalk(ob) )
				return;
			}
		}
	}

	if (turnaround !=  nodir)
	{
		ob->dir=turnaround;
		if (ob->dir != nodir)
		{
			if ( TryWalk(ob) )
				return;
		}
	}

	ob->dir = nodir;
}

void SelectRunDir (objtype *ob)
{
	int deltax,deltay,i;
	dirtype d[3];
	dirtype tdir, olddir, turnaround;


	deltax=player->tilex - ob->tilex;
	deltay=player->tiley - ob->tiley;

	if (deltax<0)
		d[1]= east;
	else
		d[1]= west;
	if (deltay<0)
		d[2]=south;
	else
		d[2]=north;

	if (abs(deltay)>abs(deltax))
	{
		tdir=d[1];
		d[1]=d[2];
		d[2]=tdir;
	}

	ob->dir=d[1];
	if (TryWalk(ob))
		return;

	ob->dir=d[2];
	if (TryWalk(ob))
		return;

	if (US_RndT()>128)
	{
		for (tdir=north;tdir<=west;tdir++)
		{
			ob->dir=tdir;
			if ( TryWalk(ob) )
				return;
		}
	}
	else
	{
		for (tdir=west;tdir>=north;tdir--)
		{
			ob->dir=tdir;
			if ( TryWalk(ob) )
			  return;
		}
	}

	ob->dir = nodir;
}

void MoveObj (objtype *ob, long move)
{
	long	deltax,deltay;

	switch (ob->dir)
	{
	case north:
		ob->y -= move;
		break;
	case northeast:
		ob->x += move;
		ob->y -= move;
		break;
	case east:
		ob->x += move;
		break;
	case southeast:
		ob->x += move;
		ob->y += move;
		break;
	case south:
		ob->y += move;
		break;
	case southwest:
		ob->x -= move;
		ob->y += move;
		break;
	case west:
		ob->x -= move;
		break;
	case northwest:
		ob->x -= move;
		ob->y -= move;
		break;

	case nodir:
		return;

	default:
		Quit ("MoveObj: bad dir!");
	}

	if (areabyplayer[ob->areanumber])
	{
		deltax = ob->x - player->x;
		if (deltax < -MINACTORDIST || deltax > MINACTORDIST)
			goto moveok;
		deltay = ob->y - player->y;
		if (deltay < -MINACTORDIST || deltay > MINACTORDIST)
			goto moveok;

		switch (ob->dir)
		{
		case north:
			ob->y += move;
			break;
		case northeast:
			ob->x -= move;
			ob->y += move;
			break;
		case east:
			ob->x -= move;
			break;
		case southeast:
			ob->x -= move;
			ob->y -= move;
			break;
		case south:
			ob->y -= move;
			break;
		case southwest:
			ob->x += move;
			ob->y -= move;
			break;
		case west:
			ob->x += move;
			break;
		case northwest:
			ob->x += move;
			ob->y += move;
			break;

		case nodir:
			return;
		}
		return;
	}
moveok:
	ob->distance -=move;
}

void DropItem (stat_t itemtype, int tilex, int tiley)
{
	int	x,y,xl,xh,yl,yh;

	if (!actorat[tilex][tiley])
	{
		PlaceItemType (itemtype, tilex,tiley);
		return;
	}

	xl = tilex-1;
	xh = tilex+1;
	yl = tiley-1;
	yh = tiley+1;

	for (x=xl ; x<= xh ; x++)
		for (y=yl ; y<= yh ; y++)
			if (!actorat[x][y])
			{
				PlaceItemType (itemtype, x,y);
				return;
			}
}

void KillActor (objtype *ob)
{
	int	tilex,tiley;

	tilex = ob->tilex = ob->x >> TILESHIFT;
	tiley = ob->tiley = ob->y >> TILESHIFT;

	switch (ob->obclass)
	{
	case mutantobj:
		GivePoints (100);
		if (ob->hitpoints<=-200)
		NewState (ob,&s_mutantexpl1);
		else
		NewState (ob,&s_mutantdie1);
		PlaceItemType (bo_ammoused,tilex,tiley);
		break;

	case chainguyobj:
		GivePoints (400);
		if (ob->hitpoints<=-280)
		NewState (ob,&s_chainguyexpl1);
		else
		NewState (ob,&s_chainguydie1);
		PlaceItemType (bo_chaingun,tilex,tiley);
		break;

	case impobj:
		GivePoints (200);
		if (ob->hitpoints<=-205)
		NewState (ob,&s_impexpl1);
		else
		NewState (ob,&s_impdie1);
		break;
	case painobj:
		GivePoints (500);
		NewState (ob,&s_paindie1);
		break;
	case soulobj:
		GivePoints (150);
		NewState (ob,&s_souldie1);
		break;
	case shotguyobj:
		GivePoints (200);
		if (ob->hitpoints<=-310)
		NewState (ob,&s_shotguyexpl1);
		else
		NewState (ob,&s_shotguydie1);
		PlaceItemType (bo_shotgun,tilex,tiley);
		break;

	case cacoobj:
		GivePoints (500);
		NewState (ob,&s_cacodie1);
		break;

	case demonobj:
		GivePoints (100);
		NewState (ob,&s_demondie1);
		break;

	case cyberobj:
		GivePoints (5000);
		NewState (ob,&s_cyberdie1);
		PlaceItemType (bo_key1,tilex,tiley);
		break;

	case mancubusobj:
		GivePoints (5000);
		gamestate.killx = player->x;
		gamestate.killy = player->y;
		NewState (ob,&s_mancubusdie1);
		break;
	case vileobj:
		GivePoints (5000);
		gamestate.killx = player->x;
		gamestate.killy = player->y;
		NewState (ob,&s_viledie1);
		break;

	case spiderobj:
		GivePoints (5000);
		gamestate.killx = player->x;
		gamestate.killy = player->y;
		NewState (ob,&s_spiderdie1);
		A_DeathScream(ob);
		break;

	case revenantobj:
		GivePoints (5000);
		NewState (ob,&s_revenantdie1);
		PlaceItemType (bo_key1,tilex,tiley);
		break;
	case bossobj:
		GivePoints (5000);
		NewState (ob,&s_bossdie1);
		A_DeathScream(ob);
		break;
	case boss2obj:
		GivePoints (5000);
		NewState (ob,&s_boss2die1);
		A_DeathScream(ob);
		break;
	case barrelobj:
		gamestate.killcount--;
		NewState (ob,&s_barreldie1);
		break;
	}

	gamestate.killcount++;
	ob->flags &= ~FL_SHOOTABLE;
	actorat[ob->tilex][ob->tiley] = NULL;
	ob->flags |= FL_NONMARK;
}

void DamageActor (objtype *ob, unsigned damage)
{
	madenoise = true;

	if ( !(ob->flags & FL_ATTACKMODE) )
		damage <<= 1;

	ob->hitpoints -= damage;

	if (ob->hitpoints<=0)
		KillActor (ob);
	else
	{
		if (! (ob->flags & FL_ATTACKMODE) )
			FirstSighting (ob);
	switch (ob->obclass)
	{
	case mutantobj:
		if (ob->hitpoints&1)
			NewState (ob,&s_mutantpain);
		else
			NewState (ob,&s_mutantpain);
		break;
	case chainguyobj:
		if (ob->hitpoints&1)
			NewState (ob,&s_chainguypain);
		else
			NewState (ob,&s_chainguypain);
		break;
	case impobj:
		if (ob->hitpoints&1)
			NewState (ob,&s_imppain);
		else
			NewState (ob,&s_imppain);
		break;
	case painobj:
		if (ob->hitpoints&1)
		{
			NewState (ob,&s_painpain);
			PlaySoundLocActor(PAINPAINSND,ob);
		}
		else
			NewState (ob,&s_painpain);
		break;
	case soulobj:
		if (ob->hitpoints&1)
			NewState (ob,&s_soulpain);
		else
			NewState (ob,&s_soulpain);
		break;
	case demonobj:
		if (ob->hitpoints&1)
		{
			NewState (ob,&s_demonpain);
			PlaySoundLocActor (PAINSND,ob);
		}
		else
			NewState (ob,&s_demonpain);
		break;
	case shotguyobj:
		if (ob->hitpoints&1)
			NewState (ob,&s_shotguypain);
		else
			NewState (ob,&s_shotguypain);
		break;
	case cacoobj:
		if (ob->hitpoints&1)
			NewState (ob,&s_cacopain);
		else
			NewState (ob,&s_cacopain);
		break;
		}
	{
	GetNewActor ();
		if (ob->obclass==barrelobj)
		new->state = &s_puff1;
		else
		new->state = &s_splat1;
		new->ticcount = 1;

		new->tilex = ob->tilex;
		new->tiley = ob->tiley;
		new->x = ob->x;
		new->y = ob->y;
		new->obclass = splatobj;
		new->dir= nodir;
		new->angle = ob->angle;
		new->speed = 0;

		new->flags = FL_NONMARK;
		new->active = true;
		}
	}
}

boolean CheckLine (objtype *ob)
{
	int	x1,y1,xt1,yt1,x2,y2,xt2,yt2;
	int	x,y;
	int	xdist,ydist,xstep,ystep;
	int	temp;
	int	partial,delta;
	long	ltemp;
	int	xfrac,yfrac,deltafrac;
	unsigned	value,intercept;

	x1 = ob->x >> UNSIGNEDSHIFT;
	y1 = ob->y >> UNSIGNEDSHIFT;
	xt1 = x1 >> 8;
	yt1 = y1 >> 8;

	x2 = plux;
	y2 = pluy;
	xt2 = player->tilex;
	yt2 = player->tiley;


	xdist = abs(xt2-xt1);

	if (xdist > 0)
	{
		if (xt2 > xt1)
		{
			partial = 256-(x1&0xff);
			xstep = 1;
		}
		else
		{
			partial = x1&0xff;
			xstep = -1;
		}

		deltafrac = abs(x2-x1);
		delta = y2-y1;
		ltemp = ((long)delta<<8)/deltafrac;
		if (ltemp > 0x7fffl)
			ystep = 0x7fff;
		else if (ltemp < -0x7fffl)
			ystep = -0x7fff;
		else
			ystep = ltemp;
		yfrac = y1 + (((long)ystep*partial) >>8);

		x = xt1+xstep;
		xt2 += xstep;
		do
		{
			y = yfrac>>8;
			yfrac += ystep;

			value = (unsigned)tilemap[x][y];
			x += xstep;

			if (!value)
				continue;

			if (value<128 || value>256)
				return false;

			value &= ~0x80;
			intercept = yfrac-ystep/2;

			if (intercept>doorposition[value])
				return false;

		} while (x != xt2);
	}

	ydist = abs(yt2-yt1);

	if (ydist > 0)
	{
		if (yt2 > yt1)
		{
			partial = 256-(y1&0xff);
			ystep = 1;
		}
		else
		{
			partial = y1&0xff;
			ystep = -1;
		}

		deltafrac = abs(y2-y1);
		delta = x2-x1;
		ltemp = ((long)delta<<8)/deltafrac;
		if (ltemp > 0x7fffl)
			xstep = 0x7fff;
		else if (ltemp < -0x7fffl)
			xstep = -0x7fff;
		else
			xstep = ltemp;
		xfrac = x1 + (((long)xstep*partial) >>8);

		y = yt1 + ystep;
		yt2 += ystep;
		do
		{
			x = xfrac>>8;
			xfrac += xstep;

			value = (unsigned)tilemap[x][y];
			y += ystep;

			if (!value)
				continue;

			if (value<128 || value>256)
				return false;

			value &= ~0x80;
			intercept = xfrac-xstep/2;

			if (intercept>doorposition[value])
				return false;
		} while (y != yt2);
	}

	return true;
}

#define MINSIGHT	0x18000l

boolean CheckSight (objtype *ob)
{
	long		deltax,deltay;
	if (!areabyplayer[ob->areanumber])
		return false;

	deltax = player->x - ob->x;
	deltay = player->y - ob->y;

	if (deltax > -MINSIGHT && deltax < MINSIGHT
	&& deltay > -MINSIGHT && deltay < MINSIGHT)
		return true;
	if (gamestate.invisibility && !madenoise) return false;
	switch (ob->dir)
	{
	case north:
		if (deltay > 0)
			return false;
		break;

	case east:
		if (deltax < 0)
			return false;
		break;

	case south:
		if (deltay < 0)
			return false;
		break;

	case west:
		if (deltax > 0)
			return false;
		break;
	}

	return CheckLine (ob);
}

void FirstSighting (objtype *ob)
{
	switch (ob->obclass)
	{
	case mutantobj:
		PlaySoundLocActor(SHOTGUYSEE1SND,ob);
		NewState (ob,&s_mutantchase1);
		ob->speed *= 3;
		break;

	case chainguyobj:
		PlaySoundLocActor(MUTSEE2SND,ob);
		NewState (ob,&s_chainguychase1);
		ob->speed *= 3;
		break;

	case impobj:
		PlaySoundLocActor(MUTSEE1SND,ob);
		NewState (ob,&s_impchase1);
		ob->speed *= 2;
		break;

	case painobj:
		PlaySoundLocActor(PAINSEESND,ob);
		NewState (ob,&s_painchase1);
		ob->speed *= 2;
		break;

	case soulobj:
		NewState (ob,&s_soulchase1);
		ob->speed *= 3;
		break;
	case shotguyobj:
		PlaySoundLocActor(SHOTGUYSEE1SND,ob);
		NewState (ob,&s_shotguychase1);
		ob->speed *= 2;
		break;

	case cacoobj:
		PlaySoundLocActor(CACOSEESND,ob);
		NewState (ob,&s_cacochase1);
		ob->speed *= 2;
		break;

	case demonobj:
		PlaySoundLocActor(DEMONSEESND,ob);
		NewState (ob,&s_demonchase1);
		ob->speed *= 2;
		break;

	case cyberobj:
		SD_PlaySound(CYBERSEESND);
		NewState (ob,&s_cyberchase1);
		ob->speed = SPDPATROL*3;
		break;

	case mancubusobj:
		SD_PlaySound(MANSEESND);
		NewState (ob,&s_mancubuschase1);
		ob->speed *= 3;
		break;
	case vileobj:
		SD_PlaySound(VILESEESND);
		NewState (ob,&s_vilechase1);
		ob->speed *= 3;
		break;
	case spiderobj:
		SD_PlaySound(SPIDERSEESND);
		NewState (ob,&s_spiderchase1);
		ob->speed *= 3;
		break;

	case revenantobj:
		SD_PlaySound(REVSEESND);
		NewState (ob,&s_revenantchase1);
		ob->speed *= 3;
		break;

	case bossobj:
		SD_PlaySound(BOSSSEESND);
		NewState (ob,&s_bosschase1);
		ob->speed *= 3;
		break;
	case boss2obj:
		SD_PlaySound(DTHKNTSEESND);
		NewState (ob,&s_boss2chase1);
		ob->speed *= 3;
		break;
	}

	if (ob->distance < 0)
		ob->distance = 0;

	ob->flags |= FL_ATTACKMODE|FL_FIRSTATTACK;
}

boolean SightPlayer (objtype *ob)
{
	if (ob->flags & FL_ATTACKMODE)
		Quit ("An actor in ATTACKMODE called SightPlayer!");

	if (ob->temp2)
	{
		ob->temp2 -= tics;
		if (ob->temp2 > 0)
			return false;
		ob->temp2 = 0;
	}
	else
	{
		if (!areabyplayer[ob->areanumber])
			return false;

		if (ob->flags & FL_AMBUSH)
		{
			if (!CheckSight (ob))
				return false;
			ob->flags &= ~FL_AMBUSH;
		}
		else
		{
			if (!madenoise && !CheckSight (ob))
				return false;
		}
		switch (ob->obclass)
		{
		case mutantobj:
			ob->temp2 = 1+US_RndT()/4;
			break;
		case chainguyobj:
			ob->temp2 = 2;
			break;
		case impobj:
		case painobj:
		case soulobj:
			ob->temp2 = 1+US_RndT()/4;
			break;
		case shotguyobj:
			ob->temp2 = 1+US_RndT()/6;
			break;
		case cacoobj:
			ob->temp2 = 1+US_RndT()/6;
			break;
		case demonobj:
			ob->temp2 = 1+US_RndT()/8;
			break;
		case cyberobj:
		case spiderobj:
		case revenantobj:
		case bossobj:
		case boss2obj:
		case mancubusobj:
		case vileobj:
			ob->temp2 = 1;
			break;
		}
		return false;
	}

	FirstSighting (ob);

	return true;
}
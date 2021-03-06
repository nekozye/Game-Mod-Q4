#include "../../idlib/precompiled.h"
#pragma hdrstop

#include "../Game_local.h"
#include "../Weapon.h"

#define BLASTER_SPARM_CHARGEGLOW		6

class rvWeaponBlaster : public rvWeapon {
public:

	CLASS_PROTOTYPE( rvWeaponBlaster );

	rvWeaponBlaster ( void );

	virtual void		Spawn				( void );
	void				Save				( idSaveGame *savefile ) const;
	void				Restore				( idRestoreGame *savefile );
	void				PreSave		( void );
	void				PostSave	( void );
	//Engineering Mod START
	void				SpawnEntityBasedOnContact(void);
	idVec3				HitFirstTrace(const idDict& hitscanDict, const idVec3& origOrigin, const idVec3& origDir, const idVec3& origFxOrigin, idEntity* owner, bool noFX, idEntity* additionalIgnore, int areas[2]);
	//Engineering Mod END

protected:

	bool				UpdateAttack		( void );
	bool				UpdateFlashlight	( void );
	void				Flashlight			( bool on );

private:

	int					chargeTime;
	int					chargeDelay;
	idVec2				chargeGlow;
	bool				fireForced;
	int					fireHeldTime;

	//engineering mod start
	static char*		spawnEntity;
	//engineering mod end

	stateResult_t		State_Raise				( const stateParms_t& parms );
	stateResult_t		State_Lower				( const stateParms_t& parms );
	stateResult_t		State_Idle				( const stateParms_t& parms );
	stateResult_t		State_Charge			( const stateParms_t& parms );
	stateResult_t		State_Charged			( const stateParms_t& parms );
	stateResult_t		State_Fire				( const stateParms_t& parms );
	stateResult_t		State_Flashlight		( const stateParms_t& parms );
	
	CLASS_STATES_PROTOTYPE ( rvWeaponBlaster );
};

CLASS_DECLARATION( rvWeapon, rvWeaponBlaster )
END_CLASS

//Engineering Mod START
void rvWeaponBlaster::SpawnEntityBasedOnContact() {

	const idEntity* player;
	trace_t	tracer;
	idVec3 muzzleOrigin;
	idVec3 start;
	idVec3 end;
	idVec3	dir;
	idDict  dict;
	float	ang;
	float	spin;
	int		areas[2];
	
	
	
	player = owner;
	dict = attackDict;

	//calculate the muzzle origin
	if (barrelJointView != INVALID_JOINT && spawnArgs.GetBool("launchFromBarrel")) {
		// there is an explicit joint for the muzzle
		GetGlobalJointTransform(true, barrelJointView, muzzleOrigin, muzzleAxis);
	}
	else {
		// go straight out of the view
		muzzleOrigin = playerViewOrigin;
		muzzleAxis = playerViewAxis;
		muzzleOrigin += playerViewAxis[0] * muzzleOffset;
	}


	// Track down traces
	ang = 0;
	spin = 0;

	dir = playerViewAxis[0];
	dir.Normalize();


	//Start Hitscan Vector Mathing for constructing vector origining from the muzzle


	idVec3  fxOrigin;
	idMat3  fxAxis;

	GetGlobalJointTransform(true, flashJointView, fxOrigin, fxAxis, dict.GetVector("fxOriginOffset"));

	float spreadRad = DEG2RAD(spread);
		if (weaponDef->dict.GetBool("machinegunSpreadStyle")) {
			float r = gameLocal.random.RandomFloat() * idMath::PI * 2.0f;
			float u = idMath::Sin(r) * gameLocal.random.CRandomFloat() * spread * 16;
			r = idMath::Cos(r) * gameLocal.random.CRandomFloat() * spread * 16;

			end = muzzleOrigin + ((8192 * 16) * playerViewAxis[0]);
			end += (r * playerViewAxis[1]);
			end += (u * playerViewAxis[2]);

			dir = end - muzzleOrigin;
		}
		else if (weaponDef->dict.GetBool("shotgunSpreadStyle")) {
			float r = gameLocal.random.CRandomFloat() * spread * 16;
			float u = gameLocal.random.CRandomFloat() * spread * 16;
			end = muzzleOrigin + ((8192 * 16) * playerViewAxis[0]);
			end += (r * playerViewAxis[1]);
			end += (u * playerViewAxis[2]);
			dir = end - muzzleOrigin;
		}
		else {
			ang = idMath::Sin(spreadRad * gameLocal.random.RandomFloat());
			spin = (float)DEG2RAD(360.0f) * gameLocal.random.RandomFloat();
			dir = playerViewAxis[0] + playerViewAxis[2] * (ang * idMath::Sin(spin)) - playerViewAxis[1] * (ang * idMath::Cos(spin));
		}
		dir.Normalize();

		//trace using the tracer bound
		idVec3 firstPointToHit;
		
		firstPointToHit = HitFirstTrace(dict, muzzleOrigin, dir, fxOrigin, owner, false, NULL, areas);
		

		//"snap" the vector to the gridspace of turrets. 

		float pointz = firstPointToHit.z;

		firstPointToHit.SnapInt();

		firstPointToHit.z = pointz;

		float pointx = firstPointToHit.x;
		float pointy = firstPointToHit.y;

		pointx = pointx / 160.0f;

		//TODO: finish the "snapping" of the grid to largegrid in map



		//construct spawning entity
		idDict entityDict;
		float yaw;

		yaw = owner->viewAngles.yaw;
		

		//set class. this will be different. later if we make more, please change this to multiple turret.
		entityDict.Set("classname", "monster_turret");
		//set angle as 0, so new spawned turret faces one place, not the opposite of player.
		entityDict.Set("angle", "0");
		entityDict.Set("origin", firstPointToHit.ToString());

		


		idEntity *newEnt = NULL;
		gameLocal.SpawnEntityDef(entityDict, &newEnt);
	
}
		//Engineering Mod END

//hitscan copy

idVec3 rvWeaponBlaster::HitFirstTrace(
	const idDict&	hitscanDict,
	const idVec3&	origOrigin,
	const idVec3&	origDir,
	const idVec3&	origFxOrigin,
	idEntity*		owner,
	bool			noFX,
	// twhitaker: added additionalIgnore parameter
	idEntity*		additionalIgnore,
	int				areas[2])
{
	trace_t		tr;
	idVec3		dir;
	idVec3		origin;
	idVec3		fxOrigin;
	idVec3		fxDir;
	idVec3		impulse;
	idVec4		hitscanTint(1.0f, 1.0f, 1.0f, 1.0f);
	float		tracerChance;
	idEntity*	ignore;
	float		penetrate;

	if (areas) {
		areas[0] = gameLocal.pvs.GetPVSArea(origFxOrigin);
		areas[1] = -1;
	}

	ignore = owner;
	penetrate = hitscanDict.GetFloat("penetrate");

	if (hitscanDict.GetBool("hitscanTint") && owner->IsType(idPlayer::GetClassType())) {
		hitscanTint = ((idPlayer*)owner)->GetHitscanTint();
	}

	// twhitaker: additionalIgnore parameter
	if (!additionalIgnore) {
		additionalIgnore = ignore;
	}

	origin = origOrigin;
	fxOrigin = origFxOrigin;
	dir = origDir;
	tracerChance = ((g_perfTest_weaponNoFX.GetBool()) ? 0 : hitscanDict.GetFloat("tracerchance", "0"));

		idVec3		start;
		idVec3		end;
		idEntity*	ent;
		idEntity*	actualHitEnt;
		int			contents;
		int			collisionArea;
		idVec3		collisionPoint;
		bool		tracer;

		// Calculate the end point of the trace
		start = origin;
		if (g_perfTest_hitscanShort.GetBool()) {
			end = start + (dir.ToMat3() * idVec3(idMath::ClampFloat(0, 2048, hitscanDict.GetFloat("range", "2048")), 0, 0));
		}
		else {
			end = start + (dir.ToMat3() * idVec3(hitscanDict.GetFloat("range", "40000"), 0, 0));
		}
		contents = MASK_SHOT_RENDERMODEL | CONTENTS_WATER | CONTENTS_PROJECTILE;

		gameLocal.TracePoint(owner, tr, start, end, contents, additionalIgnore);
		
		
		
		//uncomment this to see the tracking setup
		//gameRenderWorld->DebugArrow(colorRed, start, end, 10, 5000);
		
		
		
		// RAVEN END

		// If the hitscan hit a no impact surface we can just return out
		//assert( tr.c.material );
		if (tr.fraction >= 1.0f || (tr.c.material && tr.c.material->GetSurfaceFlags() & SURF_NOIMPACT)) {
			gameLocal.PlayEffect(hitscanDict, "fx_path", fxOrigin, dir.ToMat3(), false, tr.endpos, false, EC_IGNORE, hitscanTint);
			if (gameLocal.random.RandomFloat() < tracerChance) {
				gameLocal.PlayEffect(hitscanDict, "fx_tracer", fxOrigin, dir.ToMat3(), false, tr.endpos);
				tracer = true;
			}
			else {
				tracer = false;
			}

			if (areas) {
				collisionArea = gameLocal.pvs.GetPVSArea(tr.endpos);
				if (collisionArea != areas[0]) {
					areas[1] = collisionArea;
				}
			}

			idVec3 newvec;

			return newvec;
		}

		// computing the collisionArea from the collisionPoint fails sometimes
		if (areas) {
			collisionArea = gameLocal.pvs.GetPVSArea(tr.c.point);
			if (collisionArea != areas[0]) {
				areas[1] = collisionArea;
			}
		}

		collisionPoint = tr.c.point - (tr.c.normal * tr.c.point - tr.c.dist) * tr.c.normal;
		ent = gameLocal.entities[tr.c.entityNum];
		actualHitEnt = NULL;
		start = collisionPoint;

		assert(false);

		return collisionPoint;
}




/*
================
rvWeaponBlaster::rvWeaponBlaster
================
*/
rvWeaponBlaster::rvWeaponBlaster ( void ) {
}

/*
================
rvWeaponBlaster::UpdateFlashlight
================
*/
bool rvWeaponBlaster::UpdateFlashlight ( void ) {
	if ( !wsfl.flashlight ) {
		return false;
	}
	
	SetState ( "Flashlight", 0 );
	return true;		
}

/*
================
rvWeaponBlaster::Flashlight
================
*/
void rvWeaponBlaster::Flashlight ( bool on ) {
	owner->Flashlight ( on );
	
	if ( on ) {
		worldModel->ShowSurface ( "models/weapons/blaster/flare" );
		viewModel->ShowSurface ( "models/weapons/blaster/flare" );
	} else {
		worldModel->HideSurface ( "models/weapons/blaster/flare" );
		viewModel->HideSurface ( "models/weapons/blaster/flare" );
	}
}

/*
================
rvWeaponBlaster::UpdateAttack
================
*/
bool rvWeaponBlaster::UpdateAttack ( void ) {
	// Clear fire forced
	if ( fireForced ) {
		if ( !wsfl.attack ) {
			fireForced = false;
		} else {
			return false;
		}
	}

	// If the player is pressing the fire button and they have enough ammo for a shot
	// then start the shooting process.
	if ( wsfl.attack && gameLocal.time >= nextAttackTime ) {
		// Save the time which the fire button was pressed
		if ( fireHeldTime == 0 ) {		
			nextAttackTime = gameLocal.time + (fireRate * owner->PowerUpModifier ( PMOD_FIRERATE ));
			fireHeldTime   = gameLocal.time;
			viewModel->SetShaderParm ( BLASTER_SPARM_CHARGEGLOW, chargeGlow[0] );
		}
	}		

	// If they have the charge mod and they have overcome the initial charge 
	// delay then transition to the charge state.
	if ( fireHeldTime != 0 ) {
		if ( gameLocal.time - fireHeldTime > chargeDelay ) {
			SetState ( "Charge", 4 );
			return true;
		}

		// If the fire button was let go but was pressed at one point then 
		// release the shot.
		if ( !wsfl.attack ) {
			idPlayer * player = gameLocal.GetLocalPlayer();
			if( player )	{
			
				if( player->GuiActive())	{
					//make sure the player isn't looking at a gui first
					SetState ( "Lower", 0 );
				} else {
					SetState ( "Fire", 0 );
				}
			}
			return true;
		}
	}
	
	return false;
}

/*
================
rvWeaponBlaster::Spawn
================
*/
void rvWeaponBlaster::Spawn ( void ) {
	viewModel->SetShaderParm ( BLASTER_SPARM_CHARGEGLOW, 0 );
	SetState ( "Raise", 0 );
	
	chargeGlow   = spawnArgs.GetVec2 ( "chargeGlow" );
	chargeTime   = SEC2MS ( spawnArgs.GetFloat ( "chargeTime" ) );
	chargeDelay  = SEC2MS ( spawnArgs.GetFloat ( "chargeDelay" ) );

	fireHeldTime		= 0;
	fireForced			= false;
			
	Flashlight ( owner->IsFlashlightOn() );
}

/*
================
rvWeaponBlaster::Save
================
*/
void rvWeaponBlaster::Save ( idSaveGame *savefile ) const {
	savefile->WriteInt ( chargeTime );
	savefile->WriteInt ( chargeDelay );
	savefile->WriteVec2 ( chargeGlow );
	savefile->WriteBool ( fireForced );
	savefile->WriteInt ( fireHeldTime );
}

/*
================
rvWeaponBlaster::Restore
================
*/
void rvWeaponBlaster::Restore ( idRestoreGame *savefile ) {
	savefile->ReadInt ( chargeTime );
	savefile->ReadInt ( chargeDelay );
	savefile->ReadVec2 ( chargeGlow );
	savefile->ReadBool ( fireForced );
	savefile->ReadInt ( fireHeldTime );
}

/*
================
rvWeaponBlaster::PreSave
================
*/
void rvWeaponBlaster::PreSave ( void ) {

	SetState ( "Idle", 4 );

	StopSound( SND_CHANNEL_WEAPON, 0);
	StopSound( SND_CHANNEL_BODY, 0);
	StopSound( SND_CHANNEL_ITEM, 0);
	StopSound( SND_CHANNEL_ANY, false );
	
}

/*
================
rvWeaponBlaster::PostSave
================
*/
void rvWeaponBlaster::PostSave ( void ) {
}

/*
===============================================================================

	States 

===============================================================================
*/

CLASS_STATES_DECLARATION ( rvWeaponBlaster )
	STATE ( "Raise",						rvWeaponBlaster::State_Raise )
	STATE ( "Lower",						rvWeaponBlaster::State_Lower )
	STATE ( "Idle",							rvWeaponBlaster::State_Idle)
	STATE ( "Charge",						rvWeaponBlaster::State_Charge )
	STATE ( "Charged",						rvWeaponBlaster::State_Charged )
	STATE ( "Fire",							rvWeaponBlaster::State_Fire )
	STATE ( "Flashlight",					rvWeaponBlaster::State_Flashlight )
END_CLASS_STATES



/*
================
rvWeaponBlaster::State_Raise
================
*/
stateResult_t rvWeaponBlaster::State_Raise( const stateParms_t& parms ) {
	enum {
		RAISE_INIT,
		RAISE_WAIT,
	};	
	switch ( parms.stage ) {
		case RAISE_INIT:			
			SetStatus ( WP_RISING );
			PlayAnim( ANIMCHANNEL_ALL, "raise", parms.blendFrames );
			return SRESULT_STAGE(RAISE_WAIT);
			
		case RAISE_WAIT:
			if ( AnimDone ( ANIMCHANNEL_ALL, 4 ) ) {
				SetState ( "Idle", 4 );
				return SRESULT_DONE;
			}
			if ( wsfl.lowerWeapon ) {
				SetState ( "Lower", 4 );
				return SRESULT_DONE;
			}
			return SRESULT_WAIT;
	}
	return SRESULT_ERROR;	
}

/*
================
rvWeaponBlaster::State_Lower
================
*/
stateResult_t rvWeaponBlaster::State_Lower ( const stateParms_t& parms ) {
	enum {
		LOWER_INIT,
		LOWER_WAIT,
		LOWER_WAITRAISE
	};	
	switch ( parms.stage ) {
		case LOWER_INIT:
			SetStatus ( WP_LOWERING );
			PlayAnim( ANIMCHANNEL_ALL, "putaway", parms.blendFrames );
			return SRESULT_STAGE(LOWER_WAIT);
			
		case LOWER_WAIT:
			if ( AnimDone ( ANIMCHANNEL_ALL, 0 ) ) {
				SetStatus ( WP_HOLSTERED );
				return SRESULT_STAGE(LOWER_WAITRAISE);
			}
			return SRESULT_WAIT;
	
		case LOWER_WAITRAISE:
			if ( wsfl.raiseWeapon ) {
				SetState ( "Raise", 0 );
				return SRESULT_DONE;
			}
			return SRESULT_WAIT;
	}
	return SRESULT_ERROR;
}

/*
================
rvWeaponBlaster::State_Idle
================
*/
stateResult_t rvWeaponBlaster::State_Idle ( const stateParms_t& parms ) {	
	enum {
		IDLE_INIT,
		IDLE_WAIT,
	};	
	switch ( parms.stage ) {
		case IDLE_INIT:			
			SetStatus ( WP_READY );
			PlayCycle( ANIMCHANNEL_ALL, "idle", parms.blendFrames );
			return SRESULT_STAGE ( IDLE_WAIT );
			
		case IDLE_WAIT:
			if ( wsfl.lowerWeapon ) {
				SetState ( "Lower", 4 );
				return SRESULT_DONE;
			}
			
			if ( UpdateFlashlight ( ) ) { 
				return SRESULT_DONE;
			}
			if ( UpdateAttack ( ) ) {
				return SRESULT_DONE;
			}
			return SRESULT_WAIT;
	}
	return SRESULT_ERROR;
}

/*
================
rvWeaponBlaster::State_Charge
================
*/
stateResult_t rvWeaponBlaster::State_Charge ( const stateParms_t& parms ) {
	enum {
		CHARGE_INIT,
		CHARGE_WAIT,
	};	
	switch ( parms.stage ) {
		case CHARGE_INIT:
			viewModel->SetShaderParm ( BLASTER_SPARM_CHARGEGLOW, chargeGlow[0] );
			StartSound ( "snd_charge", SND_CHANNEL_ITEM, 0, false, NULL );
			PlayCycle( ANIMCHANNEL_ALL, "charging", parms.blendFrames );
			return SRESULT_STAGE ( CHARGE_WAIT );
			
		case CHARGE_WAIT:	
			if ( gameLocal.time - fireHeldTime < chargeTime ) {
				float f;
				f = (float)(gameLocal.time - fireHeldTime) / (float)chargeTime;
				f = chargeGlow[0] + f * (chargeGlow[1] - chargeGlow[0]);
				f = idMath::ClampFloat ( chargeGlow[0], chargeGlow[1], f );
				viewModel->SetShaderParm ( BLASTER_SPARM_CHARGEGLOW, f );
				
				if ( !wsfl.attack ) {
					SetState ( "Fire", 0 );
					return SRESULT_DONE;
				}
				
				return SRESULT_WAIT;
			} 
			SetState ( "Charged", 4 );
			return SRESULT_DONE;
	}
	return SRESULT_ERROR;	
}

/*
================
rvWeaponBlaster::State_Charged
================
*/
stateResult_t rvWeaponBlaster::State_Charged ( const stateParms_t& parms ) {
	enum {
		CHARGED_INIT,
		CHARGED_WAIT,
	};	
	switch ( parms.stage ) {
		case CHARGED_INIT:		
			viewModel->SetShaderParm ( BLASTER_SPARM_CHARGEGLOW, 1.0f  );

			StopSound ( SND_CHANNEL_ITEM, false );
			StartSound ( "snd_charge_loop", SND_CHANNEL_ITEM, 0, false, NULL );
			StartSound ( "snd_charge_click", SND_CHANNEL_BODY, 0, false, NULL );
			return SRESULT_STAGE(CHARGED_WAIT);
			
		case CHARGED_WAIT:
			if ( !wsfl.attack ) {
				fireForced = true;
				SetState ( "Fire", 0 );
				return SRESULT_DONE;
			}
			return SRESULT_WAIT;
	}
	return SRESULT_ERROR;
}

/*
================
rvWeaponBlaster::State_Fire
================
*/
stateResult_t rvWeaponBlaster::State_Fire ( const stateParms_t& parms ) {
	enum {
		FIRE_INIT,
		FIRE_WAIT,
	};	
	switch ( parms.stage ) {
		case FIRE_INIT:	

			StopSound ( SND_CHANNEL_ITEM, false );
			viewModel->SetShaderParm ( BLASTER_SPARM_CHARGEGLOW, 0 );
			//don't fire if we're targeting a gui.
			idPlayer* player;
			player = gameLocal.GetLocalPlayer();

			//make sure the player isn't looking at a gui first
			if( player && player->GuiActive() )	{
				fireHeldTime = 0;
				SetState ( "Lower", 0 );
				return SRESULT_DONE;
			}

			if( player && !player->CanFire() )	{
				fireHeldTime = 0;
				SetState ( "Idle", 4 );
				return SRESULT_DONE;
			}


	
			if ( gameLocal.time - fireHeldTime > chargeTime ) {	
				Attack ( true, 1, spread, 0, 1.0f );
				PlayEffect ( "fx_chargedflash", barrelJointView, false );
				PlayAnim( ANIMCHANNEL_ALL, "chargedfire", parms.blendFrames );
			} else {
				Attack ( false, 1, spread, 0, 1.0f );
				PlayEffect ( "fx_normalflash", barrelJointView, false );
				PlayAnim( ANIMCHANNEL_ALL, "fire", parms.blendFrames );
			}

			SpawnEntityBasedOnContact();

			fireHeldTime = 0;
			
			return SRESULT_STAGE(FIRE_WAIT);
		
		case FIRE_WAIT:
			if ( AnimDone ( ANIMCHANNEL_ALL, 4 ) ) {
				SetState ( "Idle", 4 );
				return SRESULT_DONE;
			}
			if ( UpdateFlashlight ( ) || UpdateAttack ( ) ) {
				return SRESULT_DONE;
			}
			return SRESULT_WAIT;
	}			
	return SRESULT_ERROR;
}

/*
================
rvWeaponBlaster::State_Flashlight
================
*/
stateResult_t rvWeaponBlaster::State_Flashlight ( const stateParms_t& parms ) {
	enum {
		FLASHLIGHT_INIT,
		FLASHLIGHT_WAIT,
	};	
	switch ( parms.stage ) {
		case FLASHLIGHT_INIT:			
			SetStatus ( WP_FLASHLIGHT );
			// Wait for the flashlight anim to play		
			PlayAnim( ANIMCHANNEL_ALL, "flashlight", 0 );
			return SRESULT_STAGE ( FLASHLIGHT_WAIT );
			
		case FLASHLIGHT_WAIT:
			if ( !AnimDone ( ANIMCHANNEL_ALL, 4 ) ) {
				return SRESULT_WAIT;
			}
			
			if ( owner->IsFlashlightOn() ) {
				Flashlight ( false );
			} else {
				Flashlight ( true );
			}
			
			SetState ( "Idle", 4 );
			return SRESULT_DONE;
	}
	return SRESULT_ERROR;
}

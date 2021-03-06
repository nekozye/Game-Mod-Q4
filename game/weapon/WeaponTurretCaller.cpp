#include "../../idlib/precompiled.h"
#pragma hdrstop

#include "../Game_local.h"
#include "../Weapon.h"


/*
	Note, this is part of the Engineering Mod, which add turrets. this would be basis of the turret spawners... hopefully.
	

*/

#define BLASTER_SPARM_CHARGEGLOW	6


class rvWeaponTurretCaller : public rvWeapon {
public:

	CLASS_PROTOTYPE(rvWeaponTurretCaller);

	rvWeaponTurretCaller(void);

	virtual void		Spawn(void);
	void				Save(idSaveGame *savefile) const;
	void				Restore(idRestoreGame *savefile);
	void				PreSave(void);
	void				PostSave(void);


protected:
	//Engineering Mod START

	void				SpawnEntityBasedOnContact(void);

	//Engineering Mod END

	
	bool				UpdateAttack(void);
	bool				UpdateFlashlight(void);
	void				Flashlight(bool on);

private:

	bool				fireForced;
	int					fireHeldTime;


	stateResult_t		State_Raise(const stateParms_t& parms);
	stateResult_t		State_Lower(const stateParms_t& parms);
	stateResult_t		State_Idle(const stateParms_t& parms);
	stateResult_t		State_Fire(const stateParms_t& parms);
	stateResult_t		State_Flashlight(const stateParms_t& parms);

	CLASS_STATES_PROTOTYPE(rvWeaponTurretCaller);
};

CLASS_DECLARATION(rvWeapon, rvWeaponTurretCaller)
END_CLASS


/*
================
rvWeaponTurretCaller::rvWeaponTurretCaller
================
*/
void rvWeaponTurretCaller::SpawnEntityBasedOnContact(void) {
	idPlayer* player;
	idVec3 end;
	trace_t tracer;
	idVec3 start;
	idMat3 viewaxe;
	idVec3 dir;


	// calculate the muzzle position
	if (barrelJointView != INVALID_JOINT && spawnArgs.GetBool("launchFromBarrel")) {
		// there is an explicit joint for the muzzle
		GetGlobalJointTransform(true, barrelJointView, start, viewaxe);
	}
	else {
		// go straight out of the view
		start = playerViewOrigin;
		start += playerViewAxis[0] * muzzleOffset;
	}


	dir = playerViewAxis[0] + playerViewAxis[2] - playerViewAxis[1];


	player = gameLocal.GetLocalPlayer();
	end = start + ((8192 * 16) * viewaxe[0]);



	gameLocal.TracePoint(player, tracer, start, dir, MASK_OPAQUE, player);


	if (tracer.fraction < 1.0)
	{
		gameLocal.Printf("coords {x:%f y:%f z:%f}",tracer.c.point.x,tracer.c.point.y,tracer.c.point.z);
	}
	else
	{
		gameLocal.Printf("No coords detected");
	}
	
}





/*
================
rvWeaponTurretCaller::rvWeaponTurretCaller
================
*/
rvWeaponTurretCaller::rvWeaponTurretCaller(void) {

}

/*
================
rvWeaponTurretCaller::UpdateFlashlight
================
*/
bool rvWeaponTurretCaller::UpdateFlashlight(void) {
	if (!wsfl.flashlight) {
		return false;
	}

	SetState("Flashlight", 0);
	return true;
}

/*
================
rvWeaponTurretCaller::Flashlight
================
*/
void rvWeaponTurretCaller::Flashlight(bool on) {
	owner->Flashlight(on);

	if (on) {
		worldModel->ShowSurface("models/weapons/blaster/flare");
		viewModel->ShowSurface("models/weapons/blaster/flare");
	}
	else {
		worldModel->HideSurface("models/weapons/blaster/flare");
		viewModel->HideSurface("models/weapons/blaster/flare");
	}
}

/*
================
rvWeaponTurretCaller::UpdateAttack
================
*/
bool rvWeaponTurretCaller::UpdateAttack(void) {
	// Clear fire forced
	if (fireForced) {
		if (!wsfl.attack) {
			fireForced = false;
		}
		else {
			return false;
		}
	}

	// If the player is pressing the fire button and they have enough ammo for a shot
	// then start the shooting process.
	if (wsfl.attack && gameLocal.time >= nextAttackTime) {
		// Save the time which the fire button was pressed
		if (fireHeldTime == 0) {
			nextAttackTime = gameLocal.time + (fireRate * owner->PowerUpModifier(PMOD_FIRERATE));
			fireHeldTime = gameLocal.time;
		}
	}

	// If they have the charge mod and they have overcome the initial charge 
	// delay then transition to the charge state.
	if (fireHeldTime != 0) {
		// If the fire button was let go but was pressed at one point then 
		// release the shot.
		if (!wsfl.attack) {
			idPlayer * player = gameLocal.GetLocalPlayer();
			if (player)	{
				if (player->GuiActive())	{
					//make sure the player isn't looking at a gui first
					SetState("Lower", 0);
				}
				else {
					SetState("Fire", 0);
				}
			}
			return true;
		}
	}

	return false;
}

/*
================
rvWeaponTurretCaller::Spawn
================
*/
void rvWeaponTurretCaller::Spawn(void) {
	viewModel->SetShaderParm(BLASTER_SPARM_CHARGEGLOW, 0);
	SetState("Raise", 0);


	fireHeldTime = 0;
	fireForced = false;

	Flashlight(owner->IsFlashlightOn());
}

/*
================
rvWeaponTurretCaller::Save
================
*/
void rvWeaponTurretCaller::Save(idSaveGame *savefile) const {
	savefile->WriteBool(fireForced);
	savefile->WriteInt(fireHeldTime);
}

/*
================
rvWeaponTurretCaller::Restore
================
*/
void rvWeaponTurretCaller::Restore(idRestoreGame *savefile) {
	savefile->ReadBool(fireForced);
	savefile->ReadInt(fireHeldTime);
}

/*
================
rvWeaponTurretCaller::PreSave
================
*/
void rvWeaponTurretCaller::PreSave(void) {

	SetState("Idle", 4);

	StopSound(SND_CHANNEL_WEAPON, 0);
	StopSound(SND_CHANNEL_BODY, 0);
	StopSound(SND_CHANNEL_ITEM, 0);
	StopSound(SND_CHANNEL_ANY, false);

}

/*
================
rvWeaponTurretCaller::PostSave
================
*/
void rvWeaponTurretCaller::PostSave(void) {
}

/*
===============================================================================

States

===============================================================================
*/

CLASS_STATES_DECLARATION(rvWeaponTurretCaller)
STATE("Raise", rvWeaponTurretCaller::State_Raise)
STATE("Lower", rvWeaponTurretCaller::State_Lower)
STATE("Idle", rvWeaponTurretCaller::State_Idle)
STATE("Fire", rvWeaponTurretCaller::State_Fire)
STATE("Flashlight", rvWeaponTurretCaller::State_Flashlight)
END_CLASS_STATES

/*
================
rvWeaponTurretCaller::State_Raise
================
*/
stateResult_t rvWeaponTurretCaller::State_Raise(const stateParms_t& parms) {
	enum {
		RAISE_INIT,
		RAISE_WAIT,
	};
	switch (parms.stage) {
	case RAISE_INIT:
		SetStatus(WP_RISING);
		PlayAnim(ANIMCHANNEL_ALL, "raise", parms.blendFrames);
		return SRESULT_STAGE(RAISE_WAIT);

	case RAISE_WAIT:
		if (AnimDone(ANIMCHANNEL_ALL, 4)) {
			SetState("Idle", 4);
			return SRESULT_DONE;
		}
		if (wsfl.lowerWeapon) {
			SetState("Lower", 4);
			return SRESULT_DONE;
		}
		return SRESULT_WAIT;
	}
	return SRESULT_ERROR;
}

/*
================
rvWeaponTurretCaller::State_Lower
================
*/
stateResult_t rvWeaponTurretCaller::State_Lower(const stateParms_t& parms) {
	enum {
		LOWER_INIT,
		LOWER_WAIT,
		LOWER_WAITRAISE
	};
	switch (parms.stage) {
	case LOWER_INIT:
		SetStatus(WP_LOWERING);
		PlayAnim(ANIMCHANNEL_ALL, "putaway", parms.blendFrames);
		return SRESULT_STAGE(LOWER_WAIT);

	case LOWER_WAIT:
		if (AnimDone(ANIMCHANNEL_ALL, 0)) {
			SetStatus(WP_HOLSTERED);
			return SRESULT_STAGE(LOWER_WAITRAISE);
		}
		return SRESULT_WAIT;

	case LOWER_WAITRAISE:
		if (wsfl.raiseWeapon) {
			SetState("Raise", 0);
			return SRESULT_DONE;
		}
		return SRESULT_WAIT;
	}
	return SRESULT_ERROR;
}

/*
================
rvWeaponTurretCaller::State_Idle
================
*/
stateResult_t rvWeaponTurretCaller::State_Idle(const stateParms_t& parms) {
	enum {
		IDLE_INIT,
		IDLE_WAIT,
	};
	switch (parms.stage) {
	case IDLE_INIT:
		SetStatus(WP_READY);
		PlayCycle(ANIMCHANNEL_ALL, "idle", parms.blendFrames);
		return SRESULT_STAGE(IDLE_WAIT);

	case IDLE_WAIT:
		if (wsfl.lowerWeapon) {
			SetState("Lower", 4);
			return SRESULT_DONE;
		}

		if (UpdateFlashlight()) {
			return SRESULT_DONE;
		}
		if (UpdateAttack()) {
			return SRESULT_DONE;
		}
		return SRESULT_WAIT;
	}
	return SRESULT_ERROR;
}

/*
================
rvWeaponTurretCaller::State_Fire
================
*/
stateResult_t rvWeaponTurretCaller::State_Fire(const stateParms_t& parms) {
	enum {
		FIRE_INIT,
		FIRE_WAIT,
	};
	switch (parms.stage) {
	case FIRE_INIT:

		StopSound(SND_CHANNEL_ITEM, false);
		viewModel->SetShaderParm(BLASTER_SPARM_CHARGEGLOW, 0);
		//don't fire if we're targeting a gui.
		idPlayer* player;
		player = gameLocal.GetLocalPlayer();

		//make sure the player isn't looking at a gui first
		if (player && player->GuiActive())	{
			fireHeldTime = 0;
			SetState("Lower", 0);
			return SRESULT_DONE;
		}

		if (player && !player->CanFire())	{
			fireHeldTime = 0;
			SetState("Idle", 4);
			return SRESULT_DONE;
		}


		//Fireing START edit this place for spawning enemies in hitscan instead
	
		Attack(false, 1, spread, 0, 1.0f);
		PlayEffect("fx_normalflash", barrelJointView, false);
		PlayAnim(ANIMCHANNEL_ALL, "fire", parms.blendFrames);
		
		fireHeldTime = 0;

		SpawnEntityBasedOnContact();

		return SRESULT_STAGE(FIRE_WAIT);

		//Fireing END edit this place for spawning enemies in hitscan instead

	case FIRE_WAIT:
		if (AnimDone(ANIMCHANNEL_ALL, 4)) {
			SetState("Idle", 4);
			return SRESULT_DONE;
		}
		if (UpdateFlashlight() || UpdateAttack()) {
			return SRESULT_DONE;
		}
		return SRESULT_WAIT;
	}
	return SRESULT_ERROR;
}

/*
================
rvWeaponTurretCaller::State_Flashlight
================
*/
stateResult_t rvWeaponTurretCaller::State_Flashlight(const stateParms_t& parms) {
	enum {
		FLASHLIGHT_INIT,
		FLASHLIGHT_WAIT,
	};
	switch (parms.stage) {
	case FLASHLIGHT_INIT:
		SetStatus(WP_FLASHLIGHT);
		// Wait for the flashlight anim to play		
		PlayAnim(ANIMCHANNEL_ALL, "flashlight", 0);
		return SRESULT_STAGE(FLASHLIGHT_WAIT);

	case FLASHLIGHT_WAIT:
		if (!AnimDone(ANIMCHANNEL_ALL, 4)) {
			return SRESULT_WAIT;
		}

		if (owner->IsFlashlightOn()) {
			Flashlight(false);
		}
		else {
			Flashlight(true);
		}

		SetState("Idle", 4);
		return SRESULT_DONE;
	}
	return SRESULT_ERROR;
}

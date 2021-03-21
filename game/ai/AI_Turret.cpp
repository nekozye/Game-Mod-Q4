
#include "../../idlib/precompiled.h"
#pragma hdrstop

#include "../Game_local.h"

class emTurret : public idAI {
public:

	CLASS_PROTOTYPE(emTurret);

	emTurret(void);

	void				InitSpawnArgsVariables(void);
	void				Spawn(void);
	void				Save(idSaveGame *savefile) const;
	void				Restore(idRestoreGame *savefile);

	virtual bool		Pain(idEntity *inflictor, idEntity *attacker, int damage, const idVec3 &dir, int location);

protected:

	virtual bool		CheckActions(void);

	stateResult_t		State_Combat(const stateParms_t& parms);

	int					maxShots;
	int					minShots;
	int					shots;

private:

	rvAIAction			actionBlasterAttack;

	stateResult_t		State_Torso_BlasterAttack(const stateParms_t& parms);

	CLASS_STATES_PROTOTYPE(emTurret);
};

CLASS_DECLARATION(idAI, emTurret)
END_CLASS

/*
================
rvMonsterTurret::rvMonsterTurret
================
*/
emTurret::emTurret() {
}

void emTurret::InitSpawnArgsVariables(void) {
	maxShots = spawnArgs.GetInt("maxShots", "1");
	minShots = spawnArgs.GetInt("minShots", "1");
}
/*
================
rvMonsterTurret::Spawn
================
*/
void emTurret::Spawn(void) {
	actionBlasterAttack.Init(spawnArgs, "action_blasterAttack", "Torso_BlasterAttack", AIACTIONF_ATTACK);

	InitSpawnArgsVariables();
	shots = 0;
}

/*
================
rvMonsterTurret::Save
================
*/
void emTurret::Save(idSaveGame *savefile) const {
	savefile->WriteInt(shots);
	actionBlasterAttack.Save(savefile);
}

/*
================
rvMonsterTurret::Restore
================
*/
void emTurret::Restore(idRestoreGame *savefile) {
	savefile->ReadInt(shots);
	actionBlasterAttack.Restore(savefile);

	InitSpawnArgsVariables();
}

/*
================
rvMonsterTurret::CheckActions
================
*/
bool emTurret::CheckActions(void) {
	// Attacks
	if (PerformAction(&actionBlasterAttack, (checkAction_t)&idAI::CheckAction_RangedAttack, &actionTimerRangedAttack)) {
		return true;
	}
	return idAI::CheckActions();
}

/*
================
rvMonsterTurret::Pain
================
*/
bool emTurret::Pain(idEntity *inflictor, idEntity *attacker, int damage, const idVec3 &dir, int location) {
	// 
	return true;
}

/*
===============================================================================

States

===============================================================================
*/

CLASS_STATES_DECLARATION(emTurret)
STATE("State_Combat", emTurret::State_Combat)

STATE("Torso_BlasterAttack", emTurret::State_Torso_BlasterAttack)
END_CLASS_STATES

/*
================
rvMonsterTurret::State_Combat
================
*/
stateResult_t emTurret::State_Combat(const stateParms_t& parms) {
	// Aquire a new enemy if we dont have one
	if (!enemy.ent) {
		CheckForEnemy(true);
	}

	FaceEnemy();

	// try moving, if there was no movement run then just try and action instead
	UpdateAction();

	return SRESULT_WAIT;
}

/*
================
rvMonsterTurret::State_Torso_BlasterAttack
================
*/
stateResult_t emTurret::State_Torso_BlasterAttack(const stateParms_t& parms) {
	enum {
		STAGE_INIT,
		STAGE_FIRE,
		STAGE_WAIT,
	};
	switch (parms.stage) {
	case STAGE_INIT:
		DisableAnimState(ANIMCHANNEL_LEGS);
		shots = (minShots + gameLocal.random.RandomInt(maxShots - minShots + 1)) * combat.aggressiveScale;
		return SRESULT_STAGE(STAGE_FIRE);

	case STAGE_FIRE:
		PlayAnim(ANIMCHANNEL_TORSO, "range_attack", 2);
		return SRESULT_STAGE(STAGE_WAIT);

	case STAGE_WAIT:
		if (AnimDone(ANIMCHANNEL_TORSO, 2)) {
			if (--shots <= 0) {
				return SRESULT_DONE;
			}
			return SRESULT_STAGE(STAGE_FIRE);
		}
		return SRESULT_WAIT;
	}
	return SRESULT_ERROR;
}

/*
Engineering Mod - Turret Spawn Placeholders
required for mod, as a place of turret spawnable capacity. Player needs to shoot these with special handgun to spawn turrets.
*/



#include "../../idlib/precompiled.h"
#pragma hdrstop

#include "../Game_local.h"

class rvPlacehold_TurretSpawn : public idEntity {
public:

	CLASS_PROTOTYPE(rvPlacehold_TurretSpawn);

	rvPlacehold_TurretSpawn(void);

	void				Spawn(void);
	void				Save(idSaveGame *savefile) const;
	void				Restore(idRestoreGame *savefile);
	bool				Pain(idEntity *inflictor, idEntity *attacker, int damage, const idVec3 &dir, int location);

protected:



private:


	CLASS_STATES_PROTOTYPE(rvPlacehold_TurretSpawn);
};

CLASS_DECLARATION(idEntity, rvPlacehold_TurretSpawn)
END_CLASS


rvPlacehold_TurretSpawn::rvPlacehold_TurretSpawn(void) {
}


void rvPlacehold_TurretSpawn::Spawn(void)
{

}

void rvPlacehold_TurretSpawn::Save(idSaveGame *savefile) const
{

}

void rvPlacehold_TurretSpawn::Restore(idRestoreGame *savefile)
{

}

bool rvPlacehold_TurretSpawn::Pain(idEntity *inflictor, idEntity *attacker, int damage, const idVec3 &dir, int location) {

	return false;
}

CLASS_STATES_DECLARATION(rvPlacehold_TurretSpawn)
END_CLASS_STATES
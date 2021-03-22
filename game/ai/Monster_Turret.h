
#ifndef __RV_MONSTER_TURRET__
#define __RV_MONSTER_TURRET__

class rvMonsterTurret : public idAI {
public:

	CLASS_PROTOTYPE(rvMonsterTurret);

	rvMonsterTurret(void);

	void				InitSpawnArgsVariables(void);
	void				Spawn(void);
	void				Save(idSaveGame *savefile) const;
	void				Restore(idRestoreGame *savefile);

	virtual bool		Pain(idEntity *inflictor, idEntity *attacker, int damage, const idVec3 &dir, int location);

	bool				upgradeTurretBasedOnStats(void);

protected:

	virtual bool		CheckActions(void);

	stateResult_t		State_Combat(const stateParms_t& parms);
	stateResult_t		State_Killed(const stateParms_t& parms);

	int					shieldHealth;
	int					maxShots;
	int					minShots;
	int					maxUpgrades;
	int					upgradeMultBase;
	int					upgradeMultVal;
	int					shots;

private:

	rvAIAction			actionBlasterAttack;

	stateResult_t		State_Torso_BlasterAttack(const stateParms_t& parms);

	CLASS_STATES_PROTOTYPE(rvMonsterTurret);
};
#endif // __RV_MONSTER_TURRET__
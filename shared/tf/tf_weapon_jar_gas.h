// Definitely not a Valve header

#ifndef TF_WEAPON_JAR_GAS
#define TF_WEAPON_JAR_GAS
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "baseentity.h"
#include "tf_weapon_jar.h"
#include "tf_point_manager.h"
#include "networkvar.h"
#include "tf_shareddefs.h"

// %REVERSED 12/23/2020
class CTFGasManager : public CTFPointManager
{
public:
	CTFGasManager(void);
	virtual ~CTFGasManager();
	
	void 				AddGas(void);
	virtual Vector 		GetAdditionalVelocity(tf_point_t const* pPoint);
	virtual Vector 		GetInitialPosition(void);
	virtual float 		GetLifeTime(void);
	virtual int 		GetMaxPoints(void);
	virtual float 		GetRadius(tf_point_t const* pPoint);
	virtual void 		OnCollide(CBaseEntity *pEnt,int iIndex);
	virtual bool 		OnPointHitWall(tf_point_t *pPoint, Vector &v1, Vector &v2, CGameTrace const &tr, float f);
	virtual bool 		ShouldCollide(CBaseEntity * pEntity);
	virtual bool 		ShouldIgnoreStartSolid(void);
	virtual void 		Update(void);
	virtual void 		UpdateOnRemove(void);

	static CTFGasManager *Create(CBaseEntity *,Vector const&);

public:
	CUtlVector<EHANDLE> m_Gassed;
	float m_flGasThinkTime;
	bool m_bShouldUpdate;
};

// END REVERSE 12/23/2020


// %REVERSED 12/21/2020
class CTFJarGas : public CTFJar
{
public:
	DECLARE_CLASS( CTFJarGas, CTFJar );
	DECLARE_NETWORKCLASS();

	virtual int			GetWeaponID( void ) const			{ return TF_WEAPON_JAR_GAS; }	// 107

	virtual float 		GetAfterburnRateOnHit( void ) 		{ return 10.0f; }
	virtual void 		OnResourceMeterFilled( void );
	virtual void 		Equip( CBaseCombatCharacter* pEntity ) { return BaseClass::Equip(pEntity); }
	virtual bool 		ShouldUpdateMeter( void );
	virtual bool 		CanAttack( void );

	void 				RemoveJarGas( CBaseCombatCharacter* pEntity );

	virtual const char*			GetEffectLabelText( void ) { return "#TF_Gas"; }

	virtual float		InternalGetEffectBarRechargeTime( void ) { return 0.0f; }
	virtual float 		GetDefaultItemChargeMeterValue( void ) { return 0.0f; }

#ifdef GAME_DLL
	virtual CTFProjectile_Jar	*CreateJarProjectile( const Vector &position, const QAngle &angles, const Vector &velocity, 
		const AngularImpulse &angVelocity, CBaseCombatCharacter *pOwner, const CTFWeaponInfo &weaponInfo );
#endif

//#ifdef CLIENT_DLL
//	const char *ModifyEventParticles(const char *token);
//	virtual bool Holster(CBaseCombatWeapon *pSwitchingTo = NULL);
//#endif
}

class CTFProjectile_JarGas : public CTFProjectile_Jar
{
public:
	DECLARE_CLASS(CTFProjectile_JarGas, CTFProjectile_Jar);
	DECLARE_NETWORKCLASS();

	//#ifdef CLIENT_DLL
	//	virtual const char*			GetTrailParticleName( void );
	//#endif

#ifdef GAME_DLL
	static CTFProjectile_JarGas *Create(const Vector &position, const QAngle &angles, const Vector &velocity,
										const AngularImpulse &angVelocity, CBaseCombatCharacter *pOwner, const CTFWeaponInfo &weaponInfo, int nSkin);
#endif

	CTFProjectile_JarGas();

	virtual int GetWeaponID(void) const OVERRIDE { return TF_WEAPON_GRENADE_JAR_GAS; }	// 108

	virtual void Precache() OVERRIDE;
	virtual void SetCustomPipebombModel() OVERRIDE;
	{
		SetModel("models/weapons/c_models/c_gascan/c_gascan.mdl");
	}

#ifdef GAME_DLL
	virtual void Explode(trace_t *pTrace, int bitsDamageType) OVERRIDE;
	virtual const char *GetImpactEffect() OVERRIDE;
	virtual ETFCond GetEffectCondition(void) OVERRIDE { return TF_COND_GAS; }	// 123

	virtual const char *GetExplodeSound() { return "Weapon_GasCan.Explode"; }
#endif
};

#endif 	// TF_WEAPON_JAR_GAS
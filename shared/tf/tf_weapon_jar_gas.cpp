
#include "tf_weapon_jar_gas.h"
#include "tf_gas_manager.h"

// %REVERSED 12/21/2020

// 100%
void CTFJarGas::OnResourceMeterFilled(void)
{
	CTFPlayer *pOwner;

	pOwner = GetTFPlayerOwner();
	if (pOwner)
		pOwner->GiveAmmo(1, GetSecondaryAmmoType(), false, (EAmmoSource)3);
}

// 100%
bool CTFJarGas::ShouldUpdateMeter(void)
{
	CBasePlayer *pOwner;
	bool bReturn = true;

	pOwner = GetPlayerOwner();
	if (pOwner && pOwner->IsPlayer())
		bReturn = pOwner->IsAlive();
	return bReturn;
}

// 100%
bool CTFJarGas::CanAttack(void)
{
	CTFPlayer *pOwner;
	bool bReturn = false;

	pOwner = GetTFPlayerOwner(); // This is actually GetPlayerOwner() but the member below is definitely a CTFPlayerShared prop
	if (pOwner && pOwner->IsPlayer() && pOwner->m_Shared.m_flItemChargeMeter[LOADOUT_POSITION_UTILITY] < 100.0f)
		bReturn = false;
	else
		bReturn = BaseClass::CanAttack();
	return bReturn;
}

// 100%
void CTFJarGas::RemoveJarGas(CBaseCombatCharacter *pEntity)
{
	if (pEntity && pEntity->IsPlayer())
	{
		CTFPlayer *pPlayer = dynamic_cast<CTFPlayer *>(pEntity);
		if (pPlayer)
		{
			pPlayer->m_Shared.SetItemChargeMeter(LOADOUT_POSITION_UTILITY, 0.0f);	// TODO
			pPlayer->RemoveAmmo(m_pWeaponInfo->GetWeaponData(m_iWeaponMode).m_iAmmoPerShot, GetPrimaryAmmoType());
		}
	}
}

// 100%
CTFProjectile_Jar *CTFJarGas::CreateJarProjectile(const Vector &position, const QAngle &angles, const Vector &velocity,
												  const AngularImpulse &angVelocity, CBaseCombatCharacter *pOwner, const CTFWeaponInfo &weaponInfo)
{
	RemoveJarGas(pOwner);
	return CTFProjectile_JarGas::Create(position, angles, velocity, angVelocity, pOwner, weaponInfo, GetSkin());
}

// 100%
void CTFProjectile_Jar::Precache()
{
	PrecacheModel("models/weapons/c_models/c_gascan/c_gascan.mdl", true);
	PrecacheParticleSystem("gas_can_blue");
	PrecacheParticleSystem("gas_can_red");
	PrecacheScriptSound("Weapon_GasCan.Explode");
	BaseClass::Precache();
}

// 100%
void CTFProjectile_Jar::Explode(trace_t *pTrace, int bitsDamageType)
{
	BaseClass::Explode(pTrace, bitsDamageType);
	SetBaseVelocity(Vector(0, 0, 0));
	DispatchUpdateTransmitState();
	AddSolidFlags(FSOLID_NOT_SOLID);
	m_takedamage = 0;

	if (pTrace->fraction != 1.0)
	{
		SetAbsOrigin(pTrace->endpos + (pTrace->plane.normal * 1.0f));
	}

	CTFPlayer *pThrower = ToTFPlayer(GetThrower());
	if (pThrower && pThrower->IsPlayer())
	{
		Vector vecOrigin = GetAbsOrigin();

		// 0x41F00000
		int flags = (CONTENTS_HITBOX | CONTENTS_ORIGIN | CONTENTS_CURRENT_180 | CONTENTS_CURRENT_270 | CONTENTS_CURRENT_UP | CONTENTS_CURRENT_DOWN);
		trace_t trace;
		CTraceFilterSimple traceFilter(this, COLLISION_GROUP_NONE);

		Vector vecEnd = vecOrigin + Vector(0.0f, 0.0f, 30.0f)
		UTIL_TraceLine(vecOrigin, vecEnd, flags, traceFilter, COLLISION_GROUP_NONE, &trace);

		Vector vecSpot = vecOrigin;
		if (trace.fraction != 1.0)
		{
			vecSpot.z += 30.0;
		}

		CTFGasManager *pGas = CTFGasManager::Create(pThrower, vecSpot);
		if (pGas)
			pGas->AddGas();
	}

	SetThink(NULL);
	SetTouch(NULL);
	AddEffects(EF_NODRAW);
	SetAbsVelocity(vec3_origin);
}
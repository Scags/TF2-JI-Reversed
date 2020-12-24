
#include "cbase.h"
#include "tf_point_manager.h"
#include "tf_player.h"
#include "tf_fx.h"
#include "tf_team.h"
#include "tf_gamestats.h"
#include "tf_gamerules.h"
#include "tf_weapon_jar_gas.h"
#include "random.h"
#include "vector.h"
#include "trace.h"
#include "tier0/dbg.h"
#include "basehandle.h"

// %REVERSED 12/23/2020
IMPLEMENT_NETWORKCLASS_ALIASED(TFGasManager, DT_TFGasManager);
LINK_ENTITY_TO_CLASS(tf_gas_manager, CTFGasManager)

// 100%
CTFGasManager::CTFGasManager()
{
	m_Gassed = CUtlVector<EHANDLE>();
	m_flGasThinkTime = -1.0;
	m_bShouldUpdate = true;
}

// 100%
CTFGasManager::~CTFGasManager()
{
	m_Points.RemoveAll();
}

// 100%
void CTFGasManager::AddGas(void)
{
	for (int i = (int)(gpGlobals->curtime / gpGlobals->interval_per_tick + 0.5f); m_Points.Count() < GetMaxPoints();)
	{
		if (!AddPoint(i))
			break;
	}
}

// 100%
Vector CTFGasManager::GetAdditionalVelocity(tf_point_t const *pPoint)
{
	return vec3_origin;
}

// 100%
Vector CTFGasManager::GetInitialPosition(void)
{
	Vector vecPos = GetAbsolutePosition();
	vecPos.x += m_Randomizer.RandomFloat(-25.0f, 25.0f);
	vecPos.z += m_Randomizer.RandomFloat(-25.0f, 25.0f);
	return vecPos;
}

// 100%
float CTFGasManager::GetLifeTime(void)
{
	return 5.0f;
}

// 100%
int CTFGasManager::GetMaxPoints(void)
{
	return 20;
}

// 100%
float CTFGasManager::GetRadius(tf_point_t const *pPoint);
{
	return 25.0f;
}

// 100%
void CTFGasManager::OnCollide(CBaseEntity *pEntity, int iIndex)
{
	CTFPlayer *pPlayer = static_cast< CTFPlayer * >(pEntity);
	if (!pPlayer->m_Shared.IsInvulnerable() 
	&& 	!pPlayer->m_Shared.InCond(TF_COND_PHASE)	// 14
	&& 	!pPlayer->m_Shared.InCond(TF_COND_PASSTIME_INTERCEPTION)	// 106
	&& 	 pPlayer->CanGetWet())
	{
		pPlayer->m_Shared.AddCond(TF_COND_GAS, 10.0, m_hOwnerEntity.Get());	// 123
	}

	m_Gassed.AddToTail(pPlayer->GetRefEHandle());
}

// 100%
void CTFGasManager::OnPointHitWall(tf_point_t *pPoint, Vector &vecPos, Vector &vecAdditionalVel, CGameTrace const &tr, float scale)
{
	vecPos = (tr.plane.normal * (GetRadius(pPoint) + 5.0f)) + tr.endpos;
	vecAdditionalVel = pPoint->m_vecVelocity;
}

// 100%
bool CTFGasManager::ShouldCollide(CBaseEntity *pEntity)
{
	if (pEntity->IsPlayer())
	{
		if (pEntity->GetTeamNumber() != CBaseEntity::GetTeamNumber(this) 
		&& (!TFGameRules() || !TFGameRules()->IsTruceActive()))
		{
			return m_Gassed.Find(pPlayer->GetRefEHandle()) == -1;
		}
	}
	return false;
}

// 100%
bool CTFGasManager::ShouldIgnoreStartSolid(void)
{
	return true;
}

// 80% ; Pretty sketchy vector math
void CTFGasManager::Update(CTFGasManager *this)
{
	// VPROF_BUDGET

	FOR_EACH_VEC_BACK(m_Points, i)
	{
		tf_point_t *pPoint = m_Points[i];
		if (enginetrace->GetPointContents(pPoint->m_vecStartPos) & 0x4030)
		|| gpGlobals->curtime > pPoint->m_flSpawnTime + pPoint->m_flLifeTime)
		{
			RemovePoint(i);
		}
	}
	if (m_Points.Count() > 0)
	{
		FOR_EACH_VEC(m_Points, i)
		{
			tf_point_t* pPoint = m_Points[i];
			pPoint->m_vecEndPos = pPoint->m_vecStartPos;
		}

		if (m_flGasThinkTime <= 1.0)
		{
			m_flGasThinkTime = gpGlobals->curtime;
		}

		if (m_bShouldUpdate)
		{
			if (m_Points.Count() <= 0)
				m_bShouldUpdate = false;
			else
			{
				float gravity = (gpGlobals->curtime - m_flGasThinkTime) * 25.0f;
				FOR_EACH_VEC(m_Points, i)
				{
					tf_point_t *pPoint = m_Points[i];
					Vector vecPos = pPoint->m_vecStartPos;
					trace_t tr;
					UTIL_TraceLine(vecPos, Vector(vecPos.x. VecPos.y, vecPos.z-25.0f), 0x400B, this, COLLISION_GROUP_NONE, &tr);
					if (tr.fraction == 1.0f)
					{
						didsomething = 1;
						pPoint->m_vecStartPos.z -= (gravity * 5.0f);
					}
				}

				bool didsomething = false;
				FOR_EACH_VEC(m_Points, i)
				{
					int somethingcount = 0;
					Vector vecMovement = Vector(0.0f);
					FOR_EACH_VEC(m_Points, j)
					{
						tf_point_t *pIPoint = m_Points[i];
						if (i != j)
						{
							tf_point_t *pJPoint = m_Points[j];
							Vector vecDiff = pIPoint->m_vecStartPos - pJPoint->m_vecStartPos;
							if (vecDiff.Length() < 47.5f)
							{
								vecMovement += vecDiff;
								++somethingcount;
							}
						}
					}
					if (somethingcount)
					{
						// TODO; clean me up!
						float scale = 1.0f / somethingcount;
						vecMovement *= scale;
						float normalized = vecMovement.NormalizeInPlace();
						vecMovement *= -10.0f;
						vecMovement *= normalized;
						trace_t tr;
						// This can't be right, it's pIPoint->m_vecStartPos - pIPoint->m_vecStartPos. Wtf?
						UTIL_TraceLine(Vector(0.0f, 0.0f, 0.0f), vecMovement, 0x400B, this, COLLISION_GROUP_NONE, &tr);
						if (tr.fraction == 1.0f)
						{
							didsomething = true;
							pIPoint->m_vecStartPos += vecMovement;
						}
					}
				}
				m_bShouldUpdate = didsomething;
				if (didsomething)
				{
					Vector vecMin = Vector(-16384.0f);
					Vector vecMax = Vector(16384.0f);
					Vector vecCenter;
					if (m_Points.Count() > 0)
					{
						FOR_EACH_VEC(m_Points, i)
						{
							tf_point_t *pPoint = m_Points[i];
							float radius = GetRadius(pPoint);
							vecMin = vecMin.Max(pPoint->m_vecStartPos - radius);
							vecMax = vecMax.Max(pPoint->m_vecStartPos + radius);
						}

						vecCenter = (vecMax - vecMin) * 0.5f;
					}
					else
					{
						vecCenter = Vector(0.0f);
					}

					SetAbsOrigin(vecCenter);
					UTIL_SetSize(this, -vecCenter, vecCenter);
				}
			}
		}
	}
	else
	{
		SetThink(&CBaseEntity::SUB_Remove);
	}
}

// 100%
void CTFGasManager::UpdateOnRemove(void)
{
	BaseClass::UpdateOnRemove();
}

CTFGasManager *CTFGasManager::Create(CBaseEntity *pEntity, Vector const&vecPos)
{
	CTFGasManager *pGas = static_cast< CTFGasManager *>(CBaseEntity::Create("tf_gas_manager", vecPos, &vec3_angle, pEntity));
	if (pGas)
	{
		pGas->SetOwnerEntity(pEntity);
		pGas->ChangeTeam(pEntity->GetTeamNumber());
	}
	return pGas;
}

// END REVERSE 12/23/2020

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
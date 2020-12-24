// Definitely not a Valve header

// %REVERSED 12/22/2020

#include "cbase.h"
#include "tf_flame.h"
#include "tf_player.h"
#include "tf_fx.h"
#include "tf_team.h"
#include "tf_gamestats.h"
#include "tf_gamerules.h"
#include "random.h"
#include "vector.h"
#include "trace.h"
#include "tier0/dbg.h"
#include "basehandle.h"
#include "shot_manipulator.h"
#include "tf_weapon_flamethrower.h"
#include "tf_weapon_compound_bow.h"
#include "takedamageinfo.h"

// %REVERSED 12/22/2020
IMPLEMENT_NETWORKCLASS_ALIASED(TFFlameManager, DT_TFFlameManager)

// Idc about the flags
BEGIN_NETWORK_TABLE(CTFFlameManager, DT_TFFlameManager)
#ifndef CLIENT_DLL
SendPropEHandle(SENDINFO(m_hWeapon)),
	SendPropEHandle(SENDINFO(m_hAttacker)),
	SendPropInt(SENDINFO(m_flSpreadDegree)),
	SendPropFloat(SENDINFO(m_flRedirectedFlameSizeMult)),
	SendPropFloat(SENDINFO(m_flFlameStartSizeMult)),
	SendPropFloat(SENDINFO(m_flFlameEndSizeMult)),
	SendPropFloat(SENDINFO(m_flFlameIgnorePlayerVelocity)),
	SendPropFloat(SENDINFO(m_flFlameReflectionAdditionalLifeTime)),
	SendPropFloat(SENDINFO(m_flFlameReflectionDamageReduction)),
	SendPropInt(SENDINFO(m_iMaxFlameReflectionCount)),
	SendPropInt(SENDINFO(m_nShouldReflec)),
	SendPropFloat(SENDINFO(m_flFlameSpeed)),
	SendPropFloat(SENDINFO(m_flFlameLifeTime)),
	SendPropFloat(SENDINFO(m_flRandomLifeTimeOffset)),
	SendPropFloat(SENDINFO(m_flFlameGravity)),
	SendPropFloat(SENDINFO(m_flFlameDrag)),
	SendPropFloat(SENDINFO(m_flFlameUp)),
	SendPropBool(SENDINFO(m_bIsFiring)),
#else
RecvPropEHandle(RECVINFO(m_hWeapon)),
	RecvPropEHandle(RECVINFO(m_hAttacker)),
	RecvPropFloat(RECVINFO(m_flSpreadDegree)),
	RecvPropFloat(RECVINFO(m_flRedirectedFlameSizeMult)),
	RecvPropFloat(RECVINFO(m_flFlameStartSizeMult)),
	RecvPropFloat(RECVINFO(m_flFlameEndSizeMult)),
	RecvPropFloat(RECVINFO(m_flFlameIgnorePlayerVelocity)),
	RecvPropFloat(RECVINFO(m_flFlameReflectionAdditionalLifeTime)),
	RecvPropFloat(RECVINFO(m_flFlameReflectionDamageReduction)),
	RecvPropInt(RECVINFO(m_iMaxFlameReflectionCount)),
	RecvPropInt(RECVINFO(m_nShouldReflect)),
	RecvPropFloat(RECVINFO(m_flFlameSpeed)),
	RecvPropFloat(RECVINFO(m_flFlameLifeTime)),
	RecvPropFloat(RECVINFO(m_flRandomLifeTimeOffset)),
	RecvPropFloat(RECVINFO(m_flFlameGravity)),
	RecvPropFloat(RECVINFO(m_flFlameDrag)),
	RecvPropFloat(RECVINFO(m_flFlameUp)),
	RecvPropBool(RECVINFO(m_bIsFiring)),
#endif
	END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS(tf_flame_manager, CTFFlameManager);

// 100%
CTFFlameManager::CTFFlameManager()
{
	m_FlameManager = ITFFlameManager(true);
	m_BurnedEntities = CUtlMap<CHandle<CBaseEntity>, burned_entity_t>();
	m_hWeapon = -1;
	m_hAttacker = -1;
	m_flSpreadDegree = 0.0;
	m_flRedirectedFlameSizeMult = 1.0;
	m_flFlameStartSizeMult = 1.0;
	m_flFlameEndSizeMult = 1.0;
	m_flFlameIgnorePlayerVelocity = 0.0;
	m_flFlameReflectionAdditionalLifeTime = 0.0;
	m_flFlameReflectionDamageReduction = 1.0;
	m_iMaxFlameReflectionCount = 0;
	m_nShouldReflect = 0;
	m_flFlameSpeed = 0.0;
	m_flFlameLifeTime = 0.0;
	m_flRandomLifeTimeOffset = 0.0;
	m_flFlameGravity = 0.0;
	m_flFlameDrag = 0.0;
	m_flFlameUp = 0.0;
	m_bIsFiring = 0;
}

// 100%
CTFFlameManager::~CTFFlameManager()
{
	ITFFlameManager::m_ITFFlameManagerAutoList.FindAndFastRemove(m_FlameManager);
	m_BurnedEntities.RemoveAll();
	m_Points.RemoveAll();
}

// 100%
float CTFFlameManager::GetInitialSpeed(void)
{
	return m_flFlameSpeed;
}

// 100%
float CTFFlameManager::GetGravity(void)
{
	return m_flFlameGravity
}

// 100%
float CTFFlameManager::GetDrag(void)
{
	return m_flFlameDrag;
}

// 100%
void CTFFlameManager::UpdateOnRemove(void)
{
	BaseClass::UpdateOnRemove();
}

// 100%
void CTFFlameManager::InitializePoint(tf_point_t *pPoint, int iIndex)
{
	flame_point_t *pFlamePoint = static_cast< flame_point_t * >(pPoint);

	CTFPointManager::InitializePoint(this, pPoint, iIndex);
	CBaseEntity *pAttacker = m_hAttacker.Get();
	if (pAttacker)
	{
		pFlamePoint->m_vecAttackerVel = pAttacker.GetLocalVelocity();
		pFlamePoint->m_vecAttackerPos = pFlamePoint->m_vecStartPos;
	}
}

// 100%
Vector CTFFlameManager::GetInitialVelocity(void)
{
	Vector vecReturn;
	CBaseEntity *pAttacker = m_hAttacker.Get();
	if (pAttacker)
	{
		QAngle vecEyes = pAttacker->EyeAngles();
		Vector vecDir;
		AngleVectors(vecEyes, &vecDir);
		if (m_flSpreadDegree > 0.0)
		{
			Vector vecWhat = Vector(0.017453292f * m_flSpreadDegree);
			CShotManipulator manipulator(vecDir);
			vecDir = manipulator.ApplySpread(vecWhat, m_flSpreadDegree);
		}
		float initialspeed = GetInitialSpeed();
		vecReturn = vecDir * initialspeed;
	}
	else
	{
		vecReturn = vec3_origin;
	}
	return vecReturn;
}

// 100%
void CTFFlameManager::AddPoint(int iIndex)
{
	BaseClass::AddPoint(iIndex);
}

// 100%
Vector CTFFlameManager::GetInitialPosition(void)
{
	CTFFlameThrower *pEntity = static_cast< CTFFlameThrower * >(m_hOwnerEntity.Get());
	if (pEntity)
	{
		return pEntity->GetFlameOriginPos();
	}
	return vec3_origin;
}

// 100%
void CTFFlameManager::ModifyAdditionalMovementInfo(tf_point_t *pPoint, float scale)
{
	// VPROF_BUDGET
	if (m_flFlameIgnorePlayerVelocity != 0.0f)
	{
		flame_point_t *pFlamePoint = static_cast< flame_point_t * >(pFlamePoint);
		Vector vecVel = pFlamePoint->m_vecAttackerVel;

		float normalized = vecVel.NormalizeInPlace();
		float drag = GetDrag();
		float computed = (1.0 - (drag * scale)) * normalized;
		float fOut = 0.0;

		if (computed >= 0.0)
			fOut = fminf(normalized, computed);
		pFlamePoint->m_vecAttackerVel *= v13;
	}
}

// 100%
bool CTFFlameManager::OnPointHitWall(tf_point_t *pPoint, Vector &vecPos, Vector &vecAdditionalVel, CGameTrace const &tr, float scale)
{
	// VPROF_BUDGET

	if (m_iMaxFlameReflectionCount <= 0 || pPoint->m_nTouches <= m_iMaxFlameReflectionCount)
	{
		pPoint->m_flLifeTime += m_flFlameReflectionAdditionalLifeTime;
		Vector vecNormal = tf->plane.normal;
		v12 = ((vecPos.y * v10) + (vecPos.x * v9)) + (vecPos.z * v11);
		float vellength = vecPos.LengthSqr();
		Vector vecMove = (vecPos - vecNormal * vellength);
		if (this->m_nShouldReflect > 0)
		{
			vecMove += vecMove - vecPos;
		}

		pPoint->m_vecAttackerVel = vec3_origin;
		vecPos = vecMove;
		vecPos = (vecNormal * GetRadius(pPoint) + tr.endpos) + (vecOut * ((1.0f - tr.fraction) * scale));
		return false;
	}
	return true;
}

// 100%
Vector CTFFlameManager::GetAdditionalVelocity(tf_point_t const *pPoint)
{
	// VPROF_BUDGET
	Vector vecVel = pPoint->m_vecVelocity;
	float fnormalized = vecVel.NormalizeInPlace();

	Vector vecOut = vecVel * fnormalized;
	vecOut.z += m_flFlameUp;
	return vecOut;
}

// 100%
void CTFFlameManager::StartFiring(void)
{
	m_bIsFiring = true;
}

// 100%
void CTFFlameManager::StopFiring(void)
{
	m_bIsFiring = false;
}

// 100%
void CTFFlameManager::Update(void)
{
	CTFPointManager::Update(this);
	if (tf_debug_flamethrower.GetBool())
	{
		if (m_Points.Count() > 0)
		{
			FOR_EACH_MAP(m_Points, i)
			{
				flame_point_t *pPoint = m_Points[i];
				float radius = GetRadius(pPoint);
				Vector vecMins = Vector(-radius);
				Vector vecMaxs = Vector(radius);
//				DebugFlame(pPoint->m_VecEndPos, pPoint->m_vecStartPos, vecMins, vecMaxs, 3.7631358e22);
				NDebugOverlay::Line(pPoint->vecEndPos, pPoint->m_vecStartPos, 255, 0, 0, false, 3.7631358e22f);
				NDebugOverlay::SweptBox(pPoint->vecEndPos, pPoint->m_vecStartPos, vecMins, vecMaxs, vec3_angle, 255, 0, 0, 3.7631358e22f);
			}
		}
	}
}

// 100%
void CTFFlameManager::UpdateDamage(int dmgflags, float a3, float a4, bool a5)
{
	m_flDamage = a3;
	m_fDamageFlags = dmgflags;
	m_flBurnDelay = a4;
	m_bIsCritical = a5;
}

// 100%
void CTFFlameManager::SetHitTarget()
{
	CTFFlameThrower *pWeapon = m_hWeapon.Get();
	if (pWeapon)
	{
		pWeapon->SetHitTarget();
	}
}

// 100%
bool CTFFlameManager::IsValidBurnTarget(CBaseEntity *pTarget)
{
	CBaseCombatCharacter *pAttacker = m_hAttacker.Get();
	if (!pAttacker || pAttacker == pTarget)
		return false;

	if (!pTarget->IsPlayer() || !pTarget->IsAlive())
	{
		if (pTarget->MyNextBotPointer() && pTarget->IsAlive())
			return true;

		if (pTarget->IsBaseObject())
		{
			if (pAttacker->GetTeamNumber() != pTarget->GetTeamNumber())
				return true;
		}
		if ( CTFRobotDestructionLogic::GetRobotDestructionLogic(pTarget) )
		{
			if (pAttacker->GetTeamNumber() != pTarget->GetTeamNumber()
			&& (pTarget->m_iClassname == "tf_robot_destruction_robot" 
			|| pTarget->ClassMatchesComplex("tf_robot_destruction_robot")))
			{
				return true;
			}
		}
		return pTarget->m_iClassname == "func_breakable" 
		|| pTarget->ClassMatchesComplex("func_breakable") 
		|| pTarget->m_iClassname == "tf_pumpkin_bomb" 
		|| pTarget->ClassMatchesComplex("tf_pumpkin_bomb") 
		|| pTarget->m_iClassname == "tf_merasmus_trick_or_treat_prop" 
		|| pTarget->ClassMatchesComplex("tf_merasmus_trick_or_treat_prop")
		|| pTarget->m_iClassname == "tf_generic_bomb"
		|| pTarget->ClassMatchesComplex("tf_generic_bomb");
	}
	return true;
}

// END REVERSE 12/22/2020

// %REVERSED 12/23/2020

// 100%
bool CTFFlameManager::ShouldCollide(CBaseEntity *pOther)
{
	// PROF_BUDGET
	return IsValidBurnTarget(pOther);
}

// 100%
float CTFFlameManager::GetFlameSizeMult(tf_point_t const *pPoint)
{
	float diff = gpGlobals->curtime - pPoint->m_flSpawnTime;
	float mult = RemapValClamped(val, 0.0f, pPoint->m_flLifeTime, m_flFlameStartSizeMult, m_flFlameEndSizeMult);
	if (pPoint->m_nTouches > 0)
		mult *= m_flRedirectedFlameSizeMult;
	return mult;
}

// 100%
float CTFFlameManager::GetRadius(tf_point_t const *pPoint)
{
	float mult = GetFlameSizeMult(pPoint);
	return mult * tf_flamethrower_boxsize.GetFloat();
}

// 100%
float CTFFlameManager::GetStartSizeMult(void)
{
	return m_flFlameStartSizeMult;
}

// 100%
float CTFFlameManager::GetEndSizeMult(void)
{
	return m_flFlameEndSizeMult;
}

// 100%
bool CTFFlameManager::ShouldIgnorePlayerVelocity(void)
{
	return m_flFlameIgnorePlayerVelocity != 0.0;
}

// 100%
float CTFFlameManager::ReflectionAdditionalLifeTime(void)
{
	return m_flFlameReflectionAdditionalLifeTime;
}

// 100%
float CTFFlameManager::ReflectionDamageReduction(void)
{
	return m_flFlameReflectionDamageReduction;
}

// 100%
int CTFFlameManager::GetMaxFlameReflectionCount()
{
	return m_iMaxFlameReflectionCount;
}

// Decomp was literally 2300 lines please give me a break if this isn't 1:1
// 100%
void CTFFlameManager::HookAttributes(void)
{
	float fval = m_flSpreadDegree;
	CALL_ATTRIB_HOOK_FLOAT_ON_OTHER(m_hAttacker, fval, flame_spread_degree);
	m_flSpreadDegree = fval;

	fval = m_flRedirectedFlameSizeMult;
	CALL_ATTRIB_HOOK_FLOAT_ON_OTHER(m_hAttacker, fval, redirected_flame_size_mult);
	m_flRedirectedFlameSizeMult = fval;

	fval = m_flFlameStartSizeMult;
	CALL_ATTRIB_HOOK_FLOAT_ON_OTHER(m_hAttacker, fval, mult_flame_size);
	m_flFlameStartSizeMult = fval;

	fval = m_flFlameEndSizeMult;
	CALL_ATTRIB_HOOK_FLOAT_ON_OTHER(m_hAttacker, fval, mult_end_flame_size);
	m_flFlameEndSizeMult = fval;

	fval = m_flFlameIgnorePlayerVelocity;
	CALL_ATTRIB_HOOK_FLOAT_ON_OTHER(m_hAttacker, fval, flame_ignore_player_velocity);
	m_flFlameIgnorePlayerVelocity = fval;

	fval = m_flFlameReflectionAdditionalLifeTime;
	CALL_ATTRIB_HOOK_FLOAT_ON_OTHER(m_hAttacker, fval, flame_reflection_add_life_time);
	m_flFlameReflectionAdditionalLifeTime = fval;

	fval = m_flFlameReflectionDamageReduction;
	CALL_ATTRIB_HOOK_FLOAT_ON_OTHER(m_hAttacker, fval, reflected_flame_dmg_reduction);
	m_flFlameReflectionDamageReduction = fval;

	fval = m_flFlameSpeed;
	CALL_ATTRIB_HOOK_FLOAT_ON_OTHER(m_hAttacker, fval, flame_speed);
	m_flFlameSpeed = fval;

	fval = m_flFlameLifeTime;
	CALL_ATTRIB_HOOK_FLOAT_ON_OTHER(m_hAttacker, fval, flame_random_life_time_offset);
	m_flFlameLifeTime = fval;

	fval = m_flRandomLifeTimeOffset;
	CALL_ATTRIB_HOOK_FLOAT_ON_OTHER(m_hAttacker, fval, flame_lifetime);
	m_flRandomLifeTimeOffset = fval;

	fval = m_flFlameGravity;
	CALL_ATTRIB_HOOK_FLOAT_ON_OTHER(m_hAttacker, fval, flame_gravity);
	m_flFlameGravity = fval;

	fval = m_flFlameDrag;
	CALL_ATTRIB_HOOK_FLOAT_ON_OTHER(m_hAttacker, fval, flame_drag);
	m_flFlameDrag = fval;

	fval = m_flFlameUp;
	CALL_ATTRIB_HOOK_FLOAT_ON_OTHER(m_hAttacker, fval, flame_up_speed);
	m_flFlameUp = fval;

	int ival = m_iMaxFlameReflectionCount;
	CALL_ATTRIB_HOOK_INT_ON_OTHER(m_hAttacker, ival, max_flame_reflection_count);
	m_iMaxFlameReflectionCount = ival;

	ival = m_nShouldReflect;
	CALL_ATTRIB_HOOK_INT_ON_OTHER(m_hAttacker, ival, flame_reflect_on_collision);
	m_nShouldReflect = ival;
}

// 100%
float CTFFlameManager::GetFlameDamageScale(tf_point_t const *pPoint, CTFPlayer *pPlayer)
{
	// VPROF_BUDGET
	// Did I do this cast right? o_0
	flame_point_t *pFlamePoint = const_cast<flame_point_t * >(pPoint);
	float flOut = 0.0;
	if (tf_flame_dmg_mode_dist.GetBool()) // *(dword_1754B9C + 48))
	{
		flOut = -0.5f * clamp(pPoint->m_vecStartPos.LengthSqr() - 22500.0f * 0.000014814815f, 0.0f, 1.0f);

	}
	else
	{
		float diff = gpGlobals->curtime - pPoint->m_flSpawnTime;
		flOut = RemapValClamped(diff, 0.0f, pPoint->m_flLifeTime * 0.5, 1.0f, 0.5f);
	}

	if (pPlayer)
	{
		unsigned short index = m_BurnedEntities.Find(pPlayer->GetRefEHandle());
		float scale = 1.0;
		if (index != -1)
		{
			burned_entity_t burned = m_BurnedEntities[index];
			scale = clamp((burned.field10 - 10.0f) * 0.025f, 0.0f, 1.0f) * 0.5f + 0.5f;
		}
		flOut *= scale;
	}

	for (int i = 0; i < pPoint->m_nTouches; ++i)
	{
		flOut *= m_flFlameReflectionDamageReduction;
	}
	return flOut;
}

// 100%
bool CTFFlameManager::BCanBurnEntityThisFrame(CBaseEntity *pEntity)
{
	if (!pEntity)
		return false;

	unsigned short index = m_BurnedEntities.Find(pEntity->GetRefEHandle());
	if (index == -1)
		return true;
	return gpGlobals->curtime - m_BurnedEntities[index].field_10 >= m_flBurnDelay;
}

// 100%
void CTFFlameManager::PostEntityThink(void)
{
	FOR_EACH_MAP_FAST(m_BurnedEntities, i)
	{
		burned_entity_t burned = m_BurnedEntities[i];
		if (gpGlobals->curtime - burned.m_flSpawnTime2 <= m_flBurnDelay)
			continue;

		m_BurnedEntities.RemoveAt(i);
	}

	CTFFlameThrower *pWeapon = static_cast< CTFFlameThrower * >(m_hWeapon.Get());
	if (pWeapon)
	{
		pWeapon->ResetFlameHitCount();
		if (m_BurnedEntities.Count())
		{
			pWeapon->IncrementFlameDamageCount();
			pWeapon->IncrementActiveFlameCount();
		}
	}
}

// 80% ; Decomp wasn't very nice with this
bool CTFFlameManager::OnCollide(CBaseEntity *pEntity, int iIndex)
{
	// VPROF_BUDGET
	CBaseEntity *pAttacker = m_hAttacker.Get();
	if (!pAttacker)
		return false;

	unsigned short findindex = m_BurnedEntities.Find(pAttacker->GetRefEHandle());
	if (findindex != -1)
	{
		burned_entity_t *burned = &m_BurnedEntities[findindex];
		burned->field_10 = (burned->field_10 + 2.0) - clamp((gpGlobals->curtime - m_Points[iIndex]->m_flSpawnTime) * 50.0, 0.0, 1.0);
	}

	if (!BCanBurnEntityThisFrame(pEntity))
	{
		return;
	}
	CTFPlayer *pPlayer = static_cast< CTFPlayer * >(pEntity);
	if (pEntity->IsPlayer() && pEntity->InSameTeam(pAttacker))
	{
		if (!pPlayer->IsPlayerClass(TF_CLASS_SNIPER))
			return

		CTFCompoundBow *pWeapon = static_cast< CTFCompoundBow * >(pPlayer->GetActiveTFWeapon());
		if (pWeapon && pWeapon->GetWeaponID() == TF_WEAPON_COMPOUND_BOW)	// 61
			pWeapon->SetArrowAlight(true);
	}
	else
	{
		int damageflags = m_fDamageFlags;
		SetHitTarget();
		flame_point_t *pFlamePoint = static_cast< flame_point_t * >(m_Points[iIndex]);
		if (pEntity->IsPlayer())
		{
			Vector vecDist = pFlamePoint->m_vecStartPos - pFlamePoint->m_vecAttackerPos;
			float dist = vecDist.NormalizeInPlace();
			QAngle vecAng = pEntity->GetAbsAngles();

			Vector vecFwd;
			AngleVectors(vecAng, &vecFwd);
			vecFwd.z = 0.0;
			vecFwd.NormalizeInPlace();
			if (vecFwd.LengthSqr() * dist > 0.8f)
			{
				if (m_bIsCritical)	// Is backburner?
					damageflags |= DMG_CRIT;

				pPlayer->HandleAchievement_Pyro_BurnFromBehind(pAttacker);
			}
			if (pPlayer->IsPlayerClass(TF_CLASS_PYRO))
			{
				if (pPlayer->m_Shared.InCond(TF_COND_TAUNTING))
				{
					static CSchemaItemDefHandle flipTaunt("Flippin' Awesome Taunt");
					if (pPlayer->GetTauntEconItemView())
					{
						if (pPlayer->GetTauntEconItemView()->GetItemDefinition() == flipTaunt)
						{
							pPlayer->AwardAchievement(ACHIEVEMENT_TF_PYRO_IGNITE_PLAYER_BEING_FLIPPED, 1);	// 1641
						}
					}
				}
				pPlayer->m_Shared.AddCond(TF_COND_HEALING_DEBUFF, 2.0f, pPlayer);	// 118
			}
		}
	
		float scale = GetFlameDamageScale(pFlamePoint, pEntity->IsPlayer() ? pPlayer : NULL);
		CBaseEntity *pOwner = m_hOwnerEntity.Get();
		float damage = fmaxf(scale * m_flDamage, 1.0);

		CTakeDamageInfo info(pOwner, pEntity, pOwner, damage, damageflags, 3);

		Vector vecPosition = pEntity->GetAbsOrigin();

		// FIXME ; what is this?
//		if ((v84 & 0x10) != 0)
//			info.SetCritType(CRIT_FULL);
		if (damageflags & DMG_CRIT)
			info.SetCritType(CRIT_FULL);

		// FIXME ; this doesn't seem right
		if (TFGameRules())
		{
			CBaseCombatCharacter *pBoss = TFGameRules()->GetActiveBoss();
			if (pBoss)
			{
				if (pBoss->GetBossType() == HALLOWEEN_BOSS_MERASMUS) // 3
				{
					info.SetDamagePosition(pBoss->GetAbsOrigin());
				}
			}
		}

		trace_t tr;
		UTIL_TraceLine(pEntity->WorldSpaceCenter(), WorldSpaceCenter(), 0x4200400B, this, COLLISION_GROUP_NONE, &tr);
		pEntity->DispatchTraceAttack(info, GetAbsVelocity(), &tr);
		ApplyMultiDamage();
	}

	burned_entity_t *burned;
	if (findindex == -1)
	{
		burned = &m_BurnedEntities[m_BurnedEntities.Insert(pEntity->GetRefEHandle())];
		burned->m_flSpawnTime = gpGlobals->curtime;
		burned->field_8 = 1.0;
		burned->m_hEntity = pEntity->GetRefEHandle();
	}
	else
	{
		burned = m_BurnedEntities[findindex];
		burned->m_flSpawnTime2 = gpGlobals->curtime;
	}
}

// 100%
float CTFFlameManager::GetLifeTime(void)
{
	float random = m_Randomizer.RandomFloat(-m_flRandomLifeTimeOffset, m_flRandomLifeTimeOffset);
	return random + m_flBurnDelay;
}

// 100%
int CTFFlameManager::GetMaxPoints(void)
{
	return 30;
}

// 100%
tf_point_t* CTFFlameManager::AllocatePoint()
{
	flame_point_t* pPoint = new flame_point_t();
	return pPoint;
}

// 100%
CTFFlameManager *CTFFlameManager::Create(CBaseEntity *pEntity, bool b)
{
	CTFFlameManager *pFlameManager = static_cast< CTFFlameManager * >(CBaseEntity::Create("tf_flame_manager", vec3_origin, vec3_angle, pEntity));
	if (pFlameManager)
	{
		pFlameManager->SetOwnerEntity(pEntity);
		pFlameManager->m_hAttacker = pEntity->GetRefEHandle();

		CTFWeaponBase *pWeapon = dynamic_cast< CTFWeaponBase * >(pEntity);
		if (pWeapon)
		{
			pFlameManager->m_hWeapon = pWeapon->GetRefEHandle();
			pFlameManager->ChangeTeam(pFlameManager->GetTeamNumber());
			pFlameManager->HookAttributes();
		}
	}
	return pFlameManager;
}
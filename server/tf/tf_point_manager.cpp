// Definitely not a Valve header

// %REVERSED 12/21/2020

#include "cbase.h"
#include "tf_point_manager.h"
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

// Idc about the flags
IMPLEMENT_NETWORKCLASS_ALIASED(TFPointManager, DT_TFPointManager)

BEGIN_NETWORK_TABLE(CTFPointManager, DT_TFPointManager)
#ifndef CLIENT_DLL
	SendPropInt( SENDINFO( m_nRandomSeed ) ),
	SendPropInt( SENDINFO( m_unNextPointIndex ) ),
	SendPropArray( SENDINFO( m_nSpawnTime ) ),
#else
	RecvPropInt( RECVINFO( m_nRandomSeed ) ),
	RecvPropInt( RECVINFO( m_unNextPointIndex ) ),
	RecvPropArray( RECVINFO( m_nSpawnTime ) ),
#endif
END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS(tf_point_manager, CTFPointManager);

// 75%, fuck this
CTFPointManager::CTFPointManager()
{
	m_Randomizer = CUniformRandomStream();
	m_nRandomSeed = 0;
	// This is what valve thinks of as random, apparently
	int v1 = -(((this + 4) & 0xF) / 4) & 3;
	int v17 = 0, v19 = 0;
	if (v1)
	{
		m_nSpawnTime[0] = 0;
		if (v1 <= 1)
		{
			v17 = 29;
			v19 = 1;
		}
		else
		{
			m_nSpawnTime[1] = 0;
			if ((-(((this + 4) & 0xF) / 4) & 3) == 3)
			{
				m_nSpawnTime[2] = 0;
				v17 = 27;
				v19 = 3;
			}
			else
			{
				v17 = 28;
				v19 = 2;
			}
		}
	}
	else
	{
		v17 = 30;
		v19 = 0;
	}
	int numpoints = 30 - v1;
	int v3 = (30 - v1) / 4;
	int v16 = 4 * v3;
	int *v4 = &m_nSpawnTime[v1];
	int v5 = 0;
	do
	{
		++v5;
		m_nSpawnTime[v1++] = 0;
	} while (v3 > v5);
	int v6 = v16 + v19;
	if (numpoints != v16)
	{
		do
			m_nSpawnTime[v6++] = 0;
		while (v6 != v19 + v17);
	}
	m_unNextPointIndex = 0;
	m_Points = CUtlVector();
	field_494 = NULL;
	m_flThinkTime = gpGlobals->curtime;
}

// 100%
void CTFPointManager::Spawn()
{
	BaseClass::Spawn();
	m_takedamage = DAMAGE_NO;

	SetSolid(SOLID_BBOX);
	SetMoveType(MOVETYPE_FLY);
	SetSolidFlags(FSOLID_TRIGGER|FSOLID_NOT_SOLID);		// 12
	SetCollisionGroup(24);	// LAST_SHARED_COLLISION_GROUP + 4 ?
	m_nRandomSeed = RandomInt(0, 9999);

	// Assuming this is what it's called, /shrug
	SetContextThink(&CTFPointManager::PointThink, gpGlobals->curtime, "PointThink");
}

// 100%
unsigned int CTFPointManager::PhysicsSolidMaskForEntity(void)
{
	unsigned int result = BaseClass::PhysicsSolidMaskForEntity();
	result |= 0x18u;
	return result;
}

// 100%
void CTFPointManager::PointThink(void)
{
	Update();
	SetNextThink(gpGlobals->curtime, "PointThink");
}

// 100%
int CTFPointManager::UpdateTransmitState()
{
	return BaseClass::SetTransmitState(FL_EDICT_PVSCHECK);
}

// END REVERSE 12/21/2020

// %REVERSED 12/22/2020
// 80%	; Not confident with the tracing
void CTFPointManager::Touch(CBaseEntity *pOther)
{
	if (ShouldCollide(pOther) && m_Points.Count() > 0)
	{
		bool go = false;
		int index = 0;
		FOR_EACH_VEC_BACK(m_Points, i)
		{
			index = i;
			tf_point_t *point = m_Points[i];
			float radius = GetRadius(point);
			int flags = 0x4200400B;

			Vector vecMins = Vector(-radius);
			Vector vecMaxs = Vector(radius);

			Ray_t ray;
			ray.Init(point->m_vecEndPos, point->m_vecStartPos, vecMins, vecMaxs);

			trace_t tr;
			enginetrace->ClipRayToEntity(ray, flags, pOther, &tr);
			if (tr.fraction < 1.0f || ray.m_IsSwept || ray.m_IsRay)
			{
				go = true;
				break;
			}
		}
		if (go)
			OnCollide(pOther, index);
	}
	return result;
}

// 100%
int CTFPointManager::RemovePoint(int iIndex)
{
	tf_point_t* point = m_Points[iIndex];
	m_nSpawnTime[point->m_iIndex] = 0;
	m_Points.FindAndRemove(point);
}

// 99%
void CTFPointManager::UpdateOnRemove(void)
{
	m_Points.RemoveAll();
	field_494 = m_Points.m_pMemory;		// TODO; This can't be right, but it shows up as such in disasm!
	BaseClass::UpdateOnRemove();
}

// 99%
void CTFPointManager::ClearPoints(void)
{
	m_Points.RemoveAll();
	field_494 = m_Points.m_pMemory;		// TODO; This can't be right, but it shows up as such in disasm!
}

// 100%
bool CTFPointManager::AddPointInternal(unsigned int index)
{
	tf_point_t *point = AllocatePoint();
	if (!point)
		return false;

	InitializePoint(point, index);
	m_Points.AddToTail(point);
	return true;
}

// 75% ; most likely operational
void CTFPointManager::Update(void)
{
	// VPROF_BUDGET(what, ever);
	
	int numpoints = m_Points.Count();
	m_flThinkTime = gpGlobals->curtime;
	Vector vMin, vMax;
	vMax = Vector(16384.0f);
	vMin = Vector(-16384.0f);

	FOR_EACH_VEC_BACK(m_Points, i)
	{
		point = m_Points[i];
		int *v5 = *(this + 0x20);	// FIXME
		int v6 = v5 ? *(v5 + 6) : 0;
		m_Randomizer.SetSeed(this->m_nSpawnTime[point->m_iIndex] + v6);

		Vector vec1, vec2, vec3;
		if ((enginetrace->GetPointContents(point->m_vecSpartPos) & 0x4030) != 0
		|| gpGlobals->curtime > (point->m_flSpawnTime + point->m_flLifetime)
		|| !UpdatePoint(point, i, m_flThinkTime, vec1, vec2, vec3))
		{
			RemovePoint(point);
			continue;
		}

		vMax = vMax.Min(vec2 + vec1);
		vMin = vMin.Max(vec1 + vec3);
	}

	if (numpoints)
	{
		if (m_Points.Count())
		{
			Vector vecMax = (vMax - vMin) * 0.5f;
			Vector vecOrigin = vecMax + vMax;
			SetAbsOrigin(vecOrigin);
			Vector vecMin = -vecMax;
			UTIL_SetSize(vecMin, vecMax);
		}
		else
		{
			UTIL_Remove(this);
		}
	}
}

// 100%
bool CTFPointManager::ShouldCollide(CBaseEntity *pOther)
{
	return true;
}

// 100%
CTFPointManager::~CTFPointManager()
{
	m_Points.Purge();
}

// 100%
bool CTFPointManager::AddPoint(int nSpawnTime)
{
	if (m_Points.Count() >= GetMaxPoints() || !AddPointInternal(m_unNextPointIndex))
		return false;

	m_nSpawnTime[m_unNextPointIndex] = nSpawnTime;
	m_unNextPointIndex = (m_unNextPointIndex + 1) % 30;
	return true;
}

// 100%
void CTFPointManager::InitializePoint(tf_point_t *pPoint, int iIndex)
{
	m_Randomizer.SetSeed(m_nRandomSeed + iIndex + m_nSpawnTime[iIndex]);
	Vector vPos = GetInitialPosition();
	pPoint->m_vecStartPos = vPos;
	pPoint->m_vecEndPos = vPos;
	pPoint->m_vecVelocity = GetInitialVelocity();
	pPoint->m_flSpawnTime = gpGlobals->curtime;
	pPoint->m_iIndex = iIndex;
	pPoint->m_flLifetime = GetLifeTime();
}

// 100%
Vector CTFPointManager::GetInitialPosition(void)
{
	return vec3_origin;
}

// 100%
Vector CTFPointManager::GetInitialVelocity(void)
{
	return vec3_origin;
}

// 100%
void CTFPointManager::OnCollide(CBaseEntity * pEntity, unsigned int i)
{

}

// 100%
tf_point_t* CTFPointManager::AllocatePoint()
{
	tf_point_t* pPoint = tf_point_t();
	return pPoint;
}

// 99% ; those dynamic casts are fishy
bool CTFPointManager::UpdatePoint(tf_point_t *pPoint, unsigned int index, float scale, Vector *a5, Vector *a6, Vector *a7)
{
	if (scale <= 0.0)
		return true;

	float radius = GetRadius();
	float radiusn = -radius;
	float gravity = GetGravity();
	float gravityscaled = gravity * scale;
	Vector vecAdditionalVelocity = GetAdditionalVelocity(pPoint) + pPoint->m_vecVelocity;
	Vector vecPos = vecAdditionalVelocity * scale + pPoint->m_vecStartPos;
	if (a6)
	{
		a6->x = radiusn;
		a6->y = radiusn;
		a6->z = radiusn;
	}
	if (a7)
	{
		a7->x = radius;
		a7->y = radius;
		a7->z = radius;
	}
	if (a5)
	{
		a5->x = vecPos.x;
		a5->y = vecPos.y;
		a5->z = vecPos.z;
	}

	Vector vecPos = pPoint->m_vecStartPos;
	Vector vecMins = Vector(-radius);
	Vector vecMaxs = Vector(radius);
	trace_t tr;
	UTIL_TraceHull(pPoint->m_vecStartPos, pPoint->m_vecStartPos, vecMins, vecMaxs, 0x200400B, this, 1, &tr);

	if (!ShouldIgnoreStartSolid(this) && tr.startsolid)
		return false;

	if (tr.fraction < 1.0)
	{
		CBaseEntity* pEnt = tr.m_pEnt;
		++pPoint->m_nTouches;
		if (pEnt)
		{
			// This can't be right, right?
			if (ShouldCollide(pEnt))
			{
				if (dynamic_cast< CTFPumpkinBomb* >(pEnt)
				|| dynamic_cast< CTFGenericBomb* >(pEnt)
				|| dynamic_cast< CTFMerasmusTrickOrTreatProp* >(pEnt))
				{
					OnCollide(pEnt, index);
				}
			}
		}
		if (OnPointHitWall(pPoint, vecPos, vecAdditionalVelocity, tr, scale))
			return false;
	}

	float drag = GetDrag();
	float scaleddrag = drag * scale;
	float normalized = pPoint->m_vecVelocity.NormalizeInPlace();
	float v24 = (1.0f - scaleddrag) * normalized;
	Vector vecVelToSet;
	if (v24 < 0.0f)
	{
		vecVelToSet = Vector(0.0f);
	}
	else
	{
		float fmin = fminf(v24, normalized);
		vecVelToSet = pPoint->m_vecVelocity * fmin;
	}

	vecVelToSet.z += gravityscaled;
	pPoint->m_vecVelocity = vecVelToSet;
	ModifyAdditionalMovementInfo(pPoint, scale);
	pPoint->m_vecEndPos = pPoint->m_vecSpartPos;
	pPoint->m_vecSpartPos = vecPos;
	return true;
}

// 99% ; Scale doesn't show up in disassembly?
bool CTFPointManager::OnPointHitWall(tf_point_t *pPoint, Vector &vecPos, Vector &vecAdditionalVel, CGameTrace const &tr, float scale)
{
	float radius = GetRadius(pPoint);
	vecPos = tr.normal * radius + tr.endpos;
	vecAdditionalVel = vec3_origin;
	return false;
}

// 100%
void CTFPointManager::ModifyAdditionalMovementInfo(tf_point_t *pPoint, float scale)
{

}

// 100%
float CTFPointManager::GetInitialSpeed(void)
{
	return 0.0f;
}

// 100%
float CTFPointManager::GetLifetime(void)
{
	return 0.0f;
}

// 100%
float CTFPointManager::GetGravity(void)
{
	return 0.0f;
}

// 100%
float CTFPointManager::GetDrag(void)
{
	return 0.0f;
}

// 100%
Vector CTFPointManager::GetAdditionalVelocity(tf_point_t const *point)
{
	return vec3_origin;
}

// 100%
float CTFPointManager::GetRadius(tf_point_t const * pPoint)
{
	return 0.0f;
}

// 100%
int CTFPointManager::GetMaxPoints(void)
{
	return 0;
}

// 100%
bool CTFPointManager::ShouldIgnoreStartSolid(void)
{
	return false;
}


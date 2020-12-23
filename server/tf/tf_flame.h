// Definitely not a Valve header

#ifndef TF_FLAME
#define TF_FLAME
#ifdef _WIN32
#pragma once
#endif

#include "tf_point_manager.h"
#include "tf_weapon_flamethrower.h"

// %REVERSED 12/22/2020
class ITFFlameManager
{
public:
	ITFlameManager(bool idk)
	{
		if (idk)
		{
			m_ITFFlameManagerAutoList.AddToTail(this);
		}
	}
	virtual ~ITFFlameManager()
	{
		m_ITFFlameManagerAutoList.FindAndFastRemove(this);
	}
public:
	static CUtlVector< ITFFlameManager* > m_ITFFlameManagerAutoList;
}

struct burned_entity_t
{
	int field_0;
	int field_4;
	int field_8;
	int field_C;
	float field_10;
}

struct flame_point_t : public tf_point_t
{
	virtual ~flame_point_t() {}
	Vector m_vecAttackerVel;
	Vector m_vecAttackerPos;
}

class CTFFlameManager : public CTFPointManager, public ITFFlameManager
{
public:
	CTFFlameManager(void);
	virtual ~CTFFlameManager();
	
	virtual float 		GetInitialSpeed(void);
	virtual float 		GetGravity(void);
	virtual float 		GetDrag(void);
	virtual void 		UpdateOnRemove(void);
	virtual void 		InitializePoint(tf_point_t *point,int index);
	virtual Vector 		GetInitialVelocity(void);
	virtual void 		AddPoint(int index);
	virtual Vector 		GetInitialPosition(void);
	virtual void 		ModifyAdditionalMovementInfo(tf_point_t *point,float f);
	virtual bool 		OnPointHitWall(tf_point_t *point,Vector &vecPos,Vector &vecAVel,CGameTrace const&tr,float gravity);
	virtual Vector 		GetAdditionalVelocity(tf_point_t const*tr);
	void 				StartFiring(void);
	void 				StopFiring(void);
	virtual void 		Update(void);
	void 				UpdateDamage(int i,float f1,float f2,bool b);
	void 				SetHitTarget(void);
	bool 				IsValidBurnTarget(CBaseEntity *pEntity);
	virtual bool 		ShouldCollide(CBaseEntity *pEntity);
	float  				GetFlameSizeMult(tf_point_t const*point);
	virtual float 		GetRadius(tf_point_t const*point);
	float 				GetStartSizeMult(void);
	float 				GetEndSizeMult(void);
	bool 				ShouldIgnorePlayerVelocity(void);
	float 				ReflectionAdditionalLifeTime(void);
	float 				ReflectionDamageReduction(void);
	int 				GetMaxFlameReflectionCount(void);
	void 				HookAttributes(void);
	float 				GetFlameDamageScale(tf_point_t const*point,CTFPlayer *pPlayer);
	bool 				BCanBurnEntityThisFrame(CBaseEntity *pEntity);
	void 				PostEntityThink(void);
	virtual void 		OnCollide(CBaseEntity *pEntity,int index);
	virtual float 		GetLifeTime(void);
	virtual int 		GetMaxPoints(void);
	virtual void 		AllocatePoint(void);
	static CTFFlameManager* Create(CBaseEntity *pEntity,bool b);

public:
//	int field_494;		// P sure this is the inherited ITFFlameManager vtable
	ITFFlameManager* m_FlameManager;
	CUtlMap<CHandle<CBaseEntity>, burned_entity_t> m_BurnedEntities;
	int field_4B8;
	float field_4BC;
	float field_4C0;
	bool field_4C4;
	CNetworkVar(CHandle< CTFFlameThrower >, m_hWeapon);
	CNetworkVar(CHandle< CBaseEntity >, m_hAttacker);
	CNetworkVar(float, m_flSpreadDegree);
	CNetworkVar(float, m_flRedirectedFlameSizeMult);
	CNetworkVar(float, m_flFlameStartSizeMult);
	CNetworkVar(float, m_flFlameEndSizeMult);
	CNetworkVar(float, m_flFlameIgnorePlayerVelocity);
	CNetworkVar(float, m_flFlameReflectionAdditionalLifeTime);
	CNetworkVar(float, m_flFlameReflectionDamageReduction);
	CNetworkVar(int, m_iMaxFlameReflectionCount);
	CNetworkVar(int, m_nShouldReflect);
	CNetworkVar(float, m_flFlameSpeed);
	CNetworkVar(float, m_flFlameLifeTime);
	CNetworkVar(float, m_flRandomLifeTimeOffset);
	CNetworkVar(float, m_flFlameGravity);
	CNetworkVar(float, m_flFlameDrag);
	CNetworkVar(float, m_flFlameUp);
	CNetworkVar(bool, m_bIsFiring);
};

#endif 	// TF_FLAME
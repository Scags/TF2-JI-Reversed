// Definitely not a Valve header

#ifndef TF_POINT_MANAGER
#define TF_POINT_MANAGER
#ifdef _WIN32
#pragma once
#endif

#include "baseentity.h"
#include "random.h"
#include "vector.h"

// %REVERSED 12/21/2020
struct tf_point_t
{
	tf_point_t()
	{
		m_vecStartPos = vec3_origin;
		m_vecVelocity = vec3_origin;
		m_flSpawnTime = 0.0f;
		m_flLifeTime = 0.0f;
		m_iIndex = 0;
		m_nTouches = 0;
		m_vecEndPos = vec3_origin;
	}
	virtual ~tf_point_t(){}
	Vector m_vecStartPos;
	Vector m_vecVelocity;	// Velocity
	float m_flSpawnTime;		// curtime
	float m_flLifeTime;		// Lifetime
	unsigned int m_iIndex;	// Index
	int m_nTouches;			// Collision count
	Vector m_vecEndPos;		// Sweeps from m_vecStartPos to m_vecEndPos
};

class CTFPointManager : public CBaseEntity
{
public:
	DECLARE_CLASS(CTFPointManager, CBaseEntity);
	DECLARE_NETWORKCLASS();
	DECLARE_DATADESC();

	CTFPointManager();

	virtual unsigned int PhysicsSolidMaskForEntity(void) const;
	void PointThink(void);
	virtual int UpdateTransmitState();
	virtual void Touch(CBaseEntity *pOther);
	virtual void Spawn(void);
	void RemovePoint(int);
	virtual void UpdateOnRemove(void);
	void ClearPoints(void);		// CODE XREF: CTFPlayer::RemoveOwnedProjectiles(void)
	tf_point_t* AddPointInternal(unsigned int index);
	virtual ~CTFPointManager();

	virtual bool 		AddPoint(int index);
	virtual void 		InitializePoint(tf_point_t * point,int i);
	virtual Vector 		GetInitialPosition(void);
	virtual Vector 		GetInitialVelocity(void);
	virtual bool 		ShouldCollide(CBaseEntity * pEntity);
	virtual void 		OnCollide(CBaseEntity * pEntity, int i);
	virtual void 		AllocatePoint(void);
	virtual void 		Update(void);
	virtual bool 		UpdatePoint(tf_point_t *point,int i,float f,Vector *v1 = NULL,Vector *v2 = NULL,Vector *v3 = NULL);
	virtual bool 		OnPointHitWall(tf_point_t *point,Vector &v1,Vector &v2,CGameTrace const&tr,float f);
	virtual void 		ModifyAdditionalMovementInfo(tf_point_t *point,float f);
	virtual float 		GetInitialSpeed(void);
	virtual float 		GetLifeTime(void);
	virtual float 		GetGravity(void);
	virtual float 		GetDrag(void);
	virtual Vector 		GetAdditionalVelocity(tf_point_t const*point);
	virtual float 		GetRadius(tf_point_t const*point);
	virtual int			GetMaxPoints(void);
	virtual bool 		ShouldIgnoreStartSolid(void);

public:
	CUniformRandomStream m_Randomizer;
	// Hope there's nothing else between these, fuuuck that
	CNetworkVar(int, m_nRandomSeed);
	CNetworkArray(int, m_nSpawnTime, 30);
	CNetworkVar(unsigned int, m_unNextPointIndex);
	float m_flThinkTime;
	CUtlVector<tf_point_t*> m_Points;
	void* field_494;	// Ptr to m_Points.m_Memory? This could be the UtlVector debug pointer but I'll leave it here to be explicit
};


#endif	// TF_POINT_MANAGER
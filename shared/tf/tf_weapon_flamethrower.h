#ifndef TF_WEAPON_FLAMETHROWER_H
#define TF_WEAPON_FLAMETHROWER_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_weaponbase_gun.h"
#include "tf_weaponbase_rocket.h"

#include "tf_projectile_rocket.h"
#include "baseentity.h"
#include "iscorer.h"

// %REVERSED 12/24/2020
class CTFFlameThrower : public CTFWeaponBaseGun, public CGameEventListener
{
public:

public:
	int field_7F8;
	CNetworkVar(EHANDLE, m_hFlameManager);
	CNetworkVar(bool, m_bHasHalloweenSpell);

	
}
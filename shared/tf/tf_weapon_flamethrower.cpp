// Definitely not a Valve header

#include "cbase.h"
#include "tf_weapon_flamethrower.h"
#include "tf_fx_shared.h"
#include "in_buttons.h"
#include "ammodef.h"
#include "tf_gamerules.h"
#include "explode.h"
#include "tf_player.h"
#include "tf_gamestats.h"
#include "ilagcompensationmanager.h"
#include "collisionutils.h"
#include "tf_team.h"
#include "tf_obj.h"
#include "tf_weapon_grenade_pipebomb.h"
#include "particle_parse.h"
#include "tf_weaponbase_grenadeproj.h"
#include "tf_weapon_compound_bow.h"
#include "tf_projectile_arrow.h"
#include "tf_gamestats.h"
#include "NextBot/NextBotManager.h"
#include "halloween/merasmus/merasmus_trick_or_treat_prop.h"
#include "tf_logic_robot_destruction.h"

// I'm just going to include files until my intellisense works 

// %REVERSED 12/24/2020

// So I have no idea how legal this is so I'll try to not use variable names that are from the leak
// So get ready for a lot of field_* member vars

// 100%
CTFFlameThrower::CTFFlameThrower()
{
	field_7e0 = 0;
	m_iWeaponState = 0;
	m_bCritFire = false;
	m_bHitTarget = false;
	field_7ec = 0;
	field_7f0 = 0;
	m_flChargeBeginTime = 0.0f;
	field_7F8 = 0;
	m_hFlameManager = (EHANDLE)-1;
	m_bHasHalloweenSpell = 0;
	WeaponReset();
	field_828 = 0;
	field_810 = 0;
	field_8A8 = 0;
	field_928 = 0;
	field_9A8 = 0;
	field_7e0 = 1;
	ListenForGameEvent("recalculate_holidays");
}

// 100%
CTFFlameThrower::~CTFFlameThrower()
{
	StopListeningForAllEvents();
}

// 100%
bool CTFFlameThrower::CanAirBlastDeflectProjectile(void)
{
	if (!GetTFPlayerOwner())
		return false;

	if (!CanAirBlast())
		return false;

	int deflectdisabled;
	CALL_ATTRIB_HOOK_INT(deflectdisabled, airblast_deflect_projectiles_disabled);
	return !!deflectdisabled;
}

// 100%
bool CTFFlameThrower::CanAirBlastPushPlayer(void)
{
	if (!GetTFPlayerOwner())
		return false;

	if (!CanAirBlast())
		return false;

	int deflectdisabled;
	CALL_ATTRIB_HOOK_INT(deflectdisabled, airblast_pushback_disabled);
	return !!deflectdisabled;
}

// 100%
bool CTFFlameThrower::CanAirBlastPutOutTeammate(void)
{
	if (!GetTFPlayerOwner())
		return false;

	if (!CanAirBlast())
		return false;

	int deflectdisabled;
	CALL_ATTRIB_HOOK_INT(deflectdisabled, airblast_put_out_teammate_disabled);
	return !!deflectdisabled;
}

// REVERSED 12/29/2020

//tf_airblast_cray                         : 1        : , "sv", "cheat"  : Use alternate cray airblast logic globally.
//tf_airblast_cray_debug                   : 0        : , "sv", "cheat"  : Enable debugging overlays & output for cray airblast.  Value is length of time to show debug overlays in seconds.
//tf_airblast_cray_ground_minz             : 100      : , "sv", "cheat"  : If set, cray airblast ensures the target has this minimum Z velocity after reflections and impulse have been applied. Set to 268.3281572999747 for exact old airblast Z behavior.
//tf_airblast_cray_ground_reflect          : 1        : , "sv", "cheat"  : If set, cray airblast reflects any airblast power directed into the ground off of it, to prevent ground-stuck and provide a bit more control over up-vs-forward vectoring
//tf_airblast_cray_lose_footing_duration   : 0        : , "sv", "cheat"  : How long the player should be unable to regain their footing after being airblast, separate from air-control stun.
//tf_airblast_cray_pitch_control           : 0        : , "sv", "cheat"  : If set, allow controlling the pitch of the airblast, in addition to the yaw.
//tf_airblast_cray_power                   : 600      : , "sv", "cheat"  : Amount of force cray airblast should apply unconditionally. Set to 0 to only perform player momentum reflection.
//tf_airblast_cray_power_relative          : 0        : , "sv", "cheat"  : If set, the blast power power also inherits from the blast's forward momentum.
//tf_airblast_cray_reflect_coeff           : 2        : , "sv", "cheat"  : The coefficient of reflective power cray airblast employs.  0   - No reflective powers  0-1 - Cancel out some/all incoming velocity  1-2 - Reflect some/all incoming velocity outwards  2+  - Reflect incoming velocity outwards and then some 
//tf_airblast_cray_reflect_cost_coeff      : 0        : , "sv", "cheat"  : What portion of power used for reflection is removed from the push effect. Note that reflecting incoming momentum requires 2x the momentum - to first neutralize and then reverse it.  Setting this to 1 means that a target running towards the blast at more than 50% blast-speed would have a net pushback half that of a stationary target, since half the power was used to negate their incoming momentum. A value of 0.5 would mean that running towards the blast would not be beneficial vs being still, while values 
//tf_airblast_cray_reflect_relative        : 0        : , "sv", "cheat"  : If set, the relative, rather than absolute, target velocity is considered for reflection.
//tf_airblast_cray_stun_amount             : 0        : , "sv", "cheat"  : Amount of control loss to apply if stun_duration is set.
//tf_airblast_cray_stun_duration           : 0        : , "sv", "cheat"  : If set, apply this duration of stun when initially hit by an airblast.  Does not apply to repeated airblasts.

// 80% ; Jesus Christ
void CTFFlameThrower::ComputeCrayAirBlastForce(CTFPlayer *pAirblasted, CTFPlayer *pAirblaster, Vector &vec_in, Vector &vec_out)
{
	float flCrayPower = tf_airblast_cray_power.GetFloat();
	CALL_ATTRIB_HOOK_FLOAT(flCrayPower, airblast_pushback_scale);
	CALL_ATTRIB_HOOK_FLOAT_ON_OTHER(pAirblasted, flCrayPower, airblast_vulnerability_multiplier);

	float flVCrayPower = 1.0f;
	CALL_ATTRIB_HOOK_FLOAT(flVCrayPower, airblast_vertical_pushback_scale);
	CALL_ATTRIB_HOOK_FLOAT_ON_OTHER(pAirblasted, flVCrayPower, airblast_vertical_vulnerability_multiplier);

	Vector vecPos = pAirblasted->GetAbsPosition();
	Vector vecVel = pAirblasted->GetAbsVelocity();

	float flVelocity;
	Vector vecAttackerVel = pAirblaster->GetAbsVelocity();

	if (!tf_airblast_cray_power_relative.GetBool() && !tf_airblast_cray_reflect_relative.GetBool())
	{
		flVelocity = -1.0f;
	}
	else
	{
		flVelocity = (vecAttackerVel + vec_in).LengthSqr();
	}

	if (tf_airblast_cray_debug.GetFloat() > 0.0f)
	{
		Vector vecMuzzle = GetMuzzlePosHelper();
		vecMuzzle.z += 20.0f;
		NDebugOverlay::Text(vecMuzzle, "Airblast Aim", true, tf_airblast_cray_debug.GetFloat());
		Vector vecEnd = (5.0f * vec_in) + vecMuzzle;
		NDebugOverlay::HorzArrow(vecMuzzle, vecEnd, 2.0f, 100, 0, 0, 255, true, tf_airblast_cray_debug.GetFloat());
	}

	float flV1Length = (vecVel * vec_in).LengthSqr();
	float flCoeff = tf_airblast_cray_reflect_coeff.GetFloat();
	float flRooted;
	if (tf_airblast_cray_reflect_relative.GetBool())
		flV1Length -= flVelocity;

	Vector vecOut;
	Vector vecCenter = pAirblasted->WorldSpaceCenter();
	float flCost;

	if (flV1Length < 0.0f)
	{
		float flReflectCoeff = -flCoeff * flV1Length;
		vecOut = flReflectCoeff * vec_in;
		if (tf_airblast_cray_debug.GetFloat() > 0.0f)
		{
			Vector vecArrow = (vec_in * flV1Length * 0.5f) + vecCenter;
			NDebugOverlay::HorzArrow(vecCenter, vecArrow, 2.0f, 100, 0, 0, 255, false, tf_airblast_cray_debug.GetFloat());
			vecArrow = (((vec_in * flReflectCoeff) + (flV1.Length * vec_in)) * 0.5f) + vecCenter;
			NDebugOverlay::HorzArrow(vecCenter, vecArrow, 2.0f, 0, 200, 0, 255, true, tf_airblast_cray_debug.GetFloat());
			vecArrow = (vec_in * flReflectCoeff * 0.5f) + vecCenter; 
			NDebugOverlay::HorzArrow(vecCenter, vecArrow, 2.0f, 150, 150, 150, 255, true, tf_airblast_cray_debug.GetFloat());

			CFmtStrN<256> s("Reflected player off blast ( into-blast momentum %f )", -flV1Length);
			NDebugOverlay::Text(vecCenter, s.Get(), true, tf_airblast_cray_debug.GetFloat());
		}

		flRooted = FastSqrt(flV1Length);
		flCost = tf_airblast_cray_reflect_cost_coeff.GetFloat() * flRooted;
	}
	else
	{
		flCost = 0.0f;
		flRooted = 0.0f;
		vecOut = Vector(0.0f);
	}

	if (!tf_airblast_cray_power_relative.GetBool())
		flVelocity = 0.0f;

	float flDiff = flVelocity + flCrayPower - flCost;
	if (flDiff > 0.0f)
	{
		Vector vecDiff = vec_in * flDiff;
		vecOut += vecDiff;
		if (tf_airblast_cray_debug.GetFloat() > 0.0f)
		{
			Vector vecMuzzle = GetMuzzlePosHelper();
			Vector vecArrow = vecMuzzle * 0.5f + vecDiff;
			NDebugOverlay::HorzArrow(vecMuzzle, vecArrow, 2.0f, 0, 0, 200, 255, 1, tf_airblast_cray_debug.GetFloat());
			CFmtStrN<256> s("Remaining power after reflection ( %f power - %f reflection * %f cost coeff )", flVelocity + flCrayPower, flDiff, flCost);
			NDebugOverlay::Text(vecArrow, s.Get(), false, tf_airblast_cray_debug.GetFloat());
		}
	}

	CBaseEntity *pGroundEnt = pAirblasted->GetGroundEntity();
	if (pGroundEnt)
	{
		if (tf_airblast_cray_ground_reflect.GetBool())
		{
			trace_t tr;
			UTIL_TraceHull(vecPos, vecPos, pAirblasted->GetPlayerMins(), pAirblasted->GetPlayerMaxs(), 0x201400B, pAirblasted, COLLISION_GROUP_PLAYER_MOVEMENT, &tr);

			if (tr.DidHit())
			{
				Vector vecMove = vecOut + vecVel;
				float flMove = vecMove.LengthSqr();
				if (flMove < 0.0f)
				{
					vecOut += vecMove * -2.0f * flMove;
					// Ugh
//					if (tf_airblast_cray_debug.GetFloat() > 0.0)
//					{
//						v72 = v20;
//						v99 = v96;
//						v102.x = (0.5 * v38) + v96;
//						v100 = v97;
//						v101 = v98;
//						v102.y = (0.5 * v79) + v97;
//						v102.z = (0.5 * v85) + v98;
//						NDebugOverlay::HorzArrow(&v99, &v102, 0x40000000, &dword_C8, 0, 200, 255, 1, SLOBYTE(tf_airblast_cray_debug.GetFloat()), v63);
//						v102.x = ((v40 + v38) * 0.5) + v99;
//						v102.y = ((v41 + v79) * 0.5) + v100;
//						v102.z = ((v42 + v85) * 0.5) + v101;
//						NDebugOverlay::HorzArrow(&v99, &v102, 0x40000000, &dword_C8, 200, 0, 255, 1, SLOBYTE(tf_airblast_cray_debug.GetFloat()), v65);
//						v54 = -v39;
//						CFmtStrN<256, false>::CFmtStrN(v125, "Reflected player off ground ( into-ground momentum %f )", SLOBYTE(v54));
//						NDebugOverlay::Text(&v99, v126, 0, SLOBYTE(tf_airblast_cray_debug.GetFloat()), v61);
//						v20 = v72;
//					}
				}
			}
		}
		if (tf_airblast_cray_ground_minz.GetFloat() > 0.0f)
		{
			Vector vecGo = Vector(0.0f);
			float flMomentum = tf_airblast_cray_ground_minz.GetFloat() - vecVel.z - vecOut.z;
			if (flMomentum > 0.0f)
			{
				// Wtf
				float fl2dlensq = vecOut.Length2DSqr();
				float fl2dlen = FastSqrt(fl2dlensq);
				float fl3dlensq = (vecOut.z * vecOut.z) + fl2dlensq;
				float min = fminf(flMomentum, fl2dlen);
				float minsq = min * min;
				if (minsq > fl3dlensq)
				{
					vecGo.z = FastSqrt(fl3dlensq);
				}
				else
				{
					float zmin = (vecOut.z + vecOut.z) * min;
					float sqrd = FastSqrt(fmaxf((fl2dlensq - minsq) - zmin, 0.0f)) / fl2dlen;

					vecGo.x = vecOut.x * sqrd;
					vecGo.y = vecOut.y * sqrd;
					vecGo.z = min + vecOut.z;
				}

//				if (tf_airblast_cray_debug.GetFloat() > 0.0)
//				{
//					v115.x = ((v35 - v95) * 0.5) + v96;
//					v115.y = ((v34 - v92) * 0.5) + v97;
//					v115.z = ((v32 - vecOut.z) * 0.5) + v98;
//					NDebugOverlay::HorzArrow(&v96, &v115, 0x40000000, &word_32, 50, 50, 255, 1, SLOBYTE(tf_airblast_cray_debug.GetFloat()), v63);
//					v55 = v13;
//					CFmtStrN<256, false>::CFmtStrN(v125, "Applied redirect to maintain minimum Z ( %f -> %f )", SLOBYTE(v55));
//					NDebugOverlay::Text(
//						&v115,
//						v126,
//						0,
//						SLOBYTE(tf_airblast_cray_debug.GetFloat()),
//						COERCE_FLOAT(COERCE_UNSIGNED_INT64(_mm_unpacklo_ps(LODWORD(v32), LODWORD(v32)).m128_f32[0])));
//				}
				vecOut = vecGo;
			}
		}
	}

	if (flVCrayPower != 1.0f)
		vecOut.z *= flVCrayPower;
//	if (tf_airblast_cray_debug.GetFloat() > 0.0)
//	{
//		v103.z = v98;
//		v115.x = v96 + 20.0;
//		v115.y = v97;
//		v115.z = (fl_z - v13) + v98;
//		v73 = v115.z;
//		v103.x = v96 + 20.0;
//		v75 = v96 + 20.0;
//		v103.y = v97;
//		v77 = v97;
//		v89 = flCrayPower * v13;
//		NDebugOverlay::HorzArrow(&v103, &v115, 0x40000000, &word_32, 90, 90, 255, 1, SLOBYTE(tf_airblast_cray_debug.GetFloat()), v63);
//		v56 = v13;
//		CFmtStrN<256, false>::CFmtStrN(
//			v125,
//			"Airblast vertical push multiplier from attributes/vulnerabilities ( %f -> %f )",
//			SLOBYTE(v56));
//		v115.x = v75;
//		v115.y = v77;
//		v115.z = v73;
//		NDebugOverlay::Text(
//			&v115,
//			v126,
//			0,
//			SLOBYTE(tf_airblast_cray_debug.GetFloat()),
//			COERCE_FLOAT(COERCE_UNSIGNED_INT64(_mm_unpacklo_ps(LODWORD(v89), LODWORD(v89)).m128_f32[0])));
//		vec_out->x = v95;
//		vec_out->y = v92;
//		vec_out->z = fl_z;
//		CTFFlameThrower::GetMuzzlePosHelper(&v103, this);
//		v115.x = (v95 * 0.5) + v103.x;
//		v115.y = v103.y + (v92 * 0.5);
//		v115.z = v103.z + (v89 * 0.5);
//		NDebugOverlay::HorzArrow(&v103, &v115, 0x40400000, &off_FC[3], 255, 255, 150, 1, SLOBYTE(tf_airblast_cray_debug.GetFloat()), v63);
//		v52 = FastSqrt(((v92 * v92) + (v95 * v95)) + (v89 * v89));
//		CFmtStrN<256, false>::CFmtStrN(&v123, "Applied Impulse: %f", SLOBYTE(v52));
//		v115.x = v103.x;
//		v115.y = v103.y;
//		v115.z = v103.z - 2.0;
//		NDebugOverlay::Text(&v115, v124, &dword_0 + 1, SLOBYTE(tf_airblast_cray_debug.GetFloat()), v59);
//		v93 = v92 + v82;
//		v103.y = v103.y - 20.0;
//		v115.x = ((v95 + v81) * 0.5) + v103.x;
//		v115.y = (v93 * 0.5) + v103.y;
//		v115.z = (0.5 * (v89 + v87)) + v103.z;
//		NDebugOverlay::HorzArrow(&v103, &v115, 0x40400000, &word_32, 50, 50, 200, 1, SLOBYTE(tf_airblast_cray_debug.GetFloat()), v64);
//		v53 = FastSqrt(((v93 * v93) + ((v95 + v81) * (v95 + v81))) + ((v89 + v87) * (v89 + v87)));
//		CFmtStrN<256, false>::CFmtStrN(v125, "Expected result speed: %f", SLOBYTE(v53));
//		v115.x = v103.x;
//		v115.y = v103.y;
//		v115.z = v103.z - 2.0;
//		NDebugOverlay::Text(&v115, v126, &dword_0 + 1, SLOBYTE(tf_airblast_cray_debug.GetFloat()), v60);
//		return __readgsdword(0x14u) ^ v127;
//	}
	vec_out = vecOut;
}

// END REVERSE 12/29/2020

// %REVERSED 1/4/2021

// 95%
bool CTFFlameThrower::DeflectPlayer(CTFPlayer *pAirblasted, CTFPlayer *pAirblaster, Vector &vecFwd)
{
	if (pAirblasted->GetTeamNumber() != pAirblasted->GetTeamNumber() || pAirblasted == pAirblaster)
	{
		if (!CanAirBlastPushPlayer())
			return false;

		if (pAirblasted->m_Shared.IsImmuneToPushback())
			return false;

		int reverseairblast;
		CALL_ATTRIB_HOOK_INT(reverseairblast, reverse_airblast);

		Vector vecGo;
		if (pAirblasted == pAirblaster)
		{
			vecGo = vecFwd;
		}
		else
		{
			vecGo = (pAirblaster->WorldSpaceCenter() - pAirblasted->WorldSpaceCenter()).Normalized();
		}

		float conescale = 1.0f;
		CALL_ATTRIB_HOOK_FLOAT(conescale, mult_airblast_cone_scale);
		conescale *= 35.0f;

		truncatedcone_t cone;
		cone.origin = pAirblaster->EyePosition();
		cone.normal = vecFwd;
		cone.h = GetDeflectionRadius() * 2.0f;	// TODO;
		cone.theta = conescale;
		if (!physcollision->IsBoxIntersectingCone(pAirblasted->GetAbsOrigin() + pAirblasted->WorldAlignMins(), pAirblasted->GetAbsOrigin() + pAirblasted->WorldAlignMaxs(), cone))
			return false;

		int noviewpunch;
		CALL_ATTRIB_HOOK_INT(noviewpunch, airblast_pushback_no_viewpunch);

		if (pAirblasted != pAirblaster && !noviewpunch)
		{
			pAirblasted->ApplyPunchImpulseX(RandomInt(10, 15));
		}

		pAirblasted->SpeakConceptIfAllowed(MP_CONCEPT_DEFLECTED, "projectile:0,victim:1");	// 68
		if (tf_airblast_cray.GetBool())	// Not sure with this cvar
		{
			Vector vecAng;
			Vector vecImpulse;
			if (tf_airblast_cray_pitch_control.GetBool())
			{
				vecAng = vecFwd;
			}
			else
			{
				QAngle vecFwdAng;
				VectorAngles(vecFwd, vecFwdAng);
				VectorAngles(vecGo, vecImpulse);
				vecFwdAng.y = vecImpulse.y;
				AngleVectors(vecFwdAng, vecAng);
			}

			ComputeCrayAirBlastForce(pAirblasted, pAirblaster, vecAng, vecImpulse);
			pAirblasted->RemoveFlag(FL_ONGROUND);
			pAirblasted->SetGroundEntity(NULL);
			if (!pAirblased->m_Shared.InCond(TF_COND_KNOCKED_INTO_AIR))	// 115
			{
				int nostun;
				CALL_ATTRIB_HOOK_INT(nostun, airblast_pushback_no_stun);
				if (!nostun)
				{
					float stundir = tf_airblast_cray_stun_duration.GetFloat();
					float stunamt = tf_airblast_cray_stun_amount.GetFloat();
					if (stundir > 0.0f && stunamt > 0.0f)
						pAirblasted->m_Shared.StunPlayer(stundir, stunamt, TF_STUN_MOVEMENT, pAirblaster);
				}
			}
			float lostfooting = tf_airblast_cray_lose_footing_duration.GetFloat();
			if (lostfooting != 0.0f)
				pAirblasted->m_Shared.AddCond(TF_COND_LOST_FOOTING, lostfooting);

			pAirblasted->m_Shared.AddCond(TF_COND_AIR_CURRENT, -1.0f);
			pAirblasted->m_Shared.AddCond(TF_COND_KNOCKED_INTO_AIR, -1.0f);
			pAirblasted->ApplyAbsVelocityImpulse(vecImpulse);
		}
		else
		{
			int pushbacknostun;
			CALL_ATTRIB_HOOK_INT(pushbacknostun, airblast_pushback_no_stun);
			if (!pushbacknostun && !pAirblasted->m_Shared.InCond(TF_COND_KNOCKED_INTO_AIR))
				pAirblasted->m_Shared.StunPlayer(0.5f, 1.0f, TF_STUN_MOVEMENT, pAirblaster);

			Vector vecBox = pAirblasted->WorldAlignMaxs() - pAirblasted->WorldAlignMins();
			float flMin = 188928.0f / vecBox.LengthSqr() * 360.0f;
			float flScale = fminf(flMin, 1000.0f);
			CALL_ATTRIB_HOOK_FLOAT(flScale, airblast_pushback_scale);

			Vector vecPush = vecGo * flScale;
			float flVertScale;
			flVertScale = tf_flamethrower_burst_zvelocity.GetFloat();
			if (reverseairblast)
			{
				vecPush = -vecPush;
				flVertScale *= 0.75f;
			}

			float flVertScaleScaled;
			CALL_ATTRIB_HOOK_FLOAT(flVertScaleScaled, airblast_vertical_pushback_scale);

			vecPush.z += flVertScaleScaled;
			pAirblasted->SetAbsVelocity(vec3_origin);
			pAirblasted->ApplyGenericPushbackImpulse(vecPush);
		}

		pAirblasted->field_201c.AddDamagerToHistory(pAirblaster->GetRefEHandle());
		SendObjectDeflectedEvent(pAirblaster, pAirblasted, 0, pAirblasted);
		pAirblasted->m_Shared.InterruptCharge();
		pAirblasted->field_201c.AddPusherToHistory(pAirblaster->GetRefEHandle());
		if (TFGameRules() && (pAirblasted->IsMiniBoss() || pAirblasted->m_Shared.IsInvulnerable()))
		{
			CTF_GameStats.Event_PlayerAwardBonusPoints(pAirblaster, pAirblasted, pAirblasted->IsMiniBoss() ? 10 : 5);
		}
		return true;
	}

	if (pAirblasted->m_Shared.InCond(TF_COND_BURNING) && CanAirBlastPutOutTeammate())
	{
		ExtinguishPlayer(this, pAirblaster, pAirblasted, "tf_weapon_flamethrower");
		int restorehp;
		CALL_ATTRIB_HOOK_INT(restorehp, extinguish_restores_health);
		if (restorehp > 0)
		{
			pAirblasted->TakeHealth((float)restorehp, 0);

			IGameEvent *pEvent = gameeventmanager->CreateEvent("player_healonhit");
			if (pEvent)
			{
				pEvent->SetInt("amount", iRestoreHealthOnExtinguish);
				pEvent->SetInt("entindex", pOwner->entindex());
				item_definition_index_t itemdef = INVALID_ITEM_DEF_INDEX;
				if (GetAttributeContainer() && GetAttributeContainer()->GetItem())
				{
					itemdef = GetAttributeContainer()->GetItem()->GetItemDefIndex();
				}
				pEvent->SetInt("weapon_def_index", itemdef);

				gameeventmanager->FireEvent(pEvent);
			}
		}
		float speedboost;
		CALL_ATTRIB_HOOK_FLOAT(speedboost, airblast_give_teammate_speed_boost);
		if (speedboost > 0.0f)
		{
			pAirblasted->m_Shared.AddCond(TF_COND_SPEED_BOOST, speedboost);
			pAirblaster->m_Shared.AddCond(TF_COND_SPEED_BOOST, speedboost + 1.0f);
		}
	}
	return false;
}
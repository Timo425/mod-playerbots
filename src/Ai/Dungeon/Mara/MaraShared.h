/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_MARASHARED_H
#define PLAYERBOTS_MARASHARED_H

#include "Define.h"
#include "ObjectGuid.h"
#include "Position.h"
#include <vector>

class Player;
class Unit;

namespace Maraudon
{
// Noxious Slime drops it where it stands; some world databases also give it to Creeping Sludge.
constexpr uint32 SPELL_NOXIOUS_CLOUD = 21070;
// Creeping Sludge's aura; its triggered spell hits everyone around the sludge.
constexpr uint32 SPELL_POISON_SHOCK = 22638;

constexpr float NOXIOUS_CLOUD_RADIUS_FALLBACK = 5.0f;
constexpr float POISON_SHOCK_RADIUS_FALLBACK = 5.0f;
constexpr float POISON_SHOCK_MARGIN = 3.0f;
constexpr float POISON_SHOCK_STEP_OUT_DISTANCE = 9.0f;

constexpr float CLOUD_SCAN_RANGE = 40.0f;
constexpr uint32 CLOUD_SCAN_INTERVAL = 1000;
constexpr uint32 RANGED_DPS_CHECK_INTERVAL = 2000;
constexpr uint32 POISON_SHOCK_STEP_COOLDOWN = 750;
constexpr uint32 PET_HOLD_MIN_TIME = 3000;

float NoxiousCloudRadius();

float PoisonShockRadius();

bool IsInNoxiousCloud(Unit* unit, std::vector<Position> const& clouds);

bool HasPoisonShock(Unit* unit);

Unit* GetNearestPoisonShockMob(Player* bot, GuidVector const& attackers, float range);

bool IsSpotNearPoisonShock(Player* bot, GuidVector const& attackers, Position const& spot, float range);

bool IsSpotNearIdleEnemy(Player* bot, GuidVector const& units, Position const& spot);

bool IsHazardTarget(Unit* target, std::vector<Position> const& clouds);

bool ShouldHoldPets(Player* bot, Unit* target, std::vector<Position> const& clouds);
}

#endif

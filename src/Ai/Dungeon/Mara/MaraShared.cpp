/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "MaraShared.h"
#include "Creature.h"
#include "ObjectAccessor.h"
#include "Pet.h"
#include "Player.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include <algorithm>

namespace Maraudon
{
namespace
{
float MaxEffectRadius(SpellInfo const* spellInfo, float fallback)
{
    if (!spellInfo)
        return fallback;

    float radius = 0.0f;
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        radius = std::max(radius, spellInfo->Effects[i].CalcRadius());

    return radius > 0.0f ? radius : fallback;
}
}  // namespace

float NoxiousCloudRadius()
{
    return MaxEffectRadius(sSpellMgr->GetSpellInfo(SPELL_NOXIOUS_CLOUD), NOXIOUS_CLOUD_RADIUS_FALLBACK);
}

float PoisonShockRadius()
{
    SpellInfo const* aura = sSpellMgr->GetSpellInfo(SPELL_POISON_SHOCK);
    if (!aura)
        return POISON_SHOCK_RADIUS_FALLBACK;

    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (uint32 const triggered = aura->Effects[i].TriggerSpell)
            return MaxEffectRadius(sSpellMgr->GetSpellInfo(triggered), POISON_SHOCK_RADIUS_FALLBACK);
    }

    return POISON_SHOCK_RADIUS_FALLBACK;
}

bool IsInNoxiousCloud(Unit* unit, std::vector<Position> const& clouds)
{
    if (!unit || clouds.empty())
        return false;

    float const reach = NoxiousCloudRadius() + unit->GetObjectSize();
    return std::any_of(clouds.begin(), clouds.end(),
                       [&](Position const& cloud) { return unit->GetExactDist(cloud) < reach; });
}

bool HasPoisonShock(Unit* unit)
{
    return unit && unit->IsAlive() && unit->HasAura(SPELL_POISON_SHOCK);
}

Unit* GetNearestPoisonShockMob(Player* bot, GuidVector const& attackers, float range)
{
    Unit* best = nullptr;
    float bestDist = 0.0f;
    for (ObjectGuid guid : attackers)
    {
        Unit* unit = ObjectAccessor::GetUnit(*bot, guid);
        if (!HasPoisonShock(unit))
            continue;

        float const dist = bot->GetExactDist(unit) - bot->GetObjectSize();
        if (dist < range && (!best || dist < bestDist))
        {
            best = unit;
            bestDist = dist;
        }
    }

    return best;
}

bool IsSpotNearPoisonShock(Player* bot, GuidVector const& attackers, Position const& spot, float range)
{
    return std::any_of(attackers.begin(), attackers.end(), [&](ObjectGuid guid)
    {
        Unit* unit = ObjectAccessor::GetUnit(*bot, guid);
        return HasPoisonShock(unit) && unit->GetExactDist(spot) - bot->GetObjectSize() < range;
    });
}

bool IsSpotNearIdleEnemy(Player* bot, GuidVector const& units, Position const& spot)
{
    return std::any_of(units.begin(), units.end(), [&](ObjectGuid guid)
    {
        Unit* unit = ObjectAccessor::GetUnit(*bot, guid);
        Creature* creature = unit ? unit->ToCreature() : nullptr;
        return creature && creature->IsAlive() && !creature->IsInCombat() && creature->IsHostileTo(bot) &&
               creature->GetDistance(spot) <= creature->GetAttackDistance(bot);
    });
}

bool IsHazardTarget(Unit* target, std::vector<Position> const& clouds)
{
    return HasPoisonShock(target) || IsInNoxiousCloud(target, clouds);
}

bool ShouldHoldPets(Player* bot, Unit* target, std::vector<Position> const& clouds)
{
    // Also a pet standing in a cloud of its own accord: called back, it walks out.
    return (target && IsHazardTarget(target, clouds)) || IsInNoxiousCloud(bot->GetPet(), clouds);
}
}

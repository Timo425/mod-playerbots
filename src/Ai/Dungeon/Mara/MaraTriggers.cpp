/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "MaraTriggers.h"
#include "EncounterHelpers.h"
#include "MaraShared.h"
#include "Playerbots.h"
#include "Timer.h"

using namespace Maraudon;

bool MaraudonPoisonShockTrigger::IsActive()
{
    auto const& attackers = AI_VALUE_REF(GuidVector, "attackers");
    if (!GetNearestPoisonShockMob(bot, attackers, PoisonShockRadius() + POISON_SHOCK_MARGIN))
        return false;

    if (!PlayerbotAI::IsMelee(bot))
        return true;

    if (!AI_VALUE(bool, "maraudon ranged dps near"))
        return false;

    // The tank holding another mob beside a sludge takes the pulse rather than leave it.
    Unit* target = AI_VALUE(Unit*, "current target");
    return !PlayerbotAI::IsTank(bot) || !target || HasPoisonShock(target);
}

bool MaraudonPetHazardTrigger::IsActive()
{
    if (!EncounterHelpers::HasControllableMinion(bot))
        return false;

    Unit* target = AI_VALUE(Unit*, "current target");
    if (ShouldHoldPets(bot, target, AI_VALUE_REF(std::vector<Position>, "maraudon noxious clouds")))
        return EncounterHelpers::IsMinionStanceWrongForHold(bot, true);

    uint32 const heldSince = AI_VALUE(uint32, "maraudon pet hold since");
    return heldSince && GetMSTimeDiffToNow(heldSince) >= PET_HOLD_MIN_TIME;
}

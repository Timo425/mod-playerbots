/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "MaraActions.h"
#include "MaraShared.h"
#include "Playerbots.h"
#include "Timer.h"
#include <algorithm>
#include <cmath>

using namespace Maraudon;

bool MaraudonMoveAwayFromPoisonShockAction::Execute(Event /*event*/)
{
    float const radius = PoisonShockRadius();
    auto const& attackers = AI_VALUE_REF(GuidVector, "attackers");
    Unit* sludge = GetNearestPoisonShockMob(bot, attackers, radius + POISON_SHOCK_MARGIN);
    if (!sludge)
        return false;

    if (_lastStep && GetMSTimeDiffToNow(_lastStep) < POISON_SHOCK_STEP_COOLDOWN)
        return false;

    float const distance = radius + POISON_SHOCK_STEP_OUT_DISTANCE;
    float const away = sludge->GetAngle(bot);
    auto const& clouds = AI_VALUE_REF(std::vector<Position>, "maraudon noxious clouds");
    GuidVector const idle = AI_VALUE(GuidVector, "possible targets no los");

    // Straight away from the sludge first, then around it; last, past it to the far side, so a bot
    // with walls behind and beside it isn't pinned.
    for (float offset : { 0.0f, float(M_PI / 4), float(-M_PI / 4), float(M_PI / 2), float(-M_PI / 2),
                          float(3 * M_PI / 4), float(-3 * M_PI / 4), float(M_PI) })
    {
        float x = sludge->GetPositionX() + std::cos(away + offset) * distance;
        float y = sludge->GetPositionY() + std::sin(away + offset) * distance;
        float z = bot->GetPositionZ();
        if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
                                                            bot->GetPositionZ(), x, y, z))
            continue;

        // Clear of every sludge, not only the nearest, and of mobs not yet in the fight.
        Position const spot(x, y, z);
        if (IsSpotNearPoisonShock(bot, attackers, spot, radius + POISON_SHOCK_MARGIN) ||
            IsSpotNearIdleEnemy(bot, idle, spot))
            continue;

        if (std::any_of(clouds.begin(), clouds.end(), [&](Position const& cloud)
                        { return cloud.GetExactDist(spot) < NoxiousCloudRadius() + bot->GetObjectSize(); }))
            continue;

        // Forced: a combat move still running (a reach, a cast's step) refuses one at its own priority.
        if (MoveTo(bot->GetMapId(), x, y, z, false, false, true, false, MovementPriority::MOVEMENT_FORCED))
        {
            _lastStep = getMSTime();
            return true;
        }
    }

    return false;
}

bool MaraudonSetPetStanceAction::Execute(Event /*event*/)
{
    Unit* target = AI_VALUE(Unit*, "current target");
    Value<uint32>* heldSince = context->GetValue<uint32>("maraudon pet hold since");
    if (ShouldHoldPets(bot, target, AI_VALUE_REF(std::vector<Position>, "maraudon noxious clouds")))
    {
        heldSince->Set(getMSTime());
        return EncounterHelpers::SetMinionStancesForHold(bot, true, _previousReactStates);
    }

    // Held at least a few seconds, so a pet at a cloud's edge isn't sent in and called back on every
    // check.
    if (heldSince->Get() && GetMSTimeDiffToNow(heldSince->Get()) < PET_HOLD_MIN_TIME)
        return false;

    heldSince->Set(0);
    return EncounterHelpers::SetMinionStancesForHold(bot, false, _previousReactStates);
}

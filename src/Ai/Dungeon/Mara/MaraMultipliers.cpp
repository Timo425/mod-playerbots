/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "MaraMultipliers.h"
#include "MaraShared.h"
#include "MovementActions.h"
#include "Playerbots.h"
#include "ReachTargetActions.h"

using namespace Maraudon;

float MaraudonHazardApproachMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<ReachMeleeAction*>(action) && !dynamic_cast<CastReachTargetSpellAction*>(action) &&
        !dynamic_cast<SetBehindTargetAction*>(action))
        return 1.0f;

    if (!PlayerbotAI::IsMelee(bot))
        return 1.0f;

    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return 1.0f;

    // Not to a sludge while ranged can kill it. Melee damage dealers don't go to a target in its pulse
    // either; the tank still does, so he keeps hold of the pull.
    if (AI_VALUE(bool, "maraudon ranged dps near"))
    {
        auto const& attackers = AI_VALUE_REF(GuidVector, "attackers");
        float const reach = PoisonShockRadius() + POISON_SHOCK_MARGIN;
        if (PlayerbotAI::IsTank(bot) ? HasPoisonShock(target)
                                     : IsSpotNearPoisonShock(bot, attackers, target->GetPosition(), reach))
            return 0.0f;
    }

    // Only damage dealers that step out of clouds wait for a mob to leave one; the tank keeps
    // holding it.
    bool const avoidsClouds = botAI->HasStrategy("avoid aoe", BOT_STATE_COMBAT) &&
                              !sPlayerbotAIConfig.AoeAvoidSpellWhitelist.count(SPELL_NOXIOUS_CLOUD) &&
                              NoxiousCloudRadius() <= sPlayerbotAIConfig.MaxAoeAvoidRadius;
    if (PlayerbotAI::IsTank(bot) || !avoidsClouds)
        return 1.0f;

    return IsInNoxiousCloud(target, AI_VALUE_REF(std::vector<Position>, "maraudon noxious clouds")) ? 0.0f : 1.0f;
}

/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ReachTargetActions.h"
#include "Event.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "MotionMaster.h"
#include "ServerFacade.h"

namespace
{
// A mob running away (low-health flee, running for help, or feared) is chased only while it stays this
// close to the master: farther out the bot drags in the next pack and leaves the healer's range. A
// runner comes back by itself, with any helpers, to where the group fights.
constexpr float RUNNER_CHASE_RANGE_FROM_MASTER = 20.0f;

bool IsRunningAway(Unit* unit)
{
    switch (unit->GetMotionMaster()->GetCurrentMovementGeneratorType())
    {
        case FLEEING_MOTION_TYPE:
        case TIMED_FLEEING_MOTION_TYPE:
        case ASSISTANCE_MOTION_TYPE:
            return true;
        default:
            return false;
    }
}
}  // namespace

bool ReachTargetAction::Execute(Event /*event*/) { return ReachCombatTo(AI_VALUE(Unit*, GetTargetName()), distance); }

bool ReachTargetAction::isUseful()
{
    // do not move while staying
    if (botAI->HasStrategy("stay", botAI->GetState()))
    {
        return false;
    }

    // do not move while casting
    if (bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL) != nullptr)
    {
        return false;
    }
    Unit* target = GetTarget();
    if (target && IsRunningAway(target))
    {
        Player* master = GetMaster();
        if (master && master != bot && master->IsInWorld() && master->GetMapId() == target->GetMapId() &&
            master->GetDistance(target) > RUNNER_CHASE_RANGE_FROM_MASTER)
            return false;
    }
    // float dis = distance + CONTACT_DISTANCE;
    return target &&
           !bot->IsWithinCombatRange(target, distance);  // ServerFacade::instance().IsDistanceGreaterThan(AI_VALUE2(float,
                                                         // "distance", GetTargetName()), distance);
}

std::string const ReachTargetAction::GetTargetName() { return "current target"; }

bool CastReachTargetSpellAction::isUseful()
{
    // do not move while staying
    if (botAI->HasStrategy("stay", botAI->GetState()))
    {
        return false;
    }

    return ServerFacade::instance().IsDistanceGreaterThan(AI_VALUE2(float, "distance", "current target"),
                                                (distance + sPlayerbotAIConfig.ContactDistance));
}

ReachSpellAction::ReachSpellAction(PlayerbotAI* botAI)
    : ReachTargetAction(botAI, "reach spell", botAI->GetRange("spell"))
{
}

ReachPartyMemberToHealAction::ReachPartyMemberToHealAction(PlayerbotAI* botAI)
    : ReachTargetAction(botAI, "reach party member to heal", botAI->GetRange("heal"))
{
}

std::string const ReachPartyMemberToHealAction::GetTargetName() { return "party member to heal"; }

ReachPartyMemberToResurrectAction::ReachPartyMemberToResurrectAction(PlayerbotAI* botAI)
    : ReachTargetAction(botAI, "reach party member to resurrect", botAI->GetRange("spell"))
{
}

std::string const ReachPartyMemberToResurrectAction::GetTargetName() { return "party member to resurrect"; }

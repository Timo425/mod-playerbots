/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ReachTargetActions.h"
#include "Creature.h"
#include "Event.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "MotionMaster.h"
#include "ServerFacade.h"

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
    if (botAI->IsRunnerOutOfChase(target))
        return false;
    // Local: no walking into a mob's damage aura (PlayerbotAI::GetDamageAuraRadius); a reach that
    // stops outside it (spell range) still goes.
    if (target)
    {
        float const auraRadius = PlayerbotAI::GetDamageAuraRadius(target);
        if (auraRadius > 0.0f && distance + target->GetCombatReach() + bot->GetCombatReach() <= auraRadius + 2.0f)
            return false;
        // Local: nor to melee a target standing in an enemy's damage cloud; it comes out after the tank.
        if (distance <= sPlayerbotAIConfig.MeleeDistance && botAI->IsInHostileDamageCloud(target))
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

    // Local: a gap-closer (Charge, Intercept, Feral Charge) lands in melee, inside a damage aura.
    Unit* target = AI_VALUE(Unit*, "current target");
    if (PlayerbotAI::GetDamageAuraRadius(target) > 0.0f || botAI->IsInHostileDamageCloud(target))
        return false;

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

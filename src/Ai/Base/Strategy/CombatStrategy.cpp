/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "CombatStrategy.h"
#include "Strategy.h"

void CombatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "enemy out of spell",
            {
                NextAction("reach spell", ACTION_HIGH)
            }
        )
    );
    // drop target relevance 99 (lower than Worldpacket triggers)
    triggers.push_back(
        new TriggerNode(
            "invalid target",
            {
                NextAction("drop target", 99)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "mounted",
            {
                NextAction("check mount state", 54)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "combat stuck",
            {
                NextAction("reset", 1.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "not facing target",
            {
                NextAction("set facing", ACTION_MOVE + 7)
            }
        )
    );
    // The pet-attack trigger is commented out because it was forcing the bot's pet to attack, overriding stay and follow commands.
    // Pets will automatically attack the bot's enemy if they are in "defensive" or "aggressive"
    // stance, or if the master issues an attack command.

    // Local: a pet chasing a runner gets the same limit as the bots (PlayerbotAI::IsRunnerOutOfChase).
    // Above "set facing": the recall is instant and should come before the owner's next shot.
    triggers.push_back(
        new TriggerNode(
            "pet chases runner",
            {
                NextAction("recall pet from runner", ACTION_MOVE + 8)
            }
        )
    );
}

AvoidAoeStrategy::AvoidAoeStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

std::vector<NextAction> AvoidAoeStrategy::getDefaultActions()
{
    return {
        NextAction("avoid aoe", ACTION_EMERGENCY)
    };
}

void AvoidAoeStrategy::InitTriggers(std::vector<TriggerNode*>& /*triggers*/)
{
}

void AvoidAoeStrategy::InitMultipliers(std::vector<Multiplier*>& /*multipliers*/)
{
}

TankFaceStrategy::TankFaceStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

std::vector<NextAction> TankFaceStrategy::getDefaultActions()
{
    return {
        NextAction("tank face", ACTION_MOVE)
    };
}

void TankFaceStrategy::InitTriggers(std::vector<TriggerNode*>& /*triggers*/)
{
}

std::vector<NextAction> CombatFormationStrategy::getDefaultActions()
{
    return {
        NextAction("combat formation move", ACTION_NORMAL)
    };
}

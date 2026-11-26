/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "MaraStrategy.h"
#include "MaraMultipliers.h"
#include "MaraShared.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"

void VanillaDungeonMaraudonStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("maraudon poison shock", {
        NextAction("maraudon move away from poison shock", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("maraudon pet hazard", {
        NextAction("maraudon set pet stance", ACTION_EMERGENCY + 1) }));
}

void VanillaDungeonMaraudonStrategy::AppendTargetExclusions(GuidSet& exclusions, TargetValueExclusionType type)
{
    // Melee and the tank leave sludges to the ranged and pick another target, rather than choose one
    // they then keep off.
    if (type != TargetValueExclusionType::TankTarget && type != TargetValueExclusionType::DpsTarget)
        return;

    Player* bot = botAI->GetBot();
    AiObjectContext* context = botAI->GetAiObjectContext();
    if (!PlayerbotAI::IsMelee(bot) || !AI_VALUE(bool, "maraudon ranged dps near"))
        return;

    for (ObjectGuid guid : AI_VALUE_REF(GuidVector, "attackers"))
    {
        if (Maraudon::HasPoisonShock(ObjectAccessor::GetUnit(*bot, guid)))
            exclusions.insert(guid);
    }
}

void VanillaDungeonMaraudonStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new MaraudonHazardApproachMultiplier(botAI));
}

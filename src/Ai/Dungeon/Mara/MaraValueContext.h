/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_MARAVALUECONTEXT_H
#define PLAYERBOTS_MARAVALUECONTEXT_H

#include "EncounterHelpers.h"
#include "Group.h"
#include "MaraShared.h"
#include "NamedObjectContext.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "Value.h"
#include <vector>

class MaraudonNoxiousCloudsValue : public CalculatedValue<std::vector<Position>>
{
public:
    MaraudonNoxiousCloudsValue(PlayerbotAI* botAI)
        : CalculatedValue<std::vector<Position>>(botAI, "maraudon noxious clouds", Maraudon::CLOUD_SCAN_INTERVAL)
    {
    }

protected:
    std::vector<Position> Calculate() override
    {
        if (!bot->IsInCombat())
            return {};

        return EncounterHelpers::GetDynamicObjectPositions(bot, Maraudon::CLOUD_SCAN_RANGE,
                                                           Maraudon::SPELL_NOXIOUS_CLOUD);
    }
};

class MaraudonRangedDpsNearValue : public CalculatedValue<bool>
{
public:
    MaraudonRangedDpsNearValue(PlayerbotAI* botAI)
        : CalculatedValue<bool>(botAI, "maraudon ranged dps near", Maraudon::RANGED_DPS_CHECK_INTERVAL)
    {
    }

protected:
    bool Calculate() override
    {
        Group* group = bot->GetGroup();
        if (!group)
            return PlayerbotAI::IsRangedDps(bot);

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && member->IsInWorld() && member->IsAlive() && member->GetMapId() == bot->GetMapId() &&
                bot->GetDistance(member) <= sPlayerbotAIConfig.SightDistance && PlayerbotAI::IsRangedDps(member))
                return true;
        }

        return false;
    }
};

class MaraudonPetHoldSinceValue : public ManualSetValue<uint32>
{
public:
    MaraudonPetHoldSinceValue(PlayerbotAI* botAI) : ManualSetValue<uint32>(botAI, 0, "maraudon pet hold since") {}
};

class VanillaDungeonMaraudonValueContext : public NamedObjectContext<UntypedValue>
{
public:
    VanillaDungeonMaraudonValueContext()
    {
        creators["maraudon noxious clouds"] = &VanillaDungeonMaraudonValueContext::noxious_clouds;
        creators["maraudon ranged dps near"] = &VanillaDungeonMaraudonValueContext::ranged_dps_near;
        creators["maraudon pet hold since"] = &VanillaDungeonMaraudonValueContext::pet_hold_since;
    }

private:
    static UntypedValue* noxious_clouds(PlayerbotAI* botAI) { return new MaraudonNoxiousCloudsValue(botAI); }

    static UntypedValue* ranged_dps_near(PlayerbotAI* botAI) { return new MaraudonRangedDpsNearValue(botAI); }

    static UntypedValue* pet_hold_since(PlayerbotAI* botAI) { return new MaraudonPetHoldSinceValue(botAI); }
};

#endif

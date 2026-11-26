/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_MARATRIGGERCONTEXT_H
#define PLAYERBOTS_MARATRIGGERCONTEXT_H

#include "MaraTriggers.h"
#include "NamedObjectContext.h"

class VanillaDungeonMaraudonTriggerContext : public NamedObjectContext<Trigger>
{
public:
    VanillaDungeonMaraudonTriggerContext()
    {
        creators["maraudon poison shock"] = &VanillaDungeonMaraudonTriggerContext::poison_shock;
        creators["maraudon pet hazard"] = &VanillaDungeonMaraudonTriggerContext::pet_hazard;
    }

private:
    static Trigger* poison_shock(PlayerbotAI* botAI) { return new MaraudonPoisonShockTrigger(botAI); }

    static Trigger* pet_hazard(PlayerbotAI* botAI) { return new MaraudonPetHazardTrigger(botAI); }
};

#endif

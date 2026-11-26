/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_MARAACTIONCONTEXT_H
#define PLAYERBOTS_MARAACTIONCONTEXT_H

#include "MaraActions.h"
#include "NamedObjectContext.h"

class VanillaDungeonMaraudonActionContext : public NamedObjectContext<Action>
{
public:
    VanillaDungeonMaraudonActionContext()
    {
        creators["maraudon move away from poison shock"] =
            &VanillaDungeonMaraudonActionContext::move_away_from_poison_shock;
        creators["maraudon set pet stance"] = &VanillaDungeonMaraudonActionContext::set_pet_stance;
    }

private:
    static Action* move_away_from_poison_shock(PlayerbotAI* botAI)
    {
        return new MaraudonMoveAwayFromPoisonShockAction(botAI);
    }

    static Action* set_pet_stance(PlayerbotAI* botAI) { return new MaraudonSetPetStanceAction(botAI); }
};

#endif

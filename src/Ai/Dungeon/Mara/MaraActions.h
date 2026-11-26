/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_MARAACTIONS_H
#define PLAYERBOTS_MARAACTIONS_H

#include "EncounterHelpers.h"
#include "MovementActions.h"
#include <vector>

class MaraudonMoveAwayFromPoisonShockAction : public MovementAction
{
public:
    MaraudonMoveAwayFromPoisonShockAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "maraudon move away from poison shock") {}
    bool Execute(Event event) override;

private:
    uint32 _lastStep = 0;
};

class MaraudonSetPetStanceAction : public Action
{
public:
    MaraudonSetPetStanceAction(PlayerbotAI* botAI) : Action(botAI, "maraudon set pet stance") {}
    bool Execute(Event event) override;

private:
    std::vector<EncounterHelpers::SavedMinionStance> _previousReactStates;
};

#endif

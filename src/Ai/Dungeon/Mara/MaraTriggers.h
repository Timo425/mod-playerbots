/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_MARATRIGGERS_H
#define PLAYERBOTS_MARATRIGGERS_H

#include "Trigger.h"

class MaraudonPoisonShockTrigger : public Trigger
{
public:
    MaraudonPoisonShockTrigger(PlayerbotAI* botAI) : Trigger(botAI, "maraudon poison shock") {}
    bool IsActive() override;
};

class MaraudonPetHazardTrigger : public Trigger
{
public:
    MaraudonPetHazardTrigger(PlayerbotAI* botAI) : Trigger(botAI, "maraudon pet hazard") {}
    bool IsActive() override;
};

#endif

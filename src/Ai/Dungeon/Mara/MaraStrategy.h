/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_MARASTRATEGY_H
#define PLAYERBOTS_MARASTRATEGY_H

#include "Strategy.h"

class VanillaDungeonMaraudonStrategy : public Strategy
{
public:
    VanillaDungeonMaraudonStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "maraudon"; }

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;

    bool HasTargetExclusions() const override { return true; }
    void AppendTargetExclusions(GuidSet& exclusions, TargetValueExclusionType type) override;
};

#endif

/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 *
 * Ported from the CMaNGOS playerbots project (https://github.com/cmangos/playerbots), GPL v2,
 * with modifications for AzerothCore.
 * Original authors:
 *   David Parra Ausina (davidonete/Flekz) <davidparraausina@gmail.com> - original author
 *   Sebastiaan Keek (mostlikely4r) <sebastiaan.keek@gmail.com>
 *   Cyberium <cyberium@users.noreply.github.com>
 */

#include "PullActions.h"
#include "AttackersValue.h"
#include "CreatureAI.h"
#include "PlayerbotTextMgr.h"
#include "Playerbots.h"
#include "PositionValue.h"
#include "PullStrategy.h"
#include "RtiTargetValue.h"
#include <algorithm>

namespace
{
// Walk this far inside the pull range. Aiming at the exact edge left the bot standing a hair outside
// it when the target stood lower or higher (the range check is 3D): the shot was refused as out of
// range and the move as already done, so the pull looped until it timed out.
constexpr float PULL_RANGE_MARGIN = 3.0f;

float GetPullReachDistance(Player* bot, Unit* target, PullStrategy const* strategy)
{
    if (!bot || !target || !strategy)
        return 0.0f;

    float const combatDistance = bot->GetCombatReach() + target->GetCombatReach();
    return std::max(0.0f, strategy->GetRange() - combatDistance - PULL_RANGE_MARGIN);
}

bool IsWithinPullRange(Player* bot, Unit* target, PullStrategy const* strategy)
{
    return bot && target && strategy && bot->GetExactDist(target) <= strategy->GetRange();
}

// In range and in sight: the shot can be fired from here.
bool CanPullFromHere(Player* bot, Unit* target, PullStrategy const* strategy)
{
    return IsWithinPullRange(bot, target, strategy) && bot->IsWithinLOSInMap(target);
}

// Each step toward a target that is in range but out of sight.
constexpr float PULL_SIGHT_STEP = 8.0f;
}

bool PullRequestAction::Execute(Event event)
{
    PullStrategy* strategy = PullStrategy::Get(botAI);
    if (!strategy)
        return false;

    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* target = GetPullTarget(event);
    if (!target || !target->IsInWorld())
    {
        std::string const text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "pull_no_target_error", "You have no target", {});
        botAI->TellError(text);
        return false;
    }

    float const maxPullDistance = sPlayerbotAIConfig.ReactDistance * 3.0f;
    if (target->GetMapId() != bot->GetMapId() || bot->GetDistance(target) > maxPullDistance)
    {
        std::string const text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "pull_target_too_far_error", "The target is too far away", {});
        botAI->TellError(text);
        return false;
    }

    if (!AttackersValue::IsPossibleTarget(target, bot))
    {
        std::string const text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "pull_invalid_target_error", "The target can't be pulled", {});
        botAI->TellError(text);
        return false;
    }

    if (!strategy->HasAmmoForShot())
    {
        std::string const text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "pull_no_ammo_error", "Can't pull: no ammo for my ranged weapon", {});
        botAI->TellError(text);
        return false;
    }

    if (!strategy->CanDoPullAction(target))
    {
        std::string const actionName = strategy->GetPullActionName();
        std::string const text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "pull_action_unavailable_error",
            "Can't perform pull action '%action_name'",
            {{"%action_name", actionName}});
        botAI->TellError(text);
        return false;
    }

    // "pull to me": the bot brings the target to where the requester stands (around a corner, so a
    // caster has to come too) instead of to where the bot itself stood.
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    WorldObject* pullTo = bot;
    if (event.getParam() == "to me" && requester && requester->IsInWorld() &&
        requester->GetMapId() == bot->GetMapId() && bot->GetDistance(requester) <= maxPullDistance)
        pullTo = requester;

    PositionMap& posMap = AI_VALUE(PositionMap&, "position");
    PositionInfo pullPosition = posMap["pull"];
    pullPosition.Set(pullTo->GetPositionX(), pullTo->GetPositionY(), pullTo->GetPositionZ(), bot->GetMapId());
    posMap["pull"] = pullPosition;

    strategy->RequestPull(target);
    context->GetValue<Unit*>("current target")->Set(target);
    botAI->ChangeEngine(BOT_STATE_COMBAT);
    botAI->SetNextCheckDelay(sPlayerbotAIConfig.ReactDelay);
    return true;
}

Unit* PullMyTargetAction::GetPullTarget(Event event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    if (event.GetSource() == "attack anything")
        return botAI->GetCreature(event.getObject());

    return requester ? requester->GetSelectedUnit() : nullptr;
}

Unit* PullRtiTargetAction::GetPullTarget(Event /*event*/)
{
    Unit* rtiTarget = AI_VALUE(Unit*, "rti target");
    if (rtiTarget)
        return rtiTarget;

    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    std::string const rti = AI_VALUE(std::string, "rti");
    int32 const index = RtiTargetValue::GetRtiIndex(rti);
    if (index < 0)
        return nullptr;

    ObjectGuid const guid = group->GetTargetIcon(index);
    return guid.IsEmpty() ? nullptr : botAI->GetUnit(guid);
}

bool PullStartAction::Execute(Event event)
{
    PullStrategy* strategy = PullStrategy::Get(botAI);
    if (!strategy)
        return false;

    Unit* target = strategy->GetTarget();
    if (!target)
        return false;

    std::string const preActionName = strategy->GetPreActionName();
    if (!preActionName.empty() && !botAI->DoSpecificAction(preActionName, event, true))
        return false;

    if (Pet* pet = bot->GetPet())
    {
        Creature* creature = pet->ToCreature();
        if (creature)
        {
            strategy->SetPetReactState(creature->GetReactState());
            creature->SetReactState(REACT_PASSIVE);
        }
    }

    strategy->OnPullStarted();
    return true;
}

PullAction::PullAction(PlayerbotAI* botAI, std::string const name) : CastSpellAction(botAI, name) { InitPullAction(); }

Unit* PullAction::GetTarget()
{
    PullStrategy* strategy = PullStrategy::Get(botAI);
    if (!strategy)
        return nullptr;

    return strategy->GetTarget();
}

std::vector<NextAction> PullAction::getPrerequisites()
{
    PullStrategy* strategy = PullStrategy::Get(botAI);
    Unit* target = strategy ? strategy->GetTarget() : nullptr;
    if (!strategy || !target)
        return {};

    return CanPullFromHere(bot, target, strategy) ? std::vector<NextAction>{}
                                                   : std::vector<NextAction>{ NextAction("reach pull", ACTION_MOVE) };
}

bool PullAction::Execute(Event event)
{
    InitPullAction();

    PullStrategy* strategy = PullStrategy::Get(botAI);
    if (!strategy)
        return false;

    Unit* target = strategy->GetTarget();
    if (!target || !target->IsInWorld())
        return false;

    if (target->IsInCombat())
        return false;

    if (!CanPullFromHere(bot, target, strategy))
    {
        strategy->RequestPull(target, false);
        return false;
    }

    if (bot->isMoving())
    {
        bot->StopMoving();
        strategy->RequestPull(target, false);
        return false;
    }

    context->GetValue<Unit*>("current target")->Set(target);
    if (!botAI->DoSpecificAction(strategy->GetPullActionName(), event, true))
        return false;

    return true;
}

bool PullAction::isPossible()
{
    InitPullAction();

    PullStrategy* strategy = PullStrategy::Get(botAI);
    if (!strategy)
        return false;

    Unit* target = strategy->GetTarget();
    std::string const spellName = strategy->GetSpellName();
    if (!target || !target->IsInWorld() || target->GetMapId() != bot->GetMapId() || spellName.empty())
        return false;

    return true;
}

void PullAction::InitPullAction()
{
    PullStrategy* strategy = PullStrategy::Get(botAI);
    if (!strategy)
        return;

    std::string const spellName = strategy->GetSpellName();
    if (spellName.empty())
        return;

    spell = spellName;

    bool isShoot = (spellName == "shoot" || spellName == "shoot bow" ||
                    spellName == "shoot gun" || spellName == "shoot crossbow" ||
                    spellName == "throw");
    range = botAI->GetRange(isShoot ? "shoot" : "spell");
}

bool PullEndAction::Execute(Event /*event*/)
{
    PullStrategy* strategy = PullStrategy::Get(botAI);
    if (!strategy)
        return false;

    Unit* pullTarget = strategy->GetTarget();

    if (!strategy->HasPullStarted() && !strategy->IsPullPendingToStart() && !strategy->HasTarget())
        return false;

    if (Pet* pet = bot->GetPet())
    {
        Creature* creature = pet->ToCreature();
        if (creature)
            creature->SetReactState(strategy->GetPetReactState());
    }

    PositionMap& posMap = AI_VALUE(PositionMap&, "position");
    PositionInfo pullPosition = posMap["pull"];
    if (pullPosition.isSet())
        posMap.erase("pull");

    if (pullTarget && context->GetValue<Unit*>("current target")->Get() == pullTarget)
        context->GetValue<Unit*>("current target")->Set(nullptr);

    strategy->OnPullEnded();
    return true;
}

bool ReturnToPullPositionAction::Execute(Event /*event*/)
{
    PositionInfo pullPosition = AI_VALUE(PositionMap&, "position")["pull"];
    if (!pullPosition.isSet() || pullPosition.mapId != bot->GetMapId())
        return false;

    return MoveTo(pullPosition.mapId, pullPosition.x, pullPosition.y, pullPosition.z,
                  false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true);
}

bool ReturnToPullPositionAction::isUseful()
{
    PullStrategy* strategy = PullStrategy::Get(botAI);
    Unit* target = strategy ? strategy->GetTarget() : nullptr;
    if (!strategy || !target || !target->IsInCombat())
        return false;

    PositionInfo pullPosition = AI_VALUE(PositionMap&, "position")["pull"];
    return pullPosition.isSet() && pullPosition.mapId == bot->GetMapId() &&
           bot->GetDistance(pullPosition.x, pullPosition.y, pullPosition.z) > sPlayerbotAIConfig.FollowDistance;
}

bool ReachPullAction::Execute(Event /*event*/)
{
    Unit* target = GetTarget();
    PullStrategy* strategy = PullStrategy::Get(botAI);
    if (!target || !strategy)
        return false;

    float reachDistance = GetPullReachDistance(bot, target, strategy);
    // In range but behind a corner or a hill: walk closer until the target is in sight. Without
    // this the shot failed every tick and the bot stood still until the pull timed out.
    if (IsWithinPullRange(bot, target, strategy) && !bot->IsWithinLOSInMap(target))
    {
        float const combatDistance = bot->GetCombatReach() + target->GetCombatReach();
        reachDistance = std::max(0.0f, bot->GetExactDist(target) - combatDistance - PULL_SIGHT_STEP);
    }

    return ReachCombatTo(target, reachDistance);
}

bool ReachPullAction::isUseful()
{
    if (botAI->HasStrategy("stay", botAI->GetState()))
        return false;

    if (bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL) != nullptr)
        return false;

    PullStrategy* strategy = PullStrategy::Get(botAI);
    Unit* target = strategy ? strategy->GetTarget() : nullptr;
    return target && !CanPullFromHere(bot, target, strategy);
}

Unit* ReachPullAction::GetTarget()
{
    PullStrategy* strategy = PullStrategy::Get(botAI);
    if (!strategy)
        return nullptr;

    return strategy->GetTarget();
}

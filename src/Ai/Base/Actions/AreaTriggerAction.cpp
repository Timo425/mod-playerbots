/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "AreaTriggerAction.h"
#include "Event.h"
#include "LastMovementValue.h"
#include "MapMgr.h"
#include "PlayerbotTextMgr.h"
#include "Playerbots.h"
#include "Transport.h"

bool ReachAreaTriggerAction::Execute(Event event)
{
    if (IsSelfBot(bot))  // SelfBots reach their own area triggers through the client.
        return false;

    uint32 triggerId;
    WorldPacket p(event.getPacket());
    p.rpos(0);
    p >> triggerId;

    AreaTrigger const* at = sObjectMgr->GetAreaTrigger(triggerId);
    if (!at)
        return false;

    if (!sObjectMgr->GetAreaTriggerTeleport(triggerId))
    {
        WorldPacket p1(CMSG_AREATRIGGER);
        p1 << triggerId;
        p1.rpos(0);
        bot->GetSession()->HandleAreaTriggerOpcode(p1);

        return true;
    }

    if (bot->GetMapId() != at->map)
    {
        botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "area_trigger_follow_too_far_error", "I won't follow: too far away", {}));
        return true;
    }

    // Local: a ghost whose corpse lies behind this portal goes through it with its master, as the
    // core's portal handler would send it (the same entry check, then the same teleport); arriving,
    // the core revives it at the entrance as it revives the master (HandleMoveWorldportAck). The
    // client's ghost runs on water and jumps down ledges; a ghost bot swims and paths around them.
    // After a wipe in Blackfathom Deeps the bots fell behind where the master jumped into the water
    // before the tunnels, were some 50-90 yd back when he entered (their walk delay at ghost speed)
    // and never arrived before he summoned them. React range (150 yd) leaves out a ghost that
    // stayed at the graveyard (660 yd off there); that one walks to the portal as before.
    AreaTriggerTeleport const* portal = sObjectMgr->GetAreaTriggerTeleport(triggerId);
    if (!bot->IsAlive() && bot->HasCorpse() && bot->GetCorpseLocation().GetMapId() == portal->target_mapId &&
        bot->GetDistance(at->x, at->y, at->z) <= sPlayerbotAIConfig.ReactDistance &&
        sMapMgr->PlayerCannotEnter(portal->target_mapId, bot, false) == Map::CAN_ENTER)
    {
        bot->GetMotionMaster()->Clear();
        bot->StopMoving();
        return bot->TeleportTo(portal->target_mapId, portal->target_X, portal->target_Y, portal->target_Z,
                               portal->target_Orientation, TELE_TO_NOT_LEAVE_TRANSPORT);
    }

    bot->GetMotionMaster()->MovePoint(
        /*id*/ at->map,
        /*coords*/ at->x, at->y, at->z,
        /*forcedMovement*/ FORCED_MOVEMENT_NONE,
        /*speed*/ 0.0f,             // default speed (not handled here)
        /*orientation*/ 0.0f,       // keep current orientation of bot
        /*generatePath*/ true,      // true => terrain path (2d mmap); false => straight spline (3d vmap)
        /*forceDestination*/ false);

    float distance = bot->GetDistance(at->x, at->y, at->z);
    float delay = IN_MILLISECONDS * distance / bot->GetSpeed(MOVE_RUN) + sPlayerbotAIConfig.ReactDelay;
    botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
        "area_trigger_wait_for_me", "Wait for me", {}));
    botAI->SetNextCheckDelay(delay);
    context->GetValue<LastMovement&>("last area trigger")->Get().lastAreaTrigger = triggerId;

    return true;
}

bool AreaTriggerAction::Execute(Event /*event*/)
{
    LastMovement& movement = context->GetValue<LastMovement&>("last area trigger")->Get();

    uint32 triggerId = movement.lastAreaTrigger;
    movement.lastAreaTrigger = 0;

    if (!sObjectMgr->GetAreaTrigger(triggerId))
        return false;

    if (!sObjectMgr->GetAreaTriggerTeleport(triggerId))
        return true;

    WorldPacket p(CMSG_AREATRIGGER);
    p << triggerId;
    p.rpos(0);
    bot->GetSession()->HandleAreaTriggerOpcode(p);

    botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault("hello", "Hello", {}));
    return true;
}

/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SENDMAILACTION_H
#define PLAYERBOTS_SENDMAILACTION_H

#include "InventoryAction.h"

class PlayerbotAI;

class SendMailAction : public InventoryAction
{
public:
    SendMailAction(PlayerbotAI* botAI) : InventoryAction(botAI, "sendmail") {}

    bool Execute(Event event) override;
};

// Local (single-player server): "handover" mails the master, with no delivery delay, every
// tradeable item worth more to the player than to the bot (trade goods, gems, recipes, green or
// better gear the bot won't wear) and the bot's gold above a small repair reserve. Bots' gold
// buys the player nothing, and selling these to a vendor would waste them.
class HandOverAction : public InventoryAction
{
public:
    HandOverAction(PlayerbotAI* botAI) : InventoryAction(botAI, "handover") {}

    bool Execute(Event event) override;
};

#endif

/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SendMailAction.h"
#include "ChatHelper.h"
#include "Event.h"
#include "ItemUsageValue.h"
#include "ItemVisitors.h"
#include "Mail.h"
#include "PlayerbotTextMgr.h"
#include "Playerbots.h"

bool SendMailAction::Execute(Event event)
{
    uint32 account = bot->GetSession()->GetAccountId();
    bool randomBot = sPlayerbotAIConfig.IsInRandomAccountList(account);

    GuidVector gos = *context->GetValue<GuidVector>("nearest game objects");
    bool mailboxFound = false;
    for (ObjectGuid const guid : gos)
    {
        if (GameObject* go = botAI->GetGameObject(guid))
            if (go->GetGoType() == GAMEOBJECT_TYPE_MAILBOX)
            {
                mailboxFound = true;
                break;
            }
    }

    std::string const text = event.getParam();

    Player* receiver = GetMaster();
    Player* tellTo = receiver;

    if (!receiver)
        receiver = event.getOwner();

    if (!receiver || receiver == bot)
        return false;

    if (!tellTo)
        tellTo = receiver;

    if (!sPlayerbotAIConfig.BotSendMailEnabled)
    {
        bot->Whisper(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                         "send_mail_disabled", "I cannot send mail", {}),
                     LANG_UNIVERSAL, tellTo);
        return false;
    }

    if (!mailboxFound && !randomBot)
    {
        bot->Whisper(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                         "send_mail_no_mailbox_nearby", "There is no mailbox nearby", {}),
                     LANG_UNIVERSAL, tellTo);
        return false;
    }

    ItemIds ids = chat->parseItems(text);
    if (ids.size() > 1)
    {
        bot->Whisper(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                         "send_mail_one_item_only", "You can not request more than one item", {}),
                     LANG_UNIVERSAL, tellTo);
        return false;
    }

    if (ids.empty())
    {
        uint32 money = chat->parseMoney(text);
        if (!money)
            return false;

        if (randomBot)
        {
            bot->Whisper(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                             "send_mail_cannot_send_money", "I cannot send money", {}),
                         LANG_UNIVERSAL, tellTo);
            return false;
        }

        if (bot->GetMoney() < money)
        {
            botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "send_mail_not_enough_money", "I don't have enough money", {}));
            return false;
        }

        std::ostringstream body;
        body << "Hello, " << receiver->GetName() << ",\n";
        body << "\n";
        body << "Here is the money you asked for";
        body << "\n";
        body << "Thanks,\n";
        body << bot->GetName() << "\n";

        CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();

        MailDraft draft("Money you asked for", body.str());
        draft.AddMoney(money);
        bot->SetMoney(bot->GetMoney() - money);
        draft.SendMailTo(trans, MailReceiver(receiver), MailSender(bot));

        CharacterDatabase.CommitTransaction(trans);

        std::ostringstream out;
        botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "send_mail_sending_to",
            "Sending mail to %receiver",
            {{"%receiver", receiver->GetName()}}));
        return true;
    }

    std::ostringstream body;
    body << "Hello, " << receiver->GetName() << ",\n";
    body << "\n";
    body << "Here are the item(s) you asked for";
    body << "\n";
    body << "Thanks,\n";
    body << bot->GetName() << "\n";

    MailDraft draft("Item(s) you asked for", body.str());
    for (ItemIds::iterator i = ids.begin(); i != ids.end(); i++)
    {
        FindItemByIdVisitor visitor(*i);
        IterateItems(&visitor, ITERATE_ITEMS_IN_BAGS);

        std::vector<Item*> items = visitor.GetResult();
        for (Item* item : items)
        {
            if (item->IsSoulBound() || item->IsConjuredConsumable())
            {
                std::ostringstream out;
                out << PlayerbotTextMgr::instance().GetBotTextOrDefault(
                    "send_mail_cannot_send_item",
                    "Cannot send %item",
                    {{"%item", ChatHelper::FormatItem(item->GetTemplate())}});
                bot->Whisper(out.str(), LANG_UNIVERSAL, tellTo);
                continue;
            }

            ItemTemplate const* proto = item->GetTemplate();
            if (!proto)
                continue;

            if (randomBot)
            {
                uint32 price = item->GetCount() * proto->SellPrice;
                if (!price)
                {
                    std::ostringstream out;
                    out << PlayerbotTextMgr::instance().GetBotTextOrDefault(
                        "send_mail_item_not_for_sale",
                        "%item: it is not for sale",
                        {{"%item", ChatHelper::FormatItem(item->GetTemplate())}});
                    bot->Whisper(out.str(), LANG_UNIVERSAL, tellTo);
                    return false;
                }

                draft.AddCOD(price);
            }

            CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();

            bot->MoveItemFromInventory(item->GetBagSlot(), item->GetSlot(), true);
            item->DeleteFromInventoryDB(trans);
            item->SetOwnerGUID(receiver->GetGUID());
            item->SaveToDB(trans);
            draft.AddItem(item);
            draft.SendMailTo(trans, MailReceiver(receiver), MailSender(bot));

            CharacterDatabase.CommitTransaction(trans);

            std::ostringstream out;
            out << PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "send_mail_sent_to",
                "Sent mail to %receiver",
                {{"%receiver", receiver->GetName()}});
            bot->Whisper(out.str(), LANG_UNIVERSAL, tellTo);
            return true;
        }
    }

    return false;
}

namespace
{
class FindHandOverItemsVisitor : public IterateItemsVisitor
{
public:
    explicit FindHandOverItemsVisitor(AiObjectContext* context) : IterateItemsVisitor(), context(context) {}

    bool Visit(Item* item) override
    {
        ItemTemplate const* proto = item->GetTemplate();
        if (!proto || item->IsSoulBound() || item->IsConjuredConsumable() || proto->Class == ITEM_CLASS_QUEST ||
            !proto->SellPrice)
            return true;

        bool const valuable = (proto->Class == ITEM_CLASS_TRADE_GOODS && proto->Quality > ITEM_QUALITY_POOR) ||
                              proto->Class == ITEM_CLASS_GEM || proto->Class == ITEM_CLASS_RECIPE ||
                              proto->Quality >= ITEM_QUALITY_UNCOMMON;
        if (!valuable)
            return true;

        switch (context->GetValue<ItemUsage>("item usage", proto->ItemId)->Get())
        {
            case ITEM_USAGE_NONE:
            case ITEM_USAGE_DISENCHANT:
            case ITEM_USAGE_AH:
            case ITEM_USAGE_VENDOR:
                result.push_back(item);
                break;
            default:
                break;
        }
        return true;
    }

    std::vector<Item*> result;

private:
    AiObjectContext* context;
};
}  // namespace

bool HandOverAction::Execute(Event /*event*/)
{
    Player* master = GetMaster();
    if (!master || master == bot)
        return false;

    FindHandOverItemsVisitor visitor(context);
    IterateItems(&visitor, ITERATE_ITEMS_IN_BAGS);

    uint32 const reserve = bot->GetLevel() * 50;  // copper kept for repairs: 5s at 10, 30s at 60
    uint32 const money = bot->GetMoney() > reserve ? bot->GetMoney() - reserve : 0;
    if (visitor.result.empty() && !money)
    {
        botAI->TellMaster("Nothing to hand over");
        return true;
    }

    size_t sent = 0;
    bool moneySent = false;
    while (sent < visitor.result.size() || (money && !moneySent))
    {
        MailDraft draft("Loot for you", "Things worth more to you than to me.\n" + bot->GetName());
        CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();
        for (size_t n = 0; n < MAX_MAIL_ITEMS && sent < visitor.result.size(); ++n, ++sent)
        {
            Item* item = visitor.result[sent];
            bot->MoveItemFromInventory(item->GetBagSlot(), item->GetSlot(), true);
            item->DeleteFromInventoryDB(trans);
            item->SetOwnerGUID(master->GetGUID());
            item->SaveToDB(trans);
            draft.AddItem(item);
        }
        if (money && !moneySent)
        {
            draft.AddMoney(money);
            bot->ModifyMoney(-int32(money));
            moneySent = true;
        }
        draft.SendMailTo(trans, MailReceiver(master), MailSender(bot));
        CharacterDatabase.CommitTransaction(trans);
    }

    std::ostringstream out;
    out << "Mailed you " << visitor.result.size() << " items";
    if (money)
        out << " and " << ChatHelper::formatMoney(money);
    botAI->TellMaster(out.str());
    return true;
}

/*
* Copyright (C) 2008-2011 TrinityCore <http://www.trinitycore.org/>
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation; either version 2 of the License, or (at your
* option) any later version.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
* more details.
*
* You should have received a copy of the GNU General Public License along
* with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "ScriptMgr.h"
#include "AccountMgr.h"
#include "ObjectMgr.h"
#include "WorldSession.h"
#include "Chat.h"

class ban_commandscript : public CommandScript
{
public:
ban_commandscript() : CommandScript("ban_commandscript") { }

ChatCommand* GetCommands() const
{
    static ChatCommand banCommandTable[] =
    {
        { "account",        SEC_ADMINISTRATOR,  true,  &HandleBanAccountCommand,          "", NULL },
        { "character",      SEC_ADMINISTRATOR,  true,  &HandleBanCharacterCommand,        "", NULL },
        { "playeraccount",  SEC_ADMINISTRATOR,  true,  &HandleBanAccountByCharCommand,    "", NULL },
        { "ip",             SEC_ADMINISTRATOR,  true,  &HandleBanIPCommand,               "", NULL },
        { NULL,             0,                  false, NULL,                              "", NULL }
    };
    static ChatCommand commandTable[] =
    {
        { "ban",            SEC_ADMINISTRATOR,  true,  NULL,                   "", banCommandTable },
        { NULL,             0,                  false, NULL,                              "", NULL }
    };
    return commandTable;
}

static bool HandleBanAccountCommand(ChatHandler* handler, const char* args)
{
    return HandleBanHelper(BAN_ACCOUNT, args);
}

static bool HandleBanAccountByCharCommand(ChatHandler* handler, const char *args)
{
    return HandleBanHelper(BAN_CHARACTER, args);
}

static bool HandleBanIPCommand(ChatHandler* handler, const char *args)
{
    return HandleBanHelper(BAN_IP, args);
}

static bool HandleBanCharacterCommand(ChatHandler* handler, const char *args)
{
    if (!*args)
        return false;

    char* cname = strtok((char*)args, " ");
    if (!cname)
        return false;

    std::string name = cname;

    char* duration = strtok(NULL, " ");
    if (!duration || !atoi(duration))
        return false;

    char* reason = strtok(NULL, "");
    if (!reason)
        return false;

    if (!normalizePlayerName(name))
    {
        handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
        handler->SetSentErrorMessage(true);
        return false;
    }

    switch (sWorld->BanCharacter(name, duration, reason, handler->GetSession() ? handler->GetSession()->GetPlayerName() : ""))
    {
        case BAN_SUCCESS:
        {
            if (atoi(duration) > 0)
                handler->PSendSysMessage(LANG_BAN_YOUBANNED, name.c_str(), secsToTimeString(TimeStringToSecs(duration), true).c_str(), reason);
            else
                handler->PSendSysMessage(LANG_BAN_YOUPERMBANNED, name.c_str(), reason);
            break;
        }
        case BAN_NOTFOUND:
        {
            handler->PSendSysMessage(LANG_BAN_NOTFOUND, "character", name.c_str());
            handler->SetSentErrorMessage(true);
            return false;
        }
        default:
            break;
    }

    return true;
}

static bool HandleBanHelper(BanMode mode, const char* args)
{
    ChatHandler* handler;

    if (!*args)
        return false;

    char* cnameOrIP = strtok ((char*)args, " ");
    if (!cnameOrIP)
        return false;

    std::string nameOrIP = cnameOrIP;

    char* duration = strtok (NULL, " ");
    if (!duration || !atoi(duration))
        return false;

    char* reason = strtok (NULL, "");
    if (!reason)
        return false;

    switch(mode)
    {
        case BAN_ACCOUNT:
            if (!AccountMgr::normalizeString(nameOrIP))
            {
                handler->PSendSysMessage(LANG_ACCOUNT_NOT_EXIST, nameOrIP.c_str());
                handler->SetSentErrorMessage(true);
                return false;
            }
            break;
        case BAN_CHARACTER:
            if (!normalizePlayerName(nameOrIP))
            {
                handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
                handler->SetSentErrorMessage(true);
                return false;
            }
            break;
        case BAN_IP:
            if (!IsIPAddress(nameOrIP.c_str()))
                return false;
            break;
    }

    switch(sWorld->BanAccount(mode, nameOrIP, duration, reason, handler->GetSession() ? handler->GetSession()->GetPlayerName() : ""))
    {
        case BAN_SUCCESS:
            if (atoi(duration)>0)
                handler->PSendSysMessage(LANG_BAN_YOUBANNED, nameOrIP.c_str(), secsToTimeString(TimeStringToSecs(duration), true).c_str(), reason);
            else
                handler->PSendSysMessage(LANG_BAN_YOUPERMBANNED, nameOrIP.c_str(), reason);
            break;
        case BAN_SYNTAX_ERROR:
            return false;
        case BAN_NOTFOUND:
            switch(mode)
            {
                default:
                    handler->PSendSysMessage(LANG_BAN_NOTFOUND, "account", nameOrIP.c_str());
                    break;
                case BAN_CHARACTER:
                    handler->PSendSysMessage(LANG_BAN_NOTFOUND, "character", nameOrIP.c_str());
                    break;
                case BAN_IP:
                    handler->PSendSysMessage(LANG_BAN_NOTFOUND, "ip", nameOrIP.c_str());
                    break;
            }
            handler->SetSentErrorMessage(true);
            return false;
    }

    return true;
}
};

void AddSC_ban_commandscript()
{
new ban_commandscript();
}

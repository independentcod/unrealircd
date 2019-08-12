/*
 *   IRC - Internet Relay Chat, src/modules/m_dccallow
 *   (C) 2004 The UnrealIRCd Team
 *
 *   See file AUTHORS in IRC package for additional names of
 *   the programmers.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 1, or (at your option)
 *   any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "unrealircd.h"

CMD_FUNC(m_dccallow);

#define MSG_DCCALLOW 	"DCCALLOW"

ModuleHeader MOD_HEADER(dccallow)
  = {
	"dccallow",
	"5.0",
	"command /dccallow", 
	"3.2-b8-1",
	NULL 
    };

MOD_INIT(dccallow)
{
	CommandAdd(modinfo->handle, MSG_DCCALLOW, m_dccallow, 1, M_USER);
	IsupportAdd(modinfo->handle, "USERIP", NULL);
	MARK_AS_OFFICIAL_MODULE(modinfo);
	return MOD_SUCCESS;
}

MOD_LOAD(dccallow)
{
	return MOD_SUCCESS;
}

MOD_UNLOAD(dccallow)
{
	return MOD_SUCCESS;
}

/* m_dccallow:
 * HISTORY:
 * Taken from bahamut 1.8.1
 */
CMD_FUNC(m_dccallow)
{
	Link *lp;
	char *p, *s;
	aClient *acptr;
	int didlist = 0, didhelp = 0, didanything = 0;
	char **ptr;
	int ntargets = 0;
	int maxtargets = max_targets_for_command("WHOIS");
	static char *dcc_help[] =
	{
		"/DCCALLOW [<+|->nick[,<+|->nick, ...]] [list] [help]",
		"You may allow DCCs of files which are otherwise blocked by the IRC server",
		"by specifying a DCC allow for the user you want to recieve files from.",
		"For instance, to allow the user Bob to send you file.exe, you would type:",
		"/DCCALLOW +bob",
		"and Bob would then be able to send you files. Bob will have to resend the file",
		"if the server gave him an error message before you added him to your allow list.",
		"/DCCALLOW -bob",
		"Will do the exact opposite, removing him from your dcc allow list.",
		"/dccallow list",
		"Will list the users currently on your dcc allow list.",
		NULL
	};

	if (!MyClient(sptr))
		return 0;
	
	if (parc < 2)
	{
		sendnotice(sptr, "No command specified for DCCALLOW. "
			"Type '/DCCALLOW HELP' for more information.");
		return 0;
	}

	for (p = NULL, s = strtoken(&p, parv[1], ", "); s; s = strtoken(&p, NULL, ", "))
	{
		if (MyClient(sptr) && (++ntargets > maxtargets))
		{
			sendnumeric(sptr, ERR_TOOMANYTARGETS, s, maxtargets, "DCCALLOW");
			break;
		}
		if (*s == '+')
		{
			didanything = 1;
			if (!*++s)
				continue;
			
			acptr = find_person(s, NULL);
			
			if (acptr == sptr)
				continue;
			
			if (!acptr)
			{
				sendnumeric(sptr, ERR_NOSUCHNICK, s);
				continue;
			}
			add_dccallow(sptr, acptr);
		} else
		if (*s == '-')
		{
			didanything = 1;
			if (!*++s)
				continue;
			
			acptr = find_person(s, NULL);
			if (acptr == sptr)
				continue;
			if (!acptr)
			{
				sendnumeric(sptr, ERR_NOSUCHNICK, s);
				continue;
			}
			del_dccallow(sptr, acptr);
		} else
		if (!didlist && !myncmp(s, "list", 4))
		{
			didanything = didlist = 1;
			sendnumericfmt(sptr, RPL_DCCINFO, "The following users are on your dcc allow list:");
			for(lp = sptr->user->dccallow; lp; lp = lp->next)
			{
				if (lp->flags == DCC_LINK_REMOTE)
					continue;
				sendnumericfmt(sptr, RPL_DCCLIST, "%s (%s@%s)", lp->value.cptr->name,
					lp->value.cptr->user->username,
					GetHost(lp->value.cptr));
			}
			sendnumeric(sptr, RPL_ENDOFDCCLIST, s);
		} else
		if (!didhelp && !myncmp(s, "help", 4))
		{
			didanything = didhelp = 1;
			for(ptr = dcc_help; *ptr; ptr++)
				sendnumericfmt(sptr, RPL_DCCINFO, "%s", *ptr);
			sendnumeric(sptr, RPL_ENDOFDCCLIST, s);
		}
	}
	if (!didanything)
	{
		sendnotice(sptr, "Invalid syntax for DCCALLOW. Type '/DCCALLOW HELP' for more information.");
		return 0;
	}
	return 0;
}
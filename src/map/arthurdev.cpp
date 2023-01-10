// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.hpp"
#include "../common/mmo.hpp"
#include "../common/timer.hpp"
#include "../common/nullpo.hpp"
#include "../common/core.hpp"
#include "../common/showmsg.hpp"
#include "../common/malloc.hpp"
#include "../common/random.hpp"
#include "../common/socket.hpp"
#include "../common/strlib.hpp"
#include "../common/utils.hpp"
#include "../common/conf.hpp"

#include "map.hpp"
#include "atcommand.hpp"
#include "battle.hpp"
#include "battleground.hpp"
#include "chat.hpp"
#include "channel.hpp"
#include "clif.hpp"
#include "chrif.hpp"
#include "duel.hpp"
#include "instance.hpp"
#include "intif.hpp"
#include "itemdb.hpp"
#include "log.hpp"
#include "pc.hpp"
#include "pc_groups.hpp" // groupid2name
#include "status.hpp"
#include "skill.hpp"
#include "mob.hpp"
#include "npc.hpp"
#include "pet.hpp"
#include "homunculus.hpp"
#include "mail.hpp"
#include "mercenary.hpp"
#include "elemental.hpp"
#include "party.hpp"
#include "guild.hpp"
#include "script.hpp"
#include "storage.hpp"
#include "trade.hpp"
#include "unit.hpp"
#include "mapreg.hpp"
#include "quest.hpp"
#include "pc.hpp"
#include "achievement.hpp"
#include "arthurdev.hpp"
#include "npc.hpp"
#include "script.hpp"
#include "chrif.hpp"
#include "clif.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

using namespace rathena;

/// Returns the player attached to this script, identified by the rid.
/// If there is no player attached, the script is terminated.
/// RAthena cambió el método ... al no encontrar más lo implemento por aquí..
TBL_PC* arthurdev_rid2sd(struct script_state* st)
{
	TBL_PC* sd = map_id2sd(st->rid);
	if (!sd) {
		ShowError("arthurdev_rid2sd: fatal state ! player not attached!\n");
		st->state = END;
	}
	return sd;
}

int status_change_start_sub(struct block_list* bl, va_list ap) {
	struct block_list* src = va_arg(ap, struct block_list*);

	enum sc_type type = (sc_type)va_arg(ap, int);
	int rate = va_arg(ap, int);
	int val1 = va_arg(ap, int);
	int val2 = va_arg(ap, int);
	int val3 = va_arg(ap, int);
	int val4 = va_arg(ap, int);
	int tick = va_arg(ap, int);
	int flag = va_arg(ap, int);

	if (src && bl && src->id != bl->id) {
		status_change_start(src, bl, type, rate, val1, val2, val3, val4, tick, flag);
	}
	return 0;
}

const char* GetBGName(int64 bgid)
{
	switch (bgid)
	{
	case 1:	return "Conquest";
	case 2:	return "Rush";
	case 3:	return "Flavious TD";
	case 4:	return "Team vs Team";
	case 5:	return "Flavius CTF";
	case 6: return "Defend The Emperium";
	default:	return "None";
	}
}

void ReOrderBG()
{
	char BG_Var[12];
	int i, actual_array = 0;
	int64 tmparr[(MAX_BG_ARRAY - 1)];

	for (i = 1; i < MAX_BG_ARRAY; i++)
	{
		sprintf(BG_Var, "$NEXTBG_%d", i);
		if (mapreg_readreg(add_str(BG_Var)) > 0)
		{
			tmparr[actual_array] = mapreg_readreg(add_str(BG_Var));
			actual_array++;
			mapreg_setreg(add_str(BG_Var), 0);
		}
	}

	for (i = 1; i < MAX_BG_ARRAY; i++)
	{
		sprintf(BG_Var, "$NEXTBG_%d", i);
		if (tmparr[(i - 1)] && tmparr[(i - 1)] > 0)
			mapreg_setreg(add_str(BG_Var), tmparr[(i - 1)]);
	}
}

void ShowBGArray(struct map_session_data* sd)
{
	int i;
	char BG_Var[12], output[CHAT_SIZE_MAX];
	clif_displaymessage(sd->fd, "BG Array");
	clif_displaymessage(sd->fd, "-----------------------------------------------");

	for (i = 1; i <= MAX_BG_ARRAY; i++)
	{
		sprintf(BG_Var, "$NEXTBG_%d", i);
		sprintf(output, "BG %d: %s", i, GetBGName(mapreg_readreg(add_str(BG_Var))));
		clif_displaymessage(sd->fd, output);
	}

	clif_displaymessage(sd->fd, "-----------------------------------------------");
}

int DropAT(struct map_session_data* sd, va_list args)
{
	if (sd->state.autotrade == 0)
		return 0;
	clif_GM_kick(NULL, sd);
	return 0;
}

// Oboro
bool clif_effect_hide(struct map_session_data* sd, const uint8* buf) {
	unsigned int cmd = RBUFW(buf, 0);
	unsigned int skill_or_effect = RBUFW(buf, 2);

	if (!sd || !sd->state.packet_filter) {
		return false;
	}

	switch (cmd) {

		// BLESS / AGI / ASSUMPTION
	case ZC_USE_SKILL2: {
		unsigned int destId = RBUFL(buf, 8);
		unsigned int srcId = RBUFL(buf, 12);

		if (srcId && sd->status.account_id == srcId && sd->state.packet_filter)
			return false;

		return skill_can_filter(skill_or_effect, sd);
	};

					  // SLIM POTION PITCHER
	case ZC_NOTIFY_GROUNDSKILL:
		return skill_can_filter(skill_or_effect, sd) && skill_or_effect == CR_SLIMPITCHER;

		// HEAL & DAMAGE SKILLS
	case ZC_NOTIFY_SKILL2:
		return skill_can_filter(skill_or_effect, sd);

		// EFFECTS OF SKILL
	case ZC_MSG_STATE_CHANGE:
	case ZC_MSG_STATE_CHANGE3: {

		short flag = RBUFB(buf, 8);
		if (flag == 0) // if skill effect is over let it pass...
			return false;
		else
			return effect_can_filter(skill_or_effect);
	};

							 // REMOVE ACIDDEMONSTRATION EFFECT
	case ZC_USESKILL_ACK2: {
		return RBUFW(buf, 14) == CR_ACIDDEMONSTRATION;
	};

	default:
		return false;
	}
}

// Oboro - @nomagias
int clif_nomagias(struct map_session_data* sd, const uint8* buf) {
	unsigned int cmd = RBUFW(buf, 0);
	unsigned int skill_id = 0, indx = 0;
	if (!sd || !sd->state.nomagias) {
		return false;
	}

	switch (cmd) {
	case ZC_NOTIFY_GROUNDSKILL:
		skill_id = RBUFW(buf, 2);
		indx = 2;
		break;
	case ZC_USESKILL_ACK2:
		skill_id = RBUFW(buf, 14);
		indx = 14;
		break;
	}

	if (indx > 0 && (skill_id == WZ_VERMILION || skill_id == WZ_STORMGUST || skill_id == WZ_METEOR)) {
		return indx;
	}
	else
		return 0;
}

unsigned char* clif_UnitWalkingFix(unsigned char* buf, map_session_data* sd) {
	struct packet_unit_walking* p = (packet_unit_walking*)RBUFP(buf, 0);

	if (sd->state.packet_filter) {
		p->virtue = 0;
	}

	if (sd->state.nohats) {
		p->accessory = 0;
		p->accessory2 = 0;
		p->accessory3 = 0;
	}

	return (unsigned char*)p;
}


void do_init_oboro(void)
{

	//Let's create the battleground instance!
	if (!mapreg_readreg(add_str("$CURRENTBG")))
		mapreg_setreg(add_str("$CURRENTBG"), 1);

	if (!mapreg_readreg(add_str("$CURRENTPOCBG")))
		mapreg_setreg(add_str("$CURRENTPOCBG"), 1);

	if (!mapreg_readreg(add_str("$NEXTBG_1")))
		mapreg_setreg(add_str("$NEXTBG_1"), 1);

	if (!mapreg_readreg(add_str("$NEXTBG_2")))
		mapreg_setreg(add_str("$NEXTBG_2"), 1);

	if (!mapreg_readreg(add_str("$NEXTBG_3")))
		mapreg_setreg(add_str("$NEXTBG_3"), 2);

	if (!mapreg_readreg(add_str("$NEXTBG_4")))
		mapreg_setreg(add_str("$NEXTBG_4"), 4);

	if (!mapreg_readreg(add_str("$NEXTBG_5")))
		mapreg_setreg(add_str("$NEXTBG_5"), 4);

	if (!mapreg_readreg(add_str("$MINBGLIMIT")))
		mapreg_setreg(add_str("$MINBGLIMIT"), 4);
}

struct queue_member* oboro_queue_get_member(struct queue_data* qd, int position) {
	struct queue_member* head;

	if (!qd) return NULL;

	head = qd->first;

	while (head != NULL)
	{
		if (head->sd && head->position == position)
			return head;

		head = head->next;
	}

	return NULL;
}

/**
 * Oboro: return the partyleader Session data.
 * NULL if not exists.
 **/
TBL_PC* party_getpartyleaderSd(struct map_session_data* sd) {
	struct party_data* p;
	int i;

	p = party_search(sd->status.party_id);

	if (p == NULL)
		return NULL;

	ARR_FIND(0, MAX_PARTY, i, p->party.member[i].leader);

	if (i >= MAX_PARTY)
		return NULL;

	return map_id2sd(p->party.member[i].account_id);
}

/**
 * Oboro
 * Busca un miembro sin party o bien de otra party a la buscada...
 **/
TBL_PC* bg_searchNonPartyQueue(int q_id, int party_id) {
	struct queue_data* qd;

	if ((qd = bg_queue_search(q_id)) == NULL)
	{
		return NULL;
	}

	struct queue_member* head;
	struct map_session_data* sd;
	int memberCount = 0, i = 0, free = 0;

	head = qd->first;
	while (head) {
		if ((sd = head->sd) != NULL && (sd->bg_queue_type == 0 || sd->status.party_id != party_id)) {
			return sd;
		}

		head = head->next;
	}

	return NULL;
}

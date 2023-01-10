#ifndef arthurdev_HPP
#define arthurdev_HPP

#include "log.hpp"
#include "mob.hpp"

void do_init_arthurdev(void);
void do_init_oboro(void);

#define MAX_BG_ARRAY 6

TBL_PC* arthurdev_rid2sd(struct script_state* st);

int status_change_start_sub(struct block_list* bl, va_list ap);

#define skill_can_filter(skill_id, sd) (\
	skill_id == AL_HEAL\
		|| skill_id == HP_ASSUMPTIO\
		|| (skill_id == SL_KAITE && sd->class_ != 4049)\
		|| (skill_id == SL_KAUPE && sd->class_ != 4049)\
		|| skill_id == CR_SLIMPITCHER\
		|| skill_id == DC_SCREAM\
		|| skill_id == AL_BLESSING\
) // las skills que pueden ser filtradas por packet_filter Oboro

#define effect_can_filter(effect_id) (\
		effect_id == EFST_ASSUMPTIO \
		|| effect_id == EFST_ASSUMPTIO2 \
) // los effectos de las skills que pueden ser filtrados Oboro

enum OBORO_PACKETFILER_PACKET {
	ZC_USE_SKILL2			= 0x9cb, // BLESS / AGI / ASSUMPTION
	ZC_NOTIFY_GROUNDSKILL	= 0x117, // CR_SLIMPITCHER
	ZC_NOTIFY_SKILL			= 0x0114,// HEAL & DAMAGE SKILLS OLD CLIENTS
	ZC_NOTIFY_SKILL2		= 0x1de, // HEAL & DAMAGE SKILLS
	ZC_MSG_STATE_CHANGE		= 0x196, // EFFECTS OF SKILL OLD CLIENTS
	ZC_MSG_STATE_CHANGE3	= 0x983, // EFFECTS OF SKILL
	ZC_USESKILL_ACK2		= 0x7fb, // ACID_DEMONSTRATION
};

bool clif_effect_hide(struct map_session_data* sd, const uint8* buf);
int clif_nomagias(struct map_session_data* sd, const uint8* buf);
unsigned char* clif_UnitWalkingFix(unsigned char* buf, map_session_data* sd);

/// Battleground 3.0
const char* GetBGName(int64 bgid);
void ReOrderBG();
void ShowBGArray(struct map_session_data* sd);

struct queue_member* oboro_queue_get_member(struct queue_data* qd, int position);

TBL_PC* party_getpartyleaderSd(struct map_session_data* sd);
TBL_PC* bg_searchNonPartyQueue(int q_id, int party_id);


#endif /*arthurdev_HPP*/

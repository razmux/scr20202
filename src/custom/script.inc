// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

/**
 * Custom Script Commands
 * Place the body of custom script commands in this file.
 * Format:
 *	BUILDIN_FUNC(command_name)
 *	{
 *		<code>;
 *	}
 **/

//BUILDIN_FUNC(example)
//{
//	ShowInfo("buildin_example: It works!\n");
//	script_pushint(st,1);
//	return 0;
//}

/** [Cydh]
* Get security status of player
* is_security {<char_id>};
* @return sd->state.security
*/
BUILDIN_FUNC(is_security) {
	struct map_session_data* sd;

	if (script_hasdata(st,2))
		sd = map_charid2sd(script_getnum(st,2));
	else
		script_rid2sd(sd);

	if (sd == NULL) {
		ShowError("buildin_is_security: Failed to check player's security state, no player attached!\n");
		script_pushint(st,-1);
		return SCRIPT_CMD_FAILURE;
	}
	script_pushint(st,sd->state.security);
	return SCRIPT_CMD_SUCCESS;
}

/** [Cydh]
* Set security status of player
* setsecurity <status>{,<char_id>};
* @param status 1 : enable. 0 : disable
*/
BUILDIN_FUNC(setsecurity) {
	struct map_session_data* sd;

	if (script_hasdata(st,3))
		sd = map_charid2sd(script_getnum(st,3));
	else
		script_rid2sd(sd);

	if (sd == NULL) {
		ShowError("buildin_setsecurity: Failed to check player's security state, no player attached!\n");
		script_pushint(st,-1);
		return SCRIPT_CMD_FAILURE;
	}

	switch (script_getnum(st,2)) {
		case 1:
			sd->state.security = 1;
			clif_displaymessage(sd->fd,"Security is enabled!");
			break;
		default:
			sd->state.security = 0;
			clif_displaymessage(sd->fd,"Security is disabled!");
			break;
	}

	script_pushint(st,1);
	return SCRIPT_CMD_SUCCESS;
}

static int buildin_reloadmob_instance_sub(struct block_list *bl,va_list ap)	// Added by RoVeRT
{
	struct mob_data *md = ((struct mob_data *)bl);
	if (md)
		status_calc_mob_instance(md);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(instance_setdifficulty)
{
	int instance_id = script_getnum(st,2);
	e_instance_difficulty id_mode = static_cast<e_instance_difficulty>(script_getnum(st,3));

	if( instance_id <= 0 ) {
		ShowError("instance_setdifficulty: Trying to set difficulty to an invalid instance %d.\n", instance_id);
		return SCRIPT_CMD_FAILURE;
	}

	if (!(id_mode >= ID_EASY && id_mode < ID_MAX)) {
		ShowError("instance_setdifficulty: Trying to set difficulty to an unexistent difficulty mode to instance %d.\n", instance_id);
		return SCRIPT_CMD_FAILURE;
	}

	std::shared_ptr<s_instance_data> idata = util::umap_find(instances, instance_id);
	if (!idata) {
		ShowError("instance_setdifficulty: Trying to set difficulty to an unexistent instance %d.\n", instance_id);
		return SCRIPT_CMD_FAILURE;
	}
	if (idata->difficulty == id_mode) { //Same mode
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}
	idata->difficulty = id_mode;

	// Reload Mob Stats
	for (const auto &it : idata->map) {
		map_foreachinmap(buildin_reloadmob_instance_sub, it.m, BL_MOB, 0);
	}

	script_pushint(st,1);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(instance_setpenalty)
{
	int instance_id = script_getnum(st,2);
	enum sc_type type;
	//int16 m;

	if( instance_id <= 0 ) {
		ShowError("instance_setpenalty: Trying to set penalty to an invalid instance %d.\n", instance_id);
		return SCRIPT_CMD_FAILURE;
	}

	std::shared_ptr<s_instance_data> idata = util::umap_find(instances, instance_id);
	if (!idata) {
		ShowError("instance_setpenalty: Trying to set penalty to an unexistent instance %d.\n", instance_id);
		return SCRIPT_CMD_FAILURE;
	}

	type = (sc_type)script_getnum(st,3);
	int value = script_getnum(st,4);

	if (!(type > SC_NONE && type < SC_MAX)) {
		ShowError("instance_setdifficulty: Unknown SC_STATUS for instance %d.\n", instance_id);
		return SCRIPT_CMD_FAILURE;
	}
	idata->sc_penalties[type] = value;

	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(instance_getdifficulty)
{
	int instance_id = script_getnum(st,2);

	if( instance_id <= 0 ) {
		ShowError("instance_getdifficulty: Trying to get difficulty to an invalid instance %d.\n", instance_id);
		return SCRIPT_CMD_FAILURE;
	}

	std::shared_ptr<s_instance_data> idata = util::umap_find(instances, instance_id);
	if (!idata) {
		ShowError("instance_getdifficulty: Trying to get difficulty to an unexistent instance %d.\n", instance_id);
		return SCRIPT_CMD_FAILURE;
	}

	script_pushint(st, (int)idata->difficulty);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(arthurdev_map_multicount)
{
	const char* var = script_getstr(st, 2);
	struct map_session_data* sd = arthurdev_rid2sd(st);
	int cnt = 0, m;

	if (sd && (m = map_mapname2mapid(var)) >= 0)
	{
		struct map_session_data* pl_sd;
		struct s_mapiterator* iter;
		/** use gepard_id**/
		//unsigned short uid = session[sd->fd]->gepard_info.unique_id;
		/** use lastuniqueid (ip da rede)**/
		unsigned short uid = session[sd->fd]->client_addr;

		iter = mapit_getallusers();
		for (pl_sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); pl_sd = (TBL_PC*)mapit_next(iter))
		{
			if (pl_sd->bl.m != m)
				continue;

			/** use gepard_id**/
			//unsigned short pluid = session[pl_sd->fd]->gepard_info.unique_id;
			/** use lastuniqueid (ip da rede) **/
			unsigned short pluid = session[pl_sd->fd]->client_addr;

			if ((uid - pluid) == 0)
				cnt++;
		}
		mapit_free(iter);
	}

	script_pushint(st, cnt);
	return 0;
}

BUILDIN_FUNC(mclose) 
{
	TBL_PC* sd;
	script_rid2sd(sd);
	if( sd == NULL )
		return SCRIPT_CMD_SUCCESS;

	clif_scriptmes(sd, st->oid, script_getstr(st, 2));
	st->state = END;
	clif_scriptclose(sd, st->oid);
	return SCRIPT_CMD_SUCCESS;
}

/* Starts a status effect on the target unit or on the attached player.
 *
 * sc_start_area  <effect_id>,<duration>,<val1>{,<rate>,<flag>,{<unit_id>}};
 * sc_start_area2 <effect_id>,<duration>,<val1>,<val2>{,<rate,<flag>,{<unit_id>}};
 * sc_start_area4 <effect_id>,<duration>,<val1>,<val2>,<val3>,<val4>{,<rate,<flag>,{<unit_id>}};
 * <flag>: enum e_status_change_start_flags
 */
BUILDIN_FUNC(sc_start_area)
{
	TBL_NPC* nd = map_id2nd(st->oid);
	struct block_list* bl;
	enum sc_type type;
	int tick, val1, val2, val3, val4 = 0, rate, flag;
	char start_type;
	const char* command = script_getfuncname(st);

	if (strstr(command, "4"))
		start_type = 4;
	else if (strstr(command, "2"))
		start_type = 2;
	else
		start_type = 1;

	type = (sc_type)script_getnum(st, 2);
	tick = script_getnum(st, 3);
	val1 = script_getnum(st, 4);

	//If from NPC we make default flag 1 to be unavoidable
	if (nd && nd->bl.id == fake_nd->bl.id)
		flag = script_hasdata(st, 5 + start_type) ? script_getnum(st, 5 + start_type) : SCSTART_NOTICKDEF;
	else
		flag = script_hasdata(st, 5 + start_type) ? script_getnum(st, 5 + start_type) : SCSTART_NOAVOID;

	rate = script_hasdata(st, 4 + start_type) ? min(script_getnum(st, 4 + start_type), 10000) : 10000;

	if (script_hasdata(st, (6 + start_type)))
		bl = map_id2bl(script_getnum(st, (6 + start_type)));
	else
		bl = map_id2bl(st->rid);

	if (tick == 0 && val1 > 0 && type > SC_NONE && type < SC_MAX && status_db.getSkill(type) != 0)
	{// When there isn't a duration specified, try to get it from the skill_db
		tick = skill_get_time(status_db.getSkill(type), val1);
	}

	if (potion_flag == 1 && potion_target) { //skill.cpp set the flags before running the script, this is a potion-pitched effect.
		bl = map_id2bl(potion_target);
		tick /= 2;// Thrown potions only last half.
		val4 = 1;// Mark that this was a thrown sc_effect
	}

	if (!bl)
		return SCRIPT_CMD_SUCCESS;

	switch (start_type) {
	case 1:
		map_foreachinrange(status_change_start_sub, bl, 10, BL_CHAR, bl, type, rate, val1, 0, 0, val4, tick, flag);
		break;
	case 2:
		val2 = script_getnum(st, 5);
		map_foreachinrange(status_change_start_sub, bl, 10, BL_CHAR, bl, type, rate, val1, val2, 0, val4, tick, flag);
		break;
	case 4:
		val2 = script_getnum(st, 5);
		val3 = script_getnum(st, 6);
		val4 = script_getnum(st, 7);
		map_foreachinrange(status_change_start_sub, bl, 10, BL_CHAR, bl, type, rate, val1, val2, val3, val4, tick, flag);
		break;
	}
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(gethotkeys)
{
	TBL_PC *sd;
	TBL_PC *tsd;
	int i;

	if( !script_rid2sd(sd) )
		return SCRIPT_CMD_FAILURE;

	if (!script_charid2sd(2,tsd))
		return SCRIPT_CMD_FAILURE;

	for(i=0;i<MAX_HOTKEYS;i++){
		pc_setreg(sd,reference_uid(add_str("@hotkey_type"), i),tsd->status.hotkeys[i].type);
		pc_setreg(sd,reference_uid(add_str("@hotkey_id"), i),tsd->status.hotkeys[i].id);
		pc_setreg(sd,reference_uid(add_str("@hotkey_lv"), i),tsd->status.hotkeys[i].lv);
		pc_setreg(sd,reference_uid(add_str("@hotkey_pos"), i),i);
	}
	pc_setreg(sd,add_str("@hotkey_count"),i);

	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(updatehotkey)
{
	TBL_PC *sd;
	int type,id,lv,pos,j, flag = 0;
	short i;
	struct script_data *data;

	if (!script_charid2sd(7,sd))
		return SCRIPT_CMD_FAILURE;

	type = script_getnum(st,2);
	if(type < 0 || type > 1){
		clif_displaymessage(sd->fd, "Update Hotkey failed. Invalid type.");
		return SCRIPT_CMD_FAILURE;
	}
	data = script_getdata(st, 3);
	get_val(st, data); // Convert into value in case of a variable
	id = ( data_isstring(data) ? skill_name2id(script_getstr(st,3)) : script_getnum(st,3) );
	lv = script_getnum(st,4);
	pos = script_getnum(st,5);
	if(pos < 0 || pos >= MAX_HOTKEYS){
		clif_displaymessage(sd->fd, "Update Hotkey failed. Invalid hotkey position.");
		return SCRIPT_CMD_FAILURE;
	}
	if( script_hasdata(st,6) )
		flag = script_getnum(st,6);

	if(type == 0 && id > 0){ // item
		ARR_FIND(0, MAX_INVENTORY, i, sd->inventory.u.items_inventory[i].nameid == id);
		if(i<MAX_INVENTORY){
			if(sd->inventory.u.items_inventory[i].amount != lv)
				lv = sd->inventory.u.items_inventory[i].amount;
		} else
			lv = 0;
	} else if(type == 1 && id > 0){ // skill
		j = pc_checkskill(sd,id);
		if(j < lv)
			lv = j;
	}

	if(flag == 1){ // Delete duplicate on same row
		i = 0;
		if(pos >= 0 && pos <= 8){
			ARR_FIND(0, 9, i, sd->status.hotkeys[i].type == type && sd->status.hotkeys[i].id == id && sd->status.hotkeys[i].lv == lv);
			if(i < 9){
				sd->status.hotkeys[i].type = 0;
				sd->status.hotkeys[i].id = 0;
				sd->status.hotkeys[i].lv = 0;
			}
		} else if(pos >= 9 && pos <= 17){
			ARR_FIND(9, 18, i, sd->status.hotkeys[i].type == type && sd->status.hotkeys[i].id == id && sd->status.hotkeys[i].lv == lv);
			if(i < 18){
				sd->status.hotkeys[i].type = 0;
				sd->status.hotkeys[i].id = 0;
				sd->status.hotkeys[i].lv = 0;
			}
		} else if(pos >= 18 && pos <= 26){
			ARR_FIND(18, 27, i, sd->status.hotkeys[i].type == type && sd->status.hotkeys[i].id == id && sd->status.hotkeys[i].lv == lv);
			if(i < 27){
				sd->status.hotkeys[i].type = 0;
				sd->status.hotkeys[i].id = 0;
				sd->status.hotkeys[i].lv = 0;
			}
		}
#if PACKETVER >= 20090603
		else if(pos >= 27 && pos <= 35){
			ARR_FIND(27, 36, i, sd->status.hotkeys[i].type == type && sd->status.hotkeys[i].id == id && sd->status.hotkeys[i].lv == lv);
			if(i < 36){
				sd->status.hotkeys[i].type = 0;
				sd->status.hotkeys[i].id = 0;
				sd->status.hotkeys[i].lv = 0;
			}
		}
#endif
#if PACKETVER >= 20090617
		else if(pos >= 36 && pos < MAX_HOTKEYS){
			ARR_FIND(36, MAX_HOTKEYS, i, sd->status.hotkeys[i].type == type && sd->status.hotkeys[i].id == id && sd->status.hotkeys[i].lv == lv);
			if(i < MAX_HOTKEYS){
				sd->status.hotkeys[i].type = 0;
				sd->status.hotkeys[i].id = 0;
				sd->status.hotkeys[i].lv = 0;
			}
		}
#endif
	} else if(flag == 2){ // Delete duplicate on all rows
		for(i = 0; i < MAX_HOTKEYS; i++){
			if(sd->status.hotkeys[i].type == type && sd->status.hotkeys[i].id == id && sd->status.hotkeys[i].lv == lv){
				sd->status.hotkeys[i].type = 0;
				sd->status.hotkeys[i].id = 0;
				sd->status.hotkeys[i].lv = 0;
			}
		}
	}

	sd->status.hotkeys[pos].type = type;
	sd->status.hotkeys[pos].id = id;
	sd->status.hotkeys[pos].lv = lv;

	clif_hotkeys_send(sd,0);

	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(clearhotkeys)
{
	TBL_PC *sd;
	int i;

	if (!script_charid2sd(2,sd))
		return SCRIPT_CMD_FAILURE;

	for(i=0;i<MAX_HOTKEYS;i++){
		sd->status.hotkeys[i].type = 0;
		sd->status.hotkeys[i].id = 0;
		sd->status.hotkeys[i].lv = 0;
	}
	clif_hotkeys_send(sd,0);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(setunmount) {
	TBL_PC* sd;

	if (!script_charid2sd(2, sd))
		return SCRIPT_CMD_FAILURE;

	if (sd->sc.data[SC_ALL_RIDING]) {
		status_change_end(&sd->bl, SC_ALL_RIDING, INVALID_TIMER); //release mount
	}

	script_pushint(st, 1);//in both cases, return 1.
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(title) 
{
	const char *NPC_NAME = "[^8B008BroArc^000000]";
	char NPC[CHAT_SIZE_MAX];
	TBL_PC* sd;
	script_rid2sd(sd);
	if( sd == NULL )
		return SCRIPT_CMD_SUCCESS;
	if ( script_hasdata(st,2) )
		sprintf(NPC,"[^8B008B %s ^000000]",script_getstr(st,2));
	else
		sprintf(NPC,"[^8B008B %s ^000000]",NPC_NAME);
	
	clif_scriptmes(sd, st->oid, NPC);
	// clif_scriptmes(sd, st->oid, " ");
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Get the damage done to the specified mob_id
 * getmonsterdamage <mob id>;
 * by [Haruka Mayumi]
 *------------------------------------------*/
BUILDIN_FUNC(getmonsterdamage)
{
	struct block_list* bl;
	uint8 i;
	
	if(!script_rid2bl(2,bl))
	{
		script_pushint(st, -1);
		return SCRIPT_CMD_FAILURE;
	}
	
	if(bl->type != BL_MOB) {
		ShowWarning("buildin_getmonsterdamage: Invalid object type!\n");
		return SCRIPT_CMD_FAILURE;
	}
	
	TBL_MOB* md = map_id2md(bl->id);
	
	for (i = 0; i < DAMAGELOG_SIZE && md->dmglog[i].id; i++) {
		setd_sub_num(st, NULL, ".@dmglog_id", i, md->dmglog[i].id, NULL);
		setd_sub_num(st, NULL, ".@dmglog_dmg", i, md->dmglog[i].dmg, NULL);
		setd_sub_num(st, NULL, ".@dmglog_flag", i, md->dmglog[i].flag, NULL);
	}
	
	script_pushint(st, i);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(homload)
{
	int hom_id;
	TBL_PC *sd;
	
	if(!script_rid2sd(sd))
		return SCRIPT_CMD_SUCCESS;
	
	if(!pc_checkskill(sd,238)){
		script_pushint(st, 0);
		return SCRIPT_CMD_SUCCESS;
	}
	
	hom_id = script_getnum(st,2);
	
	if(sd->hd != NULL){
		hom_save(sd->hd);
		hom_delete(sd->hd, -1);
	}
	memset(&sd->hd,0,sizeof(struct homun_data));
	sd->status.hom_id = hom_id;
	hom_call(sd);
	
	script_pushint(st, 1);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(homsave)
{
	TBL_PC *sd;
	if(!script_rid2sd(sd))
		return SCRIPT_CMD_SUCCESS;
	
	if(!pc_checkskill(sd,238)){
		script_pushint(st, 0);
		return SCRIPT_CMD_SUCCESS;
	}
	
	if(sd->hd != NULL){
		hom_save(sd->hd);
		hom_delete(sd->hd, -1);
	}
	memset(&sd->hd,0,sizeof(struct homun_data));
	script_pushint(st, 1);
	return SCRIPT_CMD_SUCCESS;
}
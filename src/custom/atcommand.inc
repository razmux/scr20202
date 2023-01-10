// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

/**
 * Custom Atcommands
 * Place the body of custom atcommands in this file.
 * Format:
 *	ACMD_FUNC(command_name)
 *	{
 *		<code>;
 *	}
 **/

//ACMD_FUNC(newcommand)
//{
//	clif_displaymessage(fd, "It works!");
//	clif_specialeffect(&sd->bl, EF_HEARTCASTING, AREA);
//	return 0;
//}

ACMD_FUNC(battleinfo)
{
	if( sd->state.battleinfo )
	{
		clif_displaymessage(fd, "- Battle Information Display OFF - Kill/Death -");
		sd->state.battleinfo = 0;
	}
	else
	{
		clif_displaymessage(fd, "- Battle Information Display ON - Kill/Death -");
		sd->state.battleinfo = 1;
	}
	return 0;
}

ACMD_FUNC(dance) 
{
   nullpo_retr(-1, sd);

	if (!message || !*message || atoi(message) > 9 || atoi(message) < 1 ) {
		clif_displaymessage(fd, "@dance (1/9)");
		return -1;
	}
	switch ( atoi(message) ) {
		case 1: clif_specialeffect(&sd->bl, 413, ALL_CLIENT); break;
		case 2: clif_specialeffect(&sd->bl, 414, ALL_CLIENT); break;
		case 3: clif_specialeffect(&sd->bl, 415, ALL_CLIENT); break;
		case 4: clif_specialeffect(&sd->bl, 426, ALL_CLIENT); break;
		case 5: clif_specialeffect(&sd->bl, 458, ALL_CLIENT); break;
		case 6: clif_specialeffect(&sd->bl, 466, ALL_CLIENT); break;
		case 7: clif_specialeffect(&sd->bl, 501, ALL_CLIENT); break;
		case 8: clif_specialeffect(&sd->bl, 540, ALL_CLIENT); break;
		case 9: clif_specialeffect(&sd->bl, 550, ALL_CLIENT); break;
	}
   return 0;
}
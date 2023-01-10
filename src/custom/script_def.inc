// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

/**
 * Custom Script Commands
 * Place the definition of custom script commands in this file.
 * Format:
 *	BUILDIN_DEF(command_name,"parameters"),
 **/

//BUILDIN_DEF(example,""),
BUILDIN_DEF(is_security,"?"), //@security [Cydh]
BUILDIN_DEF(setsecurity,"i?"), //@security [Cydh]

// Instance Hard Mod!
BUILDIN_DEF(instance_setdifficulty,"ii"),
BUILDIN_DEF(instance_getdifficulty,"i"),
BUILDIN_DEF(instance_setpenalty,"iii"),

// Marioo Kart Others
BUILDIN_DEF(arthurdev_map_multicount, "s"),

BUILDIN_DEF(sc_start_area, "iii???"),
BUILDIN_DEF2(sc_start_area, "sc_start_area2", "iiii???"),
BUILDIN_DEF2(sc_start_area, "sc_start_area4", "iiiiii???"),
BUILDIN_DEF(mclose,"s"),
BUILDIN_DEF(gethotkeys,"?"),
BUILDIN_DEF(updatehotkey,"ivii??"),
BUILDIN_DEF(clearhotkeys,"?"),
BUILDIN_DEF(setunmount, "?"),//

BUILDIN_DEF(title,"?"),

// Worldboss
BUILDIN_DEF(getmonsterdamage,"i"),

// Homuculus Save
BUILDIN_DEF(homload,"i"),
BUILDIN_DEF(homsave,""),
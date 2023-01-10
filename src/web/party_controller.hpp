/*==========================================
 * Adventurer's Agency 
 *------------------------------------------*/

#ifndef PARTY_CONTROLLER_HPP
#define PARTY_CONTROLLER_HPP

#include "http.hpp"

HANDLER_FUNC(party_add);
HANDLER_FUNC(party_del);
HANDLER_FUNC(party_get);
HANDLER_FUNC(party_list);
HANDLER_FUNC(party_search);

void do_init_party(void);

#endif
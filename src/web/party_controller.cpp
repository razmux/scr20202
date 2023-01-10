/*==========================================
 * Adventurer's Agency [Shakto]
 *------------------------------------------*/

#include "party_controller.hpp"

#include <string>

#include "../common/showmsg.hpp"
#include "../common/sql.hpp"
#include "../common/strlib.hpp"

#include "auth.hpp"
#include "http.hpp"
#include "sqllock.hpp"
#include "webutils.hpp"
#include "web.hpp"

#define PARTY_MAX_RESULT 10

HANDLER_FUNC(party_list) {
	if (!req.has_file("AID") || !req.has_file("WorldName") || !req.has_file("page")) {
		res.status = 400;
		res.set_content("Error", "text/plain");
		return;
	}

	auto world_name_str = req.get_file_value("WorldName").content;
	auto world_name = world_name_str.c_str();
	auto page = std::stoi(req.get_file_value("page").content);
	std::ostringstream stream;
	StringBuf buf;
	StringBuf_Init(&buf);

	SQLLock sl(WEB_SQL_LOCK);
	sl.lock();
	auto handle = sl.getHandle();
	SqlStmt* stmt = SqlStmt_Malloc(handle);
	if (SQL_SUCCESS != SqlStmt_Prepare(stmt,
		"SELECT 1 FROM `%s` WHERE (`world_name` = ?)",
		party_agency_table)
		|| SQL_SUCCESS != SqlStmt_BindParam(stmt, 0, SQLDT_STRING, (void*)world_name, strlen(world_name))
		|| SQL_SUCCESS != SqlStmt_Execute(stmt)
		) {
		SqlStmt_ShowDebug(stmt);
		SqlStmt_Free(stmt);
		StringBuf_Destroy(&buf);
		sl.unlock();
		res.status = 400;
		res.set_content("Error", "text/plain");
		return;
	}

	uint16 offset = (page * 10) - 10;

	if (SqlStmt_NumRows(stmt) <= 0 || SqlStmt_NumRows(stmt) < (offset + 1)) {
		SqlStmt_Free(stmt);
		StringBuf_Destroy(&buf);
		sl.unlock();
		res.set_content("{\"totalPage\":0,\"data\":[],\"Type\":1}", "application/json");
		return;
	}
	uint64 total_page = (SqlStmt_NumRows(stmt) / 10) + (SqlStmt_NumRows(stmt) % 10 > 0 ? 1 : 0);

	StringBuf_Clear(&buf);
	StringBuf_Printf(&buf, "SELECT `account_id`,`char_id`,`CharName`,`MinLV`,`MaxLV`,`Tanker`,`Healer`,`Dealer`,`Assist`,`Memo`,`Type` FROM `%s` WHERE (", party_agency_table);
	StringBuf_Printf(&buf, " `world_name` = '%s'", world_name);
	StringBuf_AppendStr(&buf, ")");
	if (SqlStmt_NumRows(stmt) < PARTY_MAX_RESULT)
		StringBuf_Printf(&buf, " LIMIT %d", PARTY_MAX_RESULT);
	else
		StringBuf_Printf(&buf, " LIMIT %d, %d", offset, PARTY_MAX_RESULT);

	if (SQL_SUCCESS != SqlStmt_Prepare(stmt, StringBuf_Value(&buf))
		|| SQL_SUCCESS != SqlStmt_Execute(stmt)
		) {
		SqlStmt_ShowDebug(stmt);
		SqlStmt_Free(stmt);
		StringBuf_Destroy(&buf);
		sl.unlock();
		res.status = 400;
		res.set_content("Error", "text/plain");
		return;
	}
	StringBuf_Destroy(&buf);

	char CharName[NAME_LENGTH];
	char Memo[255];
	uint32 account_id, char_id;
	uint32 MinLV, MaxLV;
	uint32 Tanker, Healer, Dealer, Assist;
	uint32 Type;
	uint16 index = 0;

	SqlStmt_BindColumn(stmt, 0, SQLDT_UINT, &account_id, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 1, SQLDT_UINT, &char_id, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 2, SQLDT_STRING, &CharName, sizeof(CharName), NULL, NULL);
	SqlStmt_BindColumn(stmt, 3, SQLDT_UINT, &MinLV, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 4, SQLDT_UINT, &MaxLV, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 5, SQLDT_INT, &Tanker, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 6, SQLDT_INT, &Healer, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 7, SQLDT_INT, &Dealer, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 8, SQLDT_INT, &Assist, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 9, SQLDT_STRING, &Memo, sizeof(Memo), NULL, NULL);
	SqlStmt_BindColumn(stmt, 10, SQLDT_INT, &Type, 0, NULL, NULL);

	stream << "{\"totalPage\":" << total_page << ",";
	stream << "\"data\":[";
	while (SQL_SUCCESS == SqlStmt_NextRow(stmt)) {
		index++;
		stream << "{\"AID\":" << account_id;
		stream << ",\"GID\":" << char_id;
		stream << ",\"CharName\":\"" << CharName << "\"";
		stream << ",\"WorldName\":\"" << world_name << "\"";
		stream << ",\"Tanker\":" << Tanker << ",\"Dealer\":" << Dealer << ",\"Healer\":" << Healer << ",\"Assist\":" << Assist;
		stream << ",\"MinLV\":" << MinLV << ",\"MaxLV\":" << MaxLV;
		stream << ",\"Type\":" << Type;
		stream << ",\"Memo\":\"" << Memo << "\"}";
		if (index < SqlStmt_NumRows(stmt))
			stream << ",";
	}
	stream << "],\"Type\":" << Type << "}";

	SqlStmt_Free(stmt);
	sl.unlock();

	res.set_content(stream.str(), "application/json");
}

HANDLER_FUNC(party_add) {
	bool fail = false;
	if (!req.has_file("GID")) {
		ShowDebug("Missing CharID field for party add.\n");
		fail = true;
	}
	if (!req.has_file("WorldName")) {
		ShowDebug("Missing WorldName field for party add.\n");
		fail = true;
	}
	if (!req.has_file("CharName")) {
		ShowDebug("Missing CharName field for party add.\n");
		fail = true;
	}
	if (!req.has_file("MinLV")) {
		ShowDebug("Missing MinLV field for party add.\n");
		fail = true;
	}
	if (!req.has_file("MaxLV")) {
		ShowDebug("Missing MaxLV field for party add.\n");
		fail = true;
	}
	if (!req.has_file("Tanker")) {
		ShowDebug("Missing Tanker field for party add.\n");
		fail = true;
	}
	if (!req.has_file("Healer")) {
		ShowDebug("Missing Healer field for party add.\n");
		fail = true;
	}
	if (!req.has_file("Dealer")) {
		ShowDebug("Missing Dealer field for party add.\n");
		fail = true;
	}
	if (!req.has_file("Assist")) {
		ShowDebug("Missing Assist field for party add.\n");
		fail = true;
	}
	if (!req.has_file("Type")) {
		ShowDebug("Missing Type field for party add.\n");
		fail = true;
	}
	if (!req.has_file("Memo")) {
		ShowDebug("Missing Memo field for party add.\n");
		fail = true;
	}
	if (fail) {
		res.status = 400;
		res.set_content("Error", "text/plain");
		return;
	}

	auto account_id = std::stoi(req.get_file_value("AID").content);
	auto char_id = std::stoi(req.get_file_value("GID").content);
	auto world_name_str = req.get_file_value("WorldName").content;
	auto world_name = world_name_str.c_str();
	auto CharName_str = req.get_file_value("CharName").content;
	auto CharName = CharName_str.c_str();
	auto MinLV = std::stoi(req.get_file_value("MinLV").content);
	auto MaxLV = std::stoi(req.get_file_value("MaxLV").content);
	auto Tanker = std::stoi(req.get_file_value("Tanker").content);
	auto Healer = std::stoi(req.get_file_value("Healer").content);
	auto Dealer = std::stoi(req.get_file_value("Dealer").content);
	auto Assist = std::stoi(req.get_file_value("Assist").content);
	auto Memo_str = req.get_file_value("Memo").content;
	auto Memo = Memo_str.c_str();
	auto Type = std::stoi(req.get_file_value("Type").content);
	std::string data;

	SQLLock sl(WEB_SQL_LOCK);
	sl.lock();
	auto handle = sl.getHandle();
	SqlStmt* stmt = SqlStmt_Malloc(handle);
	if (SQL_SUCCESS != SqlStmt_Prepare(stmt,
		"REPLACE INTO `%s` (`account_id`, `char_id`, `world_name`, `CharName`, `MinLV`, `MaxLV`, `Tanker`, `Healer`, `Dealer`, `Assist`, `Memo`, `Type`) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
		party_agency_table)
		|| SQL_SUCCESS != SqlStmt_BindParam(stmt, 0, SQLDT_UINT32, &account_id, sizeof(account_id))
		|| SQL_SUCCESS != SqlStmt_BindParam(stmt, 1, SQLDT_UINT32, &char_id, sizeof(char_id))
		|| SQL_SUCCESS != SqlStmt_BindParam(stmt, 2, SQLDT_STRING, (void*)world_name, strlen(world_name))
		|| SQL_SUCCESS != SqlStmt_BindParam(stmt, 3, SQLDT_STRING, (void*)CharName, strlen(CharName))
		|| SQL_SUCCESS != SqlStmt_BindParam(stmt, 4, SQLDT_UINT16, &MinLV, sizeof(MinLV))
		|| SQL_SUCCESS != SqlStmt_BindParam(stmt, 5, SQLDT_UINT16, &MaxLV, sizeof(MaxLV))
		|| SQL_SUCCESS != SqlStmt_BindParam(stmt, 6, SQLDT_INT8, &Tanker, sizeof(Tanker))
		|| SQL_SUCCESS != SqlStmt_BindParam(stmt, 7, SQLDT_INT8, &Healer, sizeof(Healer))
		|| SQL_SUCCESS != SqlStmt_BindParam(stmt, 8, SQLDT_INT8, &Dealer, sizeof(Dealer))
		|| SQL_SUCCESS != SqlStmt_BindParam(stmt, 9, SQLDT_INT8, &Assist, sizeof(Assist))
		|| SQL_SUCCESS != SqlStmt_BindParam(stmt, 10, SQLDT_STRING, (void*)Memo, strlen(Memo))
		|| SQL_SUCCESS != SqlStmt_BindParam(stmt, 11, SQLDT_INT16, &Type, sizeof(Type))
		|| SQL_SUCCESS != SqlStmt_Execute(stmt)
		) {
		SqlStmt_ShowDebug(stmt);
		SqlStmt_Free(stmt);
		sl.unlock();
		res.status = 400;
		res.set_content("Error", "text/plain");
		return;
	}

	SqlStmt_Free(stmt);
	sl.unlock();

	data = "{\"Type\": 1}";
	res.set_content(data, "application/json");
}

HANDLER_FUNC(party_del) {
	bool fail = false;
	if (!req.has_file("WorldName")) {
		ShowDebug("Missing WorldName field for party del.\n");
		fail = true;
	}
	if (!req.has_file("MasterAID")) {
		ShowDebug("Missing MasterAID field for party del.\n");
		fail = true;
	}
	if (fail) {
		res.status = 400;
		res.set_content("Error", "text/plain");
		return;
	}

	auto account_id = std::stoi(req.get_file_value("AID").content);
	auto world_name_str = req.get_file_value("WorldName").content;
	auto world_name = world_name_str.c_str();
	auto MasterAID = std::stoi(req.get_file_value("MasterAID").content);

	std::string data;

	SQLLock sl(WEB_SQL_LOCK);
	sl.lock();
	auto handle = sl.getHandle();
	SqlStmt* stmt = SqlStmt_Malloc(handle);
	if (SQL_SUCCESS != SqlStmt_Prepare(stmt,
		"DELETE FROM `%s` WHERE ( `account_id` = ? AND `world_name` = ? )",
		party_agency_table)
		|| SQL_SUCCESS != SqlStmt_BindParam(stmt, 0, SQLDT_UINT32, &MasterAID, sizeof(MasterAID))
		|| SQL_SUCCESS != SqlStmt_BindParam(stmt, 1, SQLDT_STRING, (void*)world_name, strlen(world_name))
		|| SQL_SUCCESS != SqlStmt_Execute(stmt)
		) {
		SqlStmt_ShowDebug(stmt);
		SqlStmt_Free(stmt);
		sl.unlock();
		res.status = 400;
		res.set_content("Error", "text/plain");
		return;
	}

	SqlStmt_Free(stmt);
	sl.unlock();

	data = "{\"Type\": 1}";
	res.set_content(data, "application/json");
}

HANDLER_FUNC(party_get) {
	if (!req.has_file("AID") || !req.has_file("WorldName")) {
		res.status = 400;
		res.set_content("Error", "text/plain");
		return;
	}

	auto account_id = std::stoi(req.get_file_value("AID").content);
	auto world_name_str = req.get_file_value("WorldName").content;
	auto world_name = world_name_str.c_str();
	std::ostringstream stream;

	SQLLock sl(WEB_SQL_LOCK);
	sl.lock();
	auto handle = sl.getHandle();
	SqlStmt* stmt = SqlStmt_Malloc(handle);
	if (SQL_SUCCESS != SqlStmt_Prepare(stmt, "SELECT "
		"`char_id`,"
		"`CharName`,"
		"`MinLV`,`MaxLV`,"
		"`Tanker`,`Healer`,`Dealer`,`Assist`,"
		"`Memo`,"
		"`Type`"
		" FROM `%s`"
		" WHERE (`account_id` = ? AND `world_name` = ?) LIMIT 1",
		party_agency_table)
		|| SQL_SUCCESS != SqlStmt_BindParam(stmt, 0, SQLDT_UINT32, &account_id, sizeof(account_id))
		|| SQL_SUCCESS != SqlStmt_BindParam(stmt, 1, SQLDT_STRING, (void*)world_name, strlen(world_name))
		|| SQL_SUCCESS != SqlStmt_Execute(stmt)
		) {
		SqlStmt_ShowDebug(stmt);
		SqlStmt_Free(stmt);
		sl.unlock();
		res.status = 400;
		res.set_content("Error", "text/plain");
		return;
	}

	if (SqlStmt_NumRows(stmt) <= 0) {
		SqlStmt_Free(stmt);
		sl.unlock();
		res.set_content("{\"Type\":1}", "application/json");
		return;
	}

	char CharName[NAME_LENGTH];
	char Memo[NAME_LENGTH];
	uint32 char_id;
	uint32 MinLV, MaxLV;
	uint32 Tanker, Healer, Dealer, Assist;
	uint32 Type;

	SqlStmt_BindColumn(stmt, 0, SQLDT_UINT, &char_id, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 1, SQLDT_STRING, &CharName, sizeof(CharName), NULL, NULL);
	SqlStmt_BindColumn(stmt, 2, SQLDT_UINT, &MinLV, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 3, SQLDT_UINT, &MaxLV, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 4, SQLDT_INT, &Tanker, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 5, SQLDT_INT, &Healer, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 6, SQLDT_INT, &Dealer, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 7, SQLDT_INT, &Assist, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 8, SQLDT_STRING, &Memo, sizeof(Memo), NULL, NULL);
	SqlStmt_BindColumn(stmt, 9, SQLDT_INT, &Type, 0, NULL, NULL);

	stream << "{\"Type\":1,";
	stream << "\"data\":";
	if (SQL_SUCCESS == SqlStmt_NextRow(stmt)) {
		stream << "{\"AID\":" << account_id;
		stream << ",\"GID\":" << char_id;
		stream << ",\"CharName\":\"" << CharName << "\"";
		stream << ",\"WorldName\":\"" << world_name << "\"";
		stream << ",\"Tanker\":" << Tanker << ",\"Dealer\":" << Dealer << ",\"Healer\":" << Healer << ",\"Assist\":" << Assist;
		stream << ",\"MinLV\":" << MinLV << ",\"MaxLV\":" << MaxLV;
		stream << ",\"Type\":" << Type;
		stream << ",\"Memo\":\"" << Memo << "\"}";
	}
	stream << "}";

	SqlStmt_Free(stmt);
	sl.unlock();

	res.set_content(stream.str(), "application/json");
}

HANDLER_FUNC(party_search) {
	bool fail = false;
	if (!req.has_file("GID")) {
		ShowDebug("Missing CharID field for party add.\n");
		fail = true;
	}
	if (!req.has_file("WorldName")) {
		ShowDebug("Missing WorldName field for party add.\n");
		fail = true;
	}
	if (!req.has_file("MinLV")) {
		ShowDebug("Missing MinLV field for party add.\n");
		fail = true;
	}
	if (!req.has_file("MaxLV")) {
		ShowDebug("Missing MaxLV field for party add.\n");
		fail = true;
	}
	if (fail) {
		res.status = 400;
		res.set_content("Error", "text/plain");
		return;
	}

	auto account_id = std::stoi(req.get_file_value("AID").content);
	auto char_id = std::stoi(req.get_file_value("GID").content);
	auto world_name_str = req.get_file_value("WorldName").content;
	auto world_name = world_name_str.c_str();
	auto page = std::stoi(req.get_file_value("page").content);
	auto MinLV = std::stoi(req.get_file_value("MinLV").content);
	auto MaxLV = std::stoi(req.get_file_value("MaxLV").content);
	uint32 Tanker = 0, Healer = 0, Dealer = 0, Assist = 0;
	uint32 Type = 0;
	std::string Memo;
	std::ostringstream stream;
	StringBuf buf;
	StringBuf_Init(&buf);

	if (req.has_file("Tanker")) {
		Tanker = std::stoi(req.get_file_value("Tanker").content);
	}
	if (req.has_file("Healer")) {
		Healer = std::stoi(req.get_file_value("Healer").content);
	}
	if (req.has_file("Dealer")) {
		Dealer = std::stoi(req.get_file_value("Dealer").content);
	}
	if (req.has_file("Assist")) {
		Assist = std::stoi(req.get_file_value("Assist").content);
	}
	if (req.has_file("Memo")) {
		Memo = req.get_file_value("Memo").content;
	}
	if (req.has_file("Type")) {
		Type = std::stoi(req.get_file_value("Type").content);
	}

	SQLLock sl(WEB_SQL_LOCK);
	sl.lock();
	auto handle = sl.getHandle();
	SqlStmt* stmt = SqlStmt_Malloc(handle);

	StringBuf_Clear(&buf);
	StringBuf_Printf(&buf, "SELECT 1 FROM `%s` WHERE (", party_agency_table);
	StringBuf_Printf(&buf, " `world_name` = '%s'", world_name);
	StringBuf_Printf(&buf, " AND `MinLV` <= %d AND `MaxLV` >= %d", MinLV, MaxLV);
	if (Tanker)
		StringBuf_AppendStr(&buf, " AND `Tanker` = 1");
	if (Healer)
		StringBuf_AppendStr(&buf, " AND `Healer` = 1");
	if (Dealer)
		StringBuf_AppendStr(&buf, " AND `Dealer` = 1");
	if (Assist)
		StringBuf_AppendStr(&buf, " AND `Assist` = 1");
	if (!Memo.empty()) {
		StringBuf_AppendStr(&buf, " AND `Memo` LIKE '%%");
		StringBuf_Printf(&buf, "%s%%'", Memo.c_str());
	}
	if (Type)
		StringBuf_Printf(&buf, " AND `Type` = %d", Type);
	StringBuf_AppendStr(&buf, ")");

	if (SQL_SUCCESS != SqlStmt_Prepare(stmt, StringBuf_Value(&buf))
		|| SQL_SUCCESS != SqlStmt_Execute(stmt)
		) {
		SqlStmt_ShowDebug(stmt);
		SqlStmt_Free(stmt);
		StringBuf_Destroy(&buf);
		sl.unlock();
		res.status = 400;
		res.set_content("Error", "text/plain");
		return;
	}

	uint16 offset = (page * 10) - 10;

	if (SqlStmt_NumRows(stmt) <= 0 || SqlStmt_NumRows(stmt) < offset + 1) {
		SqlStmt_Free(stmt);
		StringBuf_Destroy(&buf);
		sl.unlock();
		res.set_content("{\"totalPage\":0,\"data\":[],\"Type\":1}", "application/json");
		return;
	}

	uint64 total_page = SqlStmt_NumRows(stmt) / 10 + (SqlStmt_NumRows(stmt) % 10 > 0 ? 1 : 0);

	StringBuf_Clear(&buf);
	StringBuf_Printf(&buf, "SELECT `account_id`,`char_id`,`CharName`,`MinLV`,`MaxLV`,`Tanker`,`Healer`,`Dealer`,`Assist`,`Memo`,`Type` FROM `%s` WHERE (", party_agency_table);
	StringBuf_Printf(&buf, " `world_name` = '%s'", world_name);
	StringBuf_Printf(&buf, " AND `MinLV` <= %d AND `MaxLV` >= %d", MinLV, MaxLV);
	if (Tanker)
		StringBuf_AppendStr(&buf, " AND `Tanker` = 1");
	if (Healer)
		StringBuf_AppendStr(&buf, " AND `Healer` = 1");
	if (Dealer)
		StringBuf_AppendStr(&buf, " AND `Dealer` = 1");
	if (Assist)
		StringBuf_AppendStr(&buf, " AND `Assist` = 1");
	if (!Memo.empty()) {
		StringBuf_AppendStr(&buf, " AND `Memo` LIKE '%%");
		StringBuf_Printf(&buf, "%s%%'", Memo.c_str());
	}
	if (Type)
		StringBuf_Printf(&buf, " AND `Type` = %d", Type);
	StringBuf_AppendStr(&buf, ")");
	if (SqlStmt_NumRows(stmt) < PARTY_MAX_RESULT)
		StringBuf_Printf(&buf, " LIMIT %d", PARTY_MAX_RESULT);
	else
		StringBuf_Printf(&buf, " LIMIT %d, %d", offset, PARTY_MAX_RESULT);

	if (SQL_SUCCESS != SqlStmt_Prepare(stmt, StringBuf_Value(&buf))
		|| SQL_SUCCESS != SqlStmt_Execute(stmt)
		) {
		SqlStmt_ShowDebug(stmt);
		SqlStmt_Free(stmt);
		StringBuf_Destroy(&buf);
		sl.unlock();
		res.status = 400;
		res.set_content("Error", "text/plain");
		return;
	}
	StringBuf_Destroy(&buf);

	char CharName[NAME_LENGTH];
	char r_Memo[255];
	uint32 r_account_id, r_char_id;
	uint32 r_MinLV, r_MaxLV;
	uint32 r_Tanker, r_Healer, r_Dealer, r_Assist;
	uint16 index = 0;

	SqlStmt_BindColumn(stmt, 0, SQLDT_UINT, &r_account_id, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 1, SQLDT_UINT, &r_char_id, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 2, SQLDT_STRING, &CharName, sizeof(CharName), NULL, NULL);
	SqlStmt_BindColumn(stmt, 3, SQLDT_UINT, &r_MinLV, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 4, SQLDT_UINT, &r_MaxLV, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 5, SQLDT_INT, &r_Tanker, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 6, SQLDT_INT, &r_Healer, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 7, SQLDT_INT, &r_Dealer, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 8, SQLDT_INT, &r_Assist, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 9, SQLDT_STRING, &r_Memo, sizeof(r_Memo), NULL, NULL);
	SqlStmt_BindColumn(stmt, 10, SQLDT_INT, &Type, 0, NULL, NULL);

	stream << "{\"totalPage\":" << total_page << ",";
	stream << "\"data\":[";
	while (SQL_SUCCESS == SqlStmt_NextRow(stmt)) {
		index++;
		stream << "{\"AID\":" << r_account_id;
		stream << ",\"GID\":" << r_char_id;
		stream << ",\"CharName\":\"" << CharName << "\"";
		stream << ",\"WorldName\":\"" << world_name << "\"";
		stream << ",\"Tanker\":" << r_Tanker << ",\"Dealer\":" << r_Dealer << ",\"Healer\":" << r_Healer << ",\"Assist\":" << r_Assist;
		stream << ",\"MinLV\":" << r_MinLV << ",\"MaxLV\":" << r_MaxLV;
		stream << ",\"Type\":" << Type;
		stream << ",\"Memo\":\"" << r_Memo << "\"}";
		if (index < SqlStmt_NumRows(stmt))
			stream << ",";
	}
	stream << "],\"Type\":1}";

	SqlStmt_Free(stmt);
	sl.unlock();

	res.set_content(stream.str(), "application/json");
}

void do_init_party(void) {
	//We clear the table in case of crash of the server
	SQLLock sl(WEB_SQL_LOCK);
	sl.lock();
	auto handle = sl.getHandle();
	SqlStmt* stmt = SqlStmt_Malloc(handle);
	if (SQL_SUCCESS != SqlStmt_Prepare(stmt, "TRUNCATE TABLE `%s`",
		party_agency_table)
		|| SQL_SUCCESS != SqlStmt_Execute(stmt)
		) {
		SqlStmt_ShowDebug(stmt);
		SqlStmt_Free(stmt);
		sl.unlock();
		return;
	}
	SqlStmt_Free(stmt);
	sl.unlock();
}

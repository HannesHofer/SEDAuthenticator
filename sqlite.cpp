#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <iostream>
#include <unistd.h>

#define BLOB 0
#define TEXT 1

static int callback(void *NotUsed, int argc, char **argv, char **azColName){
	int i;
	for(i=0; i<argc; i++){
		printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	printf("\n");
	return 0;
}

int addPreparedStatement(sqlite3_stmt &stmt, int type, const std::string &value, const std::string &name)
{
	int index = sqlite3_bind_parameter_index(&stmt, name.c_str());
	if (index == 0) {
		std::clog << "internal error could not bind parameters" <<  std::endl;
		exit(-1);
	}
	switch(type) {
		case BLOB:
			return sqlite3_bind_blob(&stmt, index, value.c_str(), value.length(), SQLITE_TRANSIENT);
			break;
		case TEXT:
			return sqlite3_bind_text(&stmt, index, value.c_str(), -1, SQLITE_TRANSIENT);
			break;
		default:
			break;
	}
}

std::string getPassphraseFromKey(const std::string &database, const std::string &key)
{
	std::string passphrase;
	sqlite3 *db;
	int rc = sqlite3_open(database.c_str(), &db);
	if (rc != SQLITE_OK) {
		std::clog << "could not open database: " << sqlite3_errmsg(db) << std::endl;
		return "";
	}

	const char *query = "SELECT passphrase FROM keymap WHERE key=:key)";
	
	sqlite3_stmt *stmt = NULL;
	int ret = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
	
	// Prepare & Fill statements
	addPreparedStatement(*stmt, BLOB, key, "key");
	
	// EXECUTE Statement
	while ((ret = sqlite3_step(stmt)) == SQLITE_BUSY)
		usleep(500);
	// TODO get value for query

	if (ret != SQLITE_DONE)
		std::clog << "return value of step != SQLITE_DONE: " << ret << std::endl;
	
	return passphrase;
}

int addPassphraseKey(const std::string database, const std::string key, const std::string passphrase)
{
	sqlite3 *db;
	int rc = sqlite3_open(database.c_str(), &db);
	if (rc != SQLITE_OK) {
		// CREATE Database when not exists. Warn User
		std::clog << "could not open " << database << ": "
		          << sqlite3_errmsg(db) << std::endl;
		std::clog << "creating new database " << database << std::endl;
		rc = sqlite3_open_v2(database.c_str(), &db,
							 SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
		if (rc != SQLITE_OK) {
			std::clog << "could not create " << database << ": " 
			          << sqlite3_errmsg(db) << std::endl;
			exit(-1);
		}
		// Create Table 
		char *zErrMsg = 0;
		const char *sql =  "CREATE TABLE keymap("
		                   "key BLOB NOT NULL,"
				           "passphrase TEXT NOT NULL)";
		
		rc = sqlite3_exec(db, sql, NULL, 0, &zErrMsg);
		if( rc != SQLITE_OK ){
			std::clog << "could not create table KEYMAP " << std::endl;
			exit(-1);
		}
	}

	const char *query = "INSERT OR IGNORE INTO keymap(key, passphrase)"
	                    " VALUES(:key, :passphrase)";
	sqlite3_stmt *stmt = NULL;
	int ret = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
	
	// Prepare & Fill statements
	addPreparedStatement(*stmt, BLOB, key, "key");
	addPreparedStatement(*stmt, TEXT, passphrase, "passphrase");
	
	// EXECUTE Statement
	while ((ret = sqlite3_step(stmt)) == SQLITE_BUSY)
		usleep(500);

	if (ret != SQLITE_DONE)
		std::clog << "return value of step != SQLITE_DONE: " << ret << std::endl;
		
	// delete preared statement
	ret = sqlite3_finalize(stmt);

	sqlite3_close(db);
} 

int main(int argc, char* argv[])
{
	addPassphraseKey("test.db", "brizzl", "dizzl");
	std::cout << getPassphraseFromKey("test.db", "brizzl") << std::endl;
}

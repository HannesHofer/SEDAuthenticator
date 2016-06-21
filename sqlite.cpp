#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h> 

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

int addPreparedStatement(int type, const std::string &value, const std::string name)
{
	int index = sqlite3_bind_parameter_index(stmt, name.c_str());
    	if (index == 0) {
		std::clog << "internal error could not bind parameters" <<  std::endl;
		exit(-1);
	}

	switch(type) {
		case BLOB:
			return sqlite3_bind_blob(stmt, index, key.c_str(), key.length(), SQLITE_TRANSIENT);
			break;
		case TEXT:
			return sqlite3_bind_text(stmt, index, key.c_str(), -1, SQLITE_TRANSIENT);
			break;
		default:
			break;
	}
}

int getPassphraseFromKey(const std::string &database, const std::string &key, std::string &passphrase)
{
	passphrase = "";
	sqlite3 *db;
	int rc = sqlite3_open(database.c_str(), &db);
	if (rc != SQLITE_OK) {
		std::clog << "could not open database: " << sqlite3_errmsg(db) << std::endl;
		return -1;  
	}

	char *query = "SELECT passphrase FROM keymap WHERE key=:key)";
	
	sqlite3_stmt *stmt = NULL;
    	int ret = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
	
	// Prepare & Fill statements
	addPreparedStatement(BOLB, key, "key");
	
	// EXECUTE Statement
	while ((ret = sqlite3_step(stmt)) == SQLITE_BUSY)
		usleep(500);
	// TODO get value for query

	if (ret != SQLITE_DONE)
		std::clog << "return value of step != SQLITE_DONE: " << ret << std::endl;	
	
	return 0;
}

int addPassphraseKey(const std::string database, const std::string key, const std::string passphrase) 
{
	sqlite3 *db;
	int rc = sqlite3_open(database.c_str(), &db);
	if (rc != SQLITE_OK) {
		// CREATE Database when not exists. Warn User
		std::clog << "could not open " << database << ": " << sqlite3_errmsg(db) << std::endl;
		std::clog << "creating new database " << database << std::endl;
		rc = sqlite3_open_v2(database.c_str(), &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
		if (rc != SQLITE_OK) {
			std::clog << "could not create " << database << ": " << sqlite3_errmsg(db) << std::endl;
			exit(-1);	
		}
		// Create Table 
		char *zErrMsg = 0;
		char *sql =  	"CREATE TABLE keymap("
				"key BLOB NOT NULL,"
				"passphrase TEXT NOT NULL)";
		
		rc = sqlite3_exec(db, sql, NULL, 0, &zErrMsg);
		if( rc != SQLITE_OK ){
			std::clog << "could not create table KEYMAP " << std::endl;
			exit(-1);
		}
	}

	char *query = "INSERT OR IGNORE INTO keymap(key, passphrase)"
                             " VALUES(:key, :passphrase)";
	sqlite3_stmt *stmt = NULL;
    	int ret = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
	
	// Prepare & Fill statements
	addPreparedStatement(BOLB, key, "key");
	addPreparedStatement(BOLB, key, "passphrase");
	
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
	sqlite3 *db;
	char *zErrMsg = 0;
	int  rc;
	char *sql;

	/* Open database */
	rc = sqlite3_open("test.db", &db);
	if( rc ){
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		exit(0);
	}else{
		fprintf(stdout, "Opened database successfully\n");
	}

	/* Create SQL statement */
	sql = "CREATE TABLE COMPANY("  \
	       "ID INT PRIMARY KEY     NOT NULL," \
	       "NAME           TEXT    NOT NULL," \
	       "AGE            INT     NOT NULL," \
	       "ADDRESS        CHAR(50)," \
	       "SALARY         REAL );";

	/* Execute SQL statement */
	rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
	if( rc != SQLITE_OK ){
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}else{
		fprintf(stdout, "Table created successfully\n");
	}
	sqlite3_close(db);
	return 0;
}

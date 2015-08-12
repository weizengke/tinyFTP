#include "database.h"

// static int pre_callback(void *data, int argc, char **argv, char **azColName){
//    fprintf(stdout, "%s: \n", (const char*)data);
//    for(int i=0; i<argc; i++){
//       printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
//    }
//    printf("\n");
//    return 0;
// }

static int callback(void *pDatabase, int argc, char **argv, char **azColName){
   if (pDatabase != NULL)
   {
      vector< map<string ,string> > & resultMapVector = ((Database *)pDatabase)->getResult();
      map<string, string> kvMap;
      for(int i=0; i<argc; i++){
         //printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
         kvMap.insert( std::pair<string, string>(azColName[i], argv[i]) );
      }
      resultMapVector.push_back(kvMap);
   }
   return 0;
   
}

Database::Database(const char * zDbFilename): dbFilename(zDbFilename)
{
   //clean();
   zErrMsg = NULL;
   /* Open database */
   rc = sqlite3_open(dbFilename.c_str(), &pDb);
   if( rc ){
    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(pDb));
    exit(0);
 }else{
    fprintf(stdout, "Open database successfully\n");
 }
}

void Database::init()
{
   std::map<string, string> insertParamMap0 = { {"username", "anonymous"},
   {"password", "anonymous"} };
   std::map<string, string> insertParamMap1 = { {"username", "wenchy"},
   {"password", "8285919"} };
   std::map<string, string> insertParamMap2 = { {"username", "davey"},
   {"password", "davey"} };

   std::map<string, string> selectParamMap = {  {"id", "1"}, {"username", "Paul"} };
   std::map<string, string> updateParamMap = {  {"username", "davey"}, {"password", "dddd"} };

   create();
   insert("user", insertParamMap0);
   insert("user", insertParamMap1);
   insert("user", insertParamMap2);
   //select("user", selectParamMap);

   findALL("user");
   //update("user", "2", updateParamMap);
   //find("user", "2");
}

Database & Database::create()
{
   std::cout << "Database::create" << std::endl;

   /* Create SQL statement */
   const char *sql_table_user =  "CREATE TABLE USER(" \
      "ID            INTEGER PRIMARY KEY AUTOINCREMENT   NOT NULL," \
      "USERNAME      TEXT UNIQUE                         NOT NULL," \
      "PASSWORD      TEXT                                NOT NULL," \
      "CREATE_AT     DATETIME DEFAULT (datetime('now', 'localtime'))," \
      "UPDATE_AT     DATETIME DEFAULT (datetime('now', 'localtime'))," \
      "STATE         INTEGER  DEFAULT 0 );";

const char *sql_table_file =  "CREATE TABLE FILE(" \
   "ID            INTEGER PRIMARY KEY AUTOINCREMENT   NOT NULL," \
   "MD5SUM        TEXT UNIQUE                         NOT NULL," \
   "FILENAME      TEXT                                NOT NULL," \
   "DIRECTORY     TEXT                                NOT NULL," \
   "SIZE          INTEGER                             NOT NULL," \
   "CREATE_AT     DATETIME DEFAULT (datetime('now', 'localtime'))," \
   "UPDATE_AT     DATETIME DEFAULT (datetime('now', 'localtime'))," \
   "ACCESS        INTEGER  DEFAULT 0 );";

   /* Execute SQL statement */
execute(sql_table_user, NULL);
execute(sql_table_file, NULL);

return *this;
}

Database & Database::createTable()
{
   /* Create SQL statement */
   const char *sql_user =     "CREATE TABLE USER(" \
      "ID            INTEGER PRIMARY KEY AUTOINCREMENT   NOT NULL," \
      "USERNAME      TEXT UNIQUE                         NOT NULL," \
      "PASSWORD      TEXT                                NOT NULL," \
      "CREATE_AT     DATETIME DEFAULT (datetime('now', 'localtime'))," \
      "UPDATE_AT     DATETIME DEFAULT (datetime('now', 'localtime'))," \
      "STATE         INTEGER  DEFAULT 0 );";

    /* Execute SQL statement */
execute(sql_user, NULL);

return *this;
}

bool Database::execute(const char *sql, Database * pDatabase)
{
   /* Clear resultMap */
   resultMapVector.clear();
   /* Execute SQL statement */
   rc = sqlite3_exec(pDb, sql, callback, pDatabase, &zErrMsg);
   if ( rc != SQLITE_OK ){
      fprintf(stderr, "execute: error, %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
      return false;
   } else {
      //fprintf(stdout, "insert: Records created successfully\n");
      printResult();
      return true;
   }
}

bool Database::insert(string tblname, map<string, string> & kvMap)
{
     /* Create SQL statement */
   sqlSring.clear();
   string valString;
   sqlSring += "INSERT INTO " + tblname + " ('";
   valString += "VALUES ('";

   map<string ,string>::iterator it=kvMap.begin();

   sqlSring += it->first;
   valString += it->second;
   for (++it; it!=kvMap.end(); ++it)
   {
      sqlSring += "', '" + it->first;
      valString += "', '" + it->second;
   }
   sqlSring += "') ";
   valString += "') ";
   sqlSring += valString;

   std::cout << "insert: " << sqlSring << std::endl;
      /* Execute SQL statement and return*/
   return execute(sqlSring.c_str(), this);
}

bool Database::select(string tblname, map<string, string> & kvMap)
{
    /* Construct SQL statement */
   sqlSring.clear();
   sqlSring += "SELECT * from ";
   sqlSring += tblname;
   sqlSring += " WHERE ";

   map<string ,string>::iterator it=kvMap.begin();
   sqlSring += it->first + "='" + it->second + "'";
   for (++it; it!=kvMap.end(); ++it)
   {
      sqlSring += " and " + it->first + "='" + it->second + "'";
   }
   std::cout << "query: " << sqlSring << std::endl;
   /* Execute SQL statement */
   return execute(sqlSring.c_str(), this);
}



bool Database::update(string tblname, string id, map<string, string> & kvMap)
{
   /* Construct SQL statement */
   sqlSring.clear();
   sqlSring += "UPDATE ";
   sqlSring += tblname;
   sqlSring += " SET ";

   map<string ,string>::iterator it=kvMap.begin();
   sqlSring += it->first + "='" + it->second + "'";
   for (++it; it!=kvMap.end(); ++it)
   {
      sqlSring += ", " + it->first + "='" + it->second + "'";
   }
   sqlSring += " WHERE id='" + id + "'";
   std::cout << "update: " << sqlSring << std::endl;
   /* Execute SQL statement */
   return execute(sqlSring.c_str(), this);
}

bool Database::remove(string tblname, string id)
{
   /* Create merged SQL statement */
   sqlSring.clear();
   sqlSring += "DELETE FROM "; //SET column1 = value1, column2 = value2...., columnN = valueN
   sqlSring += tblname;
   sqlSring += " WHERE id='" + id + "'";

   /* Execute SQL statement */
   return execute(sqlSring.c_str(), this);
}

bool Database::find(string tblname, string id)
{
   sqlSring.clear();
   sqlSring += "SELECT * from " + tblname + " WHERE id='";
   sqlSring += id +"'";
   std::cout << "find: " << sqlSring << std::endl;
   return execute(sqlSring.c_str(), this);
}

bool Database::findALL(string tblname)
{
   sqlSring.clear();
   sqlSring += "SELECT * from " + tblname;
   std::cout << "findALL: " << sqlSring << std::endl;
   return execute(sqlSring.c_str(), this);
}

vector< map<string ,string> >  & Database::getResult()
{
   return this->resultMapVector;
}

bool Database::first()
{

   return false;
}
void Database::printResult()
{
   for (vector< map<string ,string> >::iterator iter=resultMapVector.begin(); iter!=resultMapVector.end(); ++iter)
   {
       for (map<string, string>::iterator it=iter->begin(); it!=iter->end(); ++it)
            std::cout << it->first << ": " << it->second << '\n';
       std::cout << '\n';
   }
}
void Database::clean()
{
   if( unlink(dbFilename.c_str()) !=0 )
   {
      char buf[MAXLINE];
      fprintf(stderr, "DB clean error: %s\n", strerror_r(errno, buf, MAXLINE));
      
   } else {
      fprintf(stderr, "DB cleaned\n");
   }
}

void Database::dump()
{

}

Database::~Database()
{
   sqlite3_close(pDb);
}

// int main()
// {
//    Database db(DBFILENAME);
//     db.init();
//     return 0;
// }
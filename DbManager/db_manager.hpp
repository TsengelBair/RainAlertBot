#ifndef DB_MANAGER_HPP
#define DB_MANAGER_HPP

#include <sqlite3.h>
#include <string>
#include <iostream>

class DbManager {
public:
    DbManager(const std::string& dbName);
    ~DbManager();

    void createTable();
    void addUser(int telegram_id, const std::string& name);
    void removeUser(int telegram_id);
    bool isUserExist(int telegram_id);

private:
    sqlite3* db; 
    std::string dbName; 

    bool executeQuery(const std::string& query);
};

#endif
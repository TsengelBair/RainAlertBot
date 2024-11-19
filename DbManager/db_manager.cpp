#include "db_manager.hpp"

DbManager::DbManager(const std::string& dbName) : dbName(dbName), db(nullptr) {
    int rc = sqlite3_open(dbName.c_str(), &db); 
    if (rc != SQLITE_OK) {
        std::cerr << "Ошибка подключения к базе данных: " << sqlite3_errmsg(db) << std::endl;
    } else {
        std::cout << "Успешное подключение к базе данных" << std::endl;
    }
}

DbManager::~DbManager() {
    if (db) {
        sqlite3_close(db); 
        std::cout << "База данных закрыта" << std::endl;
    }
}

void DbManager::createTable() {

    std::string query = "CREATE TABLE IF NOT EXISTS tg_users ("
                        "telegram_id INT PRIMARY KEY, "
                        "name TEXT);";
                        
    if (executeQuery(query)) {
        std::cout << "Таблица успешно создана" << std::endl;
    } else {
        std::cout << "Ошибка при создании таблицы" << std::endl;
    }
}

void DbManager::addUser(int telegram_id, const std::string& name) {

    std::string query = "INSERT INTO tg_users (telegram_id, name) "
                        "VALUES (" + std::to_string(telegram_id) + ", '" + name + "');";
    if (executeQuery(query)) {
        std::cout << "Пользователь добавлен успешно" << std::endl;
    } else {
        std::cout << "Ошибка при добавлении пользователя" << std::endl;
    }
}

void DbManager::removeUser(int telegram_id) {

    std::string query = "DELETE FROM tg_users WHERE telegram_id = " + std::to_string(telegram_id) + ";";
    if (executeQuery(query)) {
        std::cout << "Пользователь удален успешно" << std::endl;
    } else {
        std::cout << "Ошибка при удалении пользователя" << std::endl;
    }
}

bool DbManager::isUserExist(int telegram_id) {
    std::string query = "SELECT COUNT(*) FROM tg_users WHERE telegram_id = " + std::to_string(telegram_id) + ";";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        std::cerr << "Ошибка при подготовке запроса: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        int count = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
        return count > 0;  // Если count > 0, то пользователь существует
    }

    sqlite3_finalize(stmt);
    return false;
}

bool DbManager::executeQuery(const std::string& query) {

    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, query.c_str(), 0, 0, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "Ошибка SQL: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}
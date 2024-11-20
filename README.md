## Task - make c++ tgBot sendind rain/snow alerts

TODO:

1) Launch bot
2) Create db to store users 
3) Make http request to openweaher api

### Step 1
#### [Launch bot on linux](https://github.com/reo7sp/tgbot-cpp)

Simple sample

```c++
#include <iostream>
#include <tgbot/tgbot.h>

int main() {
    
    TgBot::Bot bot("6405966767:AAF5QpiIbkV2cTZJaSAciuke5zO9DLBXabY");

    bot.getEvents().onCommand("start", [&bot](TgBot::Message::Ptr message) {
        bot.getApi().sendMessage(message->chat->id, "Hi!");
    });

    bot.getEvents().onAnyMessage([&bot](TgBot::Message::Ptr message) {
        printf("User wrote %s\n", message->text.c_str());
        if (StringTools::startsWith(message->text, "/start")) {
            return;
        }
        bot.getApi().sendMessage(message->chat->id, "Your message is: " + message->text);
    });

    /* Запуск */
    try {
        printf("Bot username: %s\n", bot.getApi().getMe()->username.c_str());
        TgBot::TgLongPoll longPoll(bot);
        while (true) {
            printf("Long poll started\n");
            longPoll.start();
        }
    } catch (TgBot::TgException& e) {
        printf("error: %s\n", e.what());
    }

    return 0;
}
```

### Step 2
#### Create sqlite db

```c++
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
```

```c++
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
```

### Step 3
#### Http request

```c++
#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <boost/beast.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <nlohmann/json.hpp>

namespace http = boost::beast::http;
using nlohmann::json;

/* Класс для отправки http запроса для получения погоды */
class Request 
{
public:
    Request(std::string host = "api.openweathermap.org", std::string apiKey = "48cddbaab3ca4b16b0f3d23c18dd61bc", std::string cityId = "498817");
    /* сеттеры */
    void setRain(bool rain);
    void setSnow(bool snow);
    /* геттеры */
    bool isRain() const;
    bool isSnow() const;
    /* Отправка запроса на получение погоды */
    void getWeather(std::string host, std::string target);
    
private:
    bool _rain;
    bool _snow;
};

#endif
```

```c++
#include <boost/beast.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <nlohmann/json.hpp>
#include "request.hpp"

Request::Request(std::string host, std::string apiKey, std::string cityId)
{
    std::string target = "/data/2.5/forecast?id=" + cityId + "&appid=" + apiKey; 
    getWeather(host, target);
}

void Request::setRain(bool rain)
{
    _rain = rain;
};

void Request::setSnow(bool snow)
{
    _snow = snow;
};

bool Request::isRain() const
{
    return _rain;
}

bool Request::isSnow() const
{
    return _snow;
}

void Request::getWeather(std::string host, std::string target)
{
    try {
        /* Настройки подключения */
        boost::asio::io_context ioc; // Объект ввода/вывода
        boost::asio::ip::tcp::resolver resolver(ioc); // Преобразует домен в IP
        boost::asio::ip::tcp::socket socket(ioc); // Сокет для соединения         

        /* Устанавливаем соединение */
        boost::asio::connect(socket, resolver.resolve(host, "80")); // 80 порт - стандартный порт для HTTP

        /* Создаем запрос */
        http::request<http::string_body> req(http::verb::get, target, 11);
        req.set(http::field::host, host);
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        // Отправляем запрос через приконекченный сокет
        http::write(socket, req);
        {
            boost::beast::flat_buffer buffer;
            http::response<http::dynamic_body> res;
            http::read(socket, buffer, res);

            // Ответ получаем в виде json
            auto jsonResponse = json::parse(boost::beast::buffers_to_string(res.body().data()));

            std::cout << jsonResponse;

            bool rainExpected = false;
            bool snowExpected = false;

            // Проходим по прогнозам с 06:00 до 00:00 (с 3-го по 8-й элемент)
            for (int i = 2; i < 8; ++i) {  
                const auto& forecast = jsonResponse["list"][i];

                // Проверяю есть ли поле rain, после проверяю есть в этом поле данные
                if (forecast.find("rain") != forecast.end() && forecast["rain"].find("3h") != forecast["rain"].end()) {
                    rainExpected = true;
                }

                if (forecast.find("snow") != forecast.end() && forecast["snow"].find("3h") != forecast["snow"].end()) {
                    snowExpected = true;
                }
            }

            setRain(rainExpected);
            setSnow(snowExpected);
        }
        // Закрываем соединение
        socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
    } catch (std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
    }
};
```

### Step 4
#### Implement db and http request to tg bot

```c++
#include <tgbot/tgbot.h>
#include <iostream>
#include "DbManager/db_manager.hpp"  
#include "http/request.hpp"  

int main() {

    TgBot::Bot bot("YOUR_BOT_TOKEN");

    DbManager db("users.db");
    db.createTable(); 

    Request weatherRequest;
    std::string requestResult;

    auto getWeatherForecast = [&weatherRequest, &requestResult](){
        weatherRequest.getWeather("api.openweathermap.org", "/data/2.5/forecast?id=498817&appid=48cddbaab3ca4b16b0f3d23c18dd61bc");
        if (weatherRequest.isRain() || weatherRequest.isSnow()) {
            requestResult = "Прогноз на сегодня:\n"; 

            if (weatherRequest.isRain()) {
                requestResult += "Ожидается дождь.\n";
            }

            if (weatherRequest.isSnow()) {
                requestResult += "Ожидается снег.\n";
            }

        } else {
            requestResult.clear(); 
        }

        std::cout << "Прогноз погоды: " << requestResult << std::endl;
    };

    bot.getEvents().onCommand("start", [&bot, &db, &getWeatherForecast](TgBot::Message::Ptr message) {
        int userId = message->chat->id;
        std::string userName = message->chat->firstName + " " + message->chat->lastName;

        if (db.isUserExist(userId)) {
            bot.getApi().sendMessage(message->chat->id, "Вы уже подписаны на уведомления.");
        } else {
            db.addUser(userId, userName);
            bot.getApi().sendMessage(message->chat->id, "Привет! Теперь вам будут приходить уведомления о дожде или снеге, чтобы уведомления перестали приходить, выберите опцию /reject");
            getWeatherForecast();
        }

    });

    bot.getEvents().onCommand("reject", [&bot, &db](TgBot::Message::Ptr message) {
        int userId = message->chat->id;

        if (db.isUserExist(userId)) {
            db.removeUser(userId);  
            bot.getApi().sendMessage(message->chat->id, "Уведомления выключены, чтобы снова подписаться на уведомления, отправьте команду /start");
        } else {
            bot.getApi().sendMessage(message->chat->id, "Вы не подписаны на уведомления.");
        }
    });

    // Настройка запуска бота (мб настроить веб хук вместо лонг пула)
    try {
        printf("Bot username: %s\n", bot.getApi().getMe()->username.c_str());
        TgBot::TgLongPoll longPoll(bot);
        while (true) {
            printf("Long poll started\n");
            longPoll.start();
        }
    } catch (TgBot::TgException& e) {
        printf("error: %s\n", e.what());
    }

    return 0;
}
```
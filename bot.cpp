#include <tgbot/tgbot.h>
#include <iostream>
#include "DbManager/db_manager.hpp"  
#include "http/request.hpp"  

int main() {

    TgBot::Bot bot("6405966767:AAF5QpiIbkV2cTZJaSAciuke5zO9DLBXabY");

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
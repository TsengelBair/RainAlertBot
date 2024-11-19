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
                // можешь проверить у openweatherapi идут ли поля с rain И snow из коробки
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


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
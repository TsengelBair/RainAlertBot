#!/bin/bash

g++ bot.cpp DbManager/db_manager.cpp http/request.cpp -o telegram_bot --std=c++14 -I/usr/local/include -lTgBot -lboost_system -lssl -lcrypto -lpthread -lcurl -lsqlite3
./telegram_bot
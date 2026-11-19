#include "client.h"

#include <iostream>

int main()
{
    try
    {
        // Создаём клиент игры
        Client game;

        // Подключаемся к серверу
        // IP: 127.0.0.1
        // UDP-порт: 9003

        game.connectServer(
            "127.0.0.1",
            9003
        );

        // Запускаем игру
        game.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Game error: "
                  << e.what() << '\n';

        return 1;
    }

    return 0;
}
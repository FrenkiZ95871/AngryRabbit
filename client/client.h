#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include <boost/asio.hpp>

#include <vector>
#include <random>
#include <cstdint>
#include <string>
#include <optional>

class Client
{
public:
    // Конструктор
    Client();

    // Запуск игры
    void run();

    // Подключение к серверу
    bool connectServer(const std::string &ip, uint16_t port);

private:
    // ОКНО ИГРЫ
    sf::RenderWindow window;

    // ИГРОК
    sf::CircleShape player;

    sf::Vector2f playerPosition;

    float playerRadius = 25.f;
    float playerSpeed = 100.f;

    // КАМЕРА
    sf::View camera;

    // ИГРОВОЕ ПОЛЕ
    const float WORLD_WIDTH = 5000.f;
    const float WORLD_HEIGHT = 5000.f;

    float gridSize = 100.f;

    // ЕДА
    std::vector<sf::Vector2f> food;

    float foodRadius = 7.f;

    // СЧЁТ
    int score = 0;

    // ВРЕМЯ
    sf::Clock clock;

    // ГЕНЕРАТОР СЛУЧАЙНЫХ ЧИСЕЛ
    std::mt19937 randomGenerator;

    // СЕТЕВОЕ ПОДКЛЮЧЕНИЕ
    boost::asio::io_context ioContext;

    boost::asio::ip::udp::socket socket;

    boost::asio::ip::udp::endpoint serverEndpoint;

    uint32_t playerId = 0;

    bool connected = false;

    // ИГРОВОЙ ЦИКЛ
    void processEvents();

    void update(float deltaTime);

    void render();

    // ЛОГИКА ИГРОКА
    void updatePlayer(float deltaTime);

    void updateCamera();

    // ЛОГИКА ЕДЫ
    void generateFood();

    void checkFoodCollision();

    // ОТРИСОВКА
    void drawGrid();

    void drawFood();

    void drawPlayer();

    void drawInterface();
};
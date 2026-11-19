#include "client.h"

#include <iostream>
#include <cmath>
#include <algorithm>

// КОНСТРУКТОР
Client::Client()
    : window(
        sf::VideoMode({1280u, 720u}),
        "Agar.io"
      ),
      socket(ioContext),
      randomGenerator(std::random_device{}())
{
    // Ограничение FPS
    window.setFramerateLimit(60);

    // Начальная позиция игрока
    playerPosition = {
        WORLD_WIDTH / 2.f,
        WORLD_HEIGHT / 2.f
    };

    // Создание клетки
    player.setRadius(playerRadius);

    // Установка центра клетки
    player.setOrigin({
        playerRadius,
        playerRadius
    });

    // Цвет клетки
    player.setFillColor(
        sf::Color(50, 150, 255)
    );

    // Обводка клетки
    player.setOutlineThickness(3.f);

    player.setOutlineColor(
        sf::Color::White
    );

    // Настройка камеры
    camera.setSize({
        1280.f,
        720.f
    });

    camera.setCenter(playerPosition);

    // Установка камеры
    window.setView(camera);

    // Генерация еды
    generateFood();
}

// ПОДКЛЮЧЕНИЕ К СЕРВЕРУ
bool Client::connectServer(
    const std::string& ip,
    uint16_t port
)
{
    try
    {
        // Адрес сервера
        serverEndpoint =
            boost::asio::ip::udp::endpoint(
                boost::asio::ip::make_address(ip),
                port
            );

        // Открытие UDP-сокета
        socket.open(
            boost::asio::ip::udp::v4()
        );

        // Генерация уникального ID в пределах
        // одного запуска генератора
        std::uniform_int_distribution<uint32_t> idDistribution(
            1,
            std::numeric_limits<uint32_t>::max()
        );

        playerId = idDistribution(randomGenerator);

        // Отправка ID серверу
        socket.send_to(
            boost::asio::buffer(
                &playerId,
                sizeof(playerId)
            ),
            serverEndpoint
        );

        connected = true;

        std::cout << "UDP packet sent!\n";

        std::cout << "Player ID: "
                  << playerId << '\n';

        return true;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Connection error: "
                  << e.what() << '\n';

        connected = false;

        return false;
    }
}

// ЗАПУСК ИГРЫ
void Client::run()
{
    while (window.isOpen())
    {
        // Обработка событий
        processEvents();

        // Получение времени кадра
        float deltaTime =
            clock.restart().asSeconds();

        // Ограничение скачков времени
        deltaTime = std::min(
            deltaTime,
            0.05f
        );

        // Обновление игровой логики
        update(deltaTime);

        // Отрисовка
        render();
    }
}

// ОБРАБОТКА СОБЫТИЙ
void Client::processEvents()
{
    while (const std::optional event = window.pollEvent())
    {
        // Закрытие окна
        if (event->is<sf::Event::Closed>())
        {
            window.close();
        }

        // Нажатие клавиш
        if (const auto* key =
            event->getIf<sf::Event::KeyPressed>())
        {
            // Выход из игры
            if (key->code == sf::Keyboard::Key::Escape)
            {
                window.close();
            }
        }
    }
}

// ОБНОВЛЕНИЕ ИГРЫ
void Client::update(float deltaTime)
{
    updatePlayer(deltaTime);

    updateCamera();

    checkFoodCollision();
}

// ДВИЖЕНИЕ ИГРОКА
void Client::updatePlayer(float deltaTime)
{
    // Получаем положение мыши в окне
    sf::Vector2i mousePixel =
        sf::Mouse::getPosition(window);

    // Переводим координаты мыши
    // в координаты игрового мира
    sf::Vector2f mousePosition =
        window.mapPixelToCoords(
            mousePixel,
            camera
        );

    // Направление движения
    sf::Vector2f direction = {
        mousePosition.x - playerPosition.x,
        mousePosition.y - playerPosition.y
    };

    // Расстояние до курсора
    float distance = std::sqrt(
        direction.x * direction.x +
        direction.y * direction.y
    );

    // Проверяем расстояние
    if (distance > 5.f)
    {
        // Нормализация направления
        direction.x /= distance;
        direction.y /= distance;

        // Перемещение игрока
        playerPosition.x +=
            direction.x * playerSpeed * deltaTime;

        playerPosition.y +=
            direction.y * playerSpeed * deltaTime;
    }

    // Ограничение игрового поля по X
    playerPosition.x = std::clamp(
        playerPosition.x,
        playerRadius,
        WORLD_WIDTH - playerRadius
    );

    // Ограничение игрового поля по Y
    playerPosition.y = std::clamp(
        playerPosition.y,
        playerRadius,
        WORLD_HEIGHT - playerRadius
    );

    // Обновляем позицию клетки
    player.setPosition(playerPosition);
}

// ОБНОВЛЕНИЕ КАМЕРЫ
void Client::updateCamera()
{
    // Камера следует за игроком
    camera.setCenter(playerPosition);

    window.setView(camera);
}

// ГЕНЕРАЦИЯ ЕДЫ
void Client::generateFood()
{
    food.clear();

    // Случайные координаты X
    std::uniform_real_distribution<float> randomX(
        20.f,
        WORLD_WIDTH - 20.f
    );

    // Случайные координаты Y
    std::uniform_real_distribution<float> randomY(
        20.f,
        WORLD_HEIGHT - 20.f
    );

    // Создаём 500 частиц еды
    for (int i = 0; i < 500; i++)
    {
        sf::Vector2f position = {
            randomX(randomGenerator),
            randomY(randomGenerator)
        };

        food.push_back(position);
    }
}

// ПРОВЕРКА СТОЛКНОВЕНИЙ С ЕДОЙ
void Client::checkFoodCollision()
{
    for (size_t i = 0; i < food.size();)
    {
        // Разница координат
        float dx =
            playerPosition.x - food[i].x;

        float dy =
            playerPosition.y - food[i].y;

        // Расстояние между центрами
        float distance = std::sqrt(
            dx * dx + dy * dy
        );

        // Проверяем столкновение
        if (distance < playerRadius + foodRadius)
        {
            // Удаляем съеденную еду
            food.erase(
                food.begin() + i
            );

            // Увеличиваем счёт
            score++;

            // Увеличиваем клетку
            playerRadius += 0.3f;

            // Обновляем размер клетки
            player.setRadius(playerRadius);

            // Обновляем центр круга
            player.setOrigin({
                playerRadius,
                playerRadius
            });
        }
        else
        {
            i++;
        }
    }
}

// ОТРИСОВКА ИГРЫ
void Client::render()
{
    // Очистка экрана
    window.clear(
        sf::Color(245, 245, 245)
    );

    // Игровая сетка
    drawGrid();

    // Еда
    drawFood();

    // Игрок
    drawPlayer();

    // Интерфейс
    drawInterface();

    // Вывод изображения
    window.display();
}

// ОТРИСОВКА ИГРОВОЙ СЕТКИ
void Client::drawGrid()
{
    // Центр камеры
    sf::Vector2f center =
        camera.getCenter();

    // Размер камеры
    sf::Vector2f size =
        camera.getSize();

    // Границы видимой области
    float left =
        center.x - size.x / 2.f;

    float right =
        center.x + size.x / 2.f;

    float top =
        center.y - size.y / 2.f;

    float bottom =
        center.y + size.y / 2.f;

    sf::Color gridColor(220, 220, 220);

    // Начальная вертикальная линия
    float startX =
        std::floor(left / gridSize) * gridSize;

    // Вертикальные линии
    for (float x = startX; x <= right; x += gridSize)
    {
        sf::RectangleShape line({
            1.f,
            size.y
        });

        line.setPosition({
            x,
            top
        });

        line.setFillColor(gridColor);

        window.draw(line);
    }

    // Начальная горизонтальная линия
    float startY =
        std::floor(top / gridSize) * gridSize;

    // Горизонтальные линии
    for (float y = startY; y <= bottom; y += gridSize)
    {
        sf::RectangleShape line({
            size.x,
            1.f
        });

        line.setPosition({
            left,
            y
        });

        line.setFillColor(gridColor);

        window.draw(line);
    }
}

// ОТРИСОВКА ЕДЫ
void Client::drawFood()
{
    sf::CircleShape foodShape(foodRadius);

    foodShape.setOrigin({
        foodRadius,
        foodRadius
    });

    foodShape.setFillColor(
        sf::Color(80, 210, 100)
    );

    // Отрисовка каждой частицы
    for (const auto& position : food)
    {
        foodShape.setPosition(position);

        window.draw(foodShape);
    }
}

// ОТРИСОВКА ИГРОКА
void Client::drawPlayer()
{
    window.draw(player);
}

// ИНТЕРФЕЙС
void Client::drawInterface()
{
    // Возвращаем стандартный вид окна,
    // чтобы интерфейс не перемещался вместе с камерой
    window.setView(window.getDefaultView());

    // Текст пока не используем,
    // поэтому показываем счёт в заголовке окна
    window.setTitle(
        "Agar.io | Score: " + std::to_string(score)
    );

    // Возвращаем камеру игрового мира
    window.setView(camera);
}
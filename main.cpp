#include <SFML/Graphics.hpp>
#include <optional>
#include <cstdlib>
#include <ctime>
#include <string>

// ---------------- CONFIG ----------------
constexpr int SCREEN_WIDTH  = 1024;
constexpr int SCREEN_HEIGHT = 768;
constexpr int MAX_ENEMIES   = 5;

// ---------------- STRUCTS ----------------
struct Player
{
    sf::RectangleShape shape;
    float speed{};
};

struct Bullet
{
    sf::RectangleShape shape;
    float speed{};
    bool active{};
};

struct Enemy
{
    sf::RectangleShape shape;
    float speed{};
    bool active{};
};

// ---------------- MAIN ----------------
int main()
{
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    // --------- WINDOW ---------
    sf::RenderWindow window(
        sf::VideoMode({SCREEN_WIDTH, SCREEN_HEIGHT}),
        "Arcade Shooter"
    );
    window.setFramerateLimit(60);

    // --------- SCORE UI ---------
    int score = 0;
    sf::Font font;
    sf::Text scoreText(font) ;

    if (font.openFromFile("/home/marius/CLionProjects/arcade-shooter/arial.ttf"))   // put arial.ttf next to executable
    {
        scoreText.setCharacterSize(22);
        scoreText.setFillColor(sf::Color::White);
        scoreText.setPosition({10.f, 10.f});
    }

    // --------- PLAYER ---------
    Player player;
    player.shape.setSize({60.f, 20.f});
    player.shape.setFillColor(sf::Color::Green);
    player.shape.setPosition({
        SCREEN_WIDTH / 2.f - player.shape.getSize().x / 2.f,
        SCREEN_HEIGHT - player.shape.getSize().y - 5.f
    });
    player.speed = 6.f;

    // --------- BULLET ---------
    Bullet bullet;
    bullet.shape.setSize({5.f, 15.f});
    bullet.shape.setFillColor(sf::Color::Red);
    bullet.speed = 10.f;
    bullet.active = false;

    // --------- ENEMIES ---------
    Enemy enemies[MAX_ENEMIES];

    for (int i = 0; i < MAX_ENEMIES; ++i)
    {
        enemies[i].shape.setSize({40.f, 20.f});
        enemies[i].shape.setFillColor(sf::Color::Yellow);
        enemies[i].speed = 2.f;
        enemies[i].active = false;
    }

    // ================= GAME LOOP =================
    while (window.isOpen())
    {
        // --------- EVENTS (ONLY ONE-TIME ACTIONS) ---------
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        // --------- PLAYER MOVEMENT (REAL-TIME INPUT) ---------
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
            player.shape.move({-player.speed, 0.f});

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
            player.shape.move({player.speed, 0.f});

        // Keep player inside screen
        auto pos = player.shape.getPosition();
        if (pos.x < 0.f) pos.x = 0.f;
        if (pos.x > SCREEN_WIDTH - player.shape.getSize().x)
            pos.x = SCREEN_WIDTH - player.shape.getSize().x;
        player.shape.setPosition(pos);

        // --------- BULLET SHOOTING ---------
        if (!bullet.active && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space))
        {
            bullet.active = true;
            bullet.shape.setPosition({
                player.shape.getPosition().x + player.shape.getSize().x / 2.f - 2.5f,
                player.shape.getPosition().y - 15.f
            });
        }

        if (bullet.active)
        {
            bullet.shape.move({0.f, -bullet.speed});
            if (bullet.shape.getPosition().y + bullet.shape.getSize().y < 0.f)
                bullet.active = false;
        }

        // --------- ENEMY SPAWNING & MOVEMENT ---------
        for (int i = 0; i < MAX_ENEMIES; ++i)
        {
            if (!enemies[i].active)
            {
                if (std::rand() % 200 == 0)
                {
                    enemies[i].active = true;
                    float x = static_cast<float>(std::rand() % (SCREEN_WIDTH - 40));
                    enemies[i].shape.setPosition({x, -20.f});
                    enemies[i].speed = 1.5f + static_cast<float>(std::rand() % 200) / 100.f;
                }
            }
            else
            {
                enemies[i].shape.move({0.f, enemies[i].speed});

                if (enemies[i].shape.getPosition().y > SCREEN_HEIGHT)
                    enemies[i].active = false;
            }
        }

        // --------- COLLISION: BULLET VS ENEMIES ---------
        if (bullet.active)
        {
            for (int i = 0; i < MAX_ENEMIES; ++i)
            {
                if (enemies[i].active &&
                    bullet.shape.getGlobalBounds()
                        .findIntersection(enemies[i].shape.getGlobalBounds()))
                {
                    enemies[i].active = false;
                    bullet.active = false;
                    ++score;
                }
            }
        }

        // --------- COLLISION: ENEMY VS PLAYER (GAME OVER) ---------
        for (int i = 0; i < MAX_ENEMIES; ++i)
        {
            if (enemies[i].active &&
                enemies[i].shape.getGlobalBounds()
                    .findIntersection(player.shape.getGlobalBounds()))
            {
                window.close();   // simple game over
            }
        }

        // --------- UPDATE UI ---------
        if (font.getInfo().family != "")
            scoreText.setString("Score: " + std::to_string(score));

        // --------- DRAW ---------
        window.clear(sf::Color::Black);

        window.draw(player.shape);

        if (bullet.active)
            window.draw(bullet.shape);

        for (int i = 0; i < MAX_ENEMIES; ++i)
            if (enemies[i].active)
                window.draw(enemies[i].shape);

        if (font.getInfo().family != "")
            window.draw(scoreText);

        window.display();
    }

    return 0;
}

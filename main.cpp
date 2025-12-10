#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <optional>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include <cmath>

// ---------------- CONFIG ----------------
constexpr int SCREEN_WIDTH  = 1024;
constexpr int SCREEN_HEIGHT = 768;
constexpr int MAX_ENEMIES   = 6;
constexpr int MAX_BULLETS   = 40;

// ---------------- STRUCTS ----------------
struct Player
{
    sf::RectangleShape shape;
    float speed{};
    int lives{};
};

enum class WeaponType
{
    Single,
    Burst,
    Heavy
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
    float animPhase{}; // for simple pulsing
};

struct LifeDrop
{
    sf::CircleShape shape;
    bool active{};
    float speed{};
};

struct WeaponDrop
{
    sf::RectangleShape shape;
    bool active{};
    float speed{};
    WeaponType type{};
};

int main()
{
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    sf::Clock fireClock;
    sf::Clock gameClock; // for dt/animations

    WeaponType currentWeapon{WeaponType::Single};

    // change this if you move textures
    const std::string texturesPath{"/home/marius/CLionProjects/arcade-shooter/textures/"};

    // --------- WINDOW ---------
    sf::RenderWindow window{
        sf::VideoMode({SCREEN_WIDTH, SCREEN_HEIGHT}),
        "Arcade Shooter - Textures + Background"
    };
    window.setFramerateLimit(60);

    // --------- UI ---------
    int score{0};
    sf::Font font;
    sf::Text scoreText{font}, livesText{font}, weaponText{font};

    if (font.openFromFile("/home/marius/CLionProjects/arcade-shooter/arial.ttf"))
    {
        scoreText.setCharacterSize(22);
        scoreText.setFillColor(sf::Color::White);
        scoreText.setPosition({10.f, 10.f});

        livesText.setCharacterSize(22);
        livesText.setFillColor(sf::Color::White);
        livesText.setPosition({10.f, 40.f});

        weaponText.setCharacterSize(22);
        weaponText.setFillColor(sf::Color::White);
        weaponText.setPosition({10.f, 70.f});
    }

    // --------- TEXTURES ---------
    sf::Texture texPlayer, texEnemy, texBullet, texLife, texWeapon, texBackground;

    if (!texPlayer.loadFromFile(texturesPath + "player.png"))
        std::cout << "ERROR: Failed to load player.png\n";
    if (!texEnemy.loadFromFile(texturesPath + "enemy.png"))
        std::cout << "ERROR: Failed to load enemy.png\n";
    if (!texBullet.loadFromFile(texturesPath + "bullet.png"))
        std::cout << "ERROR: Failed to load bullet.png\n";
    if (!texLife.loadFromFile(texturesPath + "life.png"))
        std::cout << "ERROR: Failed to load life.png\n";
    if (!texWeapon.loadFromFile(texturesPath + "weapon.png"))
        std::cout << "ERROR: Failed to load weapon.png\n";
    if (!texBackground.loadFromFile(texturesPath + "background.png"))
        std::cout << "ERROR: Failed to load background.png\n";

    texPlayer.setSmooth(true);
    texEnemy.setSmooth(true);
    texBullet.setSmooth(true);
    texLife.setSmooth(true);
    texWeapon.setSmooth(true);
    texBackground.setSmooth(true);

    // --------- SCROLLING BACKGROUND (two tiles) ---------
    sf::Vector2f bgSize{
        static_cast<float>(texBackground.getSize().x),
        static_cast<float>(texBackground.getSize().y)
    };

    sf::RectangleShape bg1{bgSize};
    sf::RectangleShape bg2{bgSize};
    bg1.setTexture(&texBackground);
    bg2.setTexture(&texBackground);
    bg1.setPosition({0.f, 0.f});
    bg2.setPosition({0.f, -bgSize.y});

    constexpr float backgroundSpeed{80.f}; // pixels per second

    // --------- SOUND ---------
    sf::SoundBuffer shootBuffer, hitBuffer;
    if (!shootBuffer.loadFromFile("/home/marius/CLionProjects/arcade-shooter/laser.wav"))
        std::cout << "ERROR: Failed to load laser.wav\n";
    if (!hitBuffer.loadFromFile("/home/marius/CLionProjects/arcade-shooter/explosion.wav"))
        std::cout << "ERROR: Failed to load explosion.wav\n";

    sf::Sound shootSound{shootBuffer};
    sf::Sound hitSound{hitBuffer};
    shootSound.setVolume(10.f);
    hitSound.setVolume(10.f);

    // --------- BACKGROUND MUSIC ---------
    sf::Music music;
    if (!music.openFromFile("/home/marius/CLionProjects/arcade-shooter/easy_music.wav"))
        std::cout << "ERROR: Could not load music file\n";
    else
    {
        music.setVolume(15.f);
        music.play();
    }

    // --------- PLAYER ---------
    Player player;
    player.shape.setSize({60.f, 30.f});
    player.shape.setOrigin({30.f, 15.f});           // center
    player.shape.setTexture(&texPlayer);
    player.shape.setPosition({
        SCREEN_WIDTH / 2.f,
        SCREEN_HEIGHT - 80.f
    });
    player.speed = 6.f;
    player.lives = 3;

    // --------- BULLETS ---------
    Bullet bullets[MAX_BULLETS];
    for (auto& b : bullets)
    {
        b.active = false;
        b.shape.setSize({16.f, 24.f});
        b.shape.setOrigin({8.f, 12.f});
        b.shape.setTexture(&texBullet);
    }

    // --------- ENEMIES ---------
    Enemy enemies[MAX_ENEMIES];
    for (auto& e : enemies)
    {
        e.active = false;
        e.speed = 2.f;
        e.shape.setSize({60.f, 40.f});
        e.shape.setOrigin({30.f, 20.f});
        e.shape.setTexture(&texEnemy);
        e.animPhase = static_cast<float>(std::rand() % 100) / 100.f * 6.28318f;
    }

    // --------- LIFE DROP ---------
    LifeDrop lifeDrop;
    lifeDrop.shape.setRadius(12.f);
    lifeDrop.shape.setOrigin({12.f, 12.f});
    lifeDrop.shape.setTexture(&texLife);
    lifeDrop.speed = 2.f;
    lifeDrop.active = false;

    // --------- WEAPON DROP ---------
    WeaponDrop weaponDrop;
    weaponDrop.shape.setSize({24.f, 24.f});
    weaponDrop.shape.setOrigin({12.f, 12.f});
    weaponDrop.shape.setTexture(&texWeapon);
    weaponDrop.speed = 2.2f;
    weaponDrop.active = false;

    // ================= GAME LOOP =================
    while (window.isOpen())
    {
        float dt{gameClock.restart().asSeconds()};
        float t{gameClock.getElapsedTime().asSeconds()};

        // keep music looping robustly
        if (music.getStatus() == sf::SoundSource::Status::Stopped)
            music.play();

        // --------- EVENTS ---------
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        // --------- PLAYER MOVE ---------
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
            player.shape.move({-player.speed, 0.f});
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
            player.shape.move({player.speed, 0.f});

        auto pPos{player.shape.getPosition()};
        float halfWidth{player.shape.getGlobalBounds().size.x / 2.f};

        if (pPos.x < halfWidth) pPos.x = halfWidth;
        if (pPos.x > SCREEN_WIDTH - halfWidth) pPos.x = SCREEN_WIDTH - halfWidth;
        player.shape.setPosition(pPos);

        // simple wiggle animation (SFML 3: uses sf::Angle)
        float wiggleDegrees{std::sin(t * 2.f) * 3.f};
        player.shape.setRotation(sf::degrees(wiggleDegrees));

        // --------- SHOOTING (SPACE + COOLDOWN) ---------
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space) &&
            fireClock.getElapsedTime().asMilliseconds() > 250)
        {
            fireClock.restart();
            shootSound.play();

            auto spawn = [&](float offsetX, float speed)
            {
                for (auto& b : bullets)
                {
                    if (!b.active)
                    {
                        b.active = true;
                        b.speed = speed;
                        b.shape.setPosition({
                            player.shape.getPosition().x + offsetX,
                            player.shape.getPosition().y - 40.f
                        });
                        break;
                    }
                }
            };

            if (currentWeapon == WeaponType::Single)
                spawn(0.f, 10.f);
            else if (currentWeapon == WeaponType::Burst)
            {
                spawn(-20.f, 10.f);
                spawn(0.f, 10.f);
                spawn(20.f, 10.f);
            }
            else // Heavy
                spawn(0.f, 6.f);
        }

        // --------- BULLETS UPDATE ---------
        for (auto& b : bullets)
        {
            if (b.active)
            {
                b.shape.move({0.f, -b.speed});
                if (b.shape.getPosition().y < -50.f)
                    b.active = false;
            }
        }

        // --------- ENEMIES ---------
        for (auto& e : enemies)
        {
            if (!e.active && std::rand() % 150 == 0)
            {
                e.active = true;
                e.shape.setPosition({
                    static_cast<float>(std::rand() % (SCREEN_WIDTH - 80) + 40),
                    -40.f
                });
            }
            else if (e.active)
            {
                e.shape.move({0.f, e.speed});
                if (e.shape.getPosition().y > SCREEN_HEIGHT + 40.f)
                    e.active = false;
            }

            if (e.active)
            {
                float pulse{0.95f + 0.07f * std::sin(t * 4.f + e.animPhase)};
                e.shape.setScale({pulse, pulse});
            }
        }

        // --------- LIFE DROP ---------
        if (!lifeDrop.active && std::rand() % 600 == 0)
        {
            lifeDrop.active = true;
            lifeDrop.shape.setPosition({
                static_cast<float>(std::rand() % SCREEN_WIDTH),
                -20.f
            });
        }

        if (lifeDrop.active)
        {
            lifeDrop.shape.move({0.f, lifeDrop.speed});
            if (lifeDrop.shape.getPosition().y > SCREEN_HEIGHT + 20.f)
                lifeDrop.active = false;
        }

        // --------- WEAPON DROP ---------
        if (!weaponDrop.active && std::rand() % 700 == 0)
        {
            weaponDrop.active = true;
            weaponDrop.shape.setPosition({
                static_cast<float>(std::rand() % SCREEN_WIDTH),
                -20.f
            });

            int w{std::rand() % 3};
            if (w == 0) weaponDrop.type = WeaponType::Single;
            if (w == 1) weaponDrop.type = WeaponType::Burst;
            if (w == 2) weaponDrop.type = WeaponType::Heavy;
        }

        if (weaponDrop.active)
        {
            weaponDrop.shape.move({0.f, weaponDrop.speed});
            if (weaponDrop.shape.getPosition().y > SCREEN_HEIGHT + 20.f)
                weaponDrop.active = false;
        }

        // --------- COLLISIONS ---------
        for (auto& b : bullets)
        {
            if (!b.active) continue;

            for (auto& e : enemies)
            {
                if (e.active &&
                    b.shape.getGlobalBounds()
                        .findIntersection(e.shape.getGlobalBounds()))
                {
                    b.active = false;
                    e.active = false;
                    hitSound.play();
                    ++score;
                }
            }
        }

        for (auto& e : enemies)
        {
            if (e.active &&
                e.shape.getGlobalBounds()
                    .findIntersection(player.shape.getGlobalBounds()))
            {
                e.active = false;
                --player.lives;
                if (player.lives <= 0)
                    window.close();
            }
        }

        if (lifeDrop.active &&
            lifeDrop.shape.getGlobalBounds()
                .findIntersection(player.shape.getGlobalBounds()))
        {
            lifeDrop.active = false;
            ++player.lives;
        }

        if (weaponDrop.active &&
            weaponDrop.shape.getGlobalBounds()
                .findIntersection(player.shape.getGlobalBounds()))
        {
            currentWeapon = weaponDrop.type;
            weaponDrop.active = false;
        }

        // --------- UI ---------
        scoreText.setString("Score: " + std::to_string(score));
        livesText.setString("Lives: " + std::to_string(player.lives));

        std::string weaponName =
            (currentWeapon == WeaponType::Single) ? "Single" :
            (currentWeapon == WeaponType::Burst)  ? "Burst"  : "Heavy";
        weaponText.setString("Weapon: " + weaponName);

        // --------- SCROLL BACKGROUND ---------
        bg1.move({0.f, backgroundSpeed * dt});
        bg2.move({0.f, backgroundSpeed * dt});

        if (bg1.getPosition().y >= bgSize.y)
            bg1.setPosition({0.f, bg2.getPosition().y - bgSize.y});
        if (bg2.getPosition().y >= bgSize.y)
            bg2.setPosition({0.f, bg1.getPosition().y - bgSize.y});

        // --------- DRAW ---------
        window.clear();
        window.draw(bg1);
        window.draw(bg2);

        for (auto& b : bullets)
            if (b.active) window.draw(b.shape);

        for (auto& e : enemies)
            if (e.active) window.draw(e.shape);

        if (lifeDrop.active)   window.draw(lifeDrop.shape);
        if (weaponDrop.active) window.draw(weaponDrop.shape);

        window.draw(player.shape);
        window.draw(scoreText);
        window.draw(livesText);
        window.draw(weaponText);

        window.display();
    }

    return 0;
}

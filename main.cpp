#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <optional>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>

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

// ---------------- MAIN ----------------
int main()
{
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    sf::Clock fireClock;

    WeaponType currentWeapon = WeaponType::Single;

    // --------- WINDOW ---------
    sf::RenderWindow window(
        sf::VideoMode({SCREEN_WIDTH, SCREEN_HEIGHT}),
        "Arcade Shooter - Pickups & Sound"
    );
    window.setFramerateLimit(60);

    // --------- UI ---------
    int score = 0;
    sf::Font font;
    sf::Text scoreText(font), livesText(font), weaponText(font);

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

    // --------- SOUND ---------
    sf::SoundBuffer shootBuffer, hitBuffer;
    if (!shootBuffer.loadFromFile("/home/marius/CLionProjects/arcade-shooter/laser.wav"))
    {
        std::cout << "ERROR: Failed to load laser.wav\n";
    }

    if (!hitBuffer.loadFromFile("/home/marius/CLionProjects/arcade-shooter/explosion.wav"))
    {
        std::cout << "ERROR: Failed to load explosion.wav\n";
    }

    sf::Sound shootSound(shootBuffer), hitSound(hitBuffer);

    shootSound.setVolume(10.f);   // 10% volume
    hitSound.setVolume(10.f);     // 10% volume


    // --------- BACKGROUND MUSIC ---------
    sf::Music music;

    if (!music.openFromFile("/home/marius/CLionProjects/arcade-shooter/easy_music.wav"))   // or .wav
    {
        std::cout << "ERROR: Could not load music file\n";
    }
    else
    {
        music.setVolume(15.f);
        music.play();
    }






    // --------- PLAYER ---------
    Player player;
    player.shape.setSize({60.f, 20.f});
    player.shape.setFillColor(sf::Color::Green);
    player.shape.setPosition({
        SCREEN_WIDTH / 2.f - 30.f,
        SCREEN_HEIGHT - 30.f
    });
    player.speed = 6.f;
    player.lives = 3;

    // --------- BULLETS ---------
    Bullet bullets[MAX_BULLETS];
    for (auto& b : bullets) b.active = false;

    // --------- ENEMIES ---------
    Enemy enemies[MAX_ENEMIES];
    for (auto& e : enemies)
    {
        e.shape.setSize({40.f, 20.f});
        e.shape.setFillColor(sf::Color::Yellow);
        e.active = false;
    }

    // --------- LIFE DROP ---------
    LifeDrop lifeDrop;
    lifeDrop.shape.setRadius(10.f);
    lifeDrop.shape.setFillColor(sf::Color::Cyan);
    lifeDrop.speed = 2.f;
    lifeDrop.active = false;

    // --------- WEAPON DROP ---------
    WeaponDrop weaponDrop;
    weaponDrop.shape.setSize({20.f, 20.f});
    weaponDrop.speed = 2.2f;
    weaponDrop.active = false;

    // ================= GAME LOOP =================
    while (window.isOpen())
    {

        // ----- PERFECT INFINITE LOOP (SFML 3 SAFE METHOD) -----
        if (music.getStatus() == sf::SoundSource::Status::Stopped)
        {
            music.play();
        }


        // --------- EVENTS ---------
        while (const std::optional event = window.pollEvent())
            if (event->is<sf::Event::Closed>())
                window.close();

        // --------- PLAYER MOVE ---------
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
            player.shape.move({-player.speed, 0});
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
            player.shape.move({player.speed, 0});

        auto pos = player.shape.getPosition();
        if (pos.x < 0) pos.x = 0;
        if (pos.x > SCREEN_WIDTH - 60) pos.x = SCREEN_WIDTH - 60;
        player.shape.setPosition(pos);

        // --------- SHOOTING (SPACE + COOLDOWN) ---------
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space) &&
            fireClock.getElapsedTime().asMilliseconds() > 250)
        {
            fireClock.restart();
            shootSound.play();

            auto spawn = [&](float offset, float speed, sf::Vector2f size)
            {
                for (auto& b : bullets)
                    if (!b.active)
                    {
                        b.active = true;
                        b.speed = speed;
                        b.shape.setSize(size);
                        b.shape.setPosition({
                            player.shape.getPosition().x + offset,
                            player.shape.getPosition().y - size.y
                        });
                        break;
                    }
            };

            if (currentWeapon == WeaponType::Single)
                spawn(30.f, 10.f, {5.f, 15.f});
            else if (currentWeapon == WeaponType::Burst)
            {
                spawn(15.f, 10.f, {5.f, 15.f});
                spawn(30.f, 10.f, {5.f, 15.f});
                spawn(45.f, 10.f, {5.f, 15.f});
            }
            else
                spawn(24.f, 6.f, {14.f, 20.f});
        }

        // --------- BULLETS UPDATE ---------
        for (auto& b : bullets)
            if (b.active)
            {
                b.shape.move({0, -b.speed});
                if (b.shape.getPosition().y < 0)
                    b.active = false;
            }

        // --------- ENEMIES ---------
        for (auto& e : enemies)
        {
            if (!e.active && std::rand() % 150 == 0)
            {
                e.active = true;
                e.shape.setPosition({float(std::rand() % (SCREEN_WIDTH - 40)), -20});
                e.speed = 2.f;
            }
            else if (e.active)
            {
                e.shape.move({0, e.speed});
                if (e.shape.getPosition().y > SCREEN_HEIGHT)
                    e.active = false;
            }
        }

        // --------- LIFE DROP ---------
        if (!lifeDrop.active && std::rand() % 600 == 0)
        {
            lifeDrop.active = true;
            lifeDrop.shape.setPosition({static_cast<float>(std::rand() % SCREEN_WIDTH), -20});
        }

        if (lifeDrop.active)
        {
            lifeDrop.shape.move({0, lifeDrop.speed});
            if (lifeDrop.shape.getPosition().y > SCREEN_HEIGHT)
                lifeDrop.active = false;
        }

        // --------- WEAPON DROP ---------
        if (!weaponDrop.active && std::rand() % 700 == 0)
        {
            weaponDrop.active = true;
            weaponDrop.shape.setPosition({float(std::rand() % SCREEN_WIDTH), -20});

            int w = std::rand() % 3;
            if (w == 0) { weaponDrop.type = WeaponType::Single; weaponDrop.shape.setFillColor(sf::Color::Cyan); }
            if (w == 1) { weaponDrop.type = WeaponType::Burst;  weaponDrop.shape.setFillColor(sf::Color::Magenta); }
            if (w == 2) { weaponDrop.type = WeaponType::Heavy;  weaponDrop.shape.setFillColor(sf::Color::Red); }
        }

        if (weaponDrop.active)
        {
            weaponDrop.shape.move({0, weaponDrop.speed});
            if (weaponDrop.shape.getPosition().y > SCREEN_HEIGHT)
                weaponDrop.active = false;
        }

        // --------- COLLISIONS ---------
        for (auto& b : bullets)
            for (auto& e : enemies)
                if (b.active && e.active &&
                    b.shape.getGlobalBounds().findIntersection(e.shape.getGlobalBounds()))
                {
                    b.active = false;
                    e.active = false;
                    hitSound.play();
                    score++;
                }

        for (auto& e : enemies)
            if (e.active &&
                e.shape.getGlobalBounds().findIntersection(player.shape.getGlobalBounds()))
            {
                e.active = false;
                player.lives--;
                if (player.lives <= 0) window.close();
            }

        if (lifeDrop.active &&
            lifeDrop.shape.getGlobalBounds().findIntersection(player.shape.getGlobalBounds()))
        {
            lifeDrop.active = false;
            player.lives++;
        }

        if (weaponDrop.active &&
            weaponDrop.shape.getGlobalBounds().findIntersection(player.shape.getGlobalBounds()))
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

        // --------- DRAW ---------
        window.clear();
        window.draw(player.shape);

        for (auto& b : bullets)
            if (b.active) window.draw(b.shape);

        for (auto& e : enemies)
            if (e.active) window.draw(e.shape);

        if (lifeDrop.active) window.draw(lifeDrop.shape);
        if (weaponDrop.active) window.draw(weaponDrop.shape);

        window.draw(scoreText);
        window.draw(livesText);
        window.draw(weaponText);
        window.display();
    }

    return 0;
}

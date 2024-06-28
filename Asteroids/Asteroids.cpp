#include <iostream>
#include "CrabbyGraphics.h"

class Console : public CrabbyGraphics
{
private:
    struct sSpaceship
    {
        triangle tShip;
        float speed;
        float fAngle;
    } sShip;

    struct sBomb
    {
        vec_2d<float> p;
        float fAngle;
        float speed;
    };

    struct sAsteroid
    {
        point_2d pCenter;
        int radius;
    };
    
    std::vector<sBomb> vecBombs;
    std::vector<sAsteroid> vecAsteroids;

public:
    Console()
    {
    }

    bool Setup() override
    {
        // Construct and move ship to the center of the screen
        sShip = { {{4,0}, {0, 10}, {8, 10} }, 0.0f, 0.0f };
        TranslateTriangle({ ScreenWidth() / 2, ScreenHeight() / 2 }, sShip.tShip, sShip.tShip);

        SpawnAsteroids();

        return true;
    }

    bool Update(float fElapsedTime) override
    {
        ClearScreen();

        HandleInput();

        // Check for asteroid-ship collisions
        // If true, reset game
        for (auto& asteroid : vecAsteroids)
        {
            if (IsPointInsideCircle(sShip.tShip.p[0], asteroid.pCenter, asteroid.radius) || IsPointInsideCircle(sShip.tShip.p[1], asteroid.pCenter, asteroid.radius) || IsPointInsideCircle(sShip.tShip.p[2], asteroid.pCenter, asteroid.radius))
            {
                vecBombs.clear();
                vecAsteroids.clear();
                Setup();
                SpawnAsteroids();
            }
        }

        // If bombs are out of screen, despawn them
        for (int i = 0; i < vecBombs.size(); i++)
        {
            if (vecBombs[i].p.x < 0 || vecBombs[i].p.x > ScreenWidth() || vecBombs[i].p.y < 0 || vecBombs[i].p.y > ScreenHeight())
                vecBombs.erase(vecBombs.begin() + i); // DO NOT ADD BOMBS LOL.
        }

        // If bombs collide with asteroids, destory the asteroids
        // Iterate over all bombs
        for (int i=0; i < vecBombs.size(); i++)
        {
            for(int j=0; j < vecAsteroids.size(); j++)
            {
                if (IsPointInsideCircle(vecBombs[i].p, vecAsteroids[j].pCenter, vecAsteroids[j].radius))
                {
                    vecAsteroids.erase(vecAsteroids.begin() + j);
                    vecBombs.erase(vecBombs.begin() + i);
                    break;
                }
            }
        }

        // Draw bombs
        for (auto& bomb : vecBombs) {

            bomb.p.x += -bomb.speed * sin(bomb.fAngle) * fElapsedTime * 50.0f;
            bomb.p.y += bomb.speed * cos(bomb.fAngle) * fElapsedTime * 50.0f;

            Pixelate(bomb.p, FG_RED);
        }

        // Draw Asteroids
        for (auto& asteroid : vecAsteroids)
        {
            DrawCircle(asteroid.pCenter, asteroid.radius);
        }

        // Draw player (ship)
        vec_2d vec_translation{ (-sShip.speed * sin(sShip.fAngle) * fElapsedTime * 50.0f), (sShip.speed * cos(sShip.fAngle) * fElapsedTime * 50.0f) };
        TranslateTriangle(vec_translation, sShip.tShip, sShip.tShip);

        // If out of screen, bring back
        point_2d mp = sShip.tShip.midpoint();
        if (mp.x > ScreenWidth())
            TranslateTriangle(vec_2d{ -ScreenWidth(), 0 }, sShip.tShip, sShip.tShip);
        if (mp.x < 0)
            TranslateTriangle(vec_2d{ ScreenWidth(), 0 }, sShip.tShip, sShip.tShip);
        if (mp.y > ScreenHeight())
            TranslateTriangle(vec_2d{ 0, -ScreenHeight() }, sShip.tShip, sShip.tShip);
        if (mp.y < 0)
            TranslateTriangle(vec_2d{ 0, ScreenHeight() }, sShip.tShip, sShip.tShip);

        // Rotate ship
        triangle triRotated;
        triRotated.fillColor = FG_GREEN;
        RotateTriangle(sShip.tShip.midpoint(), sShip.fAngle, triRotated, sShip.tShip);

        // Draw ship
        DrawTriangle(triRotated);
        FillTriangle(triRotated);
        Pixelate(triRotated.midpoint(), FG_RED);

        // If there's no asteroids, create more!!
        if(!vecAsteroids.size())
            SpawnAsteroids();

        return true;
    }

    void HandleInput()
    {
        if (m_keys[VK_UP].bHeld)
            sShip.speed = -5.0f;
        else if (m_keys[VK_DOWN].bHeld)
            sShip.speed = 5.0f;
        else
            sShip.speed = 0.0f;

        if (sShip.fAngle > 6.28318)
            sShip.fAngle = 0.0f;

        if (m_keys[VK_RIGHT].bHeld)
            sShip.fAngle += 0.02f;
        else if (m_keys[VK_LEFT].bHeld)
            sShip.fAngle -= 0.02f;

        if (m_keys[VK_SPACE].bPressed)
        {
            sBomb b;
            b.p = { (float)sShip.tShip.midpoint().x, (float)sShip.tShip.midpoint().y};
            b.speed = -2.5f;
            b.fAngle = sShip.fAngle;

            vecBombs.push_back(b);
        }
    }

    void SpawnAsteroids()
    {
        auto DoCirclesOverlap = [](int x1, int y1, int x2, int y2, int r1, int r2) {
            return (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2) <= (r1 + r2) * (r1 + r2);
            };

        while (vecAsteroids.size() < 10)
        {
            here:
            int x, y;
            point_2d pShipCoord = sShip.tShip.midpoint();
            x = ScreenWidth() * Random();
            y = ScreenHeight() * Random();
            int r = Random(2, 8);

            if ((x < pShipCoord.x - 20 || x > pShipCoord.x + 20) && (y < pShipCoord.y - 20 || y > pShipCoord.y + 20))
            {
                // Check whether circles overlap
                for (auto& asteroid : vecAsteroids)
                {
                    if (DoCirclesOverlap(x, y, asteroid.pCenter.x, asteroid.pCenter.y, r, asteroid.radius))
                    {
                        goto here;
                    }
                }


                vecAsteroids.push_back({ {x, y}, r });
            }
        }
    }

    bool IsPointInsideCircle(const point_2d& p, const point_2d& c, int r)
    {
        return ((c.x - p.x)* (c.x - p.x) + (c.y - p.y) * (c.y - p.y)) <= r * r;
    }

    ~Console()
    {
    }
};

int main()
{
    Console console;

    if (console.ConstructConsole(160, 90, 4, 4))
        console.Start();
    else
        std::wcout << L"Select a different screen resolution/font dimension." << std::endl;

    return 0;
}
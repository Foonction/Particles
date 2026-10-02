#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <string>
#include <cmath> 

// Изолируем только системные инклюды
#ifdef _WIN32
#include "windows.h"
#else
#include <unistd.h> // Потребуется в будущем для системных штук под Linux
#endif

using namespace std;

int width = 110;
int height = 46;
int count_all = 100;
int delay = 33;

struct Particle
{
    float x, y, vector_x, vector_y, weight, temperature;
};

void CustomSettings(int argc, char* argv[])
{
    if (argc >= 5)
    {
        if (atoi(argv[1]) > 110 || atoi(argv[1]) < 60)
            width = 60;
        else
            width = atoi(argv[1]);

        if (atoi(argv[2]) > 46 || atoi(argv[2]) < 25)
            height = 25;
        else
            height = atoi(argv[2]);

        if (atoi(argv[3]) > 100)
            count_all = 50;
        else
            count_all = atoi(argv[3]);

        if (atoi(argv[4]) < 0)
            delay = 33;
        else
            delay = atoi(argv[4]);
    }
}

void MainScreen(string& buffer, vector<char> screen, float average_speed) //создание экрана в котором будут двигаться частицы
{
    buffer = "\033[H";
    buffer += string(width + 2, '-') + "\n";

    for (int y = 0; y < height; ++y)
    {
        buffer += '|';
        for (int x = 0; x < width; ++x)
        {
            if (screen[y * width + x] == 1)
                buffer += "\xE2\x97\x8F";//символ ●
            else
                buffer += ' ';
        }
        buffer += "|\n";
    }

    buffer += string(width + 2, '-') + "\n";

    buffer += "particles: " + to_string(count_all) + " | average speed: " + to_string(average_speed) + " units/frame\n";

    cout << buffer << flush;
}

float AverageSpeed(vector<Particle>& particles, int count_all)//Расчет средней скорости
{
    float total_speed = 0.0f;
    for (const auto& p : particles)
    {
        total_speed += sqrt(p.vector_x * p.vector_x + p.vector_y * p.vector_y);
    }
    float average_speed = (count_all > 0) ? (total_speed / count_all) : 0.0f;
    return average_speed;
}

void resolveParticleCollision(Particle& firstParticle, Particle& secondParticle)
{
    float distanceX = secondParticle.x - firstParticle.x;
    float distanceY = secondParticle.y - firstParticle.y;
    float distanceSquared = (distanceX * distanceX) + (distanceY * distanceY);

    float firstRadius = 1.0f;
    if (firstParticle.weight == 4.0f) firstRadius = 2.0f;
    else if (firstParticle.weight == 9.0f) firstRadius = 3.0f;

    float secondRadius = 1.0f;
    if (secondParticle.weight == 4.0f) secondRadius = 2.0f;
    else if (secondParticle.weight == 9.0f) secondRadius = 3.0f;

    float minimumCollisionDistance = firstRadius + secondRadius;

    if (distanceSquared < (minimumCollisionDistance * minimumCollisionDistance) && distanceSquared > 0.0f)
    {
        float actualDistance = std::sqrt(distanceSquared);

        float overlapDepth = minimumCollisionDistance - actualDistance;
        float totalWeight = firstParticle.weight + secondParticle.weight;

        firstParticle.x -= (distanceX / actualDistance) * overlapDepth * (secondParticle.weight / totalWeight);
        firstParticle.y -= (distanceY / actualDistance) * overlapDepth * (secondParticle.weight / totalWeight);
        secondParticle.x += (distanceX / actualDistance) * overlapDepth * (firstParticle.weight / totalWeight);
        secondParticle.y += (distanceY / actualDistance) * overlapDepth * (firstParticle.weight / totalWeight);

        float normalX = distanceX / actualDistance;
        float normalY = distanceY / actualDistance;
        float tangentX = -normalY;
        float tangentY = normalX;

        float normalVelocityFirst = (firstParticle.vector_x * normalX) + (firstParticle.vector_y * normalY);
        float tangentVelocityFirst = (firstParticle.vector_x * tangentX) + (firstParticle.vector_y * tangentY);

        float normalVelocitySecond = (secondParticle.vector_x * normalX) + (secondParticle.vector_y * normalY);
        float tangentVelocitySecond = (secondParticle.vector_x * tangentX) + (secondParticle.vector_y * tangentY);

        float relativeNormalVelocity = normalVelocityFirst - normalVelocitySecond;
        if (relativeNormalVelocity > 0.0f)
        {
            float generatedHeat = 0.05f * (relativeNormalVelocity * relativeNormalVelocity) * (firstParticle.weight * secondParticle.weight) / totalWeight;
            firstParticle.temperature += generatedHeat / firstParticle.weight;
            secondParticle.temperature += generatedHeat / secondParticle.weight;
        }

        float k = 0.95f;

        float normalVelocityFirstAfter = ((normalVelocityFirst * (firstParticle.weight - secondParticle.weight * k)) + (secondParticle.weight * normalVelocitySecond * (1.0f + k))) / totalWeight;
        float normalVelocitySecondAfter = ((normalVelocitySecond * (secondParticle.weight - firstParticle.weight * k)) + (firstParticle.weight * normalVelocityFirst * (1.0f + k))) / totalWeight;

        firstParticle.vector_x = (normalVelocityFirstAfter * normalX) + (tangentVelocityFirst * tangentX);
        firstParticle.vector_y = (normalVelocityFirstAfter * normalY) + (tangentVelocityFirst * tangentY);

        secondParticle.vector_x = (normalVelocitySecondAfter * normalX) + (tangentVelocitySecond * tangentX);
        secondParticle.vector_y = (normalVelocitySecondAfter * normalY) + (tangentVelocitySecond * tangentY);
    }
}

vector<char> CreateScreen(int width, int height, vector<Particle>& particles, int count_all)//создание экрана, физики движения, столкновения частиц
{
    vector<char> screen(width * height, 0);

    for (int i = 0; i < count_all; ++i)
    {
        particles[i].x += particles[i].vector_x;
        particles[i].y += particles[i].vector_y;

        if (particles[i].temperature > 20.0f)
        {
            particles[i].temperature -= 0.02f;
            if (particles[i].temperature < 20.0f) particles[i].temperature = 20.0f;
        }

        float radius = 1.0f;
        if (particles[i].weight == 4.0f) radius = 2.0f;
        else if (particles[i].weight == 9.0f) radius = 3.0f;

        if (particles[i].x <= radius)
        {
            particles[i].x = radius;
            particles[i].vector_x *= -1;
            particles[i].temperature += 0.05f; // Легкий нагрев при ударе о левую стену
        }
        else if (particles[i].x >= width - 1 - radius)
        {
            particles[i].x = width - 1 - radius;
            particles[i].vector_x *= -1;
            particles[i].temperature += 0.05f; // Легкий нагрев при ударе о правую стену
        }

        if (particles[i].y <= radius)
        {
            particles[i].y = radius;
            particles[i].vector_y *= -1;
            particles[i].temperature += 0.05f; // Легкий нагрев при ударе о верхнюю стену
        }
        else if (particles[i].y >= height - 1 - radius)
        {
            particles[i].y = height - 1 - radius;
            particles[i].vector_y *= -1;
            particles[i].temperature += 0.05f; // Легкий нагрев при ударе о нижнюю стену
        }

        for (int j = i + 1; j < count_all; ++j)
        {
            resolveParticleCollision(particles[i], particles[j]);
        }

        int ix = (int)particles[i].x;
        int iy = (int)particles[i].y;

        if (ix >= 0 && ix < width && iy >= 0 && iy < height)
        {
            screen[iy * width + ix] = 1;
        }
    }
    return screen;
}

int main(int argc, char* argv[])
{
    // Изолируем настройки консоли под Windows
#ifdef _WIN32
    SetConsoleOutputCP(65001);
#endif

    srand(time(0));

    CustomSettings(argc, argv);

    vector<Particle> particles(count_all);
    for (auto& i : particles)
    {
        i = {
        (float)(rand() % (width - 6) + 3),
        (float)(rand() % (height - 6) + 3),
        (rand() % 20 - 10) / 20.0f,
        (rand() % 20 - 10) / 20.0f,
        (float)(pow(rand() % 3 + 1, 2)),
        20.0f // Стартовая комнатная температура частицы
        };
    }

    string buffer;
    buffer.reserve(width * height * 4);

    // Изолируем скрытие курсора под Windows
#ifdef _WIN32
    HWND hWnd = GetConsoleWindow();
    HideCaret(hWnd);
#else
    // Под Linux скрытие курсора делается изящно через одну ANSI-команду вывода
    cout << "\033[?25l" << flush;
#endif

    while (true) {
        vector<char> screen = CreateScreen(width, height, particles, count_all);

        float average_speed = AverageSpeed(particles, count_all);

        MainScreen(buffer, screen, average_speed);
        this_thread::sleep_for(chrono::milliseconds(delay));
    }
}

#pragma warning(disable: 4996)
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <string>
#include <cmath> 
#include <cstdlib> 
#include "curses.h"

#define _HAS_STD_BYTE 0
using namespace std;

#ifdef _WIN32
    #include <Windows.h>
#endif 


// Глобальные динамические размеры симуляции частиц
int width_disp = 0, height_disp = 0;
int width = 110;         // Текущая рабочая ширина (пересчитывается в цикле)
int height = 46;        // Текущая рабочая высота (пересчитывается в цикле)
int count_all = 50;     // Стартовое количество физических объектов
int delay = 33;         // Задержка кадра для анимации (~30 FPS)

// Описание свойств отдельной частицы
struct Particle
{
    double x, y;               // Положение центра частицы на экране
    double vector_x, vector_y; // Проекции скорости на оси X и Y
    float weight;             // Масса, определяющая геометрию и физику удара
    float temperature;        // Нагрев, управляющий сменой цветов палитры
};

// Загрузка кастомных настроек из входных аргументов процесса
void CustomSettings(int argc, char* argv[])
{
    if (argc >= 5)
    {
        if (atoi(argv[1]) > 110 || atoi(argv[1]) < 50) width = 50;
        else width = atoi(argv[1]);

        if (atoi(argv[2]) > 46 || atoi(argv[2]) < 20) height = 20;
        else height = atoi(argv[2]);

        if (atoi(argv[3]) > 100 || atoi(argv[3]) < 1) count_all = 50;
        else count_all = atoi(argv[3]);

        if (atoi(argv[4]) < 0) delay = 33;
        else delay = atoi(argv[4]);
    }
}

// Расчет усредненного показателя скорости всей системы
float AverageSpeed(vector<Particle>& particles, int count_all)
{
    float total_speed = 0.0f;
    for (const auto& p : particles)
    {
        total_speed += sqrt(p.vector_x * p.vector_x + p.vector_y * p.vector_y);
    }
    return (count_all > 0) ? (total_speed / count_all) : 0.0f;
}

// Скорректированное связывание веса тела с его физическим и графическим радиусом
float GetParticleRadius(float weight)
{
    if (weight == 9.0f) return 1.2f;
    if (weight == 4.0f) return 0.7f;
    return 0.5f;
}

// Расчет межчастичных упругих столкновений, наката тепла и обмена скоростями
void resolveParticleCollision(Particle& firstParticle, Particle& secondParticle)
{
    float distanceX = secondParticle.x - firstParticle.x;
    float distanceY = (secondParticle.y - firstParticle.y) * 1.8f;
    float distanceSquared = (distanceX * distanceX) + (distanceY * distanceY);

    float firstRadius = GetParticleRadius(firstParticle.weight);
    float secondRadius = GetParticleRadius(secondParticle.weight);
    float minimumCollisionDistance = firstRadius + secondRadius;

    if (distanceSquared < (minimumCollisionDistance * minimumCollisionDistance) && distanceSquared > 0.0f)
    {
        float actualDistance = std::sqrt(distanceSquared);
        float overlapDepth = minimumCollisionDistance - actualDistance;
        float totalWeight = firstParticle.weight + secondParticle.weight;

        firstParticle.x -= (distanceX / actualDistance) * overlapDepth * (secondParticle.weight / totalWeight);
        firstParticle.y -= ((distanceY / actualDistance) * overlapDepth * (secondParticle.weight / totalWeight)) / 1.8f;
        secondParticle.x += (distanceX / actualDistance) * overlapDepth * (firstParticle.weight / totalWeight);
        secondParticle.y += ((distanceY / actualDistance) * overlapDepth * (firstParticle.weight / totalWeight)) / 1.8f;

        float normalX = distanceX / actualDistance;
        float normalY = distanceY / actualDistance;
        float tangentX = -normalY;
        float tangentY = normalX;

        float normalVelocityFirst = (firstParticle.vector_x * normalX) + (firstParticle.vector_y * 1.8f * normalY);
        float tangentVelocityFirst = (firstParticle.vector_x * tangentX) + (firstParticle.vector_y * 1.8f * tangentY);
        float normalVelocitySecond = (secondParticle.vector_x * normalX) + (secondParticle.vector_y * normalY);
        float tangentVelocitySecond = (secondParticle.vector_x * tangentX) + (secondParticle.vector_y * 1.8f * tangentY);

        float relativeNormalVelocity = normalVelocityFirst - normalVelocitySecond;
        if (relativeNormalVelocity > 0.0f)
        {
            float generatedHeat = 25.0f * (relativeNormalVelocity * relativeNormalVelocity) * (firstParticle.weight * secondParticle.weight) / totalWeight;
            firstParticle.temperature += generatedHeat / firstParticle.weight;
            secondParticle.temperature += generatedHeat / secondParticle.weight;
        }

        float k = 0.95f;
        float normalVelocityFirstAfter = ((normalVelocityFirst * (firstParticle.weight - secondParticle.weight * k)) + (secondParticle.weight * normalVelocitySecond * (1.0f + k))) / totalWeight;
        float normalVelocitySecondAfter = ((normalVelocitySecond * (secondParticle.weight - firstParticle.weight * k)) + (firstParticle.weight * normalVelocityFirst * (1.0f + k))) / totalWeight;

        firstParticle.vector_x = (normalVelocityFirstAfter * normalX) + (tangentVelocityFirst * tangentX);
        firstParticle.vector_y = ((normalVelocityFirstAfter * normalY) + (tangentVelocityFirst * tangentY)) / 1.8f;
        secondParticle.vector_x = (normalVelocitySecondAfter * normalX) + (tangentVelocitySecond * tangentX);
        secondParticle.vector_y = ((normalVelocitySecondAfter * normalY) + (tangentVelocitySecond * tangentY)) / 1.8f;
    }
}

// Обновление физических координат движения и прорисовка
void UpdatePhysicsAndDraw(vector<Particle>& particles, int count_all)
{
    for (int i = 0; i < count_all; ++i)
    {
        particles[i].x += particles[i].vector_x;
        particles[i].y += particles[i].vector_y / 1.8f;

        if (particles[i].temperature > 20.0f)
        {
            particles[i].temperature -= 0.15f;
            if (particles[i].temperature < 20.0f) particles[i].temperature = 20.0f;
        }

        float radius = GetParticleRadius(particles[i].weight);

        // Отскок от динамических стен: теперь проверяем границы по текущему width и height кадра
        if (particles[i].x <= radius) { particles[i].x = radius; particles[i].vector_x *= -1; particles[i].temperature += 2.0f; }
        else if (particles[i].x >= width - radius) { particles[i].x = width - radius; particles[i].vector_x *= -1; particles[i].temperature += 2.0f; }

        if (particles[i].y <= radius / 1.8f) { particles[i].y = radius / 1.8f; particles[i].vector_y *= -1; particles[i].temperature += 2.0f; }
        else if (particles[i].y >= height - radius / 1.8f) { particles[i].y = height - radius / 1.8f; particles[i].vector_y *= -1; particles[i].temperature += 2.0f; }

        for (int j = i + 1; j < count_all; ++j)
        {
            resolveParticleCollision(particles[i], particles[j]);
        }
    }

    for (int i = 0; i < count_all; ++i)
    {
        int color_index = 1;
        if (particles[i].temperature > 23.0f) color_index = 2;

        int ix = (int)round(particles[i].x) + 1;
        int iy = (int)round(particles[i].y) + 1;

        attron(COLOR_PAIR(color_index));

        const char* particle_char = "o";

        if (particles[i].weight == 9.0f)
        {
            if (ix - 1 > 0 && ix + 1 <= width && iy - 1 > 0 && iy + 1 <= height)
            {
                mvaddstr(iy - 1, ix, particle_char);
                mvaddstr(iy, ix - 1, particle_char);
                mvaddstr(iy, ix, particle_char);
                mvaddstr(iy, ix + 1, particle_char);
                mvaddstr(iy + 1, ix, particle_char);
            }
        }
        else if (particles[i].weight == 4.0f)
        {
            if (ix > 0 && ix + 1 <= width && iy > 0 && iy <= height)
            {
                mvaddstr(iy, ix, particle_char);
                mvaddstr(iy, ix + 1, particle_char);
            }
        }
        else
        {
            if (ix > 0 && ix <= width && iy > 0 && iy <= height)
            {
                mvaddstr(iy, ix, particle_char);
            }
        }

        attroff(COLOR_PAIR(color_index));
    }
}

int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "");
    CustomSettings(argc, argv);

    initscr();
    noecho();
    curs_set(0);
    start_color();
#ifdef _WIN32
    setlocale(LC_ALL, ".UTF-8");
    SetConsoleOutputCP(CP_UTF8);
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);
#else
    use_default_colors();
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);
#endif

    srand(time(NULL));

    vector<Particle> particles;
    for (int i = 0; i < count_all; ++i)
    {
        float current_weight = 1.0f;
        if (i % 3 == 1) current_weight = 4.0f;
        else if (i % 3 == 2) current_weight = 9.0f;

        float r = GetParticleRadius(current_weight);

        float vx = ((float)(rand() % 800) - 400.0f) / 1000.0f;
        float vy = ((float)(rand() % 800) - 400.0f) / 1000.0f;

        if (abs(vx) < 0.05f) vx = 0.25f;
        if (abs(vy) < 0.05f) vy = 0.20f;

        // Изначально спавним в безопасной центральной зоне, чтобы они не застряли при изменении окна
        particles.push_back({ (float)(rand() % (width - 2) + 1), (float)(rand() % (height - 3) + 1), vx, vy, current_weight, 20.0f });
    }

    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE); // Переводим getch в неблокирующий режим (чтобы цикл не застывал)

    noecho();
    while (true)
    {
        int curr_width, curr_height;
        getmaxyx(stdscr, curr_height, curr_width);
        if (is_termresized() || width_disp != curr_width || height_disp != curr_height)
        { 
            resize_term(0,0);
            redrawwin(stdscr);
            clear();
            width_disp = curr_width;
            height_disp = curr_height;
        }
        erase();
        

        // Вывод динамической рамки поля, которая подстраивается под экран
        for (int x = 0; x < width + 2; x++) {
            mvaddch(0, x, '-');
            mvaddch(height + 1, x, '-');
        }
        for (int y = 1; y <= height; y++) {
            mvaddch(y, 0, '|');
            mvaddch(y, width + 1, '|');
        }

        UpdatePhysicsAndDraw(particles, count_all);

        float avg_speed = AverageSpeed(particles, count_all);
        mvprintw(height + 2, 0, "particles: %d | average speed: %f units/frame", count_all, avg_speed);
        refresh();
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
        
    }
}
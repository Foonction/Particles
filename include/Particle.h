#pragma once
#include "Settings.h"
// Описание свойств отдельной частицы
struct Particle
{
    double x, y;               // Положение центра частицы на экране
    double vector_x, vector_y; // Проекции скорости на оси X и Y
    float weight;             // Масса, определяющая геометрию и физику удара
    float temperature;        // Нагрев, управляющий сменой цветов палитры
};

// Скорректированное связывание веса тела с его физическим и графическим радиусом
float GetParticleRadius(float weight)
{
    if (weight == 9.0f) return 1.2f;
    if (weight == 4.0f) return 0.7f;
    return 0.5f;
}

std::vector<std::unique_ptr<Particle>> fabric()
{
    std::vector<std::unique_ptr<Particle>> particles;
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
        particles.push_back(std::make_unique<Particle>(Particle
            {
                (double)(rand() % (width - 2) + 1), 
                (double)(rand() % (height - 3) + 1), 
                (double)vx, 
                (double)vy, 
                current_weight, 
                20.0f }));
    }
    return particles;
}
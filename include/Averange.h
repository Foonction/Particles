#pragma once
#include "Settings.h"
#include "Particle.h"

// Расчет усредненного показателя скорости всей системы
double AverageSpeed(const std::vector<std::unique_ptr<Particle>>& particles, int count_all)
{
    double total_speed = 0.0f;
    for (const auto& p : particles)
    {
        total_speed += sqrt(p->vector_x * p->vector_x + p->vector_y * p->vector_y);
    }
    return (count_all > 0) ? (total_speed / count_all) : 0.0f;
}

//Расчет средней температуры всех частиц в симуляции
double AverengeTemperature(const std::vector<std::unique_ptr<Particle>>& particles, int count_all)
{
    double total_temperature = 0.0f;
    for (const auto& p : particles)
    {
        total_temperature += p->temperature;
    }
    return (count_all > 0) ? (total_temperature / count_all) : 0.0f;
}
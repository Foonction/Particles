#pragma once

#ifdef _WIN32
    #include "curses.h"
#else
    #include "ncurses.h"
#endif 
// Метод расчета и применения силы всемирного тяготения для пары объектов
void applyGravitation(const std::unique_ptr<Particle>& firstParticle, const std::unique_ptr<Particle>& secondParticle)
{
    // Расстояние между центрами с учетом вертикального шага консоли
    double distanceX = secondParticle->x - firstParticle->x;
    double distanceY = (secondParticle->y - firstParticle->y) * 1.8f;
    double distanceSquared = (distanceX * distanceX) + (distanceY * distanceY);

    // Защита от критического сближения и деления на ноль
    if (distanceSquared < 0.1) return;

    double actualDistance = sqrt(distanceSquared);

    // Гравитационная константа 


    // Классическая формула Ньютона: F = G * (m1 * m2) / r^2
    // Добавляем коэффициент 4.0, чтобы сила не улетала в бесконечность при сближении центров
    double force = G * (firstParticle->weight * secondParticle->weight) / (distanceSquared + 4.0f);


    // Расчет проекции вектора силы на оси X и Y
    double forceX = force * (distanceX / actualDistance);
    double forceY = force * (distanceY / actualDistance);

    // Изменение проекций скоростей (ускорение a = F / m)
    firstParticle->vector_x += forceX / firstParticle->weight;
    firstParticle->vector_y += (forceY / firstParticle->weight) / 1.8f;

    secondParticle->vector_x -= forceX / secondParticle->weight;
    secondParticle->vector_y -= (forceY / secondParticle->weight) / 1.8f;
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
void UpdatePhysicsAndDraw(const std::vector<std::unique_ptr<Particle>>& particles, int count_all)
{
    for (int i = 0; i < count_all; ++i)
    {
        for (int j = i + 1; j < count_all; ++j)
        {
            applyGravitation(particles[i], particles[j]);
        }

        particles[i]->x += particles[i]->vector_x;
        particles[i]->y += particles[i]->vector_y / 1.8f;

        if (particles[i]->temperature > 20.0f)
        {
            particles[i]->temperature -= 0.15f;
            if (particles[i]->temperature < 20.0f) particles[i]->temperature = 20.0f;
        }

        float radius = GetParticleRadius(particles[i]->weight);

        // Отскок от динамических стен: теперь проверяем границы по текущему width и height кадра
        if (particles[i]->x <= radius) { particles[i]->x = radius; particles[i]->vector_x *= -1; particles[i]->temperature += 2.0f; }
        else if (particles[i]->x >= width - radius) { particles[i]->x = width - radius; particles[i]->vector_x *= -1; particles[i]->temperature += 2.0f; }

        if (particles[i]->y <= radius / 1.8f) { particles[i]->y = radius / 1.8f; particles[i]->vector_y *= -1; particles[i]->temperature += 2.0f; }
        else if (particles[i]->y >= height - radius / 1.8f) { particles[i]->y = height - radius / 1.8f; particles[i]->vector_y *= -1; particles[i]->temperature += 2.0f; }

        for (int j = i + 1; j < count_all; ++j)
        {
            resolveParticleCollision(*particles[i], *particles[j]);
        }
    }

    for (int i = 0; i < count_all; ++i)
    {
        int color_index = 1;
        if (particles[i]->temperature > 23.0f) color_index = 2;

        int ix = (int)round(particles[i]->x) + 1;
        int iy = (int)round(particles[i]->y) + 1;

        attron(COLOR_PAIR(color_index));

        std::string up = "¯";
        if (particles[i]->weight == 9.0f)
        {
            if (ix - 1 > 0 && ix + 1 <= width && iy - 1 > 0 && iy + 1 <= height)
            {
                mvaddstr(iy - 1, ix, "_");
                mvaddstr(iy, ix - 1, "(");
                mvaddstr(iy, ix + 1, ")");
                mvaddstr(iy + 1, ix, "\"");
            }
        }
        else if (particles[i]->weight == 4.0f)
        {
            if (ix > 0 && ix + 1 <= width && iy > 0 && iy <= height)
            {
                mvaddstr(iy, ix, "(");
                mvaddstr(iy, ix + 1, ")");
            }
        }
        else
        {
            if (ix > 0 && ix <= width && iy > 0 && iy <= height)
            {
                mvaddstr(iy, ix, "o");
            }
        }

        attroff(COLOR_PAIR(color_index));
    }
}
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <string>
#include <cmath> 
#include "windows.h"

using namespace std;

int width = 110;
int height = 46;
int count_all = 100;
int delay = 33;

struct Particle
{
    float x, y, vector_x, vector_y;
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

    buffer += "particles: " + to_string(count_all) + " | average speed: " + to_string(average_speed) + " units/frame\n";//units/frame - это скорость прохода 
                                                                                                                        //символов в кадр
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
vector<char> CreateScreen(int width, int height, vector<Particle>& particles, int count_all)//создание экрана, физики движения, столкновения частиц
{
    vector<char> screen(width * height, 0);

    for (int i = 0; i < count_all; ++i)
    {
        particles[i].x += particles[i].vector_x;
        particles[i].y += particles[i].vector_y;

        if (particles[i].x <= 0 || particles[i].x >= width - 1)
        {
            particles[i].vector_x *= -1;
        }
        if (particles[i].y <= 0 || particles[i].y >= height - 1)
        {
            particles[i].vector_y *= -1;
        }

        for (int j = i + 1; j < count_all; ++j)
        {
            float dx = particles[j].x - particles[i].x;
            float dy = particles[j].y - particles[i].y;
            if (dx * dx + dy * dy < 1.0f)
            {
                swap(particles[i].vector_x, particles[j].vector_x);
                swap(particles[i].vector_y, particles[j].vector_y);
            }
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
    SetConsoleOutputCP(65001);

    CustomSettings(argc, argv);

    vector<Particle> particles(count_all);
    for (auto& i : particles)
    {
        i = { 
        (float)(rand() % width), 
        (float)(rand() % height),
        (rand() % 20 - 10) / 20.0f, 
        (rand() % 20 - 10) / 20.0f 
        };
    }

    string buffer;
    buffer.reserve(width * height * 4);

    HWND hWnd = GetConsoleWindow();
    HideCaret(hWnd);

    while (true) {
        vector<char> screen = CreateScreen(width, height, particles, count_all);

        float average_speed = AverageSpeed(particles, count_all);

        MainScreen(buffer, screen, average_speed);
        this_thread::sleep_for(chrono::milliseconds(delay));
    }
}

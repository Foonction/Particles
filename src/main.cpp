#pragma warning(disable: 4996)
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <string>
#include <cmath> 
#include <cstdlib> 
#include "../include/Settings.h"
#include "../include/Particle.h"
#include "../include/Physics.h"
#include "../include/Averange.h"
#ifdef _WIN32
    #include "curses.h"
    #include <Windows.h>
#else
    #include "ncurses.h"
#endif 

using namespace std;




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


int main(int argc, char* argv[])
{

#ifdef _WIN32
    // Отключаем возможность максимизировать окно и менять его размер мышкой
    HWND hwnd = GetConsoleWindow();
    DWORD style = GetWindowLong(hwnd, GWL_STYLE);
    ShowWindow(hwnd, SW_MAXIMIZE);
    style &= ~WS_MAXIMIZEBOX; // Убираем кнопку "Развернуть"
    style &= ~WS_THICKFRAME;  // Убираем рамку изменения размера
    SetWindowLong(hwnd, GWL_STYLE, style);
    SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
#endif


    setlocale(LC_ALL, "");
    CustomSettings(argc, argv);

    initscr();
    noecho();
    curs_set(0);
    start_color();
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);
#else
    //use_default_colors();
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);
#endif

    srand(time(NULL));

    std::vector<std::unique_ptr<Particle>> particles;
    
    particles = fabric();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE); // Переводим getch в неблокирующий режим (чтобы цикл не застывал)

    noecho();
    while (true)
    {   
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
        
        double avg_speed = AverageSpeed(particles, count_all);
        double avg_temp = AverengeTemperature(particles, count_all);
        mvprintw(height + 2, 0, "particles: %d | average speed: %f units/frame | average_temperature: %e *C", count_all, avg_speed, avg_temp);
        refresh();
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
        
    }
}

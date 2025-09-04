#include "AARTZE/core/Application.hpp"
#include <exception>
#include <iostream>

int main()
{
    try
    {
        Application app(1280, 720, "AARTZE Game");
        app.Run();
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
}


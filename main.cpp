#include <iostream>
#include <stdexcept>

#include "src/programs/model_application.hpp"

/*
    Feito por Israel Burlamaqui.

    O código não funciona corretamente no
    Ambiente Desktop Gnome por enquanto!
    
    Você precisa de um DE que suporte o
    sistema de janelas X11, como o XFCE4, 
    para que o código funcione corretamente.
*/

int main()
{
    ModelApplication app;

    try 
    {
        app.run();
    } 
    catch (const std::exception& e) 
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
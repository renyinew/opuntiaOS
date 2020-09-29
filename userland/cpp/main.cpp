#include <libui/WindowConnection.h>
#include <syscalls.h>

int main(int argc, char** argv)
{
    fork();
    auto window = Window::Window();
    window.run();
    while (1) {}
    return 0;
}
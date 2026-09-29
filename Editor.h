#pragma once
#include <memory>
#include "ITextBuffer.h"

class Editor
{
private:
    std::string current_filename;
    std::unique_ptr<ITextBuffer> buffer;
    int cursor_x;
    int cursor_y;
    bool running;

    void handle_input(int ch);
    void draw();

public:
    Editor(std::unique_ptr<ITextBuffer> text_buffer, const std::string &filename);
    void run();
};
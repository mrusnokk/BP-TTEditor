#pragma once
#include <memory>
#include "ITextBuffer.h"

enum class Mode
{
    EDIT,
    COMMAND
};

class Editor
{
private:
    std::string current_filename;
    std::unique_ptr<ITextBuffer> buffer;
    int cursor_x;
    int cursor_y;
    bool running;

    Mode mode;
    std::string command_buffer;
    std::string status_message;

    void handle_input(int ch);
    void handle_edit_mode(int ch);
    void handle_command_mode(int ch);
    void execute_command();
    void draw();

public:
    Editor(std::unique_ptr<ITextBuffer> text_buffer, const std::string &filename);
    void run();
};
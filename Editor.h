#pragma once
#include <memory>
#include <string>
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
    int row_offset;
    int col_offset;
    bool running;

    Mode mode;
    std::string command_buffer;
    std::string status_message;

    int get_gutter_width(size_t line_count) const;
    void update_viewport(int text_height, int text_width);
    void handle_input(int ch);
    void handle_edit_mode(int ch);
    void handle_command_mode(int ch);
    void execute_command();
    void show_help();
    void draw();

public:
    Editor(std::unique_ptr<ITextBuffer> text_buffer, const std::string &filename);
    ~Editor();
    void run();
};

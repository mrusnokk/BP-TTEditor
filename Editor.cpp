#include "Editor.h"
#include <ncurses.h>

Editor::Editor(std::unique_ptr<ITextBuffer> text_buffer)
    : buffer(std::move(text_buffer)), cursor_x(0), cursor_y(0), running(true)
{

    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
}

void Editor::run()
{
    while (running)
    {
        draw();
        int ch = getch();
        handle_input(ch);
    }
    endwin();
}

void Editor::draw()
{
    clear();
    auto lines = buffer->get_lines();
    for (size_t i = 0; i < lines.size(); ++i)
    {
        mvprintw(i, 0, "%s", lines[i].c_str());
    }
    move(cursor_y, cursor_x);
    refresh();
}

void Editor::handle_input(int ch)
{
    switch (ch)
    {
    case 27: // ESC
        running = false;
        break;
    case KEY_BACKSPACE:
    case 127:
    case '\b':
        if (cursor_x > 0)
        {
            buffer->delete_char(cursor_y, cursor_x);
            cursor_x--;
        }
        break;
    case KEY_LEFT:
        if (cursor_x > 0)
            cursor_x--;
        break;
    case KEY_RIGHT:
        cursor_x++;
        break;
    default:
        if (ch >= 32 && ch <= 126)
        {
            buffer->insert_char(ch, cursor_y, cursor_x);
            cursor_x++;
        }
        break;
    }
}
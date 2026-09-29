#include "Editor.h"
#include <ncurses.h>

Editor::Editor(std::unique_ptr<ITextBuffer> text_buffer, const std::string &filename)
    : buffer(std::move(text_buffer)), current_filename(filename), cursor_x(0), cursor_y(0), running(true)
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
        else if (cursor_y > 0)
        {
            auto lines = buffer->get_lines();

            int delka_horniho_radku = lines[cursor_y - 1].length();
            buffer->delete_char(cursor_y, cursor_x);

            cursor_y--;
            cursor_x = delka_horniho_radku;
        }
        break;
    case KEY_LEFT:
        if (cursor_x > 0)
            cursor_x--;
        break;
    case KEY_RIGHT:
    {
        auto lines = buffer->get_lines();
        if (cursor_x < static_cast<int>(lines[cursor_y].length()))
            cursor_x++;

        break;
    }
    case '\n':
    case KEY_ENTER:
        // case 10://nektere terminaly posilaji enter jako kod 10 nebo \n
        buffer->insert_newline(cursor_y, cursor_x);
        cursor_y++;
        cursor_x = 0;
        break;

    case KEY_UP:
    {
        if (cursor_y > 0)
        {
            cursor_y--; // posun o radek vys

            auto lines = buffer->get_lines();
            if (cursor_x > static_cast<int>(lines[cursor_y].length()))
            {
                cursor_x = lines[cursor_y].length(); // nastavi se na konec textu radku
            }
        }
        break;
    }

    case KEY_DOWN:
    {
        auto lines = buffer->get_lines();
        if (cursor_y < static_cast<int>(lines.size()) - 1)
        {
            cursor_y++; // posun o radek nize

            if (cursor_x > static_cast<int>(lines[cursor_y].length()))
            {
                cursor_x = lines[cursor_y].length(); // nastaveni na konec textu radku
            }
        }
        break;
    }
    case KEY_F(2): // ukladani
        buffer->save_to_file(current_filename);
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
#include "Editor.h"
#include <ncurses.h>
#include <filesystem>
#include <algorithm>

Editor::Editor(std::unique_ptr<ITextBuffer> text_buffer, const std::string &filename)
    : current_filename(filename),
      buffer(std::move(text_buffer)),
      cursor_x(0),
      cursor_y(0),
      row_offset(0),
      col_offset(0),
      running(true),
      mode(Mode::EDIT)
{
    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    scrollok(stdscr, FALSE);
    set_escdelay(25); // zkraceni prodlevy esc

    if (has_colors())
    {
        start_color();
        use_default_colors();
        init_pair(1, COLOR_CYAN, -1);
        init_pair(2, COLOR_BLACK, COLOR_WHITE);
        init_pair(3, COLOR_RED, -1);
        init_pair(4, COLOR_GREEN, -1);
        init_pair(5, COLOR_BLUE, -1);
    }
}

Editor::~Editor()
{
    if (!isendwin())
    {
        endwin();
    }
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

int Editor::get_gutter_width(size_t line_count) const
{
    int digits = 1;
    size_t temp = line_count;
    while (temp >= 10)
    {
        digits++;
        temp /= 10;
    }
    if (digits < 2)
        digits = 2;
    return digits + 3;
}

void Editor::update_viewport(int text_height, int text_width)
{
    if (cursor_y < row_offset)
    {
        row_offset = cursor_y;
    }
    if (cursor_y >= row_offset + text_height)
    {
        row_offset = cursor_y - text_height + 1;
    }

    int margin_x = 2;
    if (text_width > 8)
    {
        if (cursor_x >= col_offset + text_width - margin_x)
        {
            col_offset = cursor_x - text_width + margin_x + 1;
        }
        if (cursor_x < col_offset + margin_x)
        {
            col_offset = std::max(0, cursor_x - margin_x);
        }
    }
    else
    {
        if (cursor_x >= col_offset + text_width)
        {
            col_offset = cursor_x - text_width + 1;
        }
        if (cursor_x < col_offset)
        {
            col_offset = cursor_x;
        }
    }
}

void Editor::draw()
{
    erase();
    auto lines = buffer->get_lines();

    int text_height = LINES - 1;
    if (text_height < 1)
        text_height = 1;

    int gutter_width = get_gutter_width(lines.size());
    int digits = gutter_width - 3;
    if (COLS <= gutter_width + 5)
    {
        gutter_width = 0;
    }

    int text_width = COLS - gutter_width - 1;
    if (text_width < 1)
        text_width = 1;

    update_viewport(text_height, text_width);

    for (int i = 0; i < text_height; ++i)
    {
        int file_line = row_offset + i;
        if (file_line < static_cast<int>(lines.size()))
        {
            if (gutter_width > 0)
            {
                if (has_colors())
                    attron(COLOR_PAIR(1));
                mvprintw(i, 0, "%*d | ", digits, file_line + 1);
                if (has_colors())
                    attroff(COLOR_PAIR(1));
            }

            const std::string &line_str = lines[file_line];
            if (col_offset < static_cast<int>(line_str.length()))
            {
                std::string visible = line_str.substr(col_offset, text_width);
                mvprintw(i, gutter_width, "%s", visible.c_str());
            }
        }
        else
        {
            if (has_colors())
                attron(COLOR_PAIR(5));
            mvprintw(i, 0, "~");
            if (has_colors())
                attroff(COLOR_PAIR(5));
        }
    }

    if (mode == Mode::COMMAND)
    {
        mvprintw(LINES - 1, 0, "%s", command_buffer.c_str());
        move(LINES - 1, command_buffer.length());
    }
    else
    {
        if (has_colors())
            attron(COLOR_PAIR(2));

        int bar_width = (COLS > 1) ? COLS - 1 : 1;
        std::string bar(bar_width, ' ');
        mvprintw(LINES - 1, 0, "%s", bar.c_str());

        std::string fname_str = current_filename.empty() ? "[No Name]" : current_filename;
        std::string left_info = " -- EDIT -- | " + fname_str;

        if (!status_message.empty())
        {
            left_info = " " + status_message;
        }

        std::string right_info = "Ln " + std::to_string(cursor_y + 1) + "/" +
                                 std::to_string(lines.size()) + ", Col " +
                                 std::to_string(cursor_x + 1) + " ";

        mvprintw(LINES - 1, 0, "%s", left_info.c_str());
        if (bar_width > static_cast<int>(left_info.length() + right_info.length()))
        {
            mvprintw(LINES - 1, bar_width - right_info.length(), "%s", right_info.c_str());
        }

        if (has_colors())
            attroff(COLOR_PAIR(2));

        // umisteni kurzoru do textu
        int screen_y = cursor_y - row_offset;
        int screen_x = gutter_width + (cursor_x - col_offset);
        move(screen_y, screen_x);
    }

    refresh();
}

void Editor::handle_input(int ch)
{
    if (ch == KEY_RESIZE)
    {
        return;
    }

    if (mode == Mode::EDIT)
    {
        handle_edit_mode(ch);
    }
    else if (mode == Mode::COMMAND)
    {
        handle_command_mode(ch);
    }
}

void Editor::handle_edit_mode(int ch)
{
    switch (ch)
    {
    case 27: // prepnuti do command modu
        mode = Mode::COMMAND;
        command_buffer = ":";
        status_message.clear();
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
        {
            cursor_x--;
        }
        else if (cursor_y > 0)
        {
            cursor_y--;
            auto lines = buffer->get_lines();
            cursor_x = lines[cursor_y].length();
        }
        break;

    case KEY_RIGHT:
    {
        auto lines = buffer->get_lines();
        if (cursor_y < static_cast<int>(lines.size()))
        {
            if (cursor_x < static_cast<int>(lines[cursor_y].length()))
            {
                cursor_x++;
            }
            else if (cursor_y < static_cast<int>(lines.size()) - 1)
            {
                cursor_y++;
                cursor_x = 0;
            }
        }
        break;
    }

    case '\n':
    case '\r':
    case KEY_ENTER:
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

    case KEY_HOME:
        cursor_x = 0; // posun na zacatek radku
        break;

    case KEY_END:
    {
        auto lines = buffer->get_lines();
        if (cursor_y < static_cast<int>(lines.size()))
        {
            cursor_x = lines[cursor_y].length(); // posun na konec radku
        }
        break;
    }

    case KEY_PPAGE: // pgup
    {
        int page_size = (LINES - 2 > 0) ? LINES - 2 : 1;
        cursor_y = std::max(0, cursor_y - page_size);
        auto lines = buffer->get_lines();
        if (cursor_x > static_cast<int>(lines[cursor_y].length()))
        {
            cursor_x = lines[cursor_y].length();
        }
        break;
    }

    case KEY_NPAGE: // pgdn
    {
        int page_size = (LINES - 2 > 0) ? LINES - 2 : 1;
        auto lines = buffer->get_lines();
        cursor_y = std::min(static_cast<int>(lines.size()) - 1, cursor_y + page_size);
        if (cursor_x > static_cast<int>(lines[cursor_y].length()))
        {
            cursor_x = lines[cursor_y].length();
        }
        break;
    }

    case KEY_F(1): // help
        show_help();
        break;

    case 19: // ukladani
    case KEY_F(2):
        if (current_filename.empty())
        {
            status_message = "Chyba: Zadej nazev souboru (napr. :w novy.txt)";
        }
        else if (buffer->save_to_file(current_filename))
        {
            status_message = "Ulozeno do: " + current_filename;
        }
        else
        {
            status_message = "Kriticka chyba: Nelze zapsat do " + current_filename;
        }
        break;

    default:
        status_message.clear();
        if (ch == '\t')
        {
            for (int i = 0; i < 4; ++i)
            {
                buffer->insert_char(' ', cursor_y, cursor_x);
                cursor_x++;
            }
        }
        else if (ch >= 32 && ch <= 126)
        {
            buffer->insert_char(ch, cursor_y, cursor_x);
            cursor_x++;
        }
        break;
    }
}

void Editor::handle_command_mode(int ch)
{
    switch (ch)
    {
    case 27: // navrat k editmodu
        mode = Mode::EDIT;
        command_buffer.clear();
        break;

    case KEY_ENTER:
    case '\n':
    case '\r':
        execute_command();
        break;

    case KEY_BACKSPACE:
    case 127:
    case '\b':
        if (command_buffer.length() > 1)
        {
            command_buffer.pop_back(); // zachovani dvojtecky
        }
        else
        {
            mode = Mode::EDIT;
            command_buffer.clear();
            status_message.clear();
        }
        break;

    default:
        // klasicke pridavani znaku
        if (ch >= 32 && ch <= 126)
        {
            command_buffer += static_cast<char>(ch);
        }
        break;
    }
}

void Editor::execute_command()
{
    if (command_buffer.length() <= 1)
    {
        mode = Mode::EDIT;
        command_buffer.clear();
        status_message.clear();
        return;
    }

    std::string cmd_line = command_buffer.substr(1);
    std::string command = cmd_line;
    std::string argument = "";

    size_t space_pos = cmd_line.find(' ');
    if (space_pos != std::string::npos)
    {
        command = cmd_line.substr(0, space_pos);
        argument = cmd_line.substr(space_pos + 1);
    }

    if (command == "q")
    {
        running = false;
    }
    else if (command == "w" || command == "wq" || command == "x")
    {
        std::string target_file = argument.empty() ? current_filename : argument;

        if (target_file.empty())
        {
            status_message = "Chyba: Zadej nazev souboru (napr. :" + command + " novy.txt)";
            mode = Mode::EDIT;
            command_buffer.clear();
            return;
        }

        if (buffer->save_to_file(target_file))
        {
            current_filename = target_file;

            if (command == "wq" || command == "x")
            {
                running = false;
            }
            else
            {
                status_message = "Ulozeno do: " + current_filename;
                mode = Mode::EDIT;
                command_buffer.clear();
            }
        }
        else
        {
            status_message = "Kriticka chyba: Nelze zapsat na disk!";
            mode = Mode::EDIT;
            command_buffer.clear();
        }
    }
    else if (command == "help" || command == "h")
    {
        show_help();
        mode = Mode::EDIT;
        command_buffer.clear();
        status_message.clear();
    }
    else
    {
        status_message = "Neznamy prikaz: " + command;
        mode = Mode::EDIT;
        command_buffer.clear();
    }
}

void Editor::show_help()
{
    erase();
    int start_y = 1;
    int start_x = 3;

    if (has_colors())
        attron(COLOR_PAIR(1) | A_BOLD);
    mvprintw(start_y++, start_x, "===============================================================");
    mvprintw(start_y++, start_x, "                 TTEDIT - NAPOVEDA K OVLADANI                  ");
    mvprintw(start_y++, start_x, "===============================================================");
    if (has_colors())
        attroff(COLOR_PAIR(1) | A_BOLD);

    start_y++;
    if (has_colors())
        attron(A_BOLD);
    mvprintw(start_y++, start_x, "REZIMY EDITACE:");
    if (has_colors())
        attroff(A_BOLD);
    mvprintw(start_y++, start_x + 2, "EDIT      - Vychozi rezim pro prime psani a editaci textu");
    mvprintw(start_y++, start_x + 2, "COMMAND   - Rezim prikazu (prepnuti stiskem klavesy ESC)");

    start_y++;
    if (has_colors())
        attron(A_BOLD);
    mvprintw(start_y++, start_x, "PRIKAZY (v COMMAND rezimu):");
    if (has_colors())
        attroff(A_BOLD);
    mvprintw(start_y++, start_x + 2, ":w [soubor]  - Ulozit soubor (volitelne pod zadanym nazvem)");
    mvprintw(start_y++, start_x + 2, ":q           - Ukoncit editor");
    mvprintw(start_y++, start_x + 2, ":wq / :x     - Ulozit zmeny a ukoncit editor");
    mvprintw(start_y++, start_x + 2, ":help / :h   - Otevrit tuto obrazovku napovedy");

    start_y++;
    if (has_colors())
        attron(A_BOLD);
    mvprintw(start_y++, start_x, "POHYB A KLAVESOVE ZKRATKY:");
    if (has_colors())
        attroff(A_BOLD);
    mvprintw(start_y++, start_x + 2, "Sipky        - Pohyb kurzoru (nahoru/dolu/doleva/doprava)");
    mvprintw(start_y++, start_x + 2, "Home / End   - Skok na zacatek / konec radku");
    mvprintw(start_y++, start_x + 2, "PgUp / PgDn  - Posun o celou obrazovku nahoru / dolu");
    mvprintw(start_y++, start_x + 2, "Enter        - Novy radek (rozdeleni radku v pozici kurzoru)");
    mvprintw(start_y++, start_x + 2, "Backspace    - Smazani znaku vlevo / spojeni radku na zacatku");
    mvprintw(start_y++, start_x + 2, "Tab          - Vlozeni 4 mezer (soft tab)");
    mvprintw(start_y++, start_x + 2, "F1           - Rychle zobrazeni napovedy");
    mvprintw(start_y++, start_x + 2, "Ctrl+S / F2  - Rychle ulozeni otevreneho souboru");

    start_y += 2;
    if (has_colors())
        attron(COLOR_PAIR(2));
    mvprintw(start_y, start_x, " Stiskni libovolnou klavesu pro navrat do editoru... ");
    if (has_colors())
        attroff(COLOR_PAIR(2));

    refresh();
    getch();
}

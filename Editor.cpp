#include "Editor.h"
#include <ncurses.h>
#include <filesystem>

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
    if (mode == Mode::COMMAND)
    {
        // zobrazeni commandline dole
        mvprintw(LINES - 1, 0, "%s", command_buffer.c_str());
        move(LINES - 1, command_buffer.length());
    }
    else
    {
        if (!status_message.empty())
        {
            mvprintw(LINES - 1, 0, "%s", status_message.c_str());
        }
        // umoisteni cursoru do textu
        move(cursor_y, cursor_x);
    }
    refresh();
}

void Editor::handle_input(int ch)
{
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
        status_message.clear();
        if (ch >= 32 && ch <= 126)
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
        execute_command();
        break;

    case KEY_BACKSPACE:
    case 127:
    case '\b':
        if (command_buffer.length() > 1)
        { // zachovani dvojtecky
            command_buffer.pop_back();
        }
        else
        {
            mode = Mode::EDIT; // Smazání dvojtečky nás vrátí do editace
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

        // Pokud nezadal argument, zkusíme použít ten stávající (pokud nějaký je)
        std::string target_file = argument.empty() ? current_filename : argument;

        // 1. Zkouší uložit bezejmenný soubor bez argumentu
        if (target_file.empty())
        {
            status_message = "Chyba: Zadej nazev souboru (např. :" + command + " novy.txt)";
            mode = Mode::EDIT;
            return;
        }

        // 2. Kontrola, zda zadal NÁZEV NOVÉHO SOUBORU a ten už neexistuje
        if (!argument.empty() && std::filesystem::exists(argument))
        {
            status_message = "Chyba: Soubor '" + argument + "' uz existuje! Zvol jiny nazev.";
            mode = Mode::EDIT;
            return;
        }

        // 3. Vše je v pořádku, můžeme ukládat
        if (buffer->save_to_file(target_file))
        {
            current_filename = target_file; // Uložíme si ho pro příští :w bez argumentu

            if (command == "wq" || command == "x")
            {
                running = false;
            }
            else
            {
                status_message = "Ulozeno do: " + current_filename;
                mode = Mode::EDIT;
            }
        }
        else
        {
            status_message = "Kriticka chyba: Nelze zapsat na disk!";
            mode = Mode::EDIT;
        }
    }
    else
    {
        status_message = "Neznamy prikaz: " + command;
        mode = Mode::EDIT;
    }
}
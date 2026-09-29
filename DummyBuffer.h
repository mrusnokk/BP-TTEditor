#pragma once
#include "ITextBuffer.h"
#include <fstream>

class DummyBuffer : public ITextBuffer
{
private:
    std::vector<std::string> lines;

public:
    DummyBuffer()
    {
        lines.push_back("");
    }

    void insert_char(char c, int radek, int sloupec) override
    {
        if (radek < 0 || radek >= static_cast<int>(lines.size()))
            return;
        if (sloupec < 0 || sloupec > static_cast<int>(lines[radek].length()))
            return;

        lines[radek].insert(sloupec, 1, c);
    }

    void delete_char(int radek, int sloupec) override
    {
        if (radek < 0 || radek >= static_cast<int>(lines.size()))
            return;

        if (sloupec > 0)
        {
            if (sloupec > static_cast<int>(lines[radek].length()))
                return;
            lines[radek].erase(sloupec - 1, 1);
        }
        else if (sloupec == 0 && radek > 0)
        {
            lines[radek - 1] += lines[radek];
            lines.erase(lines.begin() + radek);
        }
    }

    std::vector<std::string> get_lines() override
    {
        return lines;
    }

    void insert_newline(int radek, int sloupec) override
    {
        if (radek < 0 || radek >= static_cast<int>(lines.size()))
            return;
        if (sloupec < 0 || sloupec > static_cast<int>(lines[radek].length()))
            return;

        std::string zbytek_radku = lines[radek].substr(sloupec);
        lines[radek].erase(sloupec);
        lines.insert(lines.begin() + radek + 1, zbytek_radku);
    }
    bool save_to_file(const std::string &filename) override
    {
        std::ofstream file(filename);
        if (!file.is_open())
            return false;
        for (const auto &line : lines)
        {
            file << line << "\n";
        }
        return true;
    }
    bool load_from_file(const std::string &filename) override
    {
        std::ifstream file(filename);
        if (!file.is_open())
            return false;

        lines.clear();
        std::string line;
        while (std::getline(file, line))
        {
            lines.push_back(line);
        }

        // vlozieni prazdneho radku v pripade ze soubor byl prazdny
        if (lines.empty())
        {
            lines.push_back("");
        }
        return true;
    }
};
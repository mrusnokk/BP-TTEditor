#pragma once
#include "ITextBuffer.h"
#include "GapBuffer.h"
#include <vector>
#include <string>
#include <cstdio>

class TextBuffer : public ITextBuffer
{
private:
    std::vector<GapBuffer> lines;

public:
    TextBuffer()
    {
        lines.emplace_back();
    }

    void insert_char(char c, int radek, int sloupec) override
    {
        if (radek >= 0 && radek < static_cast<int>(lines.size()))
        {
            lines[radek].insert_char(c, sloupec);
        }
    }

    void delete_char(int radek, int sloupec) override
    {
        if (radek < 0 || radek >= static_cast<int>(lines.size()))
            return;

        if (sloupec > 0)
        {
            lines[radek].delete_char(sloupec);
        }
        else if (sloupec == 0 && radek > 0)
        {
            lines[radek - 1].append(lines[radek]);
            lines.erase(lines.begin() + radek);
        }
    }

    void insert_newline(int radek, int sloupec) override
    {
        if (radek < 0 || radek >= static_cast<int>(lines.size()))
            return;

        GapBuffer right_part = lines[radek].split(sloupec);
        lines.insert(lines.begin() + radek + 1, std::move(right_part));
    }

    std::vector<std::string> get_lines() override
    {
        std::vector<std::string> result;
        result.reserve(lines.size());

        for (const auto &l : lines)
        {
            result.push_back(l.toString());
        }
        return result;
    }

    bool save_to_file(const std::string &filename) override
    {
        if (filename.empty())
            return false;

        FILE *f = fopen(filename.c_str(), "w");
        if (!f)
            return false;

        for (const auto &l : lines)
        {
            fprintf(f, "%s\n", l.toString().c_str());
        }
        fclose(f);
        return true;
    }

    bool load_from_file(const std::string &filename) override
    {
        if (filename.empty())
            return false;

        FILE *f = fopen(filename.c_str(), "r");
        if (!f)
            return false;

        lines.clear();
        char buffer[2048];
        std::string tempLine;

        while (fgets(buffer, sizeof(buffer), f) != nullptr)
        {
            tempLine += buffer;
            if (!tempLine.empty() && tempLine.back() == '\n')
            {
                tempLine.pop_back();
                if (!tempLine.empty() && tempLine.back() == '\r')
                {
                    tempLine.pop_back();
                }
                lines.emplace_back(tempLine.c_str(), tempLine.size());
                tempLine.clear();
            }
        }
        if (!tempLine.empty())
        {
            lines.emplace_back(tempLine.c_str(), tempLine.length());
        }
        if (lines.empty())
        {
            lines.emplace_back();
        }
        fclose(f);
        return true;
    }
};

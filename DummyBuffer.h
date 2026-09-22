#pragma once
#include "ITextBuffer.h"

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
        if (radek < 0 || radek >= static_cast<int>(lines.size()) || lines[radek].empty() || sloupec <= 0)
            return;

        lines[radek].erase(sloupec - 1, 1);
    }

    std::vector<std::string> get_lines() override
    {
        return lines;
    }
};
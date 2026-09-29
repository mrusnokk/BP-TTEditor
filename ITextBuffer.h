#pragma once
#include <string>
#include <vector>

class ITextBuffer
{
public:
    virtual ~ITextBuffer() = default;
    virtual void insert_char(char c, int radek, int sloupec) = 0;
    virtual void delete_char(int radek, int sloupec) = 0;
    virtual std::vector<std::string> get_lines() = 0;
    virtual void insert_newline(int radek, int sloupec) = 0;
    virtual bool save_to_file(const std::string &filename) = 0;
    virtual bool load_from_file(const std::string &filename) = 0;
};
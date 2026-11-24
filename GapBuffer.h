#pragma once
#include <algorithm>
#include <string>

class GapBuffer
{
private:
    char *buffer;
    char *gapStart;
    char *gapEnd;
    char *bufferEnd;
    int bufferSize;

public:
    GapBuffer()
    {
        bufferSize = 128;
        buffer = new char[bufferSize];
        gapStart = buffer;
        gapEnd = buffer + bufferSize;
        bufferEnd = buffer + bufferSize;
    }

    ~GapBuffer()
    {
        delete[] buffer;
        buffer = nullptr;
        gapStart = nullptr;
        gapEnd = nullptr;
        bufferEnd = nullptr;
        bufferSize = 0;
    }

    GapBuffer(const GapBuffer &other)
    {
        bufferSize = other.bufferSize;
        buffer = new char[bufferSize];

        int left_size = other.gapStart - other.buffer;
        int right_size = other.bufferEnd - other.gapEnd;

        std::copy(other.buffer, other.gapStart, buffer);
        std::copy(other.gapEnd, other.bufferEnd, buffer + bufferSize - right_size);

        gapStart = buffer + left_size;
        bufferEnd = buffer + bufferSize;
        gapEnd = bufferEnd - right_size;
    }

    GapBuffer &operator=(const GapBuffer &other)
    {
        if (this == &other)
            return *this;

        delete[] buffer;

        bufferSize = other.bufferSize;
        buffer = new char[bufferSize];
        int left_size = other.gapStart - other.buffer;
        int right_size = other.bufferEnd - other.gapEnd;
        std::copy(other.buffer, other.gapStart, buffer);
        std::copy(other.gapEnd, other.bufferEnd, buffer + bufferSize - right_size);
        gapStart = buffer + left_size;
        bufferEnd = buffer + bufferSize;
        gapEnd = bufferEnd - right_size;

        return *this;
    }

    GapBuffer(GapBuffer &&other) noexcept
    {
        buffer = other.buffer;
        gapStart = other.gapStart;
        gapEnd = other.gapEnd;
        bufferEnd = other.bufferEnd;
        bufferSize = other.bufferSize;

        other.buffer = nullptr;
        other.gapStart = nullptr;
        other.gapEnd = nullptr;
        other.bufferEnd = nullptr;
        other.bufferSize = 0;
    }

    GapBuffer &operator=(GapBuffer &&other) noexcept
    {
        if (this == &other)
            return *this;
        delete[] buffer;

        buffer = other.buffer;
        gapStart = other.gapStart;
        gapEnd = other.gapEnd;
        bufferEnd = other.bufferEnd;
        bufferSize = other.bufferSize;

        other.buffer = nullptr;
        other.gapStart = nullptr;
        other.gapEnd = nullptr;
        other.bufferEnd = nullptr;
        other.bufferSize = 0;
        return *this;
    }

    GapBuffer(const char *text, int length)
    {
        bufferSize = length + 128;
        buffer = new char[bufferSize];

        if (text && length > 0)
        {
            std::copy(text, text + length, buffer);
        }

        gapStart = buffer + length;
        bufferEnd = buffer + bufferSize;
        gapEnd = bufferEnd;
    }

    int length() const
    {
        if (!buffer)
            return 0;
        return (gapStart - buffer) + (bufferEnd - gapEnd);
    }

    void expand()
    {
        int left_size = gapStart - buffer;
        int right_size = bufferEnd - gapEnd;

        int new_size = bufferSize + 128;
        char *temp = new char[new_size];
        std::copy(buffer, gapStart, temp);
        char *new_gap_end = temp + new_size - right_size;
        std::copy(gapEnd, bufferEnd, new_gap_end);

        delete[] buffer;

        buffer = temp;
        bufferSize = new_size;
        gapStart = buffer + left_size;
        gapEnd = new_gap_end;
        bufferEnd = buffer + bufferSize;
    }

    void move_gap(int sloupec)
    {
        if (!buffer)
            return;
        int current_pos = gapStart - buffer;

        while (current_pos > sloupec && gapStart > buffer)
        {
            gapStart--;
            gapEnd--;
            *gapEnd = *gapStart;
            current_pos--;
        }

        while (current_pos < sloupec && gapEnd < bufferEnd)
        {
            *gapStart = *gapEnd;
            gapStart++;
            gapEnd++;
            current_pos++;
        }
    }

    void insert_char(char c, int sloupec)
    {
        if (sloupec < 0)
            return;

        move_gap(sloupec);

        if (gapStart == gapEnd)
        {
            expand();
        }

        *gapStart = c;
        gapStart++;
    }

    void delete_char(int sloupec)
    {
        if (sloupec <= 0 || sloupec > length())
        {
            return;
        }
        move_gap(sloupec);
        if (gapStart > buffer)
        {
            gapStart--;
        }
    }

    std::string toString() const
    {
        if (!buffer)
            return "";
        std::string out;
        int left_size = gapStart - buffer;
        int right_size = bufferEnd - gapEnd;

        out.reserve(left_size + right_size);
        out.append(buffer, left_size);
        out.append(gapEnd, right_size);

        return out;
    }

    GapBuffer split(int sloupec)
    {
        if (sloupec < 0)
            sloupec = 0;
        if (sloupec > length())
            sloupec = length();
        move_gap(sloupec);
        int right_size = bufferEnd - gapEnd;
        GapBuffer right(gapEnd, right_size);
        gapEnd = bufferEnd;
        return right;
    }

    void append(const std::string &str)
    {
        move_gap(length());
        while ((gapEnd - gapStart) < static_cast<int>(str.length()))
        {
            expand();
        }
        std::copy(str.begin(), str.end(), gapStart);
        gapStart += str.length();
    }

    void append(const GapBuffer &other)
    {
        append(other.toString());
    }
};

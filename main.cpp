#include "Editor.h"
#include "DummyBuffer.h"

int main(int argc, char *argv[])
{
    std::string filename;
    auto buffer = std::make_unique<DummyBuffer>();
    if (argc > 1)
    {
        filename = argv[1];
        buffer->load_from_file(argv[1]);
    }
    else
    {
        filename = "";
    }
    Editor app(std::move(buffer), filename);
    app.run();

    return 0;
}
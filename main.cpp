#include "Editor.h"
#include "DummyBuffer.h"

int main()
{
    auto buffer = std::make_unique<DummyBuffer>();
    Editor app(std::move(buffer));
    app.run();

    return 0;
}
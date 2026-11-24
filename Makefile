#compilator
CXX = g++
#copilator flags
CXXFLAGS = -Wall -Wextra -std=c++17
#nalinkovani knihoven
LDFLAGS = -lncurses
#vysledny target
TARGET = ttedit

SRCS = main.cpp Editor.cpp
HEADERS = Editor.h ITextBuffer.h TextBuffer.h GapBuffer.h DummyBuffer.h

#main
$(TARGET): $(SRCS) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)

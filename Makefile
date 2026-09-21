
#compilator
CXX = g++
#copilator flags
CXXFLAGS = -Wall -Wextra -std=c++17
#nalinkovani knihoven
LDFLAGS = -lncurses
#vysledny target
TARGET = ttedit

SRCS = NcursesTTEditor.cpp

#main
$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)
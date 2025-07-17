SFML_PATH := D:/SFML-3

CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -MMD -MP -I"$(SFML_PATH)/include"
LDFLAGS  := -L"$(SFML_PATH)/lib"
LDLIBS   := -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio

SRCS := \
    main.cpp        \
    StartScreen.cpp \
    GameSFML.cpp    \
    snake.cpp		\
	Overlay.cpp 

OBJS   := $(SRCS:.cpp=.o)
DEPS   := $(OBJS:.o=.d)
TARGET := snake.exe          

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

-include $(DEPS)            

.PHONY: run clean

run: $(TARGET)
	./$(TARGET)

clean:
	del /Q $(OBJS) $(DEPS) $(TARGET) highscores.txt 2>nul

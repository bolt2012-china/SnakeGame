CXX = g++
CXXFLAGS = -Wall -g
SFML_INCLUDE = -I/home/suzyxia/Downloads/SFML-3.0.0-linux-gcc-64-bit/SFML-3.0.0/include
SFML_LIB = -L/home/suzyxia/Downloads/SFML-3.0.0-linux-gcc-64-bit/SFML-3.0.0/lib
SFML_LIBS = -lsfml-graphics -lsfml-window -lsfml-system

SRCS = main.cpp GameSFML.cpp snake.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = snake_game

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(SFML_LIB) $(SFML_LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(SFML_INCLUDE) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
# ------------------------------------------------------------
# Makefile – Snake（SFML 3 手动路径）
# ------------------------------------------------------------

# ===== 1. 你的 SFML3 根目录（不要在行尾加空格） =====
SFML_PATH := D:/SFML-3

# ===== 2. 编译 / 链接器 =====
CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -MMD -MP -I"$(SFML_PATH)/include"
LDFLAGS  := -L"$(SFML_PATH)/lib"
LDLIBS   := -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio

# ===== 3. 源文件 & 目标 =====
SRCS := \
    main.cpp        \
    StartScreen.cpp \
    GameSFML.cpp    \
    snake.cpp		\
	Overlay.cpp 

OBJS   := $(SRCS:.cpp=.o)
DEPS   := $(OBJS:.o=.d)
TARGET := snake.exe          # Windows 下习惯加 .exe；改成 snake 也行

# ===== 4. 默认目标 =====
all: $(TARGET)

# ===== 5. 链接 =====
$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

# ===== 6. 通用编译规则 =====
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

-include $(DEPS)             # 自动依赖

# ===== 7. 方便命令 =====
.PHONY: run clean

run: $(TARGET)
	./$(TARGET)

clean:
	del /Q $(OBJS) $(DEPS) $(TARGET) highscores.txt 2>nul

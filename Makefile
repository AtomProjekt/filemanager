# ─── Makefile для FM++ File Manager ─────────────────────────────────────────
CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2 -Wno-unused-result
LDFLAGS  := -lstdc++fs

TARGET   := fm
SRCS     := main.cpp logger.cpp fileops.cpp search.cpp ui.cpp
OBJS     := $(SRCS:.cpp=.o)

.PHONY: all clean release windows

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS) -lpthread
	@echo "✔ Готово: ./$(TARGET)"

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

release: CXXFLAGS += -DNDEBUG
release: all

# Кросс-компиляция под Windows (нужен mingw-w64)
windows:
	x86_64-w64-mingw32-g++ $(CXXFLAGS) $(SRCS) -o fm.exe \
	    -static -lpthread -lstdc++fs
	@echo "✔ fm.exe готов"

clean:
	rm -f $(OBJS) $(TARGET) fm.exe

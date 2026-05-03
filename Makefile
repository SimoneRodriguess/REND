CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
SRC      = src/main.cpp src/lexer.cpp
TARGET   = rend

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)


vpath %.cpp src

CXX := g++
CXXFLAGS := -std=c++2a -Wall -Wextra -Weffc++ -Og -ggdb
CPPFLAGS := -I include
LDFLAGS :=
LIBS := -lboost_program_options

TARGET := ccc

all: $(TARGET)

$(TARGET): main.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) -I include -o $@ $^ $(LIBS)

main.o: main.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -I include -c -o $@ $^

.PHONY: clean
clean:
	rm -f *.o $(TARGET)

CXXFLAGS = -std=c++17 -g #-O2
LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

main: main.cpp
	g++ $(CXXFLAGS) -o main main.cpp $(LDFLAGS)

.PHONY: test clean

test: main
	./main

clean:
	rm -f main
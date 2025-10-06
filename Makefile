CXX = g++
CXXFLAGS = -O2 -std=c++20

SRCS := $(wildcard src/*.cpp)
OBJS := $(patsubst src/%.cpp,build/%.o,$(SRCS))

.PHONY: all
all: build_dir $(OBJS) quick_sim

quick_sim: $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o quick_sim

build_dir:
	mkdir -p build
build/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ -MMD -MP -MF build/$*.d
-include $(wildcard build/*.d)

.PHONY: clean
clean:
	rm -rf build
	rm -f quick_sim

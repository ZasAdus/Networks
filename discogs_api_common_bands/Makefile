CXX = g++
CXXFLAGS = -Wall -g
LDFLAGS = -lcurl
TARGET = main
SRCS = main.cpp discogs_api.cpp print_common_bands.cpp artist.cpp
OBJS = $(SRCS:.cpp=.o)
EXAMPLE_ARGS = 516820 532854 702387  #example args for main

all: $(TARGET) run

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET) $(EXAMPLE_ARGS)

clean:
	rm -f *.o $(TARGET)

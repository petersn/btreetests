
CXXFLAGS=-Ofast -Wall -Wextra

test: test.cpp example.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

.PHONY: clean
clean:
	rm -f test *.o


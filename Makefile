.DEFAULT_GOAL := all
all: test.cpp asio.cpp Makefile
	$(CXX) -std=c++14 -I $(REALM_SYNC_PREFIX)/src -I $(REALM_CORE_PREFIX)/src -o irontest-cpp -O3 -g test.cpp http-parser/http_parser.o -L$(REALM_SYNC_PREFIX)/src/realm -L$(REALM_CORE_PREFIX)/src/realm -lrealm-sync -lrealm $(LDFLAGS)
	$(CXX) -std=c++14 -O3 -g -I/usr/local/include asio.cpp http-parser/http_parser.o -o irontest-asio
clean:
	rm -f irontest-cpp

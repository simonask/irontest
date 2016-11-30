.DEFAULT_GOAL := all
all: test.cpp asio.cpp Makefile
	# clang++ -std=c++14 -I $(REALM_SYNC_PREFIX)/src -I $(REALM_CORE_PREFIX)/src -o irontest-cpp -O0 -g test.cpp http-parser/http_parser.o -L$(REALM_SYNC_PREFIX)/src/realm -L$(REALM_CORE_PREFIX)/src/realm -lrealm-sync -lrealm $(LDFLAGS)
	clang++ -std=c++14 -O3 -fno-omit-frame-pointer -g -I/usr/local/include asio.cpp http-parser/http_parser.o -o irontest-asio
clean:
	rm -f irontest-cpp

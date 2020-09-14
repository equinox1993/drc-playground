CXX_COMPILER ?= clang++

get_cflags = $(shell pkg-config --cflags $(1))
get_libflags = $(shell pkg-config --libs $(1))

PKG_CFLAGS = $(call get_cflags, libdrc) $(call get_cflags, x264) $(call get_cflags, libswscale)
PKG_LIBFLAGS = $(call get_libflags, libdrc) $(call get_libflags, x264) $(call get_libflags, libswscale)
CXX_FLAGS = --std=c++14 $(PKG_CFLAGS) $(PKG_LIBFLAGS)

build: make_out
	$(CXX_COMPILER) $(CXX_FLAGS) main.cc -o out/drc_helloworld

clean:
	rm -r out

make_out:
	mkdir -p out

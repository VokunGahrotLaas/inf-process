
# (gnu|c)++(17|20|2b|...)
std = c++20
# 0|1|2|3|fast|s|z
O = 2
# linux|mingw
target = linux
# static|shared|header_only
type = shared
# debug|release
mode = release
# true|false
asan = false
# true|false depends on mode
lto = false
# |native|... -march=${arch} unless empty
arch =
# ... is prefixed to run commands
prefix =

ENV_PREFIX =

CXX ?= g++
CXXFLAGS = -std=${std} -Wall -Wextra -Wpedantic -Werror -O$O -I./include
LDFLAGS =
LIB_CXXFLAGS = ${CXXFLAGS}
LIB_LDFLAGS = ${LDFLAGS}
LIB_ARFLAGS = rcs
TEST_CXXFLAGS = ${CXXFLAGS}
TEST_LDFLAGS = ${LDFLAGS}

LIB_SRC = ${wildcard src/*.cpp}
LIB_OBJ = ${LIB_SRC:.cpp=.o}
LIB_EXEC = libinf-static.a libinf.so libinf.dll

TEST_SRC = ${wildcard tests/test-*.cpp}
TEST_OBJ = ${TEST_SRC:.cpp=.o}
TEST_EXEC = ${TEST_OBJ:.o=.out}

ifeq (${mode},debug)
	CXXFLAGS += -ggdb3
	LDFLAGS += -ggdb3
	O = 0
	lto = false
else ifeq (${mode},release)
	CXXFLAGS +=
	LDFLAGS +=
	O = 2
	lto = true
else
	$(error mode must be debug or release)
endif

ifneq (${arch},)
	CXXFLAGS += -march=${arch}
	LDFLAGS += -march=${arch}
endif

ifeq (${lto},true)
	CXXFLAGS += -flto=auto -fno-fat-lto-objects
	LDFLAGS += -flto=auto -fuse-linker-plugin
else ifneq (${lto},false)
	$(error lto must be true or false)
endif

ifeq (${asan},true)
	CXXFLAGS += -fsanitize=address,undefined,leak
	LDFLAGS += -fsanitize=address,undefined,leak
else ifneq (${asan},false)
	$(error asan must be true or false)
endif

ifeq (${target},mingw)
LIB_STATIC = libinf-static.a
LIB_SHARED = libinf.dll
else ifeq (${target},linux)
LIB_STATIC = libinf-static.a
LIB_SHARED = libinf.so
else
	$(error target must be mingw or linux)
endif

ifeq (${type},static)
	TEST_LDFLAGS += -L. -linf-static
	LIB = ${LIB_STATIC}
	ifeq (${target},mingw)
		ENV_PREFIX += WINEPATH=/usr/x86_64-w64-mingw32/bin
	endif
else ifeq (${type},shared)
	LIB_CXXFLAGS += -fPIC
	LIB_LDFLAGS += -shared
	TEST_LDFLAGS += -L. -linf
	LIB = ${LIB_SHARED}
	ifeq (${target},mingw)
		ENV_PREFIX += WINEPATH=.\;/usr/x86_64-w64-mingw32/bin
	else ifeq (${target},linux)
		ENV_PREFIX += LD_LIBRARY_PATH=.
	endif
else ifeq (${type},header_only)
	TEST_CXXFLAGS += -DINF_HEADER_ONLY
	LIB =
	ifeq (${target},mingw)
		ENV_PREFIX += WINEPATH=/usr/x86_64-w64-mingw32/bin
	endif
else
	$(error type must be static, shared or header_only)
endif

all: lib

# special targets
.PHONY: all phony_explicit lib tests check clean
.WAIT:
.SECONDARY:
phony_explicit:

%.a: ${TLIB_OBJ}
	${AR} ${LIB_ARFLAGS} $@ $^

%.so: ${LIB_OBJ}
	${CXX} -o $@ $^ ${LIB_LDFLAGS}

%.dll: ${LIB_OBJ}
	${CXX} -o $@ $^ ${LIB_LDFLAGS}

src/%.o: src/%.cpp
	${CXX} ${LIB_CXXFLAGS} -o $@ -c $<

tests/%.out: tests/%.cpp ${LIB}
	${CXX} ${TEST_CXXFLAGS} -o $@ $< ${TEST_LDFLAGS}

tests/%.o: tests/%.cpp
	${CXX} ${TEST_CXXFLAGS} -o $@ -c $<

lib: ${LIB}

tests: ${TEST_EXEC}

check_%: tests/test-%.out phony_explicit
	${ENV_PREFIX} ${prefix} ./$<

check: tests .WAIT ${addprefix check_,${subst tests/test-,,${basename ${TEST_SRC}}}}

clean:
	${RM} ${LIB_OBJ} ${LIB_EXEC} ${TEST_OBJ} ${TEST_EXEC}

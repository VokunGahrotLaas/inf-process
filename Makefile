
std = c++20
O = 2
target = linux
type = shared
prefix =

CXX ?= g++
CXXFLAGS = -std=${std} -Wall -Wextra -Wpedantic -Werror -O$O -I./include
LDFLAGS =
LIB_CXXFLAGS = ${CXXFLAGS}
LIB_LDFLAGS = ${LDFLAGS}
LIB_ARFLAGS = rcs
TEST_CXXFLAGS = ${CXXFLAGS}
TEST_LDFLAGS = ${LDFLAGS}

SRC = ${wildcard src/*.cpp}
OBJ = ${SRC:.cpp=.o}
LIBS = libinf-static.a libinf.so libinf.dll

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
		prefix += WINEPATH=/usr/x86_64-w64-mingw32/bin
	endif
else ifeq (${type},shared)
	LIB_CXXFLAGS += -fPIC
	LIB_LDFLAGS += -shared
	TEST_LDFLAGS += -L. -linf
	LIB = ${LIB_SHARED}
	ifeq (${target},mingw)
		prefix += WINEPATH=.\;/usr/x86_64-w64-mingw32/bin
	else ifeq (${target},linux)
		prefix += LD_LIBRARY_PATH=.
	endif
else ifeq (${type},header_only)
	TEST_CXXFLAGS += -DINF_HEADER_ONLY
	LIB =
	ifeq (${target},mingw)
		prefix += WINEPATH=/usr/x86_64-w64-mingw32/bin
	endif
else
	$(error type must be static, shared or header_only)
endif

TEST_SRC = ${wildcard tests/test-*.cpp}
TEST_OBJ = ${TEST_SRC:.cpp=.o}
TEST_EXEC = ${TEST_OBJ:.o=.out}

all: ${LIB}

# special targets
.PHONY: all phony_explicit run_tests clean
.WAIT:
.SECONDARY:
phony_explicit:

%.a: ${OBJ}
	${AR} ${LIB_ARFLAGS} $@ $^

%.so: ${OBJ}
	${CXX} -o $@ $^ ${LIB_LDFLAGS}

%.dll: ${OBJ}
	${CXX} -o $@ $^ ${LIB_LDFLAGS}

src/%.o: src/%.cpp
	${CXX} ${LIB_CXXFLAGS} -o $@ -c $<

tests/%.out: tests/%.cpp ${LIB}
	${CXX} ${TEST_CXXFLAGS} -o $@ $< ${TEST_LDFLAGS}

tests/%.o: tests/%.cpp
	${CXX} ${TEST_CXXFLAGS} -o $@ -c $<

run_test_%: tests/test-%.out phony_explicit
	${prefix} ./$<

run_tests: ${TEST_EXEC} .WAIT ${addprefix run_test_,${subst tests/test-,,${basename ${TEST_SRC}}}}

clean:
	${RM} ${OBJ} ${LIBS} ${TEST_OBJ} ${TEST_EXEC}

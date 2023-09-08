
std = c++20
O = 2
static = true

CXX ?= g++
CXXFLAGS = -std=${std} -Wall -Wextra -Wpedantic -Werror -O$O -I./include
LDFLAGS =
LIB_CXXFLAGS = ${CXXFLAGS} -fPIC
LIB_LDFLAGS = ${LDFLAGS} -shared
LIB_ARFLAGS = rcs
TEST_CXXFLAGS = ${CXXFLAGS}
TEST_LDFLAGS = ${LDFLAGS} -L.

SRC = ${wildcard src/*.cpp}
OBJ = ${SRC:.cpp=.o}
LIBS = libinf-static.a libinf.so

ifeq (${static},true)
TEST_CXXFLAGS +=
TEST_LDFLAGS += -linf-static
LD_LIBRARY_PATH =
LIB = libinf-static.a
else ifeq (${static},false)
TEST_CXXFLAGS += -DINF_EXTERN_TEMPLATE
TEST_LDFLAGS += -linf
LD_LIBRARY_PATH = .
LIB = libinf.so
else
$(error static must be true or false)
endif

TEST_SRC = ${wildcard tests/test-*.cpp}
TEST_OBJ = ${TEST_SRC:.cpp=.o}
TEST_EXEC = ${TEST_OBJ:.o=.out}

all: ${LIB} ${TEST_EXEC} ${TEST_OBJ}

.PHONY: all phony_explicit run_tests clean

.WAIT:

phony_explicit:

%.a: ${OBJ}
	${AR} ${LIB_ARFLAGS} $@ $^

%.so: ${OBJ}
	${CXX} -o $@ $^ ${LIB_LDFLAGS}

src/%.o: src/%.cpp
	${CXX} ${LIB_CXXFLAGS} -o $@ -c $<

tests/%.out: tests/%.o | ${LIB}
	${CXX} -o $@ $< ${TEST_LDFLAGS}

tests/%.o: tests/%.cpp
	${CXX} ${TEST_CXXFLAGS} -o $@ -c $<

run_test_%: tests/test-%.out phony_explicit
ifeq (${LD_LIBRARY_PATH},)
	./$<
else
	LD_LIBRARY_PATH=${LD_LIBRARY_PATH} ./$<
endif

run_tests: ${TEST_EXEC} .WAIT ${addprefix run_test_,${subst tests/test-,,${basename ${TEST_SRC}}}}

clean:
	${RM} ${OBJ} ${LIBS} ${TEST_OBJ} ${TEST_EXEC}

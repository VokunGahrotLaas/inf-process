
std = c++20
O = 2
static = true

CXX = g++
CXXFLAGS = -std=${std} -Wall -Wextra -Wpedantic -Werror -O$O -I./include
LDFLAGS =
LIB_CXXFLAGS = ${CXXFLAGS} -fPIC
LIB_LDFLAGS = ${LDFLAGS} -shared
LIB_ARFLAGS = rcs
TEST_CXXFLAGS = ${CXXFLAGS} -DINF_EXTERN_TEMPLATE
TEST_LDFLAGS = ${LDFLAGS} -L.

SRC = ${wildcard src/*.cpp}
OBJ = ${SRC:.cpp=.o}
LIB = libinf.so libinf-static.a

ifeq (${static},true)
TEST_LDFLAGS += -linf-static
LD_LIBRARY_PATH =
else ifeq (${static},false)
TEST_LDFLAGS += -linf
LD_LIBRARY_PATH = .
else
$(error static must be true or false)
endif

TEST_SRC = ${wildcard tests/test-*.cpp}
TEST_OBJ = ${TEST_SRC:.cpp=.o}
TEST_EXEC = ${TEST_OBJ:.o=.out}

all: ${LIB} ${TEST_EXEC} ${TEST_OBJ}

.PHONY: all phony_explicit run_tests clean

phony_explicit:

%.a: ${OBJ}
	${AR} ${LIB_ARFLAGS} $@ $^

%.so: ${OBJ}
	${CXX} -o $@ $^ ${LIB_LDFLAGS}

src/%.o: src/%.cpp
	${CXX} ${LIB_CXXFLAGS} -o $@ -c $<

tests/%.out: tests/%.o
	${CXX} -o $@ $< ${TEST_LDFLAGS}

tests/%.o: tests/%.cpp
	${CXX} ${TEST_CXXFLAGS} -o $@ -c $<

run_test_%: tests/test-%.out phony_explicit
ifeq (${LD_LIBRARY_PATH},)
	./$<
else
	LD_LIBRARY_PATH=${LD_LIBRARY_PATH} ./$<
endif

run_tests: ${addprefix run_test_,${subst tests/test-,,${basename ${TEST_SRC}}}}

clean:
	${RM} ${OBJ} ${LIB} ${TEST_OBJ} ${TEST_EXEC}

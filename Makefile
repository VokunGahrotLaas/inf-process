
std = c++20
O=2

CXX = g++
CXXFLAGS = -std=${std} -Wall -Wextra -Wpedantic -Werror -O$O -I./include
LDFLAGS =
LIB_CXXFLAGS = ${CXXFLAGS} -fPIC
LIB_LDFLAGS = ${LDFLAGS} -shared
LIB_ARFLAGS = rcs
TEST_CXXFLAGS = ${CXXFLAGS} -DINF_EXTERN_TEMPLATE
TEST_LDFLAGS = ${LDFLAGS} -L. -linf-static

SRC = ${wildcard src/*.cpp}
OBJ = ${SRC:.cpp=.o}
LIB = libinf.so libinf-static.a

TEST_SRC = ${wildcard tests/*.cpp}
TEST_OBJ = ${TEST_SRC:.cpp=.o}
TEST_EXEC = ${TEST_OBJ:.o=.out}

all: ${LIB} ${TEST_EXEC} ${TEST_OBJ}

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

clean:
	${RM} ${OBJ} ${LIB} ${TEST_OBJ} ${TEST_EXEC}

.PHONY: all clean

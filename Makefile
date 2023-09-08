
std = c++20
O = 2
windows = false
static = false
ifeq (${static},true)
static_lib = true
static_test = true
else
static_lib = false
static_test = false
endif
prefix =

CXX ?= g++
CXXFLAGS = -std=${std} -Wall -Wextra -Wpedantic -Werror -O$O -I./include
LDFLAGS =
LIB_CXXFLAGS = ${CXXFLAGS}
LIB_LDFLAGS = ${LDFLAGS}
LIB_ARFLAGS = rcs
TEST_CXXFLAGS = ${CXXFLAGS}
TEST_LDFLAGS = ${LDFLAGS} -L.

SRC = ${wildcard src/*.cpp}
OBJ = ${SRC:.cpp=.o}
LIBS = libinf-static.a libinf.so libinf.dll

ifeq (${static_test},true)
TEST_LDFLAGS += -static
else ifeq (${static_test},false)
else
$(error static_test must be true or false)
endif

ifeq (${static_lib},true)
TEST_LDFLAGS += -linf-static
LIB = libinf-static.a
else ifeq (${static_lib},false)
LIB_CXXFLAGS += -fPIC
LIB_LDFLAGS += -shared
TEST_CXXFLAGS += -DINF_EXTERN_TEMPLATE
TEST_LDFLAGS += -linf
else
$(error static_lib must be true or false)
endif

ifeq (${windows},true)
ifeq (${static_lib},false)
LIB = libinf.dll
ifeq (${static_test},false)
prefix += WINEPATH=.\;/usr/x86_64-w64-mingw32/bin
else
prefix += WINEPATH=.
endif
else
ifeq (${static_test},false)
prefix += WINEPATH=/usr/x86_64-w64-mingw32/bin
endif
endif
else ifeq (${windows},false)
ifeq (${static_lib},false)
LIB = libinf.so
prefix += LD_LIBRARY_PATH=.
endif
else
$(error windows must be true or false)
endif

TEST_SRC = ${wildcard tests/test-*.cpp}
TEST_OBJ = ${TEST_SRC:.cpp=.o}
TEST_EXEC = ${TEST_OBJ:.o=.out}

all: ${LIB} ${TEST_EXEC} ${TEST_OBJ}

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

tests/%.out: tests/%.o ${LIB}
	${CXX} -o $@ $< ${TEST_LDFLAGS}

tests/%.o: tests/%.cpp
	${CXX} ${TEST_CXXFLAGS} -o $@ -c $<

run_test_%: tests/test-%.out phony_explicit
	${prefix} ./$<

run_tests: ${TEST_EXEC} .WAIT ${addprefix run_test_,${subst tests/test-,,${basename ${TEST_SRC}}}}

clean:
	${RM} ${OBJ} ${LIBS} ${TEST_OBJ} ${TEST_EXEC}

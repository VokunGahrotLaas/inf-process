
# (gnu|c)++(17|20|2b|...)
std = c++20
# 0|1|2|3|fast|s|z
O = 2
# linux|mingw|windows
target = linux
# static|shared|header_only|static_shared
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
# build directory
dir = build

SHELL = /bin/sh
# path to .dll for wine
WINEPATH ?= /usr/x86_64-w64-mingw32/bin
# path to .so for linux
LD_LIBRARY_PATH ?=
# path to .dll for windows
PATH ?=

CXX ?= g++
CPPFLAGS = -MMD -MP -D_GNU_SOURCE -Iinclude
CXXFLAGS = ${CPPFLAGS} -std=${std} -Wall -Wextra -Wpedantic -Werror -O$O
LDFLAGS =
ARFLAGS = rcs

LIB_CXXFLAGS = ${CXXFLAGS}
LIB_LDFLAGS = ${LDFLAGS}
LIB_ARFLAGS = ${ARFLAGS}

LIB_SRC = ${wildcard src/*.cpp}
LIB_OBJ = ${LIB_SRC:%.cpp=${dir}/%.o}
LIB_DEP = ${LIB_OBJ:%.o=%.d}
LIB_EXEC = ${dir}/libinf-process-static.a ${dir}/libinf-process.so ${dir}/libinf-process.dll

TEST_CXXFLAGS = ${CXXFLAGS}
TEST_LDFLAGS = ${LDFLAGS}

EXP_CXXFLAGS = ${CXXFLAGS}
EXP_LDFLAGS = ${LDFLAGS}

ifeq (${target},linux)
TEST_EXT = .out
else
TEST_EXT = .exe
endif

TEST_SRC = ${wildcard tests/test-*.cpp}
TEST_EXEC = ${TEST_SRC:%.cpp=${dir}/%${TEST_EXT}}
TEST_DEP = ${TEST_EXEC:%${TEST_EXT}=%.d}
TESTFAIL_SRC = ${wildcard tests/testfail-*.cpp}
TESTFAIL_EXEC = ${TESTFAIL_SRC:%.cpp=${dir}/%${TEST_EXT}}
TESTFAIL_DEP = ${TESTFAIL_EXEC:%${TEST_EXT}=%.d}

EXP_SRC = ${wildcard examples/*.cpp examples/*/*.cpp}
EXP_EXEC = ${EXP_SRC:%.cpp=${dir}/%${TEST_EXT}}
EXP_DEP = ${EXP_EXEC:%${TEST_EXT}=%.d}

SEP = -------------------------------
TEST_FAILURE_FILE = ${dir}/.test-failure
PRINTF_RED = printf "\e[31m%s\e[0m\n"
PRINTF_GREEN = printf "\e[32m%s\e[0m\n"
RMDIR = rmdir --ignore-fail-on-non-empty
MKDIR = mkdir -p

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

ifeq (${target},windows)
	LIB_STATIC = ${dir}/libinf-process-static.a
	LIB_SHARED = ${dir}/libinf-process.dll
	WINDOWS = true
else ifeq (${target},mingw)
	LIB_STATIC = ${dir}/libinf-process-static.a
	LIB_SHARED = ${dir}/libinf-process.dll
	WINDOWS = true
else ifeq (${target},linux)
	LIB_STATIC = ${dir}/libinf-process-static.a
	LIB_SHARED = ${dir}/libinf-process.so
	WINDOWS = false
else
	$(error target must be mingw or linux)
endif

ifeq (${type},static)
	TEST_LDFLAGS += -L${dir} -linf-process-static
	EXP_LDFLAGS += -L${dir} -linf-process-static
	LIB = ${LIB_STATIC}
	LIB_STATIC_PART = ${LIB_STATIC}
	LIB_SHARED_PART =
	ifeq (${WINDOWS},true)
		TEST_LDFLAGS += -lws2_32
		EXP_LDFLAGS += -lws2_32
	endif
else ifeq (${type},shared)
	LIB_CXXFLAGS += -fPIC
	LIB_LDFLAGS += -shared
	TEST_LDFLAGS += -L${dir} -linf-process
	EXP_LDFLAGS += -L${dir} -linf-process
	LIB = ${LIB_SHARED}
	LIB_STATIC_PART =
	LIB_SHARED_PART = ${LIB_SHARED}
	ifeq (${target},windows)
		PATH := ${PATH}\;${dir}
	else ifeq (${target},mingw)
		WINEPATH := ${WINEPATH}\;${dir}
	else ifeq (${target},linux)
		LD_LIBRARY_PATH := ${LD_LIBRARY_PATH}:${dir}
	endif
	ifeq (${WINDOWS},true)
		LIB_LDFLAGS += -lws2_32
	endif
else ifeq (${type},static_shared)
	LIB_CXXFLAGS += -fPIC
	LIB_LDFLAGS += -shared
	TEST_LDFLAGS += -L${dir} -linf-process
	EXP_LDFLAGS += -L${dir} -linf-process
	LIB = ${LIB_SHARED} ${LIB_STATIC}
	LIB_STATIC_PART =
	LIB_SHARED_PART = ${LIB_SHARED}
	ifeq (${target},windows)
		PATH := ${PATH}\;${dir}
	else ifeq (${target},mingw)
		WINEPATH := ${WINEPATH}\;${dir}
	else ifeq (${target},linux)
		LD_LIBRARY_PATH := ${LD_LIBRARY_PATH}:${dir}
	endif
	ifeq (${WINDOWS},true)
		LIB_LDFLAGS += -lws2_32
	endif
else ifeq (${type},header_only)
	TEST_CXXFLAGS += -DINF_HEADER_ONLY
	EXP_CXXFLAGS += -DINF_HEADER_ONLY
	LIB =
	LIB_STATIC_PART =
	LIB_SHARED_PART =
	ifeq (${WINDOWS},true)
		TEST_LDFLAGS += -lws2_32
		EXP_LDFLAGS += -lws2_32
	endif
else
	$(error type must be static, shared, header_only or static_shared)
endif

export WINEPATH LD_LIBRARY_PATH PATH

all: lib

# special targets
.PHONY: all phony_explicit lib tests exps pre_check check clean_tests clean_lib clean_examples clean
.WAIT:
.SECONDARY:
phony_explicit:

${LIB_STATIC}: ${LIB_OBJ}
	@mkdir -p ${dir $@}
	${AR} ${LIB_ARFLAGS} $@ $^

ifeq (${type},static_shared)
${LIB_SHARED}: ${LIB_STATIC}
	@mkdir -p ${dir $@}
	${CXX} -o $@ -Wl,--whole-archive $< -Wl,--no-whole-archive ${LIB_LDFLAGS}
else
${LIB_SHARED}: ${LIB_OBJ}
	@mkdir -p ${dir $@}
	${CXX} -o $@ $^ ${LIB_LDFLAGS}
endif

${dir}/src/%.o: src/%.cpp
	@mkdir -p ${dir $@}
	${CXX} ${LIB_CXXFLAGS} -o $@ -c $<

${dir}/tests/test%${TEST_EXT}: tests/test%.cpp ${LIB_STATIC_PART} | ${LIB_SHARED_PART}
	@mkdir -p ${dir $@}
	${CXX} ${TEST_CXXFLAGS} -o $@ $< ${TEST_LDFLAGS}

${dir}/examples/%${TEST_EXT}: examples/%.cpp ${LIB_STATIC_PART} | ${LIB_SHARED_PART}
	@mkdir -p ${dir $@}
	${CXX} ${EXP_CXXFLAGS} -o $@ $< ${EXP_LDFLAGS}

${dir}/examples/rootme/tcp52022${TEST_EXT}: EXP_LDFLAGS += -lz

lib: ${LIB}

tests: ${TEST_EXEC} ${TESTFAIL_EXEC}

exps: ${EXP_EXEC}

checkfail_%: ${dir}/tests/testfail-%${TEST_EXT} phony_explicit
	@echo ${prefix} ./$<; \
	echo /${SEP}; \
	if ${ENV_PREFIX} ${prefix} ./$<; then \
		printf "%s %s " \\${SEP} "$<"; \
		${PRINTF_RED} "FAILURE"; \
		touch ${TEST_FAILURE_FILE}; \
	else \
		printf "%s %s " \\${SEP} "$<"; \
		${PRINTF_GREEN} "SUCCESS"; \
	fi; \
	echo

check_%: ${dir}/tests/test-%${TEST_EXT} phony_explicit
	@echo ${prefix} ./$<; \
	echo /${SEP}; \
	if ${ENV_PREFIX} ${prefix} ./$<; then \
		printf "%s %s " \\${SEP} "$<"; \
		${PRINTF_GREEN} "SUCCESS"; \
	else \
		printf "%s %s " \\${SEP} "$<"; \
		${PRINTF_RED} "FAILURE"; \
		touch ${TEST_FAILURE_FILE}; \
	fi; \
	echo

pre_check:
	${RM} ${TEST_FAILURE_FILE}

check: pre_check ${TEST_EXEC} ${TESTFAIL_EXEC} .WAIT ${addprefix check_,${subst tests/test-,,${basename ${TEST_SRC}}}} ${addprefix checkfail_,${subst tests/testfail-,,${basename ${TESTFAIL_SRC}}}}
	@[ -f "${TEST_FAILURE_FILE}" ] \
		&& ${PRINTF_RED} "CHECK FAILURE" \
		|| ${PRINTF_GREEN} "CHECK SUCCESS"
	@! [ -f "${TEST_FAILURE_FILE}" ] || (${RM} "${TEST_FAILURE_FILE}"; exit 1)

exp_%: ${dir}/examples/%${TEST_EXT} phony_explicit
	${prefix} ./$< ${args}

rootme_%: ${dir}/examples/rootme/%${TEST_EXT} phony_explicit
	${prefix} ./$< ${args}

clean_tests:
	${RM} ${TEST_EXEC} ${TESTFAIL_EXEC} ${TEST_DEP} ${TESTFAIL_DEP}

clean_lib:
	${RM} ${LIB_OBJ} ${LIB_EXEC} ${LIB_DEP}

clean_examples:
	${RM} ${EXP_EXEC} ${EXP_DEP}

clean: clean_tests clean_lib clean_examples
ifneq (${realpath ${dir}},${realpath .})
	rm -rf ${dir}
endif

-include ${LIB_DEP} ${TEST_DEP} ${TESTFAIL_DEP} ${EXP_DEP}

#
#  There exist several targets which are by default empty and which can be 
#  used for execution of your targets. These targets are usually executed 
#  before and after some main targets. They are: 
#
#     .build-pre:              called before 'build' target
#     .build-post:             called after 'build' target
#     .clean-pre:              called before 'clean' target
#     .clean-post:             called after 'clean' target
#     .clobber-pre:            called before 'clobber' target
#     .clobber-post:           called after 'clobber' target
#     .all-pre:                called before 'all' target
#     .all-post:               called after 'all' target
#     .help-pre:               called before 'help' target
#     .help-post:              called after 'help' target
#
#  Targets beginning with '.' are not intended to be called on their own.
#
#  Main targets can be executed directly, and they are:
#  
#     build                    build a specific configuration
#     clean                    remove built files from a configuration
#     clobber                  remove all built files
#     all                      build all configurations
#     help                     print help mesage
#  
#  Targets .build-impl, .clean-impl, .clobber-impl, .all-impl, and
#  .help-impl are implemented in nbproject/makefile-impl.mk.
#
#  Available make variables:
#
#     CND_BASEDIR                base directory for relative paths
#     CND_DISTDIR                default top distribution directory (build artifacts)
#     CND_BUILDDIR               default top build directory (object files, ...)
#     CONF                       name of current configuration
#     CND_PLATFORM_${CONF}       platform name (current configuration)
#     CND_ARTIFACT_DIR_${CONF}   directory of build artifact (current configuration)
#     CND_ARTIFACT_NAME_${CONF}  name of build artifact (current configuration)
#     CND_ARTIFACT_PATH_${CONF}  path to build artifact (current configuration)
#     CND_PACKAGE_DIR_${CONF}    directory of package (current configuration)
#     CND_PACKAGE_NAME_${CONF}   name of package (current configuration)
#     CND_PACKAGE_PATH_${CONF}   path to package (current configuration)
#
# NOCDDL


# Environment 
MKDIR=mkdir
CP=cp
CCADMIN=CCadmin

UNITY_ROOT=../Unity



# include project implementation makefile
#include nbproject/Makefile-impl.mk

# include project make variables
#include nbproject/Makefile-variables.mk



TARGET = sudoku-solver
LIBS = -lm
CC = gcc
#yypCFLAGS = -g -Wall -std=c99
CFLAGS = -g -Wall -std=c99

TEST_TARGET = testrunner-sudoku-solver

# directory containing the source files
SRC = src

# output directory
OUT = out

# directory containing the unit test source files
TEST = test

.PHONY: clean default all .init .init_test test
.PRECIOUS: $(OUT)/$(TARGET) $(OBJECTS)

.init:
	mkdir -p $(OUT)

.init_test:
	mkdir -p $(OUT)/$(TEST)

default: .init $(OUT)/$(TARGET)
all: clean default
test: .init_test execute_tests

execute_tests: $(OUT)/$(TEST_TARGET)
	# execute tests
	./$< -v

OBJECTS = $(patsubst $(SRC)/%.c, $(OUT)/%.o, $(wildcard $(SRC)/*.c)) \
  $(patsubst $(SRC)/strategies/%.c, $(OUT)/%.o, $(wildcard $(SRC)/strategies/*.c)) \
  $(patsubst $(SRC)/containers/%.c, $(OUT)/%.o, $(wildcard $(SRC)/containers/*.c)) \
  $(patsubst $(SRC)/output/%.c, $(OUT)/%.o, $(wildcard $(SRC)/output/*.c))
TEST_OBJECTS = \
  $(filter-out $(OUT)/main.o, $(patsubst $(SRC)/%.c, $(OUT)/%.o, $(wildcard $(SRC)/*.c))) \
  $(patsubst $(TEST)/%.c, $(OUT)/test/%.o, $(wildcard $(TEST)/test*.c))
HEADERS = $(wildcard $(SRC)/*.h)

INC_DIRS=-I$(SRC) -I$(SRC)/strategies -I$(SRC)/containers -I$(SRC)/output -I$(UNITY_ROOT)/src -I$(UNITY_ROOT)/extras/fixture/src
SYMBOLS=
TEST_SRC_FILES = \
  $(UNITY_ROOT)/src/unity.c \
  $(UNITY_ROOT)/extras/fixture/src/unity_fixture.c \
  $(filter-out src/main.c, $(wildcard $(SRC)/*.c)) \
  $(TEST)/test*.c

#SRC_FILES = $(SRC)/%.c $(SRC)/strategies/%.c $(SRC)/containers/%.c
#SRC_FILES = $(SRC)/%.c $(SRC)/strategies/%.c
#SRC_FILES = $(wildcard $(SRC)/%.c)
# $(wildcard $(SRC)/strategies/%.c)
SRC_FILES = $(SRC)/*.c $(SRC)/strategies/*.c $(SRC)/containers/*.c $(SRC)/output/*.c
HEADER_FILES = $(SRC)/*.h $(SRC)/strategies/*.h $(SRC)/containers/*.h $(SRC)/output/*.h

# compile source files
#$(OUT)/%.o: $(SRC)/%.c $(HEADERS)
#	$(CC) $(CFLAGS) $(INC_DIRS) $(SYMBOLS) -c $< -o $@

$(OUT)/%.o: $(SRC)/%.c $(HEADER_FILES)
	$(CC) $(CFLAGS) $(INC_DIRS) $(SYMBOLS) -c $< -o $@
$(OUT)/%.o: $(SRC)/strategies/%.c $(HEADER_FILES)
	$(CC) $(CFLAGS) $(INC_DIRS) $(SYMBOLS) -c $< -o $@
$(OUT)/%.o: $(SRC)/containers/%.c $(HEADER_FILES)
	$(CC) $(CFLAGS) $(INC_DIRS) $(SYMBOLS) -c $< -o $@
$(OUT)/%.o: $(SRC)/output/%.c $(HEADER_FILES)
	$(CC) $(CFLAGS) $(INC_DIRS) $(SYMBOLS) -c $< -o $@

	
# compile unit test files
$(OUT)/$(TEST)/%.o: $(TEST)/%.c $(HEADERS)
	$(CC) $(CFLAGS) $(INC_DIRS) -c $< -o $@

# link files
$(OUT)/$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $@

clean:
	-rm -f $(OUT)/*.o
	-rm -f $(OUT)/$(TARGET)

#$(OUT)/%.o: $(SRC)/%.c $(HEADERS)
#	$(CC) $(CFLAGS) -c $< -o $@

#$(OUT)/$(TEST_TARGET): $(OBJECTS) $(TEST_OBJECTS)
#	$(C_COMPILER) $(CFLAGS) $(INC_DIRS) $(SYMBOLS) $(SRC_FILES1) -o $(TEST_TARGET)
#	$(CC) $(OBJECTS) $(TEST_OBJECTS) -Wall $(LIBS) -o $@
#	./$(TEST_TARGET) -v

#$(OUT)/$(TEST_TARGET): $(OBJECTS) $(TEST_OBJECTS)
#	$(CC) $(TEST_OBJECTS) -Wall -o $@
#	# execute tests
#	./$@ -v

$(OUT)/$(TEST_TARGET): $(TEST_OBJECTS)
	$(CC) $(CFLAGS) $(INC_DIRS) $(SYMBOLS) $(TEST_SRC_FILES) -o $@

print-%:
	@echo '$*=$($*)'

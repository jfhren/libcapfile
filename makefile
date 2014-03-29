CC     := gcc
AR     := ar rcs
CFLAGS := -pedantic -W -Wall -W -Werror -std=c99 -g

SRC_DIR := ./src
OBJ_DIR := ./obj
BIN_DIR := ./bin
TOOL_DIR:= ./tool
INCLUDE := -Iinclude/
LIB     := -L. -lcapfile -lzip
OBJ     := $(OBJ_DIR)/analyzed_cap_file_verbose.o \
           $(OBJ_DIR)/cap_file_analyze.o          \
           $(OBJ_DIR)/cap_file_generate.o         \
           $(OBJ_DIR)/cap_file_reader.o           \
           $(OBJ_DIR)/cap_file_verbose.o          \
           $(OBJ_DIR)/cap_file_writer.o           \
           $(OBJ_DIR)/exp_file_reader.o           \
           $(OBJ_DIR)/exp_file_verbose.o
LIBNAME := libcapfile.a

all: mkobjd $(LIBNAME)

tool: mkobjd mkbind $(LIBNAME) $(BIN_DIR)/dump_cap_file $(BIN_DIR)/dump_analyzed_cap_file $(BIN_DIR)/dump_generated_cap_file $(BIN_DIR)/dump_exp_file

.SECONDEXPANSION:
$(LIBNAME): $(OBJ)
	$(AR) $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/$$*.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $^ -o $@

$(BIN_DIR)/%: $(TOOL_DIR)/$$*.c
	$(CC) $(CFLAGS) $(INCLUDE) $^ -o $@ $(LIB)


mkobjd:
	@mkdir -p $(OBJ_DIR)

mkbind:
	@mkdir -p $(BIN_DIR)

clean:
	@rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: mkobjd mkbind clean $(LIBNAME)

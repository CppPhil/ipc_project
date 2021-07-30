# **************************************************************
# *                Simple C++ Makefile Template                *
# *                                                            *
# * Author: Arash Partow (2003)                                *
# * URL: http://www.partow.net/programming/makefile/index.html *
# *                                                            *
# * Copyright notice:                                          *
# * Free use of this C++ Makefile template is permitted under  *
# * the guidelines and in accordance with the the MIT License  *
# * http://www.opensource.org/licenses/MIT                     *
# *                                                            *
# **************************************************************
#

COMPILER         := cc
CFLAGS           := -pedantic-errors -Wall -Wextra -Werror -std=gnu99 -D_DEFAULT_SOURCE
BUILD            := ./build
APP_DIR          := $(BUILD)/apps
CLIENT_INCLUDE   := -Iclient/include
CLIENT_SRC       := $(wildcard client/src/*.c)
CLIENT_OBJ_DIR   := $(BUILD)/client_objects
LAUNCHER_INCLUDE := -Ilauncher/include
LAUNCHER_SRC     := $(wildcard launcher/src/*.c)
LAUNCHER_OBJ_DIR := $(BUILD)/launcher_objects
SERVER_INCLUDE   := -Iserver/include
SERVER_SRC       := $(wildcard server/src/*.c)
SERVER_OBJ_DIR   := $(BUILD)/server_objects

CLIENT_OBJECTS := $(CLIENT_SRC:%.c=$(CLIENT_OBJ_DIR)/%.o)
LAUNCHER_OBJECTS := $(LAUNCHER_SRC:%.c=$(LAUNCHER_OBJ_DIR)/%.o)
SERVER_OBJECTS := $(SERVER_SRC:%.c=$(SERVER_OBJ_DIR)/%.o)

all: build $(APP_DIR)/client $(APP_DIR)/launcher $(APP_DIR)/server

$(CLIENT_OBJ_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(COMPILER) $(CFLAGS) $(CLIENT_INCLUDE) -c $< -MMD -o $@

$(LAUNCHER_OBJ_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(COMPILER) $(CFLAGS) $(LAUNCHER_INCLUDE) -c $< -MMD -o $@

$(SERVER_OBJ_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(COMPILER) $(CFLAGS) $(SERVER_INCLUDE) -c $< -MMD -o $@

$(APP_DIR)/client: $(CLIENT_OBJECTS)
	@mkdir -p $(@D)
	$(COMPILER) $(CFLAGS) -o $(APP_DIR)/client $^

$(APP_DIR)/launcher: $(LAUNCHER_OBJECTS)
	@mkdir -p $(@D)
	$(COMPILER) $(CFLAGS) -o $(APP_DIR)/launcher $^

$(APP_DIR)/server: $(SERVER_OBJECTS)
	@mkdir -p $(@D)
	$(COMPILER) $(CFLAGS) -o $(APP_DIR)/server $^

.PHONY: all build clean debug release info

build:
	@mkdir -p $(APP_DIR)
	@mkdir -p $(CLIENT_OBJ_DIR)
	@mkdir -p $(LAUNCHER_OBJ_DIR)
	@mkdir -p $(SERVER_OBJ_DIR)

debug: CFLAGS += -DDEBUG -g
debug: all

release: CFLAGS += -O3 -g -DNDEBUG
release: all

clean:
	-@rm -rvf $(CLIENT_OBJ_DIR)/*
	-@rm -rvf $(LAUNCHER_OBJ_DIR)/*
	-@rm -rvf $(SERVER_OBJ_DIR)/*
	-@rm -rvf $(APP_DIR)/*

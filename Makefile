CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c11 -Iinclude
LIBS = -lncursesw -lcjson

TARGET = manager

SRC = manager.c \
	src/ui.c \
	src/logger.c \
	src/process.c \
	src/profiles.c \
	src/actions.c \
	src/checks.c \
	src/profile_config.c \
	src/install_steps.c \
	src/julia_manager.c \
	src/jupyter_manager.c \
	src/kernel_manager.c \
	src/state_manager.c \
	src/cleanup_manager.c \
	src/workspace_manager.c \
	src/setup_manager.c \
	src/octave_manager.c \
	src/profile_registry.c \
	src/scilab_manager.c 

all:
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LIBS)

run: all
	./$(TARGET)

clean:
	rm -f $(TARGET)
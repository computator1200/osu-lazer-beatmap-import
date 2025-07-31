# Makefile for osu-lazer-beatmap-import

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
INCLUDES = -Iinclude -I$(shell brew --prefix libzip)/include
LDFLAGS = -L$(shell brew --prefix libzip)/lib
LIBS = -lzip -lz

# Directories
SRCDIR = src
BUILDDIR = build

# Source files (excluding zip_err_str.c since we'll use system libzip)
SOURCES = $(filter-out $(SRCDIR)/zip_err_str.c, $(wildcard $(SRCDIR)/*.c))
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)
TARGET = lazer-beatmap-import

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) $(LIBS) -o $@

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(BUILDDIR)
	rm -f $(TARGET)

# Create build directories
$(BUILDDIR):
	mkdir -p $(BUILDDIR)

# Dependencies
$(OBJECTS): | $(BUILDDIR)

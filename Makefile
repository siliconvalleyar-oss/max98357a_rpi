VERSION := $(shell cat VERSION 2>/dev/null || echo "0.1.0")

CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -DVERSION="$(VERSION)" -Iinclude -Iinclude/core -Iinclude/drivers -Iinclude/engine -Iinclude/security -Iinclude/sound -Iinclude/oled -Iinclude/libraries
LDFLAGS := -lbcm2835 -lasound

SRCDIR := src
OBJDIR := obj
BINDIR := bin

SOURCES := $(shell find $(SRCDIR) -name '*.cpp')
OBJECTS := $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SOURCES))

all: $(BINDIR)/App

$(BINDIR)/App: $(OBJECTS) | $(BINDIR)
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BINDIR):
	mkdir -p $(BINDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR)

distclean: clean
	rm -f $(BINDIR)/App

.PHONY: all clean distclean

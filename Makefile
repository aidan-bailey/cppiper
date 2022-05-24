BINNAME := cppipe

CXX := g++
SRCDIR := src
OBJDIR := obj
BINDIR := bin
SRCEXT := cc

TARGET := $(BINDIR)/$(BINNAME)

SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS := $(patsubst $(SRCDIR)/%,$(OBJDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
CXXFLAGS := -g # -ggdb -Wall -lm
LIB := -L lib
INC := -I include

$(TARGET): $(OBJECTS)
	@echo "Linking..."
	$(CXX) $^ $(LIB) -o $(TARGET)

$(OBJECTS): $(SOURCES)
	@echo "Building.."
	@mkdir -p $(OBJDIR)
	@mkdir -p $(BINDIR)
	$(CXX) -c $(CXXFLAGS) $(INC) -o $@ $<

clean:
	@rm -rf $(BINDIR) $(OBJDIR) $(TARGET) > /dev/null 2> /dev/null
	@echo "Cleaned"

.PHONY: clean

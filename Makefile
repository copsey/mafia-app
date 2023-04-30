# Name of the executable.
EXE = mafia
# List of C++ source files to compile.
SOURCE = \
	core/game.cpp \
	core/player.cpp \
	core/role.cpp \
	core/role_ref.cpp \
	core/rulebook.cpp \
	core/wildcard.cpp \
	interface/console.cpp \
	interface/game_log.cpp \
	interface/game_screens.cpp \
	interface/help_screens.cpp \
	interface/names.cpp \
	interface/questions.cpp \
	interface/screen.cpp \
	interface/setup_screen.cpp \
	interface/text/format.cpp \
	interface/text/preprocess.cpp \
	cli/main.cpp \
# Directory where intermediate build artifacts are stored.
BUILDDIR = build
# Directory where additional headers are stored.
INCLUDEDIR = include
# List of C++ object files.
OBJECTS = $(addprefix $(BUILDDIR)/,$(SOURCE:.cpp=.o))

# Version of the C++ standard to use when compiling and linking.
# For a list of supported values, search for `-std` in your compiler's manual.
CXXSTANDARD = c++20
# Extra flags passed to the C++ compiler.
CXXFLAGS += -std=$(CXXSTANDARD)
CXXFLAGS += -I include
CXXFLAGS += -D 'APPLICATION_ROOT_DIR="$(shell pwd)"'

build: $(EXE)

run: build
	@ ./$(EXE)

clean:
	$(RM) -r $(BUILDDIR)
	$(RM) -r $(INCLUDEDIR)
	$(RM) $(EXE)

$(OBJECTS): build/%.o: %.cpp
	@ mkdir -p $(dir $@)
	$(COMPILE.cpp) -o $@ $<

$(EXE): $(OBJECTS)
	$(LINK.cpp) -o $@ $^

.PHONY: build run clean

# Microsoft's Guidlines Support Library (GSL)
include/gsl:
	curl -sL https://github.com/microsoft/GSL/archive/refs/tags/v4.0.0.tar.gz \
	  | tar -xz GSL-4.0.0/include/gsl --strip-components=1

$(OBJECTS): include/gsl

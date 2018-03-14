# Indenting must use tabs!

OUT_DIR = linux/
EXECUTABLE = jcrcheck

CORECPP = \
	cl-jcr-parser/parser.cpp \
	cl-utils/str-args.cpp \
	dsl-pa/dsl-pa-alphabet.cpp \
	dsl-pa/dsl-pa-dsl-pa.cpp \
	dsl-pa/dsl-pa-reader.cpp

MAINCPP = main/main.cpp

COREOBJ = $(addprefix $(OUT_DIR),$(CORECPP:.cpp=.o))
MAINOBJ = $(addprefix $(OUT_DIR),$(MAINCPP:.cpp=.o))

MKDIR_P ?= mkdir -p

UNDESIRABLE_CXXFLAGS = -Wno-strict-aliasing -Wno-parentheses # It would be nice to get rid of these

CXXFLAGS = -O3 -I include -Werror -Wunused-parameter -Wuninitialized -Wunused-variable -Wall $(UNDESIRABLE_CXXFLAGS) -DNDEBUG

.PHONY: all fresh clean

all: $(OUT_DIR)$(EXECUTABLE)

fresh: clean all

$(OUT_DIR)$(EXECUTABLE): $(MAINOBJ) $(COREOBJ)
	$(CXX) -static -o $(OUT_DIR)$(EXECUTABLE) $(MAINOBJ) $(COREOBJ)
	$(OUT_DIR)$(EXECUTABLE)

$(OUT_DIR)%.o : src/%.cpp
	$(MKDIR_P) $(dir $@)
	$(CXX) -c $(CXXFLAGS) $< -o $@

$(OUT_DIR)%.o : %.cpp
	$(MKDIR_P) $(dir $@)
	$(CXX) -c $(CXXFLAGS) $< -o $@

clean:
	-rm -f $(OUT_DIR)main/*.o
	-rm -f $(OUT_DIR)cl-jcr-parser/*.o
	-rm -f $(OUT_DIR)cl-utils/*.o
	-rm -f $(OUT_DIR)dsl-pa/*.o

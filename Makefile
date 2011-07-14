
DEBUG = 
OPTIMIZATION = -O2

DEP_BASE = /z/libs

GL_INCLUDES = -I $(DEP_BASE)/gl/include
GL_LIBS = -lopengl32 -lglu32

FT_INCLUDES = -I $(DEP_BASE)/freetype/include -I $(DEP_BASE)/freetype/include/freetype2
FT_LIB_DIRS = -L $(DEP_BASE)/freetype/lib
FT_LIBS = -lfreetype

INCLUDE_DIRS = $(GL_INCLUDES) $(FT_INCLUDES)
LIB_DIRS = $(FT_LIB_DIRS)
LIBS = $(GL_LIBS) $(FT_LIBS) $(WIN_LIBS)

WIN_LIBS = -lgdi32 -lwinmm -luuid -lole32 -loleaut32 -lcomctl32

CXX_MISC = -Wall -Wextra
LD_MISC = -Wl,-subsystem,windows -static-libgcc -static-libstdc++
CXXFLAGS = $(INCLUDE_DIRS) $(DEBUG) $(OPTIMIZATION) $(CXX_MISC)
LDFLAGS = $(LIB_DIRS) $(LD_MISC)

SRC_DIR = src
OUTPUT_DIR = build

SRC_FILES = $(SRC_DIR)/*.cpp
RC_FILE = "$(SRC_DIR)/res/Solar Space.rc"
RES_FILE = $(OUTPUT_DIR)/res.o
OUTPUT_FILE = "$(OUTPUT_DIR)/Solar Space"

.PHONY: all clean

all:
	mkdir -p $(OUTPUT_DIR)
	windres $(RC_FILE) $(RES_FILE)
	g++ $(CXXFLAGS) $(LDFLAGS) -o $(OUTPUT_FILE) $(SRC_FILES) $(RES_FILE) $(LIBS)

clean:
	rm -rf $(OUTPUT_DIR)

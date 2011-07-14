
# Copyright (C) 2003-2011 Victor Semionov
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#  * Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#  * Neither the name of the copyright holder nor the names of the contributors
#    may be used to endorse or promote products derived from this software
#    without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


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

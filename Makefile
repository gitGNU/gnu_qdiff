VER              = 0.8.3

TARGET           = qdiff

CXXFLAGS         = -Wall -W -Weffc++

CPPFLAGS         = -DVERSION=\"$(VER)\"

include Makefile.rules

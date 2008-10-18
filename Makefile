VER              = 0.8.2

TARGET           = qdiff

CXXFLAGS         = -Wall -W -Weffc++

CPPFLAGS         = -DVERSION=\"$(VER)\"

include Makefile.rules

include Makefile.common
bin_PROGRAMS = qdiff
TAPPFRAME_SRC += tfiletools.h tfiletools.cc terror.cc  terror.h
qdiff_SOURCES = qdiff.cc trotfile.h trotfile.cc tdiffoutput.h tdiffoutput.cc tminmax.h $(TAPPFRAME_SRC)
#man_MANS = qdiff.1
.PHONY: test

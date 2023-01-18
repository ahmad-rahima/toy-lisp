SED := sed
MV  := mv

CFLAGS := -g -gdwarf-4
ARFLAGS := rvs

PROGRAM_NAME  := lisp

main_dir    := ../main
sources_dir    := ../src
include_dir    := ../include .
libraries_dir  := ../lib
tests_dir     := ../tests
#out_dir        :=
program_dir := ../prog

CFLAGS += $(addprefix -I,$(include_dir))

main := $(main_dir)/lisp.c
program := $(program_dir)/$(PROGRAM_NAME)
sources := $(wildcard $(sources_dir)/*.c)
objects := $(addsuffix .o, \
		$(basename \
			$(notdir $(sources_dir)/%,%,$(sources)))) lisp_parser.o lisp_scanner.o
tests := $(wildcard $(tests_dir)/*.c)
deps    := $(patsubst %.o,%.d,$(objects))
library := $(libraries_dir)/lib$(PROGRAM_NAME).a

VPATH := $(sources_dir) $(include_dir) .

# abort of ran from anywhere but "out/"
ifneq ($(notdir $(CURDIR)),out)
	_ := $(error Please run make from the out directory.)
endif

.PHONY: all
all: $(objects) $(library) $(program)

#$(objects): %.o: %.c

test: $(tests:.c=)
	@echo Running all tests..
	@for test in $^; do echo testname \"`basename $$test`\":; ./$$test; echo -e '\n----------'; done
	@echo Done.
$(tests:.c=): LDLIBS += -lrt -lm
$(tests:.c=): %: %.c $(library)
	$(CC) $(CFLAGS) $(LOADLIBS) $(LDLIBS) -o $@ $^

lisp_scanner.o: lisp_scanner.c

lisp_scanner.c: lisp_scanner.l lisp_parser.h
	echo "Running"
	flex -t $< >$@

lisp_parser.o: lisp_parser.c

lisp_parser.c lisp_parser.h: lisp_parser.y
	bison --header=lisp_parser.h --output=lisp_parser.c $<

%.d: %.c
	$(CC) $(CFLAGS) $(TARGET_ARCH) -M $< | \
	$(SED) 's,\($(notdir $*)\.o\) *:,$(dir $@)\1 $@: ,' > $@.tmp; \
	$(MV) $@.tmp $@


$(library): $(objects)
	$(AR) $(ARFLAGS) $@ $^

$(program): $(main) $(library)
	$(CC) $(CFLAGS) -o $@ $^


-include $(deps)

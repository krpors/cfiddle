CPPFLAGS = -DNDEBUG
CFLAGS =  -std=gnu99 -ggdb -Wall -Wextra -pedantic -O3 -MMD -MP
CFLAGS += $(shell pkg-config --cflags sdl2)
LDLIBS =  $(shell pkg-config --libs sdl2)
LDLIBS += -lm # for math
objects = fire.o pgen.o

all: fire pgen
list: $(objects)

debug: all
debug: CPPFLAGS += -UNDEBUG
debug: CFLAGS += -ggdb -Og
debug: LDFLAGS += -ggdb -Og

clean:
	$(RM) $(objects) $(objects:.o=.d) fire pgen

-include $(objects:.o=.d)

.PHONY: all clean

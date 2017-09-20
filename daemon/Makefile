CPPFLAGS = -DNDEBUG
CFLAGS = -std=gnu99 -Wall -Wextra -pedantic -O3 -MMD -MP
objects = daemon.o

all: daemon
daemon: $(objects)

debug: all
debug: CPPFLAGS += -UNDEBUG
debug: CFLAGS += -ggdb -Og
debug: LDFLAGS += -ggdb -Og

clean:
	$(RM) $(objects) $(objects:.o=.d) daemon

-include $(objects:.o=.d)

.PHONY: all clean

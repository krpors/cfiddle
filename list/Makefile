CPPFLAGS = -DNDEBUG
CFLAGS = -std=gnu99 -Wall -Wextra -pedantic -O3 -MMD -MP
objects = list.o

all: list
list: $(objects)

debug: all
debug: CPPFLAGS += -UNDEBUG
debug: CFLAGS += -ggdb -Og
debug: LDFLAGS += -ggdb -Og

clean:
	$(RM) $(objects) $(objects:.o=.d) list

-include $(objects:.o=.d)

.PHONY: all clean

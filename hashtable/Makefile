CPPFLAGS = -DNDEBUG
CFLAGS = -std=gnu99 -ggdb -Wall -Wextra -pedantic -O3 -MMD -MP
objects = hashtable.o

all: hashtable
hashtable: $(objects)

debug: all
debug: CPPFLAGS += -UNDEBUG
debug: CFLAGS += -ggdb -Og
debug: LDFLAGS += -ggdb -Og

clean:
	$(RM) $(objects) $(objects:.o=.d) hashtable

-include $(objects:.o=.d)

.PHONY: all clean

CPPFLAGS =
CFLAGS = -std=gnu99 -Wall -Wextra -pedantic -O3 -MMD -MP -ggdb
objects = hashtable.o

all: hashtable
hashtable: $(objects)

clean:
	$(RM) $(objects) $(objects:.o=.d) hashtable

-include $(objects:.o=.d)

.PHONY: all clean

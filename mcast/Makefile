CPPFLAGS = -DNDEBUG
CFLAGS = -std=gnu99 -Wall -Wextra -pedantic -O3 -MMD -MP
objects = mcast.o

all: mcast
mcast: $(objects)

clean:
	$(RM) $(objects) $(objects:.o=.d) mcast

-include $(objects:.o=.d)

.PHONY: all clean

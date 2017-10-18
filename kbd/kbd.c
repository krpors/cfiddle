#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#include <linux/input.h>

/*
 * https://www.kernel.org/doc/Documentation/input/input.txt
 */
int main() {
	// Note: input device is not default event2
	char* device = "/dev/input/event2";
	FILE* f = fopen(device, "r");
	if (f == NULL) {
		perror(device);
		exit(1);
	}

	bool right_ctrl_press = false;

	struct input_event evt;
	size_t sz;
	while ((sz = fread(&evt, sizeof(struct input_event), 1, f) != 0)) {
		if (evt.code == KEY_RIGHTCTRL) {
			right_ctrl_press = evt.type;
		}
		if (evt.code == KEY_A && evt.value == 1 && right_ctrl_press) {
			printf("Ctrl A!\n");
		}
	}

	return 0;
}

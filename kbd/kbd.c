#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

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

		bool pressed = (evt.value == 1 || evt.value == 2);
		//  keypress _______________/                 /
		//  key repeat ______________________________/

		if (evt.code == KEY_A && pressed && right_ctrl_press) {
			struct tm* nowtm = localtime(&evt.time.tv_sec);
			char buf[64] = {0};
			strftime(buf, sizeof(buf), "%Y-%m-%d @ %H:%M:%S", nowtm);
			printf("%s Ctrl A!\n", buf);
		}
	}

	return 0;
}

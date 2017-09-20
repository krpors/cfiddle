#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct node {
	void* data;
	struct node* next;
};

struct node* node_create(void* data) {
	struct node* n = malloc(sizeof(struct node));
	n->data = data;
	n->next = NULL;
	return n;
}

struct node* node_append(struct node* list, void* data) {
	struct node* last = NULL;

	// iterate until we found the last one in the list.
	for(struct node* next = list; next != NULL; next = next->next) {
		last = next;
	}

	if (last != NULL) {
		printf("Appending '%s' -> '%s'\n", (char*) last->data, (char*) data);
		struct node* next = node_create(data);
		next->data = data;
		last->next = next;
		return next;
	}

	return NULL;
}

void node_free(struct node* start) {
	struct node* next = start;
	while (next != NULL) {
		struct node* curr = next;
		next = curr->next;

		free(curr->data);
		free(curr);
	}
}

int main() {
	struct node* list = node_create(strdup("one"));
	node_append(list, strdup("two"));
	node_append(list, strdup("three"));
	
	for (struct node* next = list; next != NULL; next = next->next) {
		char* hi = (char*) next->data;
		printf("Entry: %s\n", hi);
	}

	node_free(list);

	return 0;
}

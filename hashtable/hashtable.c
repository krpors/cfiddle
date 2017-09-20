#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h> // for srand()

#include <assert.h>

#include "hashtable.h"

/*
 * 'Simple' hash function. See http://www.cse.yorku.ca/~oz/hash.html,
 * and http://stackoverflow.com/questions/7666509/hash-function-for-string
 */
static unsigned long hash_djb2(const char* str) {
	unsigned long hash = 5381;
	int c;
	while ((c = *str++)) {
		hash = ((hash << 5) + hash) + c;
	}

	return hash;
}


struct hashtable* hashtable_create(unsigned int cap, float lf) {
	struct hashtable* tbl = malloc(sizeof(struct hashtable));
	tbl->cap = cap;
	tbl->load_factor = lf;
	tbl->table = calloc(cap, sizeof(struct entry));
	tbl->entries = 0;
	return tbl;
}


void hashtable_set(struct hashtable** ht, const char* key, const char* val) {
	// calculate hash, then mod it with the table's size to determine the bucket.
	unsigned long hash = hash_djb2(key);
	int bucket = hash % (*ht)->cap; // will result in a bucket between [0, (*ht)->cap).

	struct entry* next = (*ht)->table[bucket];

	// Bucket is empty, so we can create a brand new one.
	if (next == NULL) {
#ifndef NDEBUG
		fprintf(stderr, "Bucket %d is empty, creating new one\n", bucket);
#endif
		struct entry* e = malloc(sizeof(struct entry));
		e->key  = strdup(key);
		e->val  = strdup(val);
		e->next = NULL;
		(*ht)->table[bucket] = e;
		(*ht)->entries++;
		return;
	}

	// This points to the last (non null) entry in the bucket.
	struct entry* last = NULL;

	// Bucket is not empty, traverse the linked list, see if the same key is there.
	for(; next != NULL; next = next->next) {
		last = next;

		// Check if the requested key equals the key in the current entry.
		// If so, the value will be replaced.
		if (strcmp(key, next->key) == 0) {
#ifndef NDEBUG
			fprintf(stderr, "Bucket %d [%s = %s] will be replaced with [%s = %s]\n", bucket, next->key, next->val, key, val);
#endif
			free(next->val);
			next->val = strdup(val);
			return;
		}
	}

	// When we reach this point, no existing key is found. It's still a hash collision,
	// so we add a new entry to the back of the list.
#ifndef NDEBUG
	fprintf(stderr,"Bucket %d contained hash collision, so appending new node to end\n", bucket);
#endif
	struct entry* e = malloc(sizeof(struct entry));
	e->key = strdup(key);
	e->val = strdup(val);
	e->next = NULL;

	// Update the last element to point to the newly created entry.
	last->next = e;

	(*ht)->entries++;
	hashtable_resize(ht);
}


const char* hashtable_get(const struct hashtable* ht, const char* key) {
	unsigned long hash = hash_djb2(key);
	int bucket = hash % ht->cap;

	for (struct entry* entry = ht->table[bucket]; entry != NULL; entry = entry->next) {
		if (strcmp(key, entry->key) == 0) {
			return entry->val;
		}
	}
	return NULL;
}


bool hashtable_remove(struct hashtable* ht, const char* key) {
	unsigned long hash = hash_djb2(key);
	int bucket = hash % ht->cap;
	struct entry* first = ht->table[bucket];
	struct entry* prev  = NULL;
	struct entry* next  = first;

	// Nothing to see here. Bail out!
	if (first == NULL) {
		return false;
	}

	// There's at least one element in the linked list.
	for(; next != NULL; next = next->next) {
		// Did the key match? Then remove that one, update the list correctly.
		if (strcmp(key, next->key) == 0) {
			// Are we the first in the list?
			if (prev == NULL) {
				ht->table[bucket] = next->next;

				free(next->key);
				free(next->val);
				free(next);
			} else {
				// We're somewhere in the middle, or at the end of the list.
				prev->next = next->next;
				free(next->key);
				free(next->val);
				free(next);
			}

			ht->entries--;
			return true;
		}

		prev = next;
	}

	// At this point nothing can be found to remove.
	return false;
}


void hashtable_free(struct hashtable* ht) {
	for (unsigned int i = 0; i < ht->cap; i++) {
		struct entry* next = ht->table[i];
		while (next != NULL) {
			struct entry* curr = next;
			next = next->next;
#ifndef NDEBUG
			printf("Freeing key '%s' with value '%s'\n", curr->key, curr->val);
#endif
			free(curr->key);
			free(curr->val);
			free(curr);
		}
	}

	free(ht->table);
	free(ht);
}


void hashtable_resize(struct hashtable** ht) {
	float loadfactor = (float)(*ht)->entries / (float)(*ht)->cap;

	if (loadfactor < (*ht)->load_factor) {
		// return ourselves, nothing to be done.
		return;
	}

#ifndef NDEBUG
	fprintf(stderr, "Rehashing: Load factor is %f\n", loadfactor);
	fprintf(stderr, "About to rehash pointer %p\n", (void*)*ht);
#endif

	// Create a new hashtable, increase the size by twice
	struct hashtable* rehashed = hashtable_create((*ht)->cap * 2, (*ht)->load_factor);
	// iterate over all buckets and keys, re-add them.
	for (unsigned int i = 0; i < (*ht)->cap; i++) {
		for (struct entry* next = (*ht)->table[i]; next != NULL; next = next->next) {
			hashtable_set(&rehashed, next->key, next->val);
		}
	}
	
	// free the old hashtable
	hashtable_free(*ht);

	*ht = rehashed;
}

void hashtable_for_each(const struct hashtable* ht, void (*callback)(const char* k, const char* v)) {
	// iterate over all buckets
	for (unsigned int i = 0; i < ht->cap; i++) {
		// iterate over every entry in the bucket
		for (struct entry* next = ht->table[i]; next != NULL; next = next->next) {
			(*callback)(next->key, next->val);
		}
	}
}

void thecallback(const char* key, const char* val) {
	printf("%s = %s\n", key, val);
}

void test1() {
	srand(time(NULL));
	struct hashtable* tbl = hashtable_create(2, 0.75);
	int max = 200000;
	for (int i = 0; i < max; i++) {
		char key[16];
		char val[16];
		snprintf(key, 16, "key_%d_%d", i, rand());
		snprintf(val, 16, "val_%d_%d", i, rand());
		hashtable_set(&tbl, key, val);
	}
	printf("Inserted %d items. Bucket size is %d\n", max, tbl->cap);

	hashtable_for_each(tbl, &thecallback);

	hashtable_free(tbl);
}

void test2() {
	srand(time(NULL));
	struct hashtable* tbl = hashtable_create(16, 0.75);

	int i = 0;
	int c = 0;
	printf("Hashtable example. Type 'q' to quit. Press enter to keep adding items.\n");
	while ((c = getchar()) != 'q') {
		char key[16];
		char val[16];
		snprintf(key, 16, "key_%d_%d", i, rand());
		snprintf(val, 16, "val_%d_%d", i, rand());
		hashtable_set(&tbl, key, val);
		i++;


		hashtable_for_each(tbl, &thecallback);
		printf("================================================================================\n");
		printf("Capacity:      %d\n", tbl->cap);
		printf("Keys inserted: %d\n", i);
		printf("Load factor:   %f\n", (float)i / (float)tbl->cap);
		printf("================================================================================\n");
	}

	hashtable_free(tbl);
}

int main(/*int argc, char* argv[]*/) {
	test2();
	return 0;
}

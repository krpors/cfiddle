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
static unsigned long hash_djb2(const void* key, size_t key_len) {
	unsigned long hash = 5381;
	const unsigned char* p = key;
	for (size_t i = 0; i < key_len; i++) {
		hash = ((hash << 5) + hash) + p[i];
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


void hashtable_set(struct hashtable** ht, const void* key, size_t key_len, const void* val, size_t val_len) {
	// calculate hash, then mod it with the table's size to determine the bucket.
	unsigned long hash = hash_djb2(key, key_len);
	int bucket = hash % (*ht)->cap; // will result in a bucket between [0, (*ht)->cap).

	struct entry* next = (*ht)->table[bucket];

	// Bucket is empty, so we can create a brand new one.
	if (next == NULL) {
#ifndef NDEBUG
		fprintf(stderr, "Bucket %d is empty, creating new one\n", bucket);
#endif
		struct entry* e = malloc(sizeof(struct entry));

		e->key  = malloc(key_len);
		e->key_len = key_len;
		memcpy(e->key, key, key_len);

		e->val  = malloc(val_len);
		e->val_len = val_len;
		memcpy(e->val, val, val_len);

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
		if (memcmp(key, next->key, key_len) == 0) {
#ifndef NDEBUG
			fprintf(stderr, "Bucket %d [%s = %s] will be replaced with [%s = %s]\n", bucket, next->key, next->val, key, val);
#endif
			free(next->val);
			next->val  = malloc(val_len);
			memcpy(next->val, val, val_len);
			return;
		}
	}

	// When we reach this point, no existing key is found. It's still a hash collision,
	// so we add a new entry to the back of the list.
#ifndef NDEBUG
	fprintf(stderr,"Bucket %d contained hash collision, so appending new node to end\n", bucket);
#endif
	struct entry* e = malloc(sizeof(struct entry));
	e->key     = malloc(key_len);
	e->key_len = key_len;
	memcpy(e->key, key, key_len);

	e->val     = malloc(val_len);
	e->val_len = val_len;
	memcpy(e->val, val, val_len);

	e->next = NULL;

	// Update the last element to point to the newly created entry.
	last->next = e;

	(*ht)->entries++;
	hashtable_resize(ht);
}


const struct entry* hashtable_get(const struct hashtable* ht, const void* key, size_t key_len) {
	unsigned long hash = hash_djb2(key, key_len);
	int bucket = hash % ht->cap;

	for (struct entry* entry = ht->table[bucket]; entry != NULL; entry = entry->next) {
		if (memcmp(key, entry->key, key_len) == 0) {
			return entry;
		}
	}
	return NULL;
}


bool hashtable_remove(struct hashtable* ht, const void* key, size_t key_len) {
	unsigned long hash = hash_djb2(key, key_len);
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
			curr->key = NULL;
			free(curr->val);
			curr->val = NULL;
			free(curr);
			curr = NULL;
		}
	}

	free(ht->table);
	ht->table = NULL;
	free(ht);
	ht = NULL;
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
			printf("%s (%li) = %s (%li)\n", (char*)next->key, next->key_len, (char*)next->val, next->val_len);
			hashtable_set(&rehashed, next->key, next->key_len, next->val, next->val_len);
		}
	}
	
	// free the old hashtable
	hashtable_free(*ht);

	*ht = rehashed;
}

void hashtable_for_each(const struct hashtable* ht, void (*callback)(const struct entry*)) {
	// iterate over all buckets
	for (unsigned int i = 0; i < ht->cap; i++) {
		// iterate over every entry in the bucket
		for (struct entry* next = ht->table[i]; next != NULL; next = next->next) {
			(*callback)(next);
		}
	}
}

void thecallback(const struct entry* e) {
	printf("entry! %s -> %s\n", (char*)e->key, (char*)e->val);
}

void test1() {
	srand(time(NULL));
	struct hashtable* tbl = hashtable_create(2, 0.75);
	int max = 20000;
	for (int i = 0; i < max; i++) {
		char key[16];
		char val[16];
		snprintf(key, 16, "key_%d_%d", i, rand());
		snprintf(val, 16, "val_%d_%d", i, rand());
		hashtable_set(&tbl, key, strlen(key) + 1, val, strlen(val) + 1);
	}
	printf("Inserted %d items. Bucket size is %d\n", max, tbl->cap);

	hashtable_for_each(tbl, &thecallback);

	hashtable_free(tbl);
}

#if 0
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
#endif

int main(/*int argc, char* argv[]*/) {
	test1();
	return 0;
}

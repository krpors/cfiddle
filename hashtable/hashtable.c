#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <assert.h>

/*
 * TODO: don't permit NULL keys.
 */

/*
 * This struct is the actual bucket entry. It's designed as a linked list.
 */
struct entry {
	char* key;
	char* val;
	struct entry* next;
};

/*
 * The hashtable.
 */
struct hashtable {
	unsigned int size;    // Size of the hashtable (amount of buckets).
	struct entry** table; // Array of entry structs. This is the actual table.
	unsigned int entries; // Amount of entries in the table. Used to determine load factor.
};

// Forward declarations. TODO: put this in a header.
struct hashtable* hashtable_create(unsigned int size);
void hashtable_set(struct hashtable** ht, const char* key, const char* val);
const char* hashtable_get(const struct hashtable* ht, const char* key);
bool hashtable_remove(struct hashtable* ht, const char* key);
void hashtable_free(struct hashtable* ht);
void hashtable_rehash(struct hashtable** ht);

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

/*
 * Creates a hashtable and returns it. Size is the initial amount of 'buckets' for
 * the hashtable.
 */
struct hashtable* hashtable_create(unsigned int size) {
	struct hashtable* tbl = malloc(sizeof(struct hashtable));
	tbl->size = size;
	tbl->table = calloc(size, sizeof(struct entry));
	tbl->entries = 0;
	return tbl;
}

/*
 * Sets a key and a value in the given hashtable 'ht'. The given key must be not
 * be null. The key and value are duplicated on the heap.
 */
void hashtable_set(struct hashtable** ht, const char* key, const char* val) {
	// calculate hash, then mod it with the table's size to determine the bucket.
	unsigned long hash = hash_djb2(key);
	int bucket = hash % (*ht)->size; // will result in a bucket between [0, (*ht)->size).

	printf("Setting key %s = %s into bucket = %d, hashtable pointer %p\n", key, val, (*ht)->size, (void*) ht);
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
	fprintf(stderr, "pre-rehash: %p\n", (void*)ht);
	hashtable_rehash(ht);
}

/*
 * Fetches a key from the hashtable. A pointer to the value is returned.
 */
const char* hashtable_get(const struct hashtable* ht, const char* key) {
	unsigned long hash = hash_djb2(key);
	int bucket = hash % ht->size;

	for (struct entry* entry = ht->table[bucket]; entry != NULL; entry = entry->next) {
		if (strcmp(key, entry->key) == 0) {
			return entry->val;
		}
	}
	return NULL;
}

/*
 * Remove a key from the hashtable.
 */
bool hashtable_remove(struct hashtable* ht, const char* key) {
	unsigned long hash = hash_djb2(key);
	int bucket = hash % ht->size;
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

/*
 * Frees all memory held by the hashtable and its buckets.
 */
void hashtable_free(struct hashtable* ht) {
	for (unsigned int i = 0; i < ht->size; i++) {
		struct entry* next = ht->table[i];
		while (next != NULL) {
			struct entry* curr = next;
			next = next->next;
			printf("Freeing key '%s' with value '%s'\n", curr->key, curr->val);

			free(curr->key);
			free(curr->val);
			free(curr);
		}
	}

	free(ht->table);
	free(ht);
}

/*
 * When the load factor (entries / size) exceeds 0.75, increase the size and 'rehash'
 * the hashtable by adding all entries again. The original given hashtable is freed
 * and the pointer is reset to the newly rehashed table.
 *
 * Note that you have to use a double pointer to change the pointer (since arguments
 * are passed by value in C).
 *
 * I had help with this: https://stackoverflow.com/questions/13431108/changing-address-contained-by-pointer-using-function#13431146
 */
void hashtable_rehash(struct hashtable** ht) {
	float loadfactor = (float)(*ht)->entries / (float)(*ht)->size;

	if (loadfactor < 0.75f) {
		// return ourselves, nothing to be done.
		return;
	}

#ifndef NDEBUG
	fprintf(stderr, "Rehashing: Load factor is %f\n", loadfactor);
	fprintf(stderr, "About to rehash pointer %p\n", (void*)*ht);
#endif

	// Create a new hashtable, increase the size by twice
	struct hashtable* rehashed = hashtable_create((*ht)->size * 2);
	// iterate over all buckets and keys, re-add them.
	for (unsigned int i = 0; i < (*ht)->size; i++) {
		for (struct entry* next = (*ht)->table[i]; next != NULL; next = next->next) {
			hashtable_set(&rehashed, next->key, next->val);
		}
	}
	
	// free the old hashtable
	hashtable_free(*ht);

	*ht = rehashed;
}

/*
 * Prints out the hashtable just for debugging.
 */
void hashtable_print(struct hashtable* ht) {
	for (unsigned int i = 0; i < ht->size; i++) {
		struct entry* next = ht->table[i];
		if (next != NULL) {
			printf("Bucket %d\n", i);
			for (; next != NULL; next = next->next) {
				printf("\t[%s = %s]\n", next->key, next->val);
			}
		}
	}
}


int main(/*int argc, char* argv[]*/) {
	struct hashtable* tbl = hashtable_create(6);
	hashtable_set(&tbl, "kevin", "hai");
	hashtable_set(&tbl, "kevin", "cruft");
	hashtable_set(&tbl, "kevin", "bawls");
	hashtable_set(&tbl, "flarg", "herpdederp");
	hashtable_set(&tbl, "frood", "heist");
	hashtable_set(&tbl, "bangz", "lolz0r");
	hashtable_set(&tbl, "foobs", "NEWLOLOZ");
	hashtable_set(&tbl, "struz", "allbolrl");
	hashtable_set(&tbl, "quuux", "HARHAR");

	printf(">>> Initial table:\n");
	hashtable_print(tbl);

	hashtable_remove(tbl, "kevin");
	hashtable_remove(tbl, "quuux");

	printf("\n>>> After removing:\n");
	hashtable_print(tbl);

	//hashtable_rehash(&tbl);

	printf("\n>>> After rehashing:\n");
	hashtable_print(tbl);

	hashtable_free(tbl);

	return 0;
}

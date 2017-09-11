#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <assert.h>


/*
 * TODO: don't permit NULL keys.
 */

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
};

/*
 * Creates a hashtable and returns it. Size is the initial amount of 'buckets' for
 * the hashtable.
 */
struct hashtable* hashtable_create(unsigned int size) {
	struct hashtable* tbl = malloc(sizeof(struct hashtable));
	tbl->size = size;
	tbl->table = calloc(size, sizeof(struct entry));
	return tbl;
}

/*
 * Sets a key and a value in the given hashtable 'tbl'.
 */
void hashtable_set(struct hashtable* tbl, char* key, char* val) {
	// calculate hash, then mod it with the table's size to determine the bucket.
	unsigned long hash = hash_djb2(key);
	int bucket = hash % tbl->size; // will result in a bucket between [0, tbl->size).

	struct entry* next = tbl->table[bucket];

	// Bucket is empty, so we can create a brand new one.
	if (next == NULL) {
		printf("Bucket %d is empty, creating new one\n", bucket);
		struct entry* e = malloc(sizeof(struct entry));
		e->key  = strdup(key);
		e->val  = strdup(val);
		e->next = NULL;
		tbl->table[bucket] = e;
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
			printf("Bucket %d [%s = %s] will be replaced with [%s = %s]\n", bucket, next->key, next->val, key, val);
			free(next->val);
			next->val = strdup(val);
			return;
		}
	}

	// When we reach this point, no existing key is found. It's still a hash collision,
	// so we add a new entry to the back of the list.
	printf("Bucket %d contained hash collision, so appending new node to end\n", bucket);
	struct entry* e = malloc(sizeof(struct entry));
	e->key = strdup(key);
	e->val = strdup(val);
	e->next = NULL;

	// Update the last element to point to the newly created entry.
	last->next = e;
}

const char* hashtable_get(const struct hashtable* ht, const char* key) {
	unsigned long hash = hash_djb2(key);
	int bucket = hash % ht->size;
	printf("Key '%s' could be found in bucket %d\n", key, bucket);

	for (struct entry* entry = ht->table[bucket]; entry != NULL; entry = entry->next) {
		if (strcmp(key, entry->key) == 0) {
			return entry->val;
		}
	}
	return NULL;
}

/*
 * Frees all memory held by the hashtable and its buckets.
 */
void hashtable_free(struct hashtable* tbl) {
	for (unsigned int i = 0; i < tbl->size; i++) {
		//printf("Freeing bucket %d\n", i);
		struct entry* next = tbl->table[i];
		while (next != NULL) {
			struct entry* curr = next;
			next = next->next;

			free(curr->key);
			free(curr->val);
			free(curr);
		}
	}

	free(tbl->table);
	free(tbl);
}

void hashtable_print(struct hashtable* tbl) {
	for (unsigned int i = 0; i < tbl->size; i++) {
		struct entry* next = tbl->table[i];
		if (next != NULL) {
			printf("Bucket %d\n", i);
			for (; next != NULL; next = next->next) {
				printf("\t[%s = %s]\n", next->key, next->val);
			}
		}
	}
}


int main(/*int argc, char* argv[]*/) {
	struct hashtable* tbl = hashtable_create(900);
	hashtable_set(tbl, "kevin", "hai");
	hashtable_set(tbl, "kevin", "cruft");
	hashtable_set(tbl, "kevin", "bawls");
	hashtable_set(tbl, "nivek", "herpdederp");
	hashtable_set(tbl, "props", "heist");
	hashtable_set(tbl, "propz", "lolz0r");
	hashtable_set(tbl, "nivek", "NEWLOLOZ");
	hashtable_set(tbl, "nevik", "allbolrl");
	hashtable_set(tbl, "nevik", "HARHAR");
	printf("\nPrinting buckets:\n");
	hashtable_print(tbl);

	const char* val = hashtable_get(tbl, "nivek");
	printf("Found value '%s' for the key\n", val);

	hashtable_free(tbl);

	return 0;
}

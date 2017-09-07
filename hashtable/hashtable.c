#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

struct entry {
	char* key;
	char* val;
	struct entry* next;
};

struct hashtable {
	unsigned int size;
	struct entry** table;
};

struct hashtable* hashtable_create(unsigned int size) {
	struct hashtable* tbl = malloc(sizeof(struct hashtable));
	tbl->size = size;
	tbl->table = malloc(sizeof(struct entry) * size);
	memset(tbl->table, 0, sizeof(struct entry) * size);
	return tbl;
}

void hashtable_print(struct hashtable* tbl) {
	printf("================================================================================\n");
	for (unsigned int i = 0; i < tbl->size; i++) {
		printf("Bucket %d\n", i);
		struct entry* next = tbl->table[i];
		while (next != NULL) {
			printf("\t[%s = %s]\n", next->key, next->val);
			next = next->next;
		}
	}
	printf("================================================================================\n");
}

void hashtable_set(struct hashtable* tbl, char* key, char* val) {
	// calculate hash
	// check if bucket is empty
	unsigned long hash = hash_djb2(key);
	int bucket = hash % tbl->size;

	struct entry* next = NULL;

	next = tbl->table[bucket];

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

	// Bucket is not empty, traverse the linked list, see if the same key is there.
	while (next != NULL) {
		if (strcmp(key, next->key) == 0) {
			printf("Bucket %d [%s = %s] will be replaced with [%s = %s]\n", bucket, next->key, next->val, key, val);
			free(next->val);
			next->val = strdup(val);
			return;
		}

		// advance to the next link in the list, if any.
		if (next->next == NULL) {
			// end of the list, break out.
			break;
		}
		next = next->next;
	}

	printf("Bucket %d contained hash collision, so appending new node to end\n", bucket);
	struct entry* e = malloc(sizeof(struct entry));
	e->key = strdup(key);
	e->val = strdup(val);
	e->next = NULL;
	next->next = e;
}

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

int main(/*int argc, char* argv[]*/) {
	char* bla1 = "test string 1";
	char* bla2 = "string test 1";

	unsigned long hash1 = hash_djb2(bla1);
	unsigned long hash2 = hash_djb2(bla2);

	int buckets = 400;

	printf("%s hashed = %lu, index = %lu\n", bla1, hash1, (hash1 % buckets));
	printf("%s hashed = %lu, index = %lu\n", bla2, hash2, (hash2 % buckets));

	struct hashtable* tbl = hashtable_create(16);
	hashtable_set(tbl, "kevin", "hai");
	hashtable_set(tbl, "kevin", "cruft");
	hashtable_set(tbl, "kevin", "bawls");
	hashtable_set(tbl, "nivek", "herpdederp");
	hashtable_set(tbl, "props", "heist");
	hashtable_set(tbl, "propz", "lolz0r");
	hashtable_set(tbl, "nivek", "NEWLOLOZ");
	hashtable_set(tbl, "nevik", "allbolrl");
	hashtable_set(tbl, "nevik", "HARHAR");
	hashtable_print(tbl);
	hashtable_free(tbl);

	return 0;
}

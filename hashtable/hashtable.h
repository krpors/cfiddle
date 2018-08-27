#ifndef HASHTABLE_H
#define HASHTABLE_H

/*
 * This struct is the actual bucket entry. It's designed as a linked list.
 * When hash collisions occur (the key is put in the same bucket), an entry
 * is appended to the end of the linked list.
 */
struct entry {
	void*  key;
	size_t key_len;

	void*  val;
	size_t val_len;

	struct entry* next; // Next entry. NULL if end of the list.
};

/*
 * The hashtable itself, containing everything required for it to function.
 */
struct hashtable {
	unsigned int cap;     // Capacity of the hashtable (amount of buckets).
	float load_factor;    // Load factor. Resize will be done when entries / cap is
	                      // higher than this number.
	struct entry** table; // Array of entry structs. This is the actual table.
	unsigned int entries; // Amount of entries in the table. Used to determine load factor.
};

/**
 * Creates a new hashtable on the heap with the specified capacity (buckets), and
 * the specified load factor. The hashtable will be resized when the internal load
 * factor (entries / capacity) exceeds this load factor. A logical default choice is
 * a value of 0.75.
 */
struct hashtable* hashtable_create(unsigned int cap, float loadfactor);

/**
 * Sets a new key and value in the hashtable. When the load factor exceeds
 * 0.75 (entries / cap), the hashtable is resized. The key and value are duplicated
 * on the heap and then inserted in the hashtable.
 *
 * If the key already exists in the table, the previous value is freed, then reallocated
 * and reset.
 *
 * If the key does not exist, but a hash collision occurred, the entry is appended
 * to the end of the linked list desribed in `struct entry'.
 *
 * Since this function also does the rehashing (and thus resetting the pointer), the
 * pointer to the hashtable is a pointer itself.
 */
void hashtable_set(struct hashtable** ht, const void* key, size_t key_len, const void* val, size_t val_len);

/**
 * Gets the value belonging to the key in the hashtable. If the key is not found,
 * NULL is returned. If the key is found, a pointer to it is returned.
 */
const struct entry* hashtable_get(const struct hashtable* ht, const void* key, size_t key_len);

/**
 * Removes (and frees) an entry in the hashtable. Returns `true' if the entry was
 * successfully removed, or `false' if the key could not be removed because it did
 * not exist.
 */
bool hashtable_remove(struct hashtable* ht, const void* key, size_t key_len);

/**
 * Frees all entries, keys and values and the hashtable itself.
 */
void hashtable_free(struct hashtable* ht);

/**
 * Resizes the hashtable so the capacity (buckets) are optimally used. 'Optimally'
 * depends highly on the hash function used though.
 */
void hashtable_resize(struct hashtable** ht);

/**
 * Iterates through every bucket and link in the list, and executed the callback
 * function `callback' with the found key and value.
 */
void hashtable_for_each(const struct hashtable* ht, void (*callback)(const struct entry* e));

#endif // HASHTABLE_H

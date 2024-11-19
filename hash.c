/* 
 * hash.c -- implements a generic hash table as an indexed set of queues.
 *
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash.h"
#include "queue.h"

/* 
 * SuperFastHash() -- produces a number between 0 and the tablesize-1.
 * 
 * The following (rather complicaated) code, has been taken from Paul
 * Hsieh's website under the terms of the BSD license. It's a hash
 * function used all over the place nowadays, including Google Sparse
 * Hash.
 */
#define get16bits(d) (*((const uint16_t *) (d)))

static uint32_t SuperFastHash (const char *data,int len,uint32_t tablesize) {
  uint32_t hash = len, tmp;
  int rem;
  
  if (len <= 0 || data == NULL)
		return 0;
  rem = len & 3;
  len >>= 2;
  /* Main loop */
  for (;len > 0; len--) {
    hash  += get16bits (data);
    tmp    = (get16bits (data+2) << 11) ^ hash;
    hash   = (hash << 16) ^ tmp;
    data  += 2*sizeof (uint16_t);
    hash  += hash >> 11;
  }
  /* Handle end cases */
  switch (rem) {
  case 3: hash += get16bits (data);
    hash ^= hash << 16;
    hash ^= data[sizeof (uint16_t)] << 18;
    hash += hash >> 11;
    break;
  case 2: hash += get16bits (data);
    hash ^= hash << 11;
    hash += hash >> 17;
    break;
  case 1: hash += *data;
    hash ^= hash << 10;
    hash += hash >> 1;
  }
  /* Force "avalanching" of final 127 bits */
  hash ^= hash << 3;
  hash += hash >> 5;
  hash ^= hash << 4;
  hash += hash >> 17;
  hash ^= hash << 25;
  hash += hash >> 6;
  return hash % tablesize;
}


/* Internal representation of the hashtable */
struct hashtable {
    uint32_t hsize;          // size of the hash table
    queue_t **queues;        // array of pointers to queues
};

hashtable_t *hopen(uint32_t hsize) {
    struct hashtable *htp = (struct hashtable *)malloc(sizeof(struct hashtable));
    if (htp == NULL) {
        return NULL;
    }
    htp->hsize = hsize;
    htp->queues = (queue_t **)malloc(hsize * sizeof(queue_t *));
    if (htp->queues == NULL) {
        free(htp);
        return NULL;
    }
    for (uint32_t i = 0; i < hsize; i++) {
        htp->queues[i] = qopen();
        if (htp->queues[i] == NULL) {
            // Free previously allocated queues and the hashtable
            for (uint32_t j = 0; j < i; j++) {
                qclose(htp->queues[j]);
            }
            free(htp->queues);
            free(htp);
            return NULL;
        }
    }
    return (hashtable_t *)htp;
}

void hclose(hashtable_t *htp) {
    struct hashtable *ht = (struct hashtable *)htp;
    if (ht == NULL) {
        return;
    }
    for (uint32_t i = 0; i < ht->hsize; i++) {
        qclose(ht->queues[i]);
    }
    free(ht->queues);
    free(ht);

    htp = NULL;

}

int32_t hput(hashtable_t *htp, void *ep, const char *key, int keylen) {
    struct hashtable *ht = (struct hashtable *)htp;
    if (ht == NULL || ep == NULL || key == NULL || keylen <= 0) {
        return -1;
    }
    uint32_t index = SuperFastHash(key, keylen, ht->hsize);
    return qput(ht->queues[index], ep);
}

void *hsearch(hashtable_t *htp, 
              bool (*searchfn)(void* elementp, const void* searchkeyp), 
              const char *key, 
              int32_t keylen) {
    struct hashtable *ht = (struct hashtable *)htp;
    if (ht == NULL || searchfn == NULL || key == NULL || keylen <= 0) {
        return NULL;
    }
    uint32_t index = SuperFastHash(key, keylen, ht->hsize);
    return qsearch(ht->queues[index], searchfn, key);
}

void *hremove(hashtable_t *htp, 
              bool (*searchfn)(void* elementp, const void* searchkeyp), 
              const char *key, 
              int32_t keylen) {
    struct hashtable *ht = (struct hashtable *)htp;
    if (ht == NULL || searchfn == NULL || key == NULL || keylen <= 0) {
        return NULL;
    }
    uint32_t index = SuperFastHash(key, keylen, ht->hsize);
    return qremove(ht->queues[index], searchfn, key);
}

void happly(hashtable_t *htp, void (*fn)(void* ep)) {
    struct hashtable *ht = (struct hashtable *)htp;
    if (ht == NULL || fn == NULL) {
        return;
    }
    for (uint32_t i = 0; i < ht->hsize; i++) {
        qapply(ht->queues[i], fn);
    }
}
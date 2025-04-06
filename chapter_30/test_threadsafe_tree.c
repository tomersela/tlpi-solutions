#include <assert.h>
#include <pthread.h>
#include <stdio.h>

#include "threadsafe_tree.h"

#define NUM_THREADS 10
#define NUM_KEYS 100


typedef struct ThreadArgs {
    struct TreeNode *tree;
    int thread_id;
} ThreadArgs;


static void *add_keys(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;
    struct TreeNode *tree = args->tree;
    int thread_id = args->thread_id;

    char key[50];
    for (int i = thread_id * NUM_KEYS; i < (thread_id + 1) * NUM_KEYS; i++) {
        snprintf(key, sizeof(key), "key_%d", i);
        int *value = malloc(sizeof(int));
        *value = i;
        add(tree, key, value);
    }
    return NULL;
}


static void *lookup_keys(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;
    struct TreeNode *tree = args->tree;
    int thread_id = args->thread_id;

    char key[50];
    for (int i = thread_id * NUM_KEYS; i < (thread_id + 1) * NUM_KEYS; i++) {
        snprintf(key, sizeof(key), "key_%d", i);
        void *value = NULL;
        Boolean found = lookup(tree, key, &value);
        if (found) {
            assert(value != NULL);
            assert(*(int *)value == i);
        }
    }
    return NULL;
}


static void *delete_keys(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;
    struct TreeNode *tree = args->tree;
    int thread_id = args->thread_id;

    char key[50];
    for (int i = thread_id * NUM_KEYS; i < (thread_id + 1) * NUM_KEYS; i++) {
        snprintf(key, sizeof(key), "key_%d", i);
        delete(tree, key);
    }
    return NULL;
}


int main(void) {
    struct TreeNode *tree = new_tree();

    pthread_t threads[NUM_THREADS];
    ThreadArgs args[NUM_THREADS];

    // insert keys in parallel
    for (int i = 0; i < NUM_THREADS; i++) {
        args[i] = (ThreadArgs){.tree = tree, .thread_id = i};
        pthread_create(&threads[i], NULL, add_keys, &args[i]);
    }
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);

    // lookup keys in parallel
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_create(&threads[i], NULL, lookup_keys, &args[i]);
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);

    // delete keys in parallel
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_create(&threads[i], NULL, delete_keys, &args[i]);
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);

    // verify tree is empty
    for (int i = 0; i < NUM_THREADS * NUM_KEYS; i++) {
        char key[50];
        snprintf(key, sizeof(key), "key_%d", i);
        void *value = NULL;
        assert(!lookup(tree, key, &value));
    }

    printf("All tests passed!\n");
    return 0;
}

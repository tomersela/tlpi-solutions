#ifndef TREE_H
#define TREE_H

#include <pthread.h>
#include "tlpi_hdr.h"

struct TreeNode {
    pthread_mutex_t mtx;
    char *key;
    void *value;
    struct TreeNode *left;
    struct TreeNode *right;
};

struct TreeNode *new_tree(void);
void initialize(struct TreeNode *tree);
void add(struct TreeNode *tree, char *key, void *value);
void delete(struct TreeNode *tree, char *key);
Boolean lookup(struct TreeNode *tree, char *key, void **value);

#endif

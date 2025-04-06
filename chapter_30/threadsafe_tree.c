#include <pthread.h>
#include <string.h>
#include <assert.h>

#include "tlpi_hdr.h"
#include "threadsafe_tree.h"

#define NUM_THREADS 2
#define NUM_KEYS 1000


#ifdef DEBUG_LOCKS
    #define DEBUG(...) printf("[Thread %lu] ", pthread_self()); printf(__VA_ARGS__); fflush(stdout);
#else
    #define DEBUG(...)
#endif

#define MAX_KEY_LEN 1024


static void
delete_node(struct TreeNode *node)
{
    free(node->key);
    node->key = NULL;
    pthread_mutex_unlock(&node->mtx);
    pthread_mutex_destroy(&node->mtx);
    free(node);
}

struct TreeNode *
new_tree()
{
    struct TreeNode *tree = (struct TreeNode *) malloc(sizeof(struct TreeNode));
    if (tree == NULL) {
        errExit("malloc");
    }
    initialize(tree);
    return tree;
}


void
initialize(struct TreeNode *tree)
{
    pthread_mutex_init(&tree->mtx, NULL);
    tree->key = NULL;
    tree->value = NULL;
    tree->left = NULL;
    tree->right = NULL;
}


void
add(struct TreeNode *tree, char *key, void *value)
{
    pthread_mutex_lock(&tree->mtx);
    
    if (tree->key == NULL) {
        tree->key = strdup(key);
        tree->value = value;
        pthread_mutex_unlock(&tree->mtx);
        return;
    }

    struct TreeNode *curr = tree;
    struct TreeNode *parent = NULL; // we're also locking the parent to avoid races with the delete function
    
    while (curr) {
        if (strcmp(key, curr->key) == 0) {
            curr->value = value;
            pthread_mutex_unlock(&curr->mtx);
            if (parent)
                pthread_mutex_unlock(&parent->mtx);
            return;
        }
        
        struct TreeNode **next = strcmp(key, curr->key) < 0 ? &curr->left : &curr->right;
        if (*next == NULL) {
            struct TreeNode *newNode = new_tree();
            pthread_mutex_lock(&newNode->mtx);  // Ensure it is locked before making it visible
            newNode->key = strdup(key);
            newNode->value = value;
            *next = newNode;
            pthread_mutex_unlock(&newNode->mtx);
            pthread_mutex_unlock(&curr->mtx);
            if (parent) pthread_mutex_unlock(&parent->mtx);
            return;
        }

        pthread_mutex_lock(&(*next)->mtx);
        if (parent)
            pthread_mutex_unlock(&parent->mtx);
        parent = curr;
        curr = *next;
    }

    if (parent)
        pthread_mutex_unlock(&parent->mtx);
}


void delete(struct TreeNode *tree, char *key) {
    DEBUG("delete(%s)\n", key);
    if (!tree) return;
    
    pthread_mutex_lock(&tree->mtx);
    if (tree->key == NULL) {
        DEBUG("\tempty tree %s\n", key);
        // empty tree
        pthread_mutex_unlock(&tree->mtx);
        return;
    } else if (strncmp(key, tree->key, MAX_KEY_LEN) == 0) {
        // deleting root node
        DEBUG("\tdeleting root node (key = %s)\n", key);

        if (tree->left == NULL && tree->right == NULL) {
            // no children for root node
            DEBUG("\tno children for root node (key = %s)\n", key);
            free(tree->key);
            tree->key = NULL;
            tree->value = NULL;
            pthread_mutex_unlock(&tree->mtx);
            return;
        }

        if (tree->left != NULL && tree->right != NULL) {
            // two children for root node
            DEBUG("\ttwo children for root node (key = %s)\n", key);

            // find successor (left most node from the right sub-tree); lock our way down
            struct TreeNode *next = tree->right;
            struct TreeNode *successor = NULL;
            struct TreeNode *successor_parent = NULL;
            struct TreeNode *successor_grand_parent = NULL;

            while (next) {
                pthread_mutex_lock(&next->mtx);
                successor_grand_parent = successor_parent;
                successor_parent = successor;
                successor = next;

                if (successor_grand_parent)
                    pthread_mutex_unlock(&successor_grand_parent->mtx);

                next = next->left;
            }

            // set parrent node with successor values
            DEBUG("\tset root node with successor values. tree->key = %s successor->key = %s\n", tree->key, successor->key);
            tree->key = strdup(successor->key);
            tree->value = successor->value;

            // remove successor node
            DEBUG("\tremove successor node. successor->key = %s\n", successor->key);
            delete_node(successor);

            // update successor's parent node pointer
            if (successor == tree->right) {
                // this means our parent is actually the root node
                tree->right = NULL;
            } else if (successor_parent != NULL) {
                if (successor_parent->left == successor) {
                    successor_parent->left = NULL;
                } else {
                    successor_parent->right = NULL;
                }
            } else {
                errExit("SHOULD NEVER HAPPEN - successor_parent == NULL\n");
            }

            // unlock successor's parent
            if (successor_parent)
                pthread_mutex_unlock(&successor_parent->mtx);

            // unlock root node
            pthread_mutex_unlock(&tree->mtx);
            return;
        }

        // root has one child
        DEBUG("\troot has one child (key = %s)\n", key);
        struct TreeNode *child = tree->left ? tree->left : tree->right;
        pthread_mutex_lock(&child->mtx);

        // copy child's data
        DEBUG("\tcopy child's data -- tree->key = %s; child->key = %s\n", tree->key, child->key);
        tree->key = strdup(child->key);
        tree->value = child->value;
        tree->left = child->left;
        tree->right = child->right;

        // remove child node
        delete_node(child);

        // unlock root node
        pthread_mutex_unlock(&tree->mtx);
        return;

    } else {
        // node to delete isn't the root node - traverse the tree to find node to delete; Lock our way down
        DEBUG("\tnode to delete isn't the root node -- key = %s, tree->key = %s\n", key, tree->key);
        struct TreeNode *curr = strncmp(key, tree->key, MAX_KEY_LEN) < 0 ? tree->left : tree->right;
        if (curr) {
            DEBUG("\tcurr->key = %s (%s)\n", curr->key, curr == tree->left ? "left": "right");
        } else {
            DEBUG("\tcurr = null\n");
        }
        
        struct TreeNode *curr_parent = tree;
        struct TreeNode *curr_grand_parent = NULL;

        while (curr) {
            pthread_mutex_lock(&curr->mtx);
            DEBUG("\t\ttraversing curr->key = %s (%s); tree->key = %s\n", curr->key, curr == curr_parent->left ? "left": "right", tree->key);
            if (curr_grand_parent)
                pthread_mutex_unlock(&curr_grand_parent->mtx);

            int cmp = strncmp(key, curr->key, MAX_KEY_LEN);
            if (cmp == 0) {
                break;
            }
            
            curr_grand_parent = curr_parent;
            curr_parent = curr;
            curr = cmp < 0 ? curr->left : curr->right;
        }

        if (curr) {
            // found node to delete
            // at this point only the node to delete and its parent should be locked

            if (curr->left == NULL && curr->right == NULL) {
                // node has no children

                // update parent's child pointer
                if (curr_parent->left == curr) {
                    curr_parent->left = NULL;
                } else {
                    curr_parent->right = NULL;
                }

                // remove current node
                DEBUG("\tdestory mutex (node with no children) - curr->key = %s\n", curr->key);
                delete_node(curr);

                // unlock parent
                pthread_mutex_unlock(&curr_parent->mtx);

                return;
            }


            if (curr->left != NULL && curr->right != NULL) {
                // node has two children

                // find successor node
                struct TreeNode *successor_parent = curr;
                struct TreeNode *successor = curr->right;
                pthread_mutex_lock(&successor->mtx);  // lock successor

                while (successor->left) {
                    struct TreeNode *next = successor->left;
                    pthread_mutex_lock(&next->mtx);     // lock child first
                    pthread_mutex_unlock(&successor_parent->mtx);  // safe unlock parent

                    successor_parent = successor;
                    successor = next;
                }

                // set parrent node with successor values
                DEBUG("\tset parrent node with successor values -- curr->key = %s successor->key = %s\n", curr->key, successor->key);
                curr->key = strdup(successor->key);
                curr->value = successor->value;

                // remove successor node
                DEBUG("\tdestory mutex - successor->key = %s\n", successor->key);
                delete_node(successor);

                if (successor_parent) {
                    // update successor's parent pointer
                    if (successor_parent->left == successor) {
                        successor_parent->left = NULL;
                    } else {
                        successor_parent->right = NULL;
                    }

                    // unlock successor's parent
                    pthread_mutex_unlock(&successor_parent->mtx);
                }

                // unlock current node
                pthread_mutex_unlock(&curr->mtx);

                // unlock parent
                pthread_mutex_unlock(&curr_parent->mtx);
                return;
            }

            // node has one child
            struct TreeNode* curr_child = curr->left != NULL ? curr->left : curr->right;

            // update parent's child pointer
            if (curr_parent->left == curr) {
                curr_parent->left = curr_child;
            } else {
                curr_parent->right = curr_child;
            }

            // remove current node
            DEBUG("\tdestory mutex (node with one child) - curr->key = %s\n", curr->key);
            free(curr->key);
            curr->value = NULL;
            pthread_mutex_unlock(&curr->mtx);

            pthread_mutex_destroy(&curr->mtx);
            free(curr);

            // unlock parent
            pthread_mutex_unlock(&curr_parent->mtx);
            
            return;

        } else {
            DEBUG("\tno node found %s\n", key);
            // no node found
            // at this point the parent node is locked and the grand-parent node is locked if not NULL
            if (curr_grand_parent)
                pthread_mutex_unlock(&curr_grand_parent->mtx);
            pthread_mutex_unlock(&curr_parent->mtx);
        }
    }
}


Boolean
lookup(struct TreeNode *tree, char *key, void **value)
{
    if (tree == NULL)
        return FALSE;

    pthread_mutex_lock(&tree->mtx);

    if (tree->key == NULL) {  // head node was deleted after lock
        pthread_mutex_unlock(&tree->mtx);
        return FALSE;
    }

    // lookup node; lock our way down to prevent race conditions
    struct TreeNode *curr = tree;
    struct TreeNode *parent = NULL; // we're also locking the parent to avoid races with the delete function

    while (curr) {
        DEBUG("key = %s, curr->key = %s\n", key, curr->key);
        if (strcmp(key, curr->key) == 0)
            break;
        
        struct TreeNode *next = strcmp(key, curr->key) < 0 ? curr->left : curr->right;
        if (next)
            pthread_mutex_lock(&next->mtx);
        
        if (parent)
            pthread_mutex_unlock(&parent->mtx);
        
        parent = curr;
        curr = next;
    }

    if (curr) {
        *value = curr->value;
        pthread_mutex_unlock(&curr->mtx);
        if (parent)
            pthread_mutex_unlock(&parent->mtx);

        return TRUE;
    }

    if (parent)
        pthread_mutex_unlock(&parent->mtx);

    return FALSE;
}

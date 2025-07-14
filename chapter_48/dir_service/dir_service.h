#ifndef DIR_SERVICE_H
#define DIR_SERVICE_H

#define NAME_LEN 32          // Maximum length of a name (key), excluding null terminator
#define VALUE_LEN 128        // Maximum length of a value, excluding null terminator

/**
 * Initialize the directory service.
 *
 * This function creates the shared memory segment and semaphore used by the
 * directory service. If the shared memory and semaphore already exist,
 * this function silently succeeds.
 *
 * Must be called once before using any other directory service functions.
 *
 * @return 0 on success, -1 on failure (check errno for details).
 */
int dir_init(void);

/**
 * Set or update a name–value pair in the directory.
 *
 * If the name already exists, its value is updated. If the name does not
 * exist and there is space, a new entry is added.
 *
 * @param name The name (key) to set. Must be non-NULL and null-terminated.
 * @param value The value to associate with the name. Must be non-NULL and null-terminated.
 * @return 0 if the name was updated,
 *         1 if a new entry was added,
 *        -1 on error (e.g., full directory, IPC issues).
 */
int dir_set(const char *name, const char *value);

/**
 * Retrieve the value associated with a given name.
 *
 * @param name The name (key) to look up. Must be non-NULL and null-terminated.
 * @param value_out A buffer of at least VALUE_LEN+1 bytes to receive the value.
 * @return 0 on success (value copied into buffer),
 *        -1 if the name was not found or an error occurred.
 */
int dir_get(const char *name, char *value_out);

/**
 * Delete a name–value pair from the directory.
 *
 * Marks the entry as unused if the name is found.
 *
 * @param name The name (key) to delete. Must be non-NULL and null-terminated.
 * @return 0 on success (entry removed),
 *        -1 if the name was not found or an error occurred.
 */
int dir_delete(const char *name);

/**
 * Clean up the directory service.
 *
 * This removes the System V shared memory segment and semaphore associated
 * with the directory. Should be called when the service is no longer needed.
 * If the resources do not exist, this function does nothing.
 */
void dir_cleanup(void);

#endif // DIR_SERVICE_H

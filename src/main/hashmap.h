/*
 * Generic hashmap manipulation functions
 *
 * Originally by Elliot C Back - http://elliottback.com/wp/hashmap-implementation-in-c/
 *
 * Modified by Pete Warden to fix a serious performance problem, support strings as keys
 * and removed thread synchronization - http://petewarden.typepad.com
 *
 * Modified by Konrad Siek to fit the round hole into the square peg of this project. Yeah, I know what I said.
 */
#ifndef __HASHMAP_H__
#define __HASHMAP_H__

#include <stdint.h>

#define MAP_MISSING -3  /* No such element */
#define MAP_FULL -2 	/* Hashmap is full */
#define MAP_OMEM -1 	/* Out of Memory */
#define MAP_OK 0 	/* OK */

/*
 * any_t is a pointer.  This allows you to put arbitrary structures in
 * the hashmap.
 */
typedef void *hashmap_any_t;
typedef void *hashmap_val_t;
typedef uintptr_t hashmap_key_t;
typedef struct {hashmap_val_t value; int status;} hashmap_ret_t;

/*
 * A pointer to a function that can take two arguments: key and value of the
 * map and return an integer. The additional argument can be used for
 * returning values. Returns status code.
 */
typedef int (*hashmap_iter)(hashmap_key_t, hashmap_val_t, void *extra);

/*
 * A pointer to a function that can take one argument: a key
 * and return an integer. The additional argument can be used for
 * returning values. Returns status code.
 */
typedef int (*hashmap_key_iter)(hashmap_key_t, void *extra);

/*
 * map_t is a pointer to an internally maintained data structure.
 * Clients of this package do not need to know how hashmaps are
 * represented.  They see and manipulate only map_t's.
 */
typedef hashmap_any_t map_t;

/*
 * Return an empty hashmap. Returns NULL if empty.
 */
extern map_t hashmap_new();

/*
 * Iteratively call f with argument (key, value, data) for
 * each element data in the hashmap. The function must
 * return a map status code. If it returns anything other
 * than MAP_OK (int 0) the traversal is terminated. f must
 * not reenter any hashmap functions, or deadlock may arise.
 * There is an extra argument that can be used for arbitrary
 * purposes by the programmer, for instance: for returning
 * results.
 */
extern int hashmap_iterate(map_t in, hashmap_iter f, void *extra);

/*
 * Iteratively call f with argument (key, data) for
 * each element data in the hashmap. The function must
 * return a map status code. If it returns anything other
 * than MAP_OK (int 0) the traversal is terminated. f must
 * not reenter any hashmap functions, or deadlock may arise.
 * There is an extra argument that can be used for arbitrary
 * purposes by the programmer, for instance: for returning
 * results.
 */
extern int hashmap_iterate_keys(map_t in, hashmap_key_iter f, void *extra);

/*
 * Add an element to the hashmap. Return MAP_OK or MAP_OMEM.
 */
extern int hashmap_put(map_t in, hashmap_key_t key, hashmap_val_t value);

/*
 * Get an element from the hashmap. Return MAP_OK or MAP_MISSING.
 */
extern hashmap_ret_t hashmap_get(map_t in, hashmap_key_t key);

/*
 * Remove an element from the hashmap. Return MAP_OK or MAP_MISSING.
 */
extern int hashmap_remove(map_t in, hashmap_key_t key);

/*
 * Get any element. Return MAP_OK or MAP_MISSING.
 * remove - should the element be removed from the hashmap
 */
//extern int hashmap_get_one(map_t in, any_t *arg, int remove);

/*
 * Free the hashmap
 */
extern void hashmap_free(map_t in);

/*
 * Get the current size of a hashmap
 */
extern int hashmap_length(map_t in);

#endif //__HASHMAP_H__
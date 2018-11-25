#ifndef __SET_H__
#define __SET_H__

#include <stdint.h>

#define SET_MISSING -3  /* No such element */
#define SET_FULL -2 	/* Set is full */
#define SET_OMEM -1 	/* Out of Memory */
#define SET_OK 0 	/* OK */
#define SET_MEMBER -4 	/* Element already a member. */

typedef void *set_any_t;
typedef uintptr_t set_elem_t;

typedef int (*set_iter_t)(set_elem_t, void *extra);

/*
 * The type set_t is a pointer to an internally maintained data
 * structure.Clients of this package do not need to know how
 * sets are represented. They see and manipulate only set_t's.
 */
typedef set_any_t set_t;

/*
 * Return an empty tree set. Returns NULL if empty.
 */
extern set_t hashset_new(char *name);

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
extern int set_iterate(set_t, set_iter_t, set_any_t extra);

/*
 * Add an element to the hashmap. Return MAP_OK or MAP_OMEM.
 */
extern int set_add(set_t, set_elem_t);

/*
 * Check if element is a member of the set. Return MAP_OK for
 * true or MAP_MISSING for false.
 */
extern int set_member(set_t, set_elem_t);

/*
 * Remove an element from the set. Return MAP_OK or MAP_MISSING.
 */
extern int set_remove(set_t, set_elem_t);

/*
 * Free the set.
 */
extern void set_free(set_t);

/*
 * Clear the set.
 */
extern void set_clear(set_t);

/*
 * Get the current size of a hashmap
 */
extern unsigned long set_length(set_t);

#endif //__SET_H__
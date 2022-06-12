/*
  Copyright (c) 2021 Lu Kai
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#ifndef _AVL_H
#define _AVL_H

/**
 * @file c-avl.h
 * @brief a C version avl tree implementation
 */

#if defined(_WIN32)
#define __export __declspec(dllexport)
#elif defined(__GNUC__) && ((__GNUC__ >= 4) || (__GNUC__ == 3 && __GNUC_MINOR__ >= 3))
#define __export __attribute__((visibility("default")))
#else
#define __export
#endif

#if defined(__cplusplus)
extern "C"
{
#endif

#include <stddef.h>
#include <stdint.h>

    /**
     * @brief compare function pointer
     * @param lhs left-hand side key
     * @param rhs right-hand side key
     * @return a signed integer as the compare result
     */
    typedef int (*avl_compare)(const void *lhs, const void *rhs);

    /**
     * @brief destruct function pointer
     * @param p pointer to the key or value to be destructed
     */
    typedef void (*avl_destruct)(void *p);

    /**
     * @struct avl_config
     * @brief customizable configuration
     * @par Example codes
     * @code {.c}
        void* my_malloc(size_t s);
        void my_free(void* p);
        struct avl_config _config = {
            ._alloc = my_malloc,
            ._dealloc = my_free,
            ._reserve = 8};
     * @endcode
     */
    struct avl_config
    {
        /** customize allocator*/
        void *(*_alloc)(size_t);
        /** customize deallocator*/
        void (*_dealloc)(void *);
        /** reserve elements*/
        size_t _reserve;
    };

    /**
     * @struct avl_set
     * @brief forward declaration
     */
    struct avl_set;

    /**
     * @brief create an avl_set
     * @param cmp [<b>mandatory</b>] compare function between set elements
     * @param dtor [optional] destructor for set elements, will be called during avl_set_destroy()
     * @param cfg [optional] customizable configuration
     * @return pointer of the created avl_set, NULL on error
     * @see avl_config
     * @par Example codes
     * @code
        int my_element_compare(const void *lhs, const void* rhs)
        {
            int v1 = *((const int*)lhs);
            int v2 = *((const int*)rhs);
            return (v1 < v2 ? -1 : (v1 == v2 ? 0 : 1));
        }

        void my_element_destructor(void *e)
        {
            free(e);
        }

        struct avl_set *_default_config_set = avl_set_create(my_element_compare, my_element_destructor, NULL);
     * @endcode
     */
    struct avl_set *avl_set_create(avl_compare cmp, avl_destruct dtor, const struct avl_config *cfg);

    /**
     * @brief return the number of the avl_set elements
     * @param s target avl_set
     * @return a non-negative integer
     */
    size_t avl_set_size(const struct avl_set *s);

    /**
     * @brief remove all the elements and destroy them with the ::avl_destruct
     * @param s target avl_set
     * @see avl_set_create
     */
    void avl_set_clear(struct avl_set *s);

    /**
     * @brief destroy the avl_set (and remove all its elements)
     * @param s target avl_set
     * @see avl_set_clear
     */
    void avl_set_destroy(struct avl_set *s);

    /**
     * @brief search an element in the avl_set
     * @param s target avl_set
     * @param k the "key" element to be searched
     * @return wanted element, NULL on not found
     * @note <b>DO NOT</b> modify the "key field" of the search result
     */
    void *avl_set_search(struct avl_set *s, const void *k);

    /**
     * @brief insert an element into the avl_set
     * @param s target avl_set
     * @param k the element to be inserted
     * @return 0 on success, 1 on duplicated
     * @note duplicated element will be destroyed
     */
    int avl_set_insert(struct avl_set *s, void *k);

    /**
     * @brief delete an element from the avl_set
     * @param s target avl_set
     * @param k the element to be deleted
     * @return 0 on success, -1 on not found
     */
    int avl_set_delete(struct avl_set *s, const void *k);

    struct avl_map;
    struct avl_map *avl_map_create(avl_compare cmp, avl_destruct kd, avl_destruct vd, const struct avl_config *cfg);
    size_t avl_map_size(const struct avl_map *m);
    void avl_map_clear(struct avl_map *m);
    void avl_map_destroy(struct avl_map *m);
    int avl_map_insert(struct avl_map *m, const void *k, const void *v);
    int avl_map_search(void **v, struct avl_map *m, const void *k);
    int avl_map_delete(struct avl_map *m, const void *k);

#if defined(__cplusplus)
}
#endif

#endif
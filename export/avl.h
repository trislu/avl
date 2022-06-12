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

#if defined(_WIN32)
#define __export __declspec(dllexport)
#elif defined(__GNUC__) && ((__GNUC__ >= 4) || (__GNUC__ == 3 && __GNUC_MINOR__ >= 3))
#define __export __attribute__((visibility("default")))
#else
#define __export
#endif

/*! @file avltree.h
 *  @brief An array version AVL tree implementation
 */

#include <stddef.h>
#include <stdint.h>

#if defined(__cplusplus)
extern "C"
{
#endif
    typedef int (*avl_compare)(const void *lhs, const void *rhs);
    typedef void (*avl_destruct)(void *p);
    typedef void *(*avl_malloc)(size_t size);
    typedef void (*avl_free)(void *p);

    typedef struct _avl_config
    {
        avl_malloc _malloc;
        avl_free _free;
        size_t _reserve;
    } avl_config;

    struct avl_set;
    struct avl_set *avl_set_create(avl_compare cmp, avl_destruct kdtor, const avl_config *cfg);
    size_t avl_set_size(const struct avl_set *s);
    void avl_set_clear(struct avl_set *s);
    void avl_set_destroy(struct avl_set *s);
    int avl_set_search(void **rslt, struct avl_set *s, const void *k);
    int avl_set_insert(struct avl_set *s, const void *k);
    int avl_set_delete(struct avl_set *s, const void *k);

    struct avl_map;
    struct avl_map *avl_map_create(avl_compare cmp, avl_destruct kdtor, avl_destruct vdtor, const avl_config *cfg);
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
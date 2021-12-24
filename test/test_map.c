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

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "avl.h"

int integer_compare(const void *lhs, const void* rhs)
{
    uintptr_t l = (uintptr_t)lhs;
    uintptr_t r = (uintptr_t)rhs;
    return l < r ? -1 : ( l > r ? 1 : 0);
}

int main(int argc, char **argv)
{
    struct avl_map *s = avl_map_create(integer_compare, NULL, NULL, NULL);

    const char *_names[5] = {
        "alice",
        "bob",
        "carl",
        "david",
        "eve"};

    size_t i = 0;
    for (i = 0; i < 5; i++)
    {
        assert(0 == avl_map_insert(s, (const void*)((uintptr_t)i), (_names[i])));
        printf("inserting %s\n", _names[i]);
    }

    for (i = 0; i < 5; i++)
    {
        void *_rslt = NULL;
        assert(0 == avl_map_search(&_rslt, s, (const void*)((uintptr_t)i)));
        const char *_str = (const char *)_rslt;
        assert(0 == strcmp(_str, _names[i]));
        printf("we got [%zu] %s\n", i,_str);
    }

    int ret = 0;
    printf("------------\n deleting 3\n-----------\n");
    ret = avl_map_delete(s, (const void*)((uintptr_t)3));
    assert(0 == ret);

    for (i = 0; i < 5; i++)
    {
        printf("attempt to find %zu ", i);
        void *_rslt = NULL;
        int ret = avl_map_search(&_rslt, s, (const void*)((uintptr_t)i));
        if (0 == ret) {
            const char *_str = (const char *)_rslt;
            printf("==> found [%zu] %s!\n", i, _str);
        }
        else {
            printf("==> not found [%zu]!\n", i);
        }
    }

    printf("------------\n deleting 2\n-----------\n");
    ret = avl_map_delete(s, (const void*)((uintptr_t)2));
    assert(0 == ret);

    for (i = 0; i < 5; i++)
    {
        printf("attempt to find %zu ", i);
        void *_rslt = NULL;
        int ret = avl_map_search(&_rslt, s, (const void*)((uintptr_t)i));
        if (0 == ret) {
            const char *_str = (const char *)_rslt;
            printf("==> found [%zu] %s!\n", i, _str);
        }
        else {
            printf("==> not found [%zu]!\n", i);
        }
    }

    printf("------------\n adding [2]carl back \n-----------\n");
    assert(0 == avl_map_insert(s, (const void*)((uintptr_t)2), "carl"));

    for (i = 0; i < 5; i++)
    {
        printf("attempt to find %zu ", i);
        void *_rslt = NULL;
        int ret = avl_map_search(&_rslt, s, (const void*)((uintptr_t)i));
        if (0 == ret) {
            const char *_str = (const char *)_rslt;
            printf("==> found [%zu] %s!\n", i, _str);
        }
        else {
            printf("==> not found [%zu]!\n", i);
        }
    }

    return 0;
}

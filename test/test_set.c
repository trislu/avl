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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "avl.h"

#define ASSERT_AND_ABORT(stmt) \
    do { if (!(stmt)) {abort();} } while(0)

int string_compare(const void *lhs, const void *rhs)
{
    const char *l = (const char *)lhs;
    const char *r = (const char *)rhs;
    return strcmp(l, r);
}

int main(int argc, char **argv)
{
    struct avl_set *s = avl_set_create(string_compare, NULL, NULL);

    const char *_names[5] = {
        "alice",
        "bob",
        "carl",
        "david",
        "eve"};

    size_t i = 0;
    for (i = 0; i < 5; i++)
    {
        ASSERT_AND_ABORT(0 == avl_set_insert(s, (_names[i])));
        printf("inserting %s\n", _names[i]);
    }

    for (i = 0; i < 5; i++)
    {
        void *_rslt = NULL;
        ASSERT_AND_ABORT(0 == avl_set_search(&_rslt, s, _names[i]));
        const char *_str = (const char *)_rslt;
        ASSERT_AND_ABORT(0 == strcmp(_str, _names[i]));
        printf("we got %s\n", _str);
    }

    printf("------------\n deleting carl\n-----------\n");
    ASSERT_AND_ABORT(0 == avl_set_delete(s, "carl"));

    for (i = 0; i < 5; i++)
    {
        printf("attempt to find %s ", _names[i]);
        void *_rslt = NULL;
        int ret = avl_set_search(&_rslt, s, _names[i]);
        if (0 == ret) {
            const char *_str = (const char *)_rslt;
            printf("==> found %s!\n", _str);
        }
        else {
            printf("==> not found %d!\n", ret);
        }
    }

    printf("------------\n deleting alice\n-----------\n");
    ASSERT_AND_ABORT(0 == avl_set_delete(s, "alice"));

    for (i = 0; i < 5; i++)
    {
        printf("attempt to find %s ", _names[i]);
        void *_rslt = NULL;
        int ret = avl_set_search(&_rslt, s, _names[i]);
        if (0 == ret) {
            const char *_str = (const char *)_rslt;
            printf("==> found %s!\n", _str);
        }
        else {
            printf("==> not found %d!\n", ret);
        }
    }

    printf("------------\n adding carl back \n-----------\n");
    ASSERT_AND_ABORT(0 == avl_set_insert(s, "carl"));

    for (i = 0; i < 5; i++)
    {
        printf("attempt to find %s ", _names[i]);
        void *_rslt = NULL;
        int ret = avl_set_search(&_rslt, s, _names[i]);
        if (0 == ret) {
            const char *_str = (const char *)_rslt;
            printf("==> found %s!\n", _str);
        }
        else {
            printf("==> not found %d!\n", ret);
        }
    }

    return 0;
}

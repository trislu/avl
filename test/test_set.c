#include "avl.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

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

    for (size_t i = 0; i < 5; i++)
    {
        assert(0 == avl_set_insert(s, (_names[i])));
        printf("inserting %s\n", _names[i]);
    }

    for (size_t i = 0; i < 5; i++)
    {
        void *_rslt = NULL;
        assert(0 == avl_set_search(&_rslt, s, _names[i]));
        const char *_str = (const char *)_rslt;
        assert(0 == strcmp(_str, _names[i]));
        printf("we got %s\n", _str);
    }

    int ret = 0;
    printf("------------\n deleting carl\n-----------\n");
    ret = avl_set_delete(s, "carl");
    assert(0 == ret);

    for (size_t i = 0; i < 5; i++)
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
    ret = avl_set_delete(s, "alice");
    assert(0 == ret);

    for (size_t i = 0; i < 5; i++)
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
    assert(0 == avl_set_insert(s, "carl"));

    for (size_t i = 0; i < 5; i++)
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

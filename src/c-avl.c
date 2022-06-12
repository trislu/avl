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
#include <stdlib.h>
#include <string.h>
#include "c-avl.h"

#define _AVL_MAX(a, b) ((a) > (b) ? (a) : (b))
#define _AVL_MEM_THRESHOLD (1024 * 1024)

/*! @struct avl_node */
typedef struct _avl_node
{
    /*! left child */
    uintptr_t left;
    /*! right child */
    uintptr_t right;
    /*! height */
    int height;
} avl_node;

static int _avl_height(avl_node *n)
{
    return n ? n->height : 0;
}

static void _avl_update_height(avl_node *n)
{
    if (n)
    {
        n->height = _AVL_MAX(_avl_height((avl_node *)(n->left)), _avl_height((avl_node *)(n->right))) + 1;
    }
}

static int __avl_balance_factor(avl_node *n)
{
    return n ? (_avl_height((avl_node *)(n->left)) - _avl_height((avl_node *)(n->right))) : 0;
}

static avl_node *avl_single_rotate_right(avl_node *root)
{
    avl_node *left = (avl_node *)(root->left);
    root->left = left->right;
    left->right = (uintptr_t)root;
    _avl_update_height(root);
    _avl_update_height(left);
    return left;
}

static avl_node *avl_single_rotate_left(avl_node *root)
{
    avl_node *right = (avl_node *)(root->right);
    root->right = right->left;
    right->left = (uintptr_t)root;
    _avl_update_height(root);
    _avl_update_height(right);
    return right;
}

typedef struct _avl_stack
{
    size_t size;
    size_t tail;
    size_t array[];
} avl_stack;

static int __avl_stack_pop(size_t *e, avl_stack *s)
{
    if (s->tail == 0)
    {
        return -1;
    }

    s->tail--;
    if (e)
    {
        *e = s->array[s->tail];
    }
    return 0;
}

static void __avl_stack_push(avl_stack *s, size_t e)
{
    assert(s);
    s->array[s->tail] = e;
    s->tail++;
}

static void __avl_stack_clear(avl_stack *s)
{
    assert(s);
    s->tail = 0;
}

static size_t __avl_stack_bytesize(const avl_stack *s)
{
    assert(s);
    return sizeof(avl_stack) + sizeof(size_t) * s->size;
}

/*! @struct avl_set_element */
typedef struct _avl_set_element
{
    /* data */
    avl_node node;
    uintptr_t key;
} avl_set_element;

/*! @struct avl_set */
struct avl_set
{
    /* data */
    avl_compare _compare;
    avl_destruct _key_destruct;
    avl_config _config;
    size_t _size;
    size_t _rindex;
    avl_stack *_slots;
    avl_set_element *_tree;
};

struct avl_set *avl_set_create(avl_compare cmp, avl_destruct kdtor, const avl_config *cfg)
{
    if (NULL == cmp)
    {
        return NULL;
    }

    avl_config _config = {
        ._malloc = malloc,
        ._free = free,
        ._reserve = 8};

    if (cfg)
    {
        if (cfg->_malloc && cfg->_free)
        {
            _config._malloc = cfg->_malloc;
            _config._free = cfg->_free;
        }
        if (cfg->_reserve > 0)
        {
            _config._reserve = cfg->_reserve;
        }
    }

    struct avl_set *_s = (struct avl_set *)(_config._malloc(sizeof(struct avl_set)));
    if (NULL == _s)
    {
        /*! @note panic */
        return NULL;
    }

    memset(_s, 0, sizeof(struct avl_set));
    /*! @brief initialization */
    _s->_compare = cmp;
    _s->_config = _config;
    _s->_key_destruct = kdtor;
    _s->_rindex = 0;
    _s->_size = 0;

    size_t _bytes = sizeof(avl_set_element) * _config._reserve;
    _s->_tree = (avl_set_element *)(_config._malloc(_bytes));
    memset(_s->_tree, 0, _bytes);

    /*! @note create a stack to record available slots */
    avl_stack *_stack = (avl_stack *)(_config._malloc(sizeof(avl_stack) + sizeof(size_t) * _config._reserve));
    _stack->size = _config._reserve;
    _stack->tail = 0;

    size_t i;
    for (i = _config._reserve; i != 0; i--)
    {
        __avl_stack_push(_stack, i - 1);
    }
    _s->_slots = _stack;
    return _s;
}

size_t avl_set_size(const struct avl_set *s)
{
    return s->_size;
}

void avl_set_clear(struct avl_set *s)
{
    if (s)
    {
        avl_destruct _d = s->_key_destruct;
        size_t i;
        for (i = 0; i < s->_size; i++)
        {
            void *_key = (void *)(s->_tree[i].key);
            if (_d)
                _d(_key);
        }
        memset(s->_tree, 0, sizeof(avl_set_element) * s->_config._reserve);
        s->_size = 0;
        s->_rindex = 0;

        /*! @note maintain available slots */
        __avl_stack_clear(s->_slots);
    }
}

void avl_set_destroy(struct avl_set *s)
{
    avl_set_clear(s);
    if (s)
    {
        /*! free tree array */
        avl_free _f = s->_config._free;
        _f(s->_tree);
        s->_tree = NULL;
        /*! free available slots */
        _f(s->_slots);
        s->_slots = NULL;
        memset(s, 0, sizeof(struct avl_set));
        _f(s);
    }
}

static void __avl_set_reserve_one(struct avl_set *s)
{
    /*! ensure enough size */
    if (s->_size < s->_config._reserve)
    {
        /*! @note there is still enough room for one element */
        return;
    }

    size_t new_rsv_size = s->_size + 1;
    if (new_rsv_size < _AVL_MEM_THRESHOLD)
    {
        new_rsv_size *= 2;
    }
    else
    {
        new_rsv_size += _AVL_MEM_THRESHOLD;
    }

    /*! manually reallocate : allocate new tree */
    size_t _new_bytes = sizeof(avl_set_element) * new_rsv_size;
    avl_set_element *ntree = (avl_set_element *)(s->_config._malloc(_new_bytes));

    /*! manually reallocate : copy from old tree */
    size_t _old_bytes = sizeof(avl_set_element) * s->_config._reserve;
    memcpy(ntree, s->_tree, _old_bytes);
    uint8_t *_rest = (uint8_t *)ntree + _old_bytes;
    memset(_rest, 0, _new_bytes - _old_bytes);

    /*! clean up old tree*/
    memset(s->_tree, 0, _old_bytes);
    s->_config._free(s->_tree);

    /*! manually reallocate : allocate new slots */
    size_t _slot_size = sizeof(avl_stack) + sizeof(size_t) * new_rsv_size;
    avl_stack *nslots = (avl_stack *)(s->_config._malloc(_slot_size));
    memset(nslots, 0, _slot_size);
    /*! set new slots size*/
    nslots->size = new_rsv_size;

    /*! manually reallocate : copy available old slots*/
    size_t _old_slot_size = __avl_stack_bytesize(s->_slots);
    size_t i;
    for (i = 0; i < s->_slots->tail; i++)
    {
        __avl_stack_push(nslots, s->_slots->array[i]);
    }
    /*! newly allocated slots are also available */
    size_t j;
    for (j = s->_slots->size; j < new_rsv_size; j++)
    {
        /*! @note this loop may take quite a while */
        __avl_stack_push(nslots, s->_slots->array[j]);
    }

    /*! clean up old slots*/
    memset(s->_slots, 0, _old_slot_size);
    s->_config._free(s->_slots);

    /*! the new setup */
    s->_tree = ntree;
    s->_slots = nslots;
    s->_config._reserve = new_rsv_size;
}

static avl_set_element *__avl_set_search(struct avl_set *s, avl_set_element *e, const void *k)
{
    assert(e);
    int cmpret = s->_compare(k, (const void *)(e->key));
    if (0 == cmpret)
    {
        /*! @brief found */
        return e;
    }
    else if (0 > cmpret)
    {
        avl_node *lchild = (avl_node *)(e->node.left);
        if (lchild)
        {
            avl_set_element *left = (avl_set_element *)lchild;
            return __avl_set_search(s, left, k);
        }
    }
    else
    {
        avl_node *rchild = (avl_node *)(e->node.right);
        if (rchild)
        {
            avl_set_element *right = (avl_set_element *)rchild;
            return __avl_set_search(s, right, k);
        }
    }
    /*! @brief left or right is NULL, not found */
    return NULL;
}

int avl_set_search(void **rslt, struct avl_set *s, const void *k)
{
    assert(s);
    if (0 == s->_size)
    {
        /*! @brief empty set */
        return -1;
    }
    avl_set_element *root = &(s->_tree[s->_rindex]);
    avl_set_element *ret = __avl_set_search(s, root, k);
    if (NULL == ret)
    {
        /*! @brief not found */
        return -1;
    }
    if (rslt)
    {
        *rslt = (void *)(ret->key);
    }
    return 0;
}

static avl_set_element *__avl_set_insert(struct avl_set *s, avl_set_element *e, const void *k)
{
    if (NULL == e)
    {
        /*! @note on edge */
        size_t empty_slot = 0;
        if (0 != __avl_stack_pop(&empty_slot, s->_slots))
        {
            assert(0);
            return NULL;
        }
        avl_set_element *ret = &(s->_tree[empty_slot]);
        ret->node.left = (uintptr_t)NULL;
        ret->node.right = (uintptr_t)NULL;
        ret->node.height = 1;
        ret->key = (uintptr_t)k;
        s->_size++;
        return ret;
    }
    int cmpret = s->_compare(k, (const void *)(e->key));
    if (0 == cmpret)
    {
        /*! @note duplication insert, do nothing */
        return e;
    }
    else if (0 > cmpret)
    {
        avl_node *lchild = (avl_node *)(e->node.left);
        if (lchild)
        {
            avl_set_element *left = (avl_set_element *)lchild;
            avl_set_element *newleft = __avl_set_insert(s, left, k);
            e->node.left = (uintptr_t)(&(newleft->node));
        }
        else
        {
            avl_set_element *newleft = __avl_set_insert(s, NULL, k);
            e->node.left = (uintptr_t)(&(newleft->node));
        }
    }
    else
    {
        avl_node *rchild = (avl_node *)(e->node.right);
        if (rchild)
        {
            avl_set_element *right = (avl_set_element *)rchild;
            avl_set_element *newright = __avl_set_insert(s, right, k);
            e->node.right = (uintptr_t)(&(newright->node));
        }
        else
        {
            avl_set_element *newright = __avl_set_insert(s, NULL, k);
            e->node.right = (uintptr_t)(&(newright->node));
        }
    }
    /*! @note do some AVL stuff */
    avl_node *self = &e->node;
    _avl_update_height(self);
    int balance_factor = __avl_balance_factor(self);
    if (balance_factor > 1)
    {
        avl_node *left = (avl_node *)(e->node.left);
        int lbf = __avl_balance_factor(left);
        if (lbf > 0)
        {
            avl_node *_new_root = avl_single_rotate_right(self);
            avl_set_element *root_elem = (avl_set_element *)_new_root;
            return root_elem;
        }
        else if (lbf < 0)
        {
            avl_node *_new_left = avl_single_rotate_left(left);
            self->left = (uintptr_t)_new_left;
            avl_node *_new_root = avl_single_rotate_right(self);
            avl_set_element *root_elem = (avl_set_element *)_new_root;
            return root_elem;
        }
    }
    else if (balance_factor < -1)
    {
        avl_node *right = (avl_node *)(e->node.right);
        int rbf = __avl_balance_factor(right);
        if (rbf < 0)
        {
            avl_node *_new_root = avl_single_rotate_left(self);
            avl_set_element *root_elem = (avl_set_element *)_new_root;
            return root_elem;
        }
        else if (rbf > 0)
        {
            avl_node *_new_right = avl_single_rotate_right(right);
            self->right = (uintptr_t)_new_right;
            avl_node *_new_root = avl_single_rotate_left(self);
            avl_set_element *root_elem = (avl_set_element *)_new_root;
            return root_elem;
        }
    }
    return e;
}

int avl_set_insert(struct avl_set *s, const void *k)
{
    assert(s);
    /*! empty set */
    if (0 == s->_size)
    {
        avl_set_element *nroot = __avl_set_insert(s, NULL, k);
        s->_rindex = (uint32_t)(nroot - s->_tree);
        return 0;
    }
    /*! check reserve */
    __avl_set_reserve_one(s);
    /*! record current size */
    size_t cur_size = s->_size;
    /*! current root */
    avl_set_element *relem = &(s->_tree[s->_rindex]);
    /*! perform insertion */
    avl_set_element *nroot = __avl_set_insert(s, relem, k);
    /*! update root index */
    s->_rindex = (uint32_t)(nroot - s->_tree);
    /*! success on size increasing, no change means failure*/
    return s->_size > cur_size ? 0 : -1;
}

static avl_set_element *__avl_set_delete(struct avl_set *s, avl_set_element *e, const void *k, int replace)
{
    if (NULL == e)
    {
        return NULL;
    }
    /*! @brief record on this frame */
    avl_set_element *self = e;
    int cmpret = s->_compare(k, (const void *)(self->key));
    if (0 > cmpret)
    {
        /*! @note deletion is performed on left-tree, may need a new left child */
        avl_node *lchild = (avl_node *)(self->node.left);
        avl_set_element *left = (avl_set_element *)lchild;
        avl_set_element *_new_left = __avl_set_delete(s, left, k, replace);
        self->node.left = (uintptr_t)_new_left;
        /*! @note self balance check */
        if (__avl_balance_factor(&(self->node)) < -1)
        {
            avl_node *right = (avl_node *)(self->node.right);
            int rbf = __avl_balance_factor(right);
            if (rbf < 0)
            {
                avl_node *_new_root = avl_single_rotate_left(&(self->node));
                self = (avl_set_element *)_new_root;
            }
            else if (rbf > 0)
            {
                avl_node *_new_right = avl_single_rotate_right(right);
                self->node.right = (uintptr_t)_new_right;
                avl_node *_new_root = avl_single_rotate_left(&(self->node));
                self = (avl_set_element *)_new_root;
            }
        }
    }
    else if (0 < cmpret)
    {
        /*! @note deletion is performed on right-tree, may need a new right child */
        avl_node *rchild = (avl_node *)(self->node.right);
        avl_set_element *right = (avl_set_element *)rchild;
        avl_set_element *_new_right = __avl_set_delete(s, right, k, replace);
        self->node.right = (uintptr_t)_new_right;
        /*! @note self balance check */
        if (__avl_balance_factor(&(self->node)) > 1)
        {
            avl_node *left = (avl_node *)(self->node.left);
            int lbf = __avl_balance_factor(left);
            if (lbf > 0)
            {
                avl_node *_new_root = avl_single_rotate_right(&(self->node));
                self = (avl_set_element *)_new_root;
            }
            else if (lbf < 0)
            {
                avl_node *_new_left = avl_single_rotate_left(left);
                self->node.left = (uintptr_t)_new_left;
                avl_node *_new_root = avl_single_rotate_right(&(self->node));
                self = (avl_set_element *)_new_root;
            }
        }
    }
    else
    {
        /*! @note target found, record it */
        avl_set_element _record = *self;
        /*! check target status */
        avl_node *left = (avl_node *)(self->node.left);
        avl_node *right = (avl_node *)(self->node.right);
        if (left && right)
        {
            /*! @note target has left and right children */
            int bf = __avl_balance_factor(&(self->node));
            if (0 > bf)
            {
                /* right tree is higher, find the smallest element of the right tree */
                avl_node *_smallest = right;
                while (1)
                {
                    avl_node *_smaller = (avl_node *)(_smallest->left);
                    if (NULL == _smaller)
                    {
                        break;
                    }
                    _smallest = _smaller;
                }
                avl_set_element *_victim = (avl_set_element *)_smallest;
                /*! @note save the key of the victim */
                self->key = _victim->key;
                /*! @note perform deletion on right tree */
                avl_set_element *_new_right = __avl_set_delete(s, (avl_set_element *)right, (const void *)(_victim->key), 1);
                /*! @note update new right child */
                self->node.right = (uintptr_t)(&(_new_right->node));
            }
            else
            {
                /* left tree is higher (or equal), find the largest element of the left tree */
                avl_node *_largest = left;
                while (1)
                {
                    avl_node *_larger = (avl_node *)(_largest->right);
                    if (NULL == _larger)
                    {
                        break;
                    }
                    _largest = _larger;
                }
                avl_set_element *_victim = (avl_set_element *)_largest;
                /*! @note save the key of the victim */
                self->key = _victim->key;
                /*! @note perform deletion on left tree */
                avl_set_element *_new_left = __avl_set_delete(s, (avl_set_element *)left, (const void *)(_victim->key), 1);
                /*! @note update new left child */
                self->node.left = (uintptr_t)(&(_new_left->node));
            }
        }
        else
        {
            /*! @note target slot can be recycled */
            memset(self, 0, sizeof(avl_set_element));
            size_t _slotid = self - s->_tree;
            __avl_stack_push(s->_slots, _slotid);
            if ((NULL == left) && (NULL == right))
            {
                /*! @note target is a leaf */
                self = NULL;
            }
            else if (NULL == right)
            {
                /*! @note target only has left child */
                self = (avl_set_element *)left;
            }
            else if (NULL == left)
            {
                /*! @note target only has right child */
                self = (avl_set_element *)right;
            }
        }
        /*! finally, cleanup job */
        if (!replace)
        {
            /*! @note target is not replace, destruct key if needed */
            if (s->_key_destruct)
                s->_key_destruct((void *)(_record.key));
            /*! update size */
            s->_size--;
        }
    }
    /*! @note update height */
    _avl_update_height(&(self->node));
    return self;
}

int avl_set_delete(struct avl_set *s, const void *k)
{
    assert(s);
    if (0 == s->_size)
    {
        /*! @brief empty set */
        return -1;
    }
    size_t _rec_size = s->_size;
    avl_set_element *root = &(s->_tree[s->_rindex]);
    root = __avl_set_delete(s, root, k, 0);
    /*! @note if the element is deleted */
    if (_rec_size == s->_size)
    {
        /*! @note target not found */
        return -1;
    }
    s->_rindex = (root - s->_tree);
    return 0;
}

/*! @struct avl_map_element */
typedef struct _avl_map_element
{
    /* data */
    avl_node node;
    uintptr_t key;
    uintptr_t val;
} avl_map_element;

/*! @struct avl_map */
struct avl_map
{
    /* data */
    avl_compare _compare;
    avl_destruct _key_destruct;
    avl_destruct _val_destruct;
    avl_config _config;
    size_t _size;
    size_t _rindex;
    avl_stack *_slots;
    avl_map_element *_tree;
};

struct avl_map *avl_map_create(avl_compare cmp, avl_destruct kdtor, avl_destruct vdtor, const avl_config *cfg)
{
    if (NULL == cmp)
    {
        return NULL;
    }

    avl_config _config = {
        ._malloc = malloc,
        ._free = free,
        ._reserve = 8};

    if (cfg)
    {
        if (cfg->_malloc && cfg->_free)
        {
            _config._malloc = cfg->_malloc;
            _config._free = cfg->_free;
        }
        if (cfg->_reserve > 0)
        {
            _config._reserve = cfg->_reserve;
        }
    }

    struct avl_map *_m = (struct avl_map *)(_config._malloc(sizeof(struct avl_map)));
    if (NULL == _m)
    {
        /*! @note panic */
        return NULL;
    }

    memset(_m, 0, sizeof(struct avl_map));
    /*! @brief initialization */
    _m->_compare = cmp;
    _m->_config = _config;
    _m->_key_destruct = kdtor;
    _m->_val_destruct = vdtor;
    _m->_rindex = 0;
    _m->_size = 0;

    size_t _bytes = sizeof(avl_map_element) * _config._reserve;
    _m->_tree = (avl_map_element *)(_config._malloc(_bytes));
    memset(_m->_tree, 0, _bytes);

    /*! @note create a stack to record available slots */
    avl_stack *_stack = (avl_stack *)(_config._malloc(sizeof(avl_stack) + sizeof(size_t) * _config._reserve));
    _stack->size = _config._reserve;
    _stack->tail = 0;
    size_t i;
    for (i = _config._reserve; i != 0; i--)
    {
        __avl_stack_push(_stack, i - 1);
    }
    _m->_slots = _stack;
    return _m;
}

size_t avl_map_size(const struct avl_map *m)
{
    return m->_size;
}

void avl_map_clear(struct avl_map *m)
{
    if (m)
    {
        avl_destruct _kd = m->_key_destruct;
        avl_destruct _vd = m->_val_destruct;
        size_t i;
        for (i = 0; i < m->_size; i++)
        {
            void *_key = (void *)(m->_tree[i].key);
            if (_kd)
                _kd(_key);
            void *_val = (void *)(m->_tree[i].val);
            if (_vd)
                _vd(_val);
        }
        memset(m->_tree, 0, sizeof(avl_map_element) * m->_config._reserve);
        m->_size = 0;
        m->_rindex = 0;

        /*! @note maintain available slots */
        __avl_stack_clear(m->_slots);
    }
}

void avl_map_destroy(struct avl_map *m)
{
    avl_map_clear(m);
    if (m)
    {
        /*! free tree array */
        avl_free _f = m->_config._free;
        _f(m->_tree);
        m->_tree = NULL;
        /*! free available slots */
        _f(m->_slots);
        m->_slots = NULL;
        memset(m, 0, sizeof(struct avl_map));
        _f(m);
    }
}

static void __avl_map_reserve_one(struct avl_map *m)
{
    /*! ensure enough size */
    if (m->_size < m->_config._reserve)
    {
        /*! @note there is still enough room for one element */
        return;
    }

    size_t new_rsv_size = m->_size + 1;
    if (new_rsv_size < _AVL_MEM_THRESHOLD)
    {
        new_rsv_size *= 2;
    }
    else
    {
        new_rsv_size += _AVL_MEM_THRESHOLD;
    }

    /*! manually reallocate : allocate new tree */
    size_t _new_bytes = sizeof(avl_map_element) * new_rsv_size;
    avl_map_element *ntree = (avl_map_element *)(m->_config._malloc(_new_bytes));

    /*! manually reallocate : copy from old tree */
    size_t _old_bytes = sizeof(avl_map_element) * m->_config._reserve;
    memcpy(ntree, m->_tree, _old_bytes);
    uint8_t *_rest = (uint8_t *)ntree + _old_bytes;
    memset(_rest, 0, _new_bytes - _old_bytes);

    /*! clean up old tree*/
    memset(m->_tree, 0, _old_bytes);
    m->_config._free(m->_tree);

    /*! manually reallocate : allocate new slots */
    size_t _slot_size = sizeof(avl_stack) + sizeof(size_t) * new_rsv_size;
    avl_stack *nslots = (avl_stack *)(m->_config._malloc(_slot_size));
    memset(nslots, 0, _slot_size);
    /*! set new slots size*/
    nslots->size = new_rsv_size;

    /*! manually reallocate : copy available old slots*/
    size_t _old_slot_size = __avl_stack_bytesize(m->_slots);
    size_t i;
    for (i = 0; i < m->_slots->tail; i++)
    {
        __avl_stack_push(nslots, m->_slots->array[i]);
    }
    /*! newly allocated slots are also available */
    size_t j;
    for (j = m->_slots->size; j < new_rsv_size; j++)
    {
        /*! @note this loop may take quite a while */
        __avl_stack_push(nslots, m->_slots->array[j]);
    }

    /*! clean up old slots*/
    memset(m->_slots, 0, _old_slot_size);
    m->_config._free(m->_slots);

    /*! the new setup */
    m->_tree = ntree;
    m->_slots = nslots;
    m->_config._reserve = new_rsv_size;
}

static avl_map_element *__avl_map_search(struct avl_map *s, avl_map_element *e, const void *k)
{
    assert(e);
    int cmpret = s->_compare(k, (const void *)(e->key));
    if (0 == cmpret)
    {
        /*! @brief found */
        return e;
    }
    else if (0 > cmpret)
    {
        avl_node *lchild = (avl_node *)(e->node.left);
        if (lchild)
        {
            avl_map_element *left = (avl_map_element *)lchild;
            return __avl_map_search(s, left, k);
        }
    }
    else
    {
        avl_node *rchild = (avl_node *)(e->node.right);
        if (rchild)
        {
            avl_map_element *right = (avl_map_element *)rchild;
            return __avl_map_search(s, right, k);
        }
    }
    /*! @brief left or right is NULL, not found */
    return NULL;
}

int avl_map_search(void **val, struct avl_map *s, const void *k)
{
    assert(s);
    if (0 == s->_size)
    {
        /*! @brief empty set */
        return -1;
    }
    avl_map_element *root = &(s->_tree[s->_rindex]);
    avl_map_element *ret = __avl_map_search(s, root, k);
    if (NULL == ret)
    {
        /*! @brief not found */
        return -1;
    }
    if (val)
    {
        *val = (void *)(ret->val);
    }
    return 0;
}

static avl_map_element *__avl_map_insert(struct avl_map *s, avl_map_element *e, const void *k, const void *v)
{
    if (NULL == e)
    {
        /*! @note on edge */
        size_t empty_slot = 0;
        if (0 != __avl_stack_pop(&empty_slot, s->_slots))
        {
            assert(0);
            return NULL;
        }
        avl_map_element *ret = &(s->_tree[empty_slot]);
        ret->node.left = (uintptr_t)NULL;
        ret->node.right = (uintptr_t)NULL;
        ret->node.height = 1;
        ret->key = (uintptr_t)k;
        ret->val = (uintptr_t)v;
        s->_size++;
        return ret;
    }
    int cmpret = s->_compare(k, (const void *)(e->key));
    if (0 == cmpret)
    {
        /*! @note duplication insert, do nothing */
        return e;
    }
    else if (0 > cmpret)
    {
        avl_node *lchild = (avl_node *)(e->node.left);
        if (lchild)
        {
            avl_map_element *left = (avl_map_element *)lchild;
            avl_map_element *newleft = __avl_map_insert(s, left, k, v);
            e->node.left = (uintptr_t)(&(newleft->node));
        }
        else
        {
            avl_map_element *newleft = __avl_map_insert(s, NULL, k, v);
            e->node.left = (uintptr_t)(&(newleft->node));
        }
    }
    else
    {
        avl_node *rchild = (avl_node *)(e->node.right);
        if (rchild)
        {
            avl_map_element *right = (avl_map_element *)rchild;
            avl_map_element *newright = __avl_map_insert(s, right, k, v);
            e->node.right = (uintptr_t)(&(newright->node));
        }
        else
        {
            avl_map_element *newright = __avl_map_insert(s, NULL, k, v);
            e->node.right = (uintptr_t)(&(newright->node));
        }
    }
    /*! @note do some AVL stuff */
    avl_node *self = &e->node;
    _avl_update_height(self);
    int balance_factor = __avl_balance_factor(self);
    if (balance_factor > 1)
    {
        avl_node *left = (avl_node *)(e->node.left);
        int lbf = __avl_balance_factor(left);
        if (lbf > 0)
        {
            avl_node *_new_root = avl_single_rotate_right(self);
            avl_map_element *root_elem = (avl_map_element *)_new_root;
            return root_elem;
        }
        else if (lbf < 0)
        {
            avl_node *_new_left = avl_single_rotate_left(left);
            self->left = (uintptr_t)_new_left;
            avl_node *_new_root = avl_single_rotate_right(self);
            avl_map_element *root_elem = (avl_map_element *)_new_root;
            return root_elem;
        }
    }
    else if (balance_factor < -1)
    {
        avl_node *right = (avl_node *)(e->node.right);
        int rbf = __avl_balance_factor(right);
        if (rbf < 0)
        {
            avl_node *_new_root = avl_single_rotate_left(self);
            avl_map_element *root_elem = (avl_map_element *)_new_root;
            return root_elem;
        }
        else if (rbf > 0)
        {
            avl_node *_new_right = avl_single_rotate_right(right);
            self->right = (uintptr_t)_new_right;
            avl_node *_new_root = avl_single_rotate_left(self);
            avl_map_element *root_elem = (avl_map_element *)_new_root;
            return root_elem;
        }
    }
    return e;
}

int avl_map_insert(struct avl_map *s, const void *k, const void *v)
{
    assert(s);
    /*! empty set */
    if (0 == s->_size)
    {
        avl_map_element *nroot = __avl_map_insert(s, NULL, k, v);
        s->_rindex = (nroot - s->_tree);
        return 0;
    }
    /*! check reserve */
    __avl_map_reserve_one(s);
    /*! record current size */
    size_t cur_size = s->_size;
    /*! current root */
    avl_map_element *relem = &(s->_tree[s->_rindex]);
    /*! perform insertion */
    avl_map_element *nroot = __avl_map_insert(s, relem, k, v);
    /*! update root index */
    s->_rindex = (nroot - s->_tree);
    /*! success on size increasing, no change means failure*/
    return s->_size > cur_size ? 0 : -1;
}

static avl_map_element *__avl_map_delete(struct avl_map *s, avl_map_element *e, const void *k, int replace)
{
    if (NULL == e)
    {
        return NULL;
    }
    /*! @brief record on this frame */
    avl_map_element *self = e;
    int cmpret = s->_compare(k, (const void *)(self->key));
    if (0 > cmpret)
    {
        /*! @note deletion is performed on left-tree, may need a new left child */
        avl_node *lchild = (avl_node *)(self->node.left);
        avl_map_element *left = (avl_map_element *)lchild;
        avl_map_element *_new_left = __avl_map_delete(s, left, k, replace);
        self->node.left = (uintptr_t)_new_left;
        /*! @note self balance check */
        if (__avl_balance_factor(&(self->node)) < -1)
        {
            avl_node *right = (avl_node *)(self->node.right);
            int rbf = __avl_balance_factor(right);
            if (rbf < 0)
            {
                avl_node *_new_root = avl_single_rotate_left(&(self->node));
                self = (avl_map_element *)_new_root;
            }
            else if (rbf > 0)
            {
                avl_node *_new_right = avl_single_rotate_right(right);
                self->node.right = (uintptr_t)_new_right;
                avl_node *_new_root = avl_single_rotate_left(&(self->node));
                self = (avl_map_element *)_new_root;
            }
        }
    }
    else if (0 < cmpret)
    {
        /*! @note deletion is performed on right-tree, may need a new right child */
        avl_node *rchild = (avl_node *)(self->node.right);
        avl_map_element *right = (avl_map_element *)rchild;
        avl_map_element *_new_right = __avl_map_delete(s, right, k, replace);
        self->node.right = (uintptr_t)_new_right;
        /*! @note self balance check */
        if (__avl_balance_factor(&(self->node)) > 1)
        {
            avl_node *left = (avl_node *)(self->node.left);
            int lbf = __avl_balance_factor(left);
            if (lbf > 0)
            {
                avl_node *_new_root = avl_single_rotate_right(&(self->node));
                self = (avl_map_element *)_new_root;
            }
            else if (lbf < 0)
            {
                avl_node *_new_left = avl_single_rotate_left(left);
                self->node.left = (uintptr_t)_new_left;
                avl_node *_new_root = avl_single_rotate_right(&(self->node));
                self = (avl_map_element *)_new_root;
            }
        }
    }
    else
    {
        /*! @note target found, record it */
        avl_map_element _record = *self;
        /*! check target status */
        avl_node *left = (avl_node *)(self->node.left);
        avl_node *right = (avl_node *)(self->node.right);
        if (left && right)
        {
            /*! @note target has left and right children */
            int bf = __avl_balance_factor(&(self->node));
            if (0 > bf)
            {
                /* right tree is higher, find the smallest element of the right tree */
                avl_node *_smallest = right;
                while (1)
                {
                    avl_node *_smaller = (avl_node *)(_smallest->left);
                    if (NULL == _smaller)
                    {
                        break;
                    }
                    _smallest = _smaller;
                }
                avl_map_element *_victim = (avl_map_element *)_smallest;
                /*! @note switch content with the victim */
                self->key = _victim->key;
                self->val = _victim->val;
                /*! @note perform deletion on right tree */
                avl_map_element *_new_right = __avl_map_delete(s, (avl_map_element *)right, (const void *)(_victim->key), 1);
                /*! @note update new right child */
                self->node.right = (uintptr_t)(&(_new_right->node));
            }
            else
            {
                /* left tree is higher (or equal), find the largest element of the left tree */
                avl_node *_largest = left;
                while (1)
                {
                    avl_node *_larger = (avl_node *)(_largest->right);
                    if (NULL == _larger)
                    {
                        break;
                    }
                    _largest = _larger;
                }
                avl_map_element *_victim = (avl_map_element *)_largest;
                /*! @note switch content with the victim */
                self->key = _victim->key;
                self->val = _victim->val;
                /*! @note perform deletion on left tree */
                avl_map_element *_new_left = __avl_map_delete(s, (avl_map_element *)left, (const void *)(_victim->key), 1);
                /*! @note update new left child */
                self->node.left = (uintptr_t)(&(_new_left->node));
            }
        }
        else
        {
            /*! @note target slot can be recycled */
            memset(self, 0, sizeof(avl_map_element));
            size_t _slotid = self - s->_tree;
            __avl_stack_push(s->_slots, _slotid);
            if ((NULL == left) && (NULL == right))
            {
                /*! @note target is a leaf */
                self = NULL;
            }
            else if (NULL == right)
            {
                /*! @note target only has left child */
                self = (avl_map_element *)left;
            }
            else if (NULL == left)
            {
                /*! @note target only has right child */
                self = (avl_map_element *)right;
            }
        }
        /*! finally, cleanup job */
        if (!replace)
        {
            /*! @note target is not replace, destruct key if needed */
            if (s->_key_destruct)
                s->_key_destruct((void *)(_record.key));
            if (s->_val_destruct)
                s->_val_destruct((void *)(_record.val));
            /*! update size */
            s->_size--;
        }
    }
    /*! @note update height */
    _avl_update_height(&(self->node));
    return self;
}

int avl_map_delete(struct avl_map *s, const void *k)
{
    assert(s);
    if (0 == s->_size)
    {
        /*! @brief empty set */
        return -1;
    }
    size_t _rec_size = s->_size;
    avl_map_element *root = &(s->_tree[s->_rindex]);
    root = __avl_map_delete(s, root, k, 0);
    /*! @note if the element is deleted */
    if (_rec_size == s->_size)
    {
        /*! @note target not found */
        return -1;
    }
    s->_rindex = (root - s->_tree);
    return 0;
}

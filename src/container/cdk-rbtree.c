/** Copyright (c), Wu Jin <wujin.developer@gmail.com>
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to
 *  deal in the Software without restriction, including without limitation the
 *  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 *  sell copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *  IN THE SOFTWARE.
 */

#include "cdk/cdk-types.h"
#include <stdlib.h>
#include <string.h>

#define	RB_RED		0
#define	RB_BLACK	1

static inline int cdk_rbtree_builtin_cmp_i8(int8_t k1, int8_t k2) {
	
	if (k1 < k2) {
		return -1;
	}
	if (k1 > k2) {
		return 1;
	}
	return 0;
}

static inline int cdk_rbtree_builtin_cmp_i16(int16_t k1, int16_t k2) {
	
	if (k1 < k2) {
		return -1;
	}
	if (k1 > k2) {
		return 1;
	}
	return 0;
}

static inline int cdk_rbtree_builtin_cmp_i32(int32_t k1, int32_t k2) {
	
	if (k1 < k2) {
		return -1;
	}
	if (k1 > k2) {
		return 1;
	}
	return 0;
}

static inline int cdk_rbtree_builtin_cmp_i64(int64_t k1, int64_t k2) {
	
	if (k1 < k2) {
		return -1;
	}
	if (k1 > k2) {
		return 1;
	}
	return 0;
}

static inline int cdk_rbtree_builtin_cmp_u8(uint8_t k1, uint8_t k2) {
	
	if (k1 < k2) {
		return -1;
	}
	if (k1 > k2) {
		return 1;
	}
	return 0;
}

static inline int cdk_rbtree_builtin_cmp_u16(uint16_t k1, uint16_t k2) {
	
	if (k1 < k2) {
		return -1;
	}
	if (k1 > k2) {
		return 1;
	}
	return 0;
}

static inline int cdk_rbtree_builtin_cmp_u32(uint32_t k1, uint32_t k2) {
	
	if (k1 < k2) {
		return -1;
	}
	if (k1 > k2) {
		return 1;
	}
	return 0;
}

static inline int cdk_rbtree_builtin_cmp_u64(uint64_t k1, uint64_t k2) {

	if (k1 < k2) {
		return -1;
	}
	if (k1 > k2) {
		return 1;
	}
	return 0;
}

static inline int cdk_rbtree_builtin_cmp_str(char* k1, char* k2) {

	return strcmp(k1, k2);
}

static inline int cdk_rbtree_builtin_cmp(cdk_rbtree_node_key_t k1, cdk_rbtree_node_key_t k2, cdk_rbtree_node_keytype_t type) {

	int r;
	switch (type)
	{
	case RB_KEYTYPE_INT8:
		r = cdk_rbtree_builtin_cmp_i8(k1.i8, k2.i8);
		break;
	case RB_KEYTYPE_INT16:
		r = cdk_rbtree_builtin_cmp_i16(k1.i16, k2.i16);
		break;
	case RB_KEYTYPE_INT32:
		r = cdk_rbtree_builtin_cmp_i32(k1.i32, k2.i32);
		break;
	case RB_KEYTYPE_INT64:
		r = cdk_rbtree_builtin_cmp_i64(k1.i64, k2.i64);
		break;
	case RB_KEYTYPE_UINT8:
		r = cdk_rbtree_builtin_cmp_u8(k1.u8, k2.u8);
		break;
	case RB_KEYTYPE_UINT16:
		r = cdk_rbtree_builtin_cmp_u16(k1.u16, k2.u16);
		break;
	case RB_KEYTYPE_UINT32:
		r = cdk_rbtree_builtin_cmp_u32(k1.u32, k2.u32);
		break;
	case RB_KEYTYPE_UINT64:
		r = cdk_rbtree_builtin_cmp_u64(k1.u64, k2.u64);
		break;
	case RB_KEYTYPE_STR:
		r = cdk_rbtree_builtin_cmp_str(k1.str, k2.str);
		break;
	default:
		abort();
		break;
	}
	return r;
}

static inline void cdk_rbtree_rotate_left(cdk_rbtree_node_t* node, cdk_rbtree_t* tree)
{
	cdk_rbtree_node_t* right = node->rb_right;

	if ((node->rb_right = right->rb_left)) {

		right->rb_left->rb_parent = node;
	}
	right->rb_left = node;

	if ((right->rb_parent = node->rb_parent)) {

		if (node == node->rb_parent->rb_left) {

			node->rb_parent->rb_left = right;
		}
		else {
			node->rb_parent->rb_right = right;
		}
	}
	else {
		tree->rb_root = right;
	}
	node->rb_parent = right;
}

static inline void cdk_rbtree_rotate_right(cdk_rbtree_node_t* node, cdk_rbtree_t* tree)
{
	cdk_rbtree_node_t* left = node->rb_left;

	if ((node->rb_left = left->rb_right)) {

		left->rb_right->rb_parent = node;
	}
	left->rb_right = node;

	if ((left->rb_parent = node->rb_parent)) {

		if (node == node->rb_parent->rb_right) {

			node->rb_parent->rb_right = left;
		}
		else {
			node->rb_parent->rb_left = left;
		}
	}
	else {
		tree->rb_root = left;
	}
	node->rb_parent = left;
}

void cdk_rbtree_init(cdk_rbtree_t* tree, cdk_rbtree_node_keytype_t type) {

	tree->rb_root = NULL;
	tree->rb_keytype = type;
}

static inline void cdk_rbtree_link_node(cdk_rbtree_node_t* node, cdk_rbtree_node_t* parent, cdk_rbtree_node_t** rb_link)
{
	node->rb_parent = parent;
	node->rb_color = RB_RED;
	node->rb_left = node->rb_right = NULL;
	*rb_link = node;
}

static inline void cdk_rbtree_insert_color(cdk_rbtree_t* tree, cdk_rbtree_node_t* node)
{
	cdk_rbtree_node_t* parent, * gparent;

	while ((parent = node->rb_parent) && parent->rb_color == RB_RED)
	{
		gparent = parent->rb_parent;

		if (parent == gparent->rb_left)
		{
			{
				register cdk_rbtree_node_t* uncle = gparent->rb_right;
				if (uncle && uncle->rb_color == RB_RED)
				{
					uncle->rb_color   = RB_BLACK;
					parent->rb_color  = RB_BLACK;
					gparent->rb_color = RB_RED;
					node = gparent;
					continue;
				}
			}
			if (parent->rb_right == node)
			{
				register cdk_rbtree_node_t* tmp;
				cdk_rbtree_rotate_left(parent, tree);
				tmp    = parent;
				parent = node;
				node   = tmp;
			}
			parent->rb_color  = RB_BLACK;
			gparent->rb_color = RB_RED;
			cdk_rbtree_rotate_right(gparent, tree);
		}
		else {
			{
				register cdk_rbtree_node_t* uncle = gparent->rb_left;
				if (uncle && uncle->rb_color == RB_RED)
				{
					uncle->rb_color   = RB_BLACK;
					parent->rb_color  = RB_BLACK;
					gparent->rb_color = RB_RED;
					node = gparent;
					continue;
				}
			}
			if (parent->rb_left == node)
			{
				register cdk_rbtree_node_t* tmp;
				cdk_rbtree_rotate_right(parent, tree);
				tmp    = parent;
				parent = node;
				node   = tmp;
			}
			parent->rb_color  = RB_BLACK;
			gparent->rb_color = RB_RED;
			cdk_rbtree_rotate_left(gparent, tree);
		}
	}
	tree->rb_root->rb_color = RB_BLACK;
}

static inline void cdk_rbtree_erase_color(cdk_rbtree_node_t* node, cdk_rbtree_node_t* parent, cdk_rbtree_t* tree)
{
	cdk_rbtree_node_t* other;

	while ((!node || node->rb_color == RB_BLACK) && node != tree->rb_root)
	{
		if (parent->rb_left == node)
		{
			other = parent->rb_right;
			if (other->rb_color == RB_RED)
			{
				other->rb_color  = RB_BLACK;
				parent->rb_color = RB_RED;
				cdk_rbtree_rotate_left(parent, tree);
				other = parent->rb_right;
			}
			if ((!other->rb_left || other->rb_left->rb_color == RB_BLACK)
				&& (!other->rb_right || other->rb_right->rb_color == RB_BLACK))
			{
				other->rb_color = RB_RED;
				node   = parent;
				parent = node->rb_parent;
			}
			else
			{
				if (!other->rb_right || other->rb_right->rb_color == RB_BLACK)
				{
					if (other->rb_left) {
						other->rb_left->rb_color = RB_BLACK;
					}
					other->rb_color = RB_RED;
					cdk_rbtree_rotate_right(other, tree);
					other = parent->rb_right;
				}
				other->rb_color  = parent->rb_color;
				parent->rb_color = RB_BLACK;

				if (other->rb_right) {
					other->rb_right->rb_color = RB_BLACK;
				}
				cdk_rbtree_rotate_left(parent, tree);
				node = tree->rb_root;
				break;
			}
		}
		else
		{
			other = parent->rb_left;

			if (other) {
				if (other->rb_color == RB_RED)
				{
					other->rb_color  = RB_BLACK;
					parent->rb_color = RB_RED;
					cdk_rbtree_rotate_right(parent, tree);
					other = parent->rb_left;
				}
				if ((!other->rb_left || other->rb_left->rb_color == RB_BLACK)
					&& (!other->rb_right || other->rb_right->rb_color == RB_BLACK))
				{
					other->rb_color = RB_RED;
					node   = parent;
					parent = node->rb_parent;
				}
				else
				{
					if (!other->rb_left || other->rb_left->rb_color == RB_BLACK)
					{
						if (other->rb_right) {
							other->rb_right->rb_color = RB_BLACK;
						}
						other->rb_color = RB_RED;
						cdk_rbtree_rotate_left(other, tree);
						other = parent->rb_left;
					}
					other->rb_color  = parent->rb_color;
					parent->rb_color = RB_BLACK;

					if (other->rb_left) {
						other->rb_left->rb_color = RB_BLACK;
					}
					cdk_rbtree_rotate_right(parent, tree);
					node = tree->rb_root;
					break;
				}
			}
		}
	}
	if (node) {
		node->rb_color = RB_BLACK;
	}
}

void cdk_rbtree_erase(cdk_rbtree_t* tree, cdk_rbtree_node_t* node)
{
	cdk_rbtree_node_t* child, * parent;
	int color;

	if (!node->rb_left) {
		child = node->rb_right;
	}
	else if (!node->rb_right)
	{
		child = node->rb_left;
	}
	else
	{
		cdk_rbtree_node_t* old = node, * left;

		node = node->rb_right;
		while ((left = node->rb_left)) {
			node = left;
		}

		if (old->rb_parent) {
			if (old->rb_parent->rb_left == old) {

				old->rb_parent->rb_left = node;
			}
			else {
				old->rb_parent->rb_right = node;
			}
		}
		else {
			tree->rb_root = node;
		}
		child  = node->rb_right;
		parent = node->rb_parent;
		color  = node->rb_color;

		if (parent == old) {
			parent = node;
		}
		else {
			if (child) {
				child->rb_parent = parent;
			}
			parent->rb_left = child;
			node->rb_right  = old->rb_right;
			old->rb_right->rb_parent = node;
		}
		node->rb_parent = old->rb_parent;
		node->rb_color  = old->rb_color;
		node->rb_left   = old->rb_left;
		old->rb_left->rb_parent = node;

		goto color;
	}

	parent = node->rb_parent;
	color  = node->rb_color;

	if (child)
		child->rb_parent = parent;
	if (parent)
	{
		if (parent->rb_left == node) {
			parent->rb_left = child;
		}
		else {
			parent->rb_right = child;
		}
	}
	else {
		tree->rb_root = child;
	}
color:
	if (color == RB_BLACK) {
		cdk_rbtree_erase_color(child, parent, tree);
	}
}

void cdk_rbtree_insert(cdk_rbtree_t* tree, cdk_rbtree_node_t* node) {

	cdk_rbtree_node_t** p = &(tree->rb_root);
	cdk_rbtree_node_t* parent = NULL;

	while (*p) {

		parent = *p;

		int r = cdk_rbtree_builtin_cmp(node->rb_key, parent->rb_key, tree->rb_keytype);
		if (r < 0) {
			p = &(*p)->rb_left;
		}
		else if (r > 0) {
			p = &(*p)->rb_right;
		}
		else {
			return;
		}
	}
	cdk_rbtree_link_node(node, parent, p);
	cdk_rbtree_insert_color(tree, node);
}

cdk_rbtree_node_t* cdk_rbtree_find(cdk_rbtree_t* tree, cdk_rbtree_node_key_t key) {

	cdk_rbtree_node_t* n = tree->rb_root;

	while (n) {
		int r = cdk_rbtree_builtin_cmp(key, n->rb_key, tree->rb_keytype);
		if (r < 0) {
			n = n->rb_left;
		}
		else if (r > 0) {
			n = n->rb_right;
		}
		else {
			return n;
		}
	}
	return NULL;
}

cdk_rbtree_node_t* cdk_rbtree_first(cdk_rbtree_t* tree)
{
	cdk_rbtree_node_t* n;

	n = tree->rb_root;
	if (!n) {
		return NULL;
	}
	while (n->rb_left) {
		n = n->rb_left;
	}
	return n;
}

cdk_rbtree_node_t* cdk_rbtree_last(cdk_rbtree_t* tree)
{
	cdk_rbtree_node_t* n;

	n = tree->rb_root;
	if (!n) {
		return NULL;
	}
	while (n->rb_right) {
		n = n->rb_right;
	}
	return n;
}

bool cdk_rbtree_empty(cdk_rbtree_t* tree) {

	return cdk_rbtree_first(tree) == NULL;
}

cdk_rbtree_node_t* cdk_rbtree_next(cdk_rbtree_node_t* node)
{
	cdk_rbtree_node_t* parent;

	if (node->rb_parent == node) {
		return NULL;
	}
	/* If we have a right-hand child, go down and then left as far
	   as we can. */
	if (node->rb_right) {
		node = node->rb_right;
		while (node->rb_left) {
			node = node->rb_left;
		}
		return (cdk_rbtree_node_t*)node;
	}

	/* No right-hand children.  Everything down and left is
	   smaller than us, so any 'next' node must be in the general
	   direction of our parent. Go up the tree; any time the
	   ancestor is a right-hand child of its parent, keep going
	   up. First time it's a left-hand child of its parent, said
	   parent is our 'next' node. */
	while ((parent = node->rb_parent) && node == parent->rb_right) {
		node = parent;
	}
	return parent;
}

cdk_rbtree_node_t* cdk_rbtree_prev(cdk_rbtree_node_t* node)
{
	cdk_rbtree_node_t* parent;

	if (node->rb_parent == node)
		return NULL;

	/* If we have a left-hand child, go down and then right as far
	   as we can. */
	if (node->rb_left) {
		node = node->rb_left;
		while (node->rb_right) {
			node = node->rb_right;
		}
		return (cdk_rbtree_node_t*)node;
	}

	/* No left-hand children. Go up till we find an ancestor which
	   is a right-hand child of its parent */
	while ((parent = node->rb_parent) && node == parent->rb_left) {
		node = parent;
	}
	return parent;
}

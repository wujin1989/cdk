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

static inline int __builtin_cmp_i8(int8_t k1, int8_t k2) {
	
	if (k1 < k2) {
		return -1;
	}
	if (k1 > k2) {
		return 1;
	}
	return 0;
}

static inline int __builtin_cmp_i16(int16_t k1, int16_t k2) {
	
	if (k1 < k2) {
		return -1;
	}
	if (k1 > k2) {
		return 1;
	}
	return 0;
}

static inline int __builtin_cmp_i32(int32_t k1, int32_t k2) {
	
	if (k1 < k2) {
		return -1;
	}
	if (k1 > k2) {
		return 1;
	}
	return 0;
}

static inline int __builtin_cmp_i64(int64_t k1, int64_t k2) {
	
	if (k1 < k2) {
		return -1;
	}
	if (k1 > k2) {
		return 1;
	}
	return 0;
}

static inline int __builtin_cmp_u8(uint8_t k1, uint8_t k2) {
	
	if (k1 < k2) {
		return -1;
	}
	if (k1 > k2) {
		return 1;
	}
	return 0;
}

static inline int __builtin_cmp_u16(uint16_t k1, uint16_t k2) {
	
	if (k1 < k2) {
		return -1;
	}
	if (k1 > k2) {
		return 1;
	}
	return 0;
}

static inline int __builtin_cmp_u32(uint32_t k1, uint32_t k2) {
	
	if (k1 < k2) {
		return -1;
	}
	if (k1 > k2) {
		return 1;
	}
	return 0;
}

static inline int __builtin_cmp_u64(uint64_t k1, uint64_t k2) {

	if (k1 < k2) {
		return -1;
	}
	if (k1 > k2) {
		return 1;
	}
	return 0;
}

static inline int __builtin_cmp_str(char* k1, char* k2) {

	return strcmp(k1, k2);
}

static inline void __rb_rotate_left(cdk_rb_node_t* node, cdk_rb_tree_t* tree)
{
	cdk_rb_node_t* right = node->rb_right;

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

static inline void __rb_rotate_right(cdk_rb_node_t* node, cdk_rb_tree_t* tree)
{
	cdk_rb_node_t* left = node->rb_left;

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

void cdk_rb_create(cdk_rb_tree_t* tree, cdk_rb_node_keytype_t type) {

	tree->rb_root = NULL;
	tree->rb_keytype = type;
}

static inline void __rb_link_node(cdk_rb_node_t* node, cdk_rb_node_t* parent, cdk_rb_node_t** rb_link)
{
	node->rb_parent = parent;
	node->rb_color = RB_RED;
	node->rb_left = node->rb_right = NULL;
	*rb_link = node;
}

static inline void __rb_insert_color(cdk_rb_tree_t* tree, cdk_rb_node_t* node)
{
	cdk_rb_node_t* parent, * gparent;

	while ((parent = node->rb_parent) && parent->rb_color == RB_RED)
	{
		gparent = parent->rb_parent;

		if (parent == gparent->rb_left)
		{
			{
				register cdk_rb_node_t* uncle = gparent->rb_right;
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
				register cdk_rb_node_t* tmp;
				__rb_rotate_left(parent, tree);
				tmp    = parent;
				parent = node;
				node   = tmp;
			}
			parent->rb_color  = RB_BLACK;
			gparent->rb_color = RB_RED;
			__rb_rotate_right(gparent, tree);
		}
		else {
			{
				register cdk_rb_node_t* uncle = gparent->rb_left;
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
				register cdk_rb_node_t* tmp;
				__rb_rotate_right(parent, tree);
				tmp    = parent;
				parent = node;
				node   = tmp;
			}
			parent->rb_color  = RB_BLACK;
			gparent->rb_color = RB_RED;
			__rb_rotate_left(gparent, tree);
		}
	}
	tree->rb_root->rb_color = RB_BLACK;
}

static inline void __rb_erase_color(cdk_rb_node_t* node, cdk_rb_node_t* parent, cdk_rb_tree_t* tree)
{
	cdk_rb_node_t* other;

	while ((!node || node->rb_color == RB_BLACK) && node != tree->rb_root)
	{
		if (parent->rb_left == node)
		{
			other = parent->rb_right;
			if (other->rb_color == RB_RED)
			{
				other->rb_color  = RB_BLACK;
				parent->rb_color = RB_RED;
				__rb_rotate_left(parent, tree);
				other = parent->rb_right;
			}
			if ((!other->rb_left || other->rb_left->rb_color == RB_BLACK)
				&& (!other->rb_right || other->rb_right->rb_color == RB_BLACK))
			{
				other->rb_color = RB_RED;
				node            = parent;
				parent          = node->rb_parent;
			}
			else
			{
				if (!other->rb_right || other->rb_right->rb_color == RB_BLACK)
				{
					if (other->rb_left) {
						other->rb_left->rb_color = RB_BLACK;
					}
					other->rb_color = RB_RED;
					__rb_rotate_right(other, tree);
					other = parent->rb_right;
				}
				other->rb_color  = parent->rb_color;
				parent->rb_color = RB_BLACK;

				if (other->rb_right) {
					other->rb_right->rb_color = RB_BLACK;
				}
				__rb_rotate_left(parent, tree);
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
					__rb_rotate_right(parent, tree);
					other = parent->rb_left;
				}
				if ((!other->rb_left || other->rb_left->rb_color == RB_BLACK)
					&& (!other->rb_right || other->rb_right->rb_color == RB_BLACK))
				{
					other->rb_color = RB_RED;
					node            = parent;
					parent          = node->rb_parent;
				}
				else
				{
					if (!other->rb_left || other->rb_left->rb_color == RB_BLACK)
					{
						if (other->rb_right) {
							other->rb_right->rb_color = RB_BLACK;
						}
						other->rb_color = RB_RED;
						__rb_rotate_left(other, tree);
						other = parent->rb_left;
					}
					other->rb_color  = parent->rb_color;
					parent->rb_color = RB_BLACK;

					if (other->rb_left) {
						other->rb_left->rb_color = RB_BLACK;
					}
					__rb_rotate_right(parent, tree);
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

void cdk_rb_erase(cdk_rb_tree_t* tree, cdk_rb_node_t* node)
{
	cdk_rb_node_t* child, * parent;
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
		cdk_rb_node_t* old = node, * left;

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
		node->rb_parent         = old->rb_parent;
		node->rb_color          = old->rb_color;
		node->rb_left           = old->rb_left;
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
		__rb_erase_color(child, parent, tree);
	}
}

void cdk_rb_insert(cdk_rb_tree_t* tree, cdk_rb_node_t* node) {

	cdk_rb_node_t** p = &(tree->rb_root);
	cdk_rb_node_t* parent = NULL;

	while (*p) {

		parent = *p;

		int r;
		switch (tree->rb_keytype)
		{
		case RB_KEYTYPE_INT8:
			r = __builtin_cmp_i8(node->rb_key.i8, parent->rb_key.i8);
			break;
		case RB_KEYTYPE_INT16:
			r = __builtin_cmp_i16(node->rb_key.i16, parent->rb_key.i16);
			break;
		case RB_KEYTYPE_INT32:
			r = __builtin_cmp_i32(node->rb_key.i32, parent->rb_key.i32);
			break;
		case RB_KEYTYPE_INT64:
			r = __builtin_cmp_i64(node->rb_key.i64, parent->rb_key.i64);
			break;
		case RB_KEYTYPE_UINT8:
			r = __builtin_cmp_u8(node->rb_key.u8, parent->rb_key.u8);
			break;
		case RB_KEYTYPE_UINT16:
			r = __builtin_cmp_u16(node->rb_key.u16, parent->rb_key.u16);
			break;
		case RB_KEYTYPE_UINT32:
			r = __builtin_cmp_u32(node->rb_key.u32, parent->rb_key.u32);
			break;
		case RB_KEYTYPE_UINT64:
			r = __builtin_cmp_u64(node->rb_key.u64, parent->rb_key.u64);
			break;
		case RB_KEYTYPE_STR:
			r = __builtin_cmp_str(node->rb_key.str, parent->rb_key.str);
			break;
		default:
			abort();
			break;
		}
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
	__rb_link_node(node, parent, p);
	__rb_insert_color(tree, node);
}

cdk_rb_node_t* cdk_rb_find(cdk_rb_tree_t* tree, cdk_rb_node_key_t key) {

	cdk_rb_node_t* n = tree->rb_root;

	while (n) {
		int r;
		switch (tree->rb_keytype)
		{
		case RB_KEYTYPE_INT8:
			r = __builtin_cmp_i8(key.i8, n->rb_key.i8);
			break;
		case RB_KEYTYPE_INT16:
			r = __builtin_cmp_i16(key.i16, n->rb_key.i16);
			break;
		case RB_KEYTYPE_INT32:
			r = __builtin_cmp_i32(key.i32, n->rb_key.i32);
			break;
		case RB_KEYTYPE_INT64:
			r = __builtin_cmp_i64(key.i64, n->rb_key.i64);
			break;
		case RB_KEYTYPE_UINT8:
			r = __builtin_cmp_u8(key.u8, n->rb_key.u8);
			break;
		case RB_KEYTYPE_UINT16:
			r = __builtin_cmp_u16(key.u16, n->rb_key.u16);
			break;
		case RB_KEYTYPE_UINT32:
			r = __builtin_cmp_u32(key.u32, n->rb_key.u32);
			break;
		case RB_KEYTYPE_UINT64:
			r = __builtin_cmp_u64(key.u64, n->rb_key.u64);
			break;
		case RB_KEYTYPE_STR:
			r = __builtin_cmp_str(key.str, n->rb_key.str);
			break;
		default:
			abort();
			break;
		}
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

cdk_rb_node_t* cdk_rb_first(cdk_rb_tree_t* tree)
{
	cdk_rb_node_t* n;

	n = tree->rb_root;
	if (!n) {
		return NULL;
	}
	while (n->rb_left) {
		n = n->rb_left;
	}
	return n;
}

cdk_rb_node_t* cdk_rb_last(cdk_rb_tree_t* tree)
{
	cdk_rb_node_t* n;

	n = tree->rb_root;
	if (!n) {
		return NULL;
	}
	while (n->rb_right) {
		n = n->rb_right;
	}
	return n;
}

cdk_rb_node_t* cdk_rb_next(cdk_rb_node_t* node)
{
	cdk_rb_node_t* parent;

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
		return (cdk_rb_node_t*)node;
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

cdk_rb_node_t* cdk_rb_prev(cdk_rb_node_t* node)
{
	cdk_rb_node_t* parent;

	if (node->rb_parent == node)
		return NULL;

	/* If we have a left-hand child, go down and then right as far
	   as we can. */
	if (node->rb_left) {
		node = node->rb_left;
		while (node->rb_right) {
			node = node->rb_right;
		}
		return (cdk_rb_node_t*)node;
	}

	/* No left-hand children. Go up till we find an ancestor which
	   is a right-hand child of its parent */
	while ((parent = node->rb_parent) && node == parent->rb_left) {
		node = parent;
	}
	return parent;
}

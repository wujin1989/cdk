/** Copyright (c) 2022, Wu Jin <wujin.developer@gmail.com>
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

#include "cdk/cdk-rbtree.h"

static void __rb_rotate_left(rb_node_t* node, rb_tree_t* tree)
{
	rb_node_t* right = node->rb_right;

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

static void __rb_rotate_right(rb_node_t* node, rb_tree_t* tree)
{
	rb_node_t* left = node->rb_left;

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

void cdk_rb_create(rb_tree_t* tree) {

	tree->rb_root = NULL;
}

void rb_insert_color(rb_tree_t* tree, rb_node_t* node)
{
	rb_node_t* parent, * gparent;

	while ((parent = node->rb_parent) && parent->rb_color == RB_RED)
	{
		gparent = parent->rb_parent;

		if (parent == gparent->rb_left)
		{
			{
				register rb_node_t* uncle = gparent->rb_right;
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
				register rb_node_t* tmp;
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
				register rb_node_t* uncle = gparent->rb_left;
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
				register rb_node_t* tmp;
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

static void __rb_erase_color(rb_node_t* node, rb_node_t* parent, rb_tree_t* tree)
{
	rb_node_t* other;

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

void rb_erase(rb_tree_t* tree, rb_node_t* node)
{
	rb_node_t* child, * parent;
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
		rb_node_t* old = node, * left;

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

rb_node_t* cdk_rb_first(rb_tree_t* tree)
{
	rb_node_t* n;

	n = tree->rb_root;
	if (!n) {
		return NULL;
	}
	while (n->rb_left) {
		n = n->rb_left;
	}
	return n;
}

rb_node_t* cdk_rb_last(rb_tree_t* tree)
{
	rb_node_t* n;

	n = tree->rb_root;
	if (!n) {
		return NULL;
	}
	while (n->rb_right) {
		n = n->rb_right;
	}
	return n;
}

rb_node_t* cdk_rb_next(rb_node_t* node)
{
	rb_node_t* parent;

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
		return (rb_node_t*)node;
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

rb_node_t* cdk_rb_prev(rb_node_t* node)
{
	rb_node_t* parent;

	if (node->rb_parent == node)
		return NULL;

	/* If we have a left-hand child, go down and then right as far
	   as we can. */
	if (node->rb_left) {
		node = node->rb_left;
		while (node->rb_right) {
			node = node->rb_right;
		}
		return (rb_node_t*)node;
	}

	/* No left-hand children. Go up till we find an ancestor which
	   is a right-hand child of its parent */
	while ((parent = node->rb_parent) && node == parent->rb_left) {
		node = parent;
	}
	return parent;
}

void rb_link_node(rb_node_t* node, rb_node_t* parent, rb_node_t** rb_link)
{
	node->rb_parent = parent;
	node->rb_color  = RB_RED;
	node->rb_left   = node->rb_right = NULL;
	*rb_link        = node;
}

void rb_replace_node(rb_tree_t* tree, rb_node_t* victim, rb_node_t* new)
{
	rb_node_t* parent = victim->rb_parent;

	/* Set the surrounding nodes to point to the replacement */
	if (parent) {
		if (victim == parent->rb_left) {
			parent->rb_left = new;
		}
		else {
			parent->rb_right = new;
		}
	}
	else {
		tree->rb_root = new;
	}
	if (victim->rb_left) {
		victim->rb_left->rb_parent = new;
	}
	if (victim->rb_right) {
		victim->rb_right->rb_parent = new;
	}
	/* Copy the pointers/colour from the victim to the replacement */
	*new = *victim;
}

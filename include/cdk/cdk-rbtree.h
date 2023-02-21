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
#ifndef __CDK_RBTREE_H__
#define __CDK_RBTREE_H__

#include <stdio.h>
#include <stddef.h>
#include "cdk-types.h"

/**
 * to use rbtrees you'll have to implement your own insert and search cores.
 * this will avoid us to use callbacks and to drop drammatically performances.
 * i know it's not the cleaner way,  but in c (not in c++) to get
 * performances and genericity...
 * 
 * example:
 * 
 *  typedef struct _rb_entry_t{
 *		char*     key;
 *      int       val;
 *		rb_node_t n;
 *  }rb_entry_t;
 * 
 *	static inline void cdk_rb_insert(rb_tree_t* tree, char* key, rb_node_t* node){
 *
 *		rb_node_t** p      = &(tree->rb_root);
 *		rb_node_t*  parent = NULL;
 * 
 *		while(*p){
 *			
 *			parent        = *p;
 *			rb_entry_t* e = cdk_rb_entry(parent, rb_entry_t, n);
 * 
 *			int r = strcmp(key, e->key);
 *			if(r < 0){
 *				p = &(*p)->rb_left;
 *			}else if(r > 0){
 *				p = &(*p)->rb_right;
 *			}else{
 *				return;
 *			}
 *		}
 *		cdk_rb_link_node(node, parent, p);
 *		cdk_rb_insert_color(tree, node);
 *	}
 * 
 *	static inline rb_entry_t* cdk_rb_find(rb_tree_t* tree, const char* key){
 * 
 *		rb_node_t* n = tree->rb_root;
 * 
 *		while(n){
 *			rb_entry_t* e = cdk_rb_entry(n, rb_entry_t, n);
 *          
 *			int r = strcmp(key, e->key);
 *			if(r < 0){
 *				n = n->rb_left;
 *			}else if(r > 0){
 *				n = n->rb_right;
 *			}else{
 *				return e;
 *			}
 *		}
 *		return NULL;
 *  }
 * TODO:
 *	wait for the c standard to support similar gnu typeof (currently not supported by msvc), 
 *  so that the insert and search methods can implement common operations through typeof and C11 _Generic.
 * 
 */
                  

/**
 *  create a empty rbtree.
 *
 *  @param tree [in] rbtree.
 *  @return N/A.
 */
extern void cdk_rb_create(rb_tree_t* tree);

/**
 *  erase a entry from rbtree.
 *
 *  @param tree [in] rbtree.
 *  @param node [in] rbtree node.
 *  @return N/A.
 */
extern void cdk_rb_erase(rb_tree_t* tree, rb_node_t* node);

/**
 *  retrive the successive element for the specified node.
 *	the successive element is an element with greater key value.
 *
 *  @param node [in] rbtree node.
 *  @return the successive node, or NULL if the node has no successor.
 */
extern rb_node_t* cdk_rb_next(rb_node_t* node);

/**
 *  retrive the previous element for the specified node.
 *	the previous element is an element with less key value.
 *
 *  @param node [in] rbtree node.
 *  @return the previous tree node, or NULL if the node has no previous node.
 */
extern rb_node_t* cdk_rb_prev(rb_node_t* node);

/**
 *  retrive first element from rbtree.
 *	the first element always has the least value for the key.
 *
 *  @param tree [in] rbtree.
 *  @return the tree node, or NULL if the tree has no element.
 */
extern rb_node_t* cdk_rb_first(rb_tree_t* tree);

/**
 *  retrive last element from rbtree.
 *	the last element always has the greatest value for the key.
 *
 *  @param tree [in] rbtree.
 *  @return the tree node, or NULL if the tree has no element.
 */
extern rb_node_t* cdk_rb_last(rb_tree_t* tree);

/**
 * the following functions are only used internally by rbtree.
 */
#define	cdk_rb_entry(n, t, m)                          \
			((t *) ((char *) (n) - offsetof(t, m)))   

extern void cdk_rb_link_node(rb_node_t* node, rb_node_t* parent, rb_node_t** rb_link);
extern void cdk_rb_insert_color(rb_tree_t* tree, rb_node_t* node);

#endif /* __CDK_RBTREE_H__ */
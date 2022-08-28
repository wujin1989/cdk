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
 *	static inline void cdk_rb_insert(rb_tree_t* tree, char* key, rb_node_t* node){
 *
 *		rb_node_t** p      = &(tree->rb_root);
 *		rb_node_t*  parent = NULL;
 * 
 *		while(*p){
 *			
 *			T* t = cdk_rb_entry(node, T, rb_node);
 * 
 *			int r = strcmp(key, t->key);
 *          if(r < 0){
 *				p = &(*p)->rb_left;
 *          }else if(r > 0){
 *				p = &(*p)->rb_right;
 *          }else{
 *				return;
 *			}
 *		}
 *		cdk_rb_link_node(node, parent, p);
 *		cdk_rb_insert_color(tree, node);
 *	}
 * 
 *	static inline T* cdk_rb_find(rb_tree_t* tree, const char* key){
 * 
 *		rb_node_t* n = tree->rb_root;
 * 
 *      while(n){
 *			T* t = cdk_rb_entry(n, T, rb_node);
 *          
 *          int r = strcmp(key, t->key);
 *          if(r < 0){
 *				n = n->rb_left;
 *          }else if(r > 0){
 *				n = n->rb_right;
 *          }else{
 *				return t;
 *			}
 *      }
 *		return NULL;
 *  }
 */

#define	RB_RED		0
#define	RB_BLACK	1

#define	cdk_rb_entry(n, t, m)                          \
			((t *) ((char *) (n) - offsetof(t, m)))                     

extern void cdk_rb_create(rb_tree_t* tree);

extern rb_node_t* cdk_rb_next(rb_node_t* node);
extern rb_node_t* cdk_rb_prev(rb_node_t* node);
extern rb_node_t* cdk_rb_first(rb_tree_t* tree);
extern rb_node_t* cdk_rb_last(rb_tree_t* tree);

extern void cdk_rb_link_node(rb_node_t* node, rb_node_t* parent, rb_node_t** rb_link);
extern void cdk_rb_insert_color(rb_tree_t* tree, rb_node_t* node);
extern void cdk_rb_erase(rb_tree_t* tree, rb_node_t* node);
extern void cdk_rb_replace_node(rb_tree_t* tree, rb_node_t* victim, rb_node_t* new);

#endif /* __CDK_RBTREE_H__ */
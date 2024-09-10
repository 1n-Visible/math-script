#include <stdlib.h>
#include <stdbool.h>
#include <wchar.h>

#include "charset.h"

#ifndef DICT_TREE_H
#define DICT_TREE_H

typedef struct DictTree DictTree;

DictTree *new_DictTree();
void free_DictTree(DictTree *node);
void print_DictTree(DictTree *node);

short get_depth(DictTree *node);
short get_size(DictTree *node);

bool contains_word(DictTree *root, const wchar_t *word);
void *get_word(DictTree *root, const wchar_t *word);
bool add_word(DictTree *root, const wchar_t *word);

bool set_word(DictTree *root, const wchar_t *word, const void *object);
bool remove_word(DictTree *root, const wchar_t *word);

void optimize_DictTree(DictTree *node);

typedef struct iterDictTree iterDictTree;

iterDictTree *new_iterDictTree(DictTree *root);
void free_iterDictTree(iterDictTree *iter_dt);
void *iterDT_next(iterDictTree *iter_dt);

#endif

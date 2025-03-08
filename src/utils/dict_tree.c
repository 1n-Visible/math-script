#include "dict_tree.h"

struct DictTree {
    DictTree *nodes[CHARSET_SIZE];
    void *object;
};

DictTree *new_DictTree() {
    DictTree *node=malloc(sizeof(DictTree));
    
    for (short i=0; i<CHARSET_SIZE; i++)
        node->nodes[i]=NULL;
    node->object=NULL;
    
    return node;
}

void free_DictTree(DictTree *node) {
    if (node==NULL)
        return;
    
    for (short i=0; i<CHARSET_SIZE; i++)
        free_DictTree(node->nodes[i]);
    free(node);
}

void print_DictTree(DictTree *node) {
    wprintf(L"DictTree(object=%p", node->object);
    
    DictTree *child;
    for (short i=0; i<CHARSET_SIZE; i++) {
        child=node->nodes[i];
        if (child==NULL) continue;
        wprintf(L", %lc=", index_to_alnum(i));
        print_DictTree(child);
    }
    
    putwchar(L')');
}

short get_depth(DictTree *node) {
    if (node==NULL)
        return 0;

    short i, depth, max_depth=0;
    for (i=0; i<CHARSET_SIZE; i++) {
        depth=get_depth(node->nodes[i]);
        if (max_depth<depth)
            max_depth=depth;
    }

    return max_depth;
}

short get_size(DictTree *node) {
    if (node==NULL)
        return 0;

    short i, size=1;
    for (i=0; i<CHARSET_SIZE; i++)
        size+=get_size(node->nodes[i]);
    return size;
}

bool contains_word(DictTree *root, const wchar_t *word) {
    return get_word(root, word)!=NULL;
}

void *get_word(DictTree *root, const wchar_t *word) {
    DictTree *node=root;
    wchar_t c=word[0];

    short i, index;
    for (i=0; c!=L'\0'; c=word[++i]) {
        index=alnum_to_index(c);
        if (index==-1)
            return NULL;

        node=node->nodes[index];
        if (node==NULL)
            return NULL;
    }

    return node->object;
}

bool add_word(DictTree *root, const wchar_t *word) {
    return set_word(root, word, (const void *)true);
}

bool set_word(DictTree *root, const wchar_t *word, const void *object) {
    DictTree *node;
    wchar_t c=word[0];

    short i, index;
    for (i=0; c!=L'\0'; c=word[++i]) {
        index=alnum_to_index(c);
        if (index==-1)
            return false;

        node=root->nodes[index];
        if (node==NULL) {
            node=new_DictTree();
            root->nodes[index]=node;
        }
        root=node;
    }

    root->object=object;
    return true;
}

bool remove_word(DictTree *root, const wchar_t *word) {
    wchar_t c=word[0];

    short i, index;
    for (i=0; c!=L'\0'; c=word[++i]) {
        index=alnum_to_index(c);
        if (index==-1)
            return false;
        
        root=root->nodes[index];
        if (root==NULL)
            return false;
    }

    root->object=NULL;
    return true;
}

void optimize_DictTree(DictTree *node);

struct iterDictTree {
    iterDictTree *child;
    DictTree *node;
    short index;
};

iterDictTree *new_iterDictTree(DictTree *root) {
    if (root==NULL)
        return NULL;

    iterDictTree *iter_dt=malloc(sizeof(iterDictTree));
    iter_dt->child=NULL;
    iter_dt->node=root;
    iter_dt->index=-1;
    return iter_dt;
}

void free_iterDictTree(iterDictTree *iter_dt) {
    if (iter_dt==NULL)
        return;

    free_iterDictTree(iter_dt->child);
    free(iter_dt);
}

void *iterDT_next(iterDictTree *iter_dt) {
    if (iter_dt==NULL)
        return NULL;

    void *object;
    while (iter_dt->index<CHARSET_SIZE) {
        object=iterDT_next(iter_dt->child);
        if (object!=NULL) {
            ;
            return object;
        }
        
        free_iterDictTree(iter_dt->child);
        iter_dt->child=new_iterDictTree(iter_dt->node->nodes[iter_dt->index++]);
    }

    return NULL;
}

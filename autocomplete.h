#ifndef AUTOCOMPLETE_H
#define AUTOCOMPLETE_H

#include <stdlib.h>

#define DA_INIT_CAPACITY 8192
#define DA_REALLOC(oldptr, oldsz, newsz) realloc(oldptr, newsz)
#define da_append(da, item)                                                    \
    do {                                                                       \
        if ((da)->count >= (da)->capacity) {                                   \
            size_t new_capacity = (da)->capacity * 2;                          \
            if (new_capacity == 0) {                                           \
                new_capacity = DA_INIT_CAPACITY;                               \
            }                                                                  \
                                                                               \
            (da)->items = DA_REALLOC((da)->items,                              \
                                     (da)->capacity * sizeof((da)->items[0]),  \
                                     new_capacity * sizeof((da)->items[0]));   \
            (da)->capacity = new_capacity;                                     \
        }                                                                      \
                                                                               \
        (da)->items[(da)->count++] = (item);                                   \
    } while (0)

struct ac_trie;

struct ac_suggestions {
        char **items;
        size_t count;
        size_t capacity;
};

struct ac_trie *ac_trie_new();
void ac_trie_insert(struct ac_trie *t, char *word);
struct ac_suggestions *ac_trie_suggest(struct ac_trie *t, char *prefix);
void ac_trie_from_file(struct ac_trie *t, char *name);
void ac_trie_free(struct ac_trie *t);
unsigned int ac_top_suggestions(struct ac_suggestions *s, unsigned int n);
void ac_suggestions_sort(struct ac_suggestions *s);
void ac_suggestions_free(struct ac_suggestions *s);

#endif // AUTOCOMPLETE_H

#ifdef AUTOCOMPLETE_IMPLEMENTATION

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define min(a, b) ((a) < (b) ? (a) : (b))

#define ALPHABET_SIZE 26
#define MAX_LEN 256

unsigned int char_to_index(char c) {
    return (unsigned int)c - (unsigned int)'a';
}

char index_to_char(unsigned int i) { return (char)((unsigned int)'a' + i); }

char *lowercase(char *word) {
    for (char *c = word; *c != '\0'; c++) {
        *c = (char)tolower(*c);
    }

    return word;
}

unsigned int valid_word(char *word) {
    for (char *c = word; *c != '\0'; c++) {
        if (!isalpha(*c)) {
            return 0;
        }
    }

    return 1;
}

int cmp_length_asc(const void *a, const void *b) {
    return strlen(*(char **)a) - strlen(*(char **)b);
}

struct node {
        struct node *children[ALPHABET_SIZE];
        unsigned int is_end_of_word;
};

struct node *node_new() {
    struct node *n = malloc(sizeof(struct node));
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        n->children[i] = NULL;
    }
    n->is_end_of_word = 0;
    return n;
}

void node_find_words(struct node *n, char *prefix, struct ac_suggestions *s) {
    if (n->is_end_of_word) {
        da_append(s, strdup(prefix));
    }

    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (n->children[i] != NULL) {
            char *new_prefix = malloc(strlen(prefix) + 2);
            strcpy(new_prefix, prefix);
            new_prefix[strlen(prefix)] = index_to_char(i);
            new_prefix[strlen(prefix) + 1] = '\0';
            node_find_words(n->children[i], new_prefix, s);
        }
    }

    free(prefix);
}

void node_free(struct node *n) {
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (n->children[i] != NULL) {
            node_free(n->children[i]);
        }
    }
    free(n);
}

struct ac_trie {
        struct node *root;
};

struct ac_trie *ac_trie_new() {
    struct ac_trie *t = malloc(sizeof(struct ac_trie));
    t->root = node_new();
    return t;
}

void ac_trie_insert(struct ac_trie *t, char *word) {
    struct node *n = t->root;

    for (char *c = word; *c != '\0'; c++) {
        unsigned int index = char_to_index(*c);

        if (n->children[index] == NULL) {
            n->children[index] = node_new();
        }

        n = n->children[index];
    }

    n->is_end_of_word = 1;
}

struct ac_suggestions *ac_trie_suggest(struct ac_trie *t, char *prefix) {
    struct node *n = t->root;

    for (char *c = prefix; *c != '\0'; c++) {
        unsigned int index = char_to_index(*c);

        if (n->children[index] == NULL) {
            return NULL;
        }

        n = n->children[index];
    }

    struct ac_suggestions *s = malloc(sizeof(struct ac_suggestions));
    s->items = NULL;
    s->count = 0;
    s->capacity = 0;

    node_find_words(n, strdup(prefix), s);

    return s;
}

void ac_trie_from_file(struct ac_trie *t, char *name) {
    FILE *f = fopen(name, "r");

    char buffer[MAX_LEN];
    while (fgets(buffer, MAX_LEN, f)) {
        strchr(buffer, '\n')[0] = '\0';
        if (valid_word(buffer)) {
            ac_trie_insert(t, lowercase(buffer));
        }
    }

    fclose(f);
}

void ac_trie_free(struct ac_trie *t) {
    node_free(t->root);
    free(t);
}

unsigned int ac_top_suggestions(struct ac_suggestions *s, unsigned int n) {
    return min(s->count, n);
}

void ac_suggestions_sort(struct ac_suggestions *s) {
    qsort(s->items, s->count, sizeof(char *), cmp_length_asc);
}

void ac_suggestions_free(struct ac_suggestions *s) {
    for (int i = 0; i < s->count; i++) {
        free(s->items[i]);
    }
    free(s->items);
    free(s);
}

#endif // AUTOCOMPLETE_IMPLEMENTATION

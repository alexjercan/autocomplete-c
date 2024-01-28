#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define min(a, b) ((a) < (b) ? (a) : (b))

#define ALPHABET_SIZE 26
#define MAX_LEN 256

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

struct node {
    struct node *children[ALPHABET_SIZE];
    unsigned int is_end_of_word;
};

struct trie {
    struct node *root;
};

struct trie *trie_new();
void trie_insert(struct trie *t, char *word);
struct suggestions *trie_suggest(struct trie *t, char *prefix);

struct node *node_new() {
    struct node *n = malloc(sizeof(struct node));
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        n->children[i] = NULL;
    }
    n->is_end_of_word = 0;
    return n;
}

struct trie *trie_new() {
    struct trie *t = malloc(sizeof(struct trie));
    t->root = node_new();
    return t;
}

unsigned int char_to_index(char c) {
    return (unsigned int) c - (unsigned int) 'a';
}

char index_to_char(unsigned int i) {
    return (char) ((unsigned int) 'a' + i);
}

void trie_insert(struct trie *t, char *word) {
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

struct suggestions {
    char **items;
    size_t count;
    size_t capacity;
};

void node_find_words(struct node *n, char *prefix, struct suggestions *s) {
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

struct suggestions *trie_suggest(struct trie *t, char *prefix) {
    struct node *n = t->root;

    for (char *c = prefix; *c != '\0'; c++) {
        unsigned int index = char_to_index(*c);

        if (n->children[index] == NULL) {
            return NULL;
        }

        n = n->children[index];
    }

    struct suggestions *s = malloc(sizeof(struct suggestions));
    s->items = NULL;
    s->count = 0;
    s->capacity = 0;

    node_find_words(n, strdup(prefix), s);

    return s;
}

void node_free(struct node *n) {
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (n->children[i] != NULL) {
            node_free(n->children[i]);
        }
    }
    free(n);
}

void trie_free(struct trie *t) {
    node_free(t->root);
    free(t);
}

char *lowercase(char *word) {
    for (char *c = word; *c != '\0'; c++) {
        *c = (char) tolower(*c);
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

void trie_from_file(struct trie *t, char *name) {
    FILE *f = fopen(name, "r");

    char buffer[MAX_LEN];
    while (fgets(buffer, MAX_LEN, f)) {
        strchr(buffer, '\n')[0] = '\0';
        if (valid_word(buffer)) {
            trie_insert(t, lowercase(buffer));
        }
    }

    fclose(f);
}

int cmp_length_asc (const void *a, const void *b) {
    return strlen(*(char **) a) - strlen(*(char **) b);
}

int main() {
    struct trie *t = trie_new();

    char buffer[MAX_LEN];
    printf("Enter a word: ");
    scanf("%s", buffer);

    trie_from_file(t, "words.txt");

    struct suggestions *s = trie_suggest(t, buffer);

    trie_free(t);

    qsort(s->items, s->count, sizeof(char *), cmp_length_asc);

    unsigned int top_suggestions = min(s->count, 3);

    printf("Did you mean:\n");
    for (int i = 0; i < top_suggestions; i++) {
        printf("%s\n", s->items[i]);
    }

    for (int i = 0; i < s->count; i++) {
        free(s->items[i]);
    }
    free(s->items);
    free(s);

    return 0;
}

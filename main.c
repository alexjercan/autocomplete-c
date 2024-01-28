#define AUTOCOMPLETE_IMPLEMENTATION
#include "autocomplete.h"
#include <stdio.h>

int main() {
    struct ac_trie *t = ac_trie_new();

    char buffer[MAX_LEN];
    printf("Enter a word: ");
    scanf("%s", buffer);

    ac_trie_from_file(t, "words.txt");

    struct ac_suggestions *s = ac_trie_suggest(t, buffer);

    ac_trie_free(t);

    printf("Did you mean:\n");
    for (int i = 0; i < ac_top_suggestions(s, 3); i++) {
        printf("%s\n", s->items[i]);
    }

    ac_suggestions_free(s);

    return 0;
}

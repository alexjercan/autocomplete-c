#define AUTOCOMPLETE_IMPLEMENTATION
#include "autocomplete.h"

#define ARGPARSE_IMPLEMENTATION
#include "argparse.h"

#include <stdio.h>

int main(int argc, char **argv) {
    struct argparse_parser *parser = argparse_new(
        "autocomplete", "example for autocomplete program", "0.1.0");

    argparse_add_argument(parser, 'f', "file", "input file name",
                          ARGUMENT_TYPE_VALUE);

    argparse_parse(parser, argc, argv);

    char *file = argparse_get_value(parser, "file");

    if (!file) {
        fprintf(stderr, "Error: file name is required\n");
        return 1;
    }

    struct ac_trie *t = ac_trie_new();

    char buffer[MAX_LEN];
    printf("Enter a word: ");
    scanf("%s", buffer);

    ac_trie_from_file(t, file);

    struct ac_suggestions *s = ac_trie_suggest(t, buffer);

    printf("Did you mean:\n");
    for (int i = 0; i < ac_top_suggestions(s, 3); i++) {
        printf("%s\n", s->items[i]);
    }

    ac_trie_free(t);
    ac_suggestions_free(s);
    argparse_free(parser);

    return 0;
}

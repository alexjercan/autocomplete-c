#ifndef ARGPARSE_H
#define ARGPARSE_H

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

enum argument_type {
    ARGUMENT_TYPE_VALUE,
    ARGUMENT_TYPE_FLAG,
};

struct argparse_parser;

struct argparse_parser *argparse_new(char *name, char *description,
                                     char *version);
void argparse_add_argument(struct argparse_parser *parser, char short_name,
                           char *long_name, char *description,
                           enum argument_type type);
void argparse_parse(struct argparse_parser *parser, int argc, char *argv[]);
char *argparse_get_value(struct argparse_parser *parser, char *long_name);
unsigned int argparse_get_flag(struct argparse_parser *parser, char *long_name);

void argparse_print_help(struct argparse_parser *parser);
void argparse_print_version(struct argparse_parser *parser);

void argparse_free(struct argparse_parser *parser);

#endif // ARGPARSE_H

#ifdef ARGPARSE_IMPLEMENTATION

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct argument {
        char short_name;
        char *long_name;
        char *description;
        enum argument_type type;
        char *value;
        unsigned int flag;
};

struct argparse_parser {
        char *name;
        char *description;
        char *version;
        struct argument *items;
        size_t count;
        size_t capacity;
};

struct argparse_parser *argparse_new(char *name, char *description,
                                     char *version) {
    struct argparse_parser *parser = malloc(sizeof(struct argparse_parser));

    parser->name = name;
    parser->description = description;
    parser->version = version;
    parser->items = NULL;
    parser->count = 0;
    parser->capacity = 0;

    return parser;
}

void argparse_add_argument(struct argparse_parser *parser, char short_name,
                           char *long_name, char *description,
                           enum argument_type type) {
    struct argument arg = {
        .short_name = short_name,
        .long_name = long_name,
        .description = description,
        .type = type,
        .value = NULL,
        .flag = 0,
    };

    da_append(parser, arg);
}

void argparse_parse(struct argparse_parser *parser, int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        char *name = argv[i];

        if (name[0] == '-') {
            struct argument *arg = NULL;

            for (size_t j = 0; j < parser->count; j++) {
                struct argument *item = &parser->items[j];

                if ((name[1] == '-' && item->long_name != NULL &&
                     strcmp(name + 2, item->long_name) == 0) ||
                    (name[1] != '-' && item->short_name != '\0' &&
                     name[1] == item->short_name)) {
                    arg = item;
                    break;
                }
            }

            if (arg == NULL) {
                fprintf(stderr, "error: invalid argument: %s\n", name);
                exit(1);
            }

            if (arg->type == ARGUMENT_TYPE_FLAG) {
                arg->flag = 1;
            } else if (arg->type == ARGUMENT_TYPE_VALUE) {
                if (i + 1 >= argc) {
                    fprintf(stderr, "error: missing value for argument: %s\n",
                            name);
                    exit(1);
                }

                arg->value = argv[++i];
            } else {
                fprintf(stderr, "error: type not supported for argument: %s\n",
                        name);
                exit(1);
            }
        } else {
            fprintf(stderr, "error: invalid argument: %s\n", name);
            exit(1);
        }
    }
}

char *argparse_get_value(struct argparse_parser *parser, char *long_name) {
    for (size_t i = 0; i < parser->count; i++) {
        struct argument *item = &parser->items[i];

        if (item->long_name != NULL &&
            strcmp(long_name, item->long_name) == 0) {
            if (item->type != ARGUMENT_TYPE_VALUE) {
                fprintf(stdout, "warning: argument is not a value: %s\n",
                        long_name);
            }
            return item->value;
        }
    }

    return NULL;
}

unsigned int argparse_get_flag(struct argparse_parser *parser,
                               char *long_name) {
    for (size_t i = 0; i < parser->count; i++) {
        struct argument *item = &parser->items[i];

        if (item->long_name != NULL &&
            strcmp(long_name, item->long_name) == 0) {
            if (item->type != ARGUMENT_TYPE_FLAG) {
                fprintf(stdout, "warning: argument is not a flag: %s\n",
                        long_name);
            }
            return item->flag;
        }
    }

    return 0;
}

void argparse_print_help(struct argparse_parser *parser) {
    printf("usage: %s [options]\n", parser->name);
    printf("\n");
    printf("%s\n", parser->description);
    printf("\n");
    printf("options:\n");

    for (size_t i = 0; i < parser->count; i++) {
        struct argument *item = &parser->items[i];

        printf("  -%c, --%s", item->short_name, item->long_name);

        if (item->type == ARGUMENT_TYPE_VALUE) {
            printf(" <value>");
        }

        printf("\n");
        printf("      %s\n", item->description);
        printf("\n");
    }
}

void argparse_print_version(struct argparse_parser *parser) {
    printf("%s %s\n", parser->name, parser->version);
}

void argparse_free(struct argparse_parser *parser) {
    free(parser->items);
    free(parser);
}

#endif // ARGPARSE_IMPLEMENTATION

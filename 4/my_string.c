#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct my_string {
    char *string;
    int length;
    int capacity;
} typedef my_string;

void print_string(my_string *string) {
    for (int i = 0; i < string->length; ++i) {
        printf("%c", string->string[i]);
    }
    printf("\n");
}

int get_capacity(my_string *string) {
    return string->capacity;
}

int get_length(my_string *string) {
    return string->length;
}

void add_char(my_string *string, char c) {
    if (string->length == string->capacity) {
        string->capacity *= 2;
        string->string = realloc(string->string, string->capacity);
    }
    string->string[string->length] = c;
    string->length++;
}

my_string *create_my_string(char *value) {
    my_string *string = malloc(sizeof(my_string));
    string->length = 0;
    string->capacity = 1;
    string->string = malloc(string->capacity);
    size_t char_length = strlen(value);
    for (size_t i = 0; i < char_length; ++i) {
        add_char(string, value[i]);
    }
    return string;
}

void delete_string(my_string *string) {
    free(string->string);
    free(string);
}

void set_string(my_string *string, const char *value, int value_length) {
    delete_string(string);
    string = malloc(sizeof(my_string));
    string->length = value_length;
    string->capacity = value_length;
    string->string = malloc(string->capacity);
    for (int i = 0; i < value_length; ++i) {
        string->string[i] = value[i];
    }
}

my_string *combining_strings(my_string *string1, my_string *string2) {
    int string1_alphabet[128];
    int string2_alphabet[128];

    for (int i = 0; i < 128; ++i) {
        string1_alphabet[i] = 0;
        string2_alphabet[i] = 0;
    }

    for (int i = 0; i < string1->length; ++i) {
        if (string1_alphabet[string1->string[i]] == 0) {
            string1_alphabet[string1->string[i]] = 1;
        }
    }

    for (int i = 0; i < string2->length; ++i) {
        if (string2_alphabet[string2->string[i]] == 0) {
            string2_alphabet[string2->string[i]] = 1;
        }
    }

    my_string *result = create_my_string("");
    for (char i = 1; i != 0; ++i) {
        if (string1_alphabet[i] == 1 && string2_alphabet[i] == 1) {
            add_char(result, i);
        }
    }
    if (result->length == 0) {
        char *message = "WARNING: Нет общих символов\n";
        set_string(result, message, (int) strlen(message));
    }
    return result;
}


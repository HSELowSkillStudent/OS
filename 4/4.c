#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include "my_string.c"


const int BUFFER_SIZE = 1024;

int big_parent_process(int fd[], char *input_file, char buffer[]);
int big_child_process(int fd[], char *output_file, char buffer[]);
int small_parent(int fd[], my_string *result_string);
int small_child(int fd[], char *output_file, char buffer[]);

int big_parent_process(int fd[], char *input_file, char buffer[]) {
    // Первый процесс
    if (close(fd[0]) < 0) {
        printf("Parent can\'t close reading end of pipe\n");
        return 0;
    }

    int input_fd = open(input_file, O_RDONLY);
    if (input_fd < 0) {
        printf("Parent can\'t open file %s for reading", input_file);
        return 0;
    }
    size_t read_bytes;
    while ((read_bytes = read(input_fd, buffer, BUFFER_SIZE)) > 0) {
        if (write(fd[1], buffer, read_bytes) != read_bytes) {
            printf("Parent can\'t write all bytes to pipe\n");
            return 0;
        }
    }

    if (close(fd[1]) < 0) {
        printf("Parent can\'t close writing end of pipe\n");
        return 0;
    }

    if (close(input_fd) < 0) {
        printf("Parent can\'t close file %s\n", input_file);
        return 0;
    }
    return 1;
}

int big_child_process(int fd[], char *output_file, char buffer[]) {
    // Второй процесс
    if (close(fd[1]) < 0) {
        printf("Child can\'t close writing end of pipe\n");
        return 0;
    }

    my_string *first_line = create_my_string("");
    my_string *second_line = create_my_string("");
    size_t read_bytes;
    int need_to_add_to_first_line = 1, exit_flag = 0;
    while ((read_bytes = read(fd[0], buffer, BUFFER_SIZE)) > 0) {
        for (int i = 0; i < read_bytes; ++i) {
            if (buffer[i] == '\n' && need_to_add_to_first_line) {
                need_to_add_to_first_line = 0;
                continue;
            } else if (buffer[i] == '\n' && !need_to_add_to_first_line) {
                exit_flag = 1;
                continue;
            }
            if (!exit_flag) {
                if (need_to_add_to_first_line) {
                    add_char(first_line, buffer[i]);
                } else {
                    add_char(second_line, buffer[i]);
                }
            }
        }
    }

    my_string *result_string = combining_strings(first_line, second_line);

    delete_string(first_line);
    delete_string(second_line);

    if (close(fd[0]) < 0) {
        printf("Child can\'t close reading end of pipe\n");
        return 0;
    }

    pipe(fd);

    int result = fork();
    if (result < 0) {
        printf("Can\'t fork child\n");
        return 0;
    } else if (result > 0) {  // Подродитель
        if (!small_parent(fd, result_string)) {
            return 0;
        }
        printf("Child exit\n");
        return 1;
    } else {  // Подребенок
        if (!small_child(fd, output_file, buffer)) {
            return 0;
        }
        printf("Child-child exit\n");
        return 1;
    }
}

int small_parent(int fd[], my_string *result_string) {
    // Продолжение второго процесса
    if (close(fd[0]) < 0) {
        printf("Parent can\'t close reading end of pipe\n");
        return 0;
    }

    if (write(fd[1], result_string->string, result_string->length) != result_string->length) {
        printf("Parent can\'t write all bytes to pipe\n");
        return 0;
    }

    if (close(fd[1]) < 0) {
        printf("Parent can\'t close writing end of pipe\n");
        return 0;
    }
    printf("Child-parent exit\n");
    return 1;
}

int small_child(int fd[], char *output_file, char buffer[]) {
    // Третий процесс
    if (close(fd[1]) < 0) {
        printf("Child can\'t close writing end of pipe\n");
        return 0;
    }

    int output_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (output_fd < 0) {
        printf("Child can\'t open file %s for writing", output_file);
        return 0;
    }

    size_t read_bytes;
    while ((read_bytes = read(fd[0], buffer, BUFFER_SIZE)) > 0) {
        if (write(output_fd, buffer, read_bytes) != read_bytes) {
            printf("Child can\'t write all bytes to file\n");
            return 0;
        }
    }
    return 1;
}

int main(int argc, char *argv[]) {
    umask(0);
    if (argc == 2) {
        if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
            printf("arguments - <input_file> <output_file>\n");
            return 0;
        }
    }

    if (argc != 3) {
        printf("Wrong number of arguments\n");
        exit(-1);
    }

    char *input_file = argv[1];
    char *output_file = argv[2];

    int fd[2];
    char buffer[BUFFER_SIZE];
    pipe(fd);

    int result = fork();  // разделяем процесс
    if (result < 0) {
        printf("Can\'t fork child\n");
        exit(-1);
    } else if (result > 0) {  // Родитель
        if (!big_parent_process(fd, input_file, buffer)) {
            exit(-1);
        }
        printf("Parent exit\n");
    } else {  // Ребенок
        if (!big_child_process(fd, output_file, buffer)) {
            exit(-1);
        }
        printf("Big child exit\n");
    }
    return 0;
}


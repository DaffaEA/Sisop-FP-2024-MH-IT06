#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h> // Include stdbool.h for bool type
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define USER_FILE "./discorit/users.csv" // Assuming you have a users.csv file with user data

bool login_user(const char *username, const char *password) {
    FILE *file = fopen(USER_FILE, "r");
    if (!file) {
        printf("Error opening user file.\n");
        return false;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char stored_username[100], stored_password[100], stored_role[10];
        sscanf(line, "%*d,%[^,],%[^,],%s", stored_username, stored_password, stored_role);
        if (strcmp(stored_username, username) == 0) {
            fclose(file);
            if (strcmp(stored_password, password) == 0) {
                printf("%s logged in successfully\n", username);
                return true;
            } else {
                printf("Incorrect password\n");
                return false;
            }
        }
    }

    fclose(file);
    printf("Username not found\n");
    return false;
}


void see_chat(const char *channelname, const char *roomname) {
    char path[256];
    snprintf(path, sizeof(path), "./discorit/%s/%s/chat.csv", channelname, roomname);
    FILE *chat_file = fopen(path, "r");
    if (!chat_file) {
        printf("Error opening chat file.\n");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), chat_file)) {
        printf("%s", line);
    }

    fclose(chat_file);
}

int main(int argc, char const *argv[]) {
    if (argc < 2) {
        printf("Usage: ./discorit <command> <args>\n");
        return 1;
    }

    if (strcmp(argv[1], "LOGIN") == 0 && argc == 5 && strcmp(argv[3], "-p") == 0) {
        const char *username = argv[2];
        const char *password = argv[4];
        if (login_user(username, password)) {
            // Successfully logged in, continue with chat functionality
            long last_pos = 0; // To keep track of last read position in chat file

            while (1) {
                char input[100];
                printf("[%s] ", username);
                fgets(input, sizeof(input), stdin);
                char *firstword = strtok(input, " ");
                char *channelname = strtok(NULL, " ");
                char *thirdword = strtok(NULL, " ");
                char *roomname = strtok(NULL, "\n");
                if(strcmp(firstword, "-channel") == 0 && strcmp(thirdword, "-room") == 0) {
                while(1){
                        see_chat(channelname, roomname);
                        sleep(1);
                        system("clear");
                    }
                } else {
                    printf("Invalid command\n");
                }
                
            }
        }
    }

    return 0;
}

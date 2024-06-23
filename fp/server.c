#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include <stdbool.h>   
#include <sys/stat.h>
#include <sys/types.h>
#include <asm-generic/socket.h>


#define CHANNEL_FILE "./discorit/channels.csv"
#define USER_FILE "./discorit/users.csv"
#define PORT 8080


void list_user(int client_socket) {
    FILE *file = fopen(USER_FILE, "r");
    if (!file) {
        char *error_message = "Error opening user file.\n";
        send(client_socket, error_message, strlen(error_message), 0);
        return;
    }

    char line[256];
    char user_list[1024] = "";

    while (fgets(line, sizeof(line), file)) {
        char stored_username[100], stored_password[100], stored_role[10];
        int id;
        sscanf(line, "%d,%[^,],%s,%s", &id, stored_username, stored_password, stored_role);
        strcat(user_list, stored_username);
        strcat(user_list, " ");
    }

    fclose(file);
    send(client_socket, user_list, strlen(user_list), 0);
}



void edit_user_name(const char *old_name, const char *new_name) {
    FILE *file = fopen(USER_FILE, "r");
    FILE *temp = fopen("temp.csv", "w");
    if (!file || !temp) {
        printf("Error opening user file.\n");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char stored_username[100], stored_password[100], stored_role[10];
        int id;
        sscanf(line, "%d,%[^,],%s,%s", &id, stored_username, stored_password, stored_role);
        if (strcmp(stored_username, old_name) == 0) {
            fprintf(temp, "%d,%s,%s,%s\n", id, new_name, stored_password, stored_role);
        } else {
            fprintf(temp, "%d,%s,%s,%s\n", id, stored_username, stored_password, stored_role);
        }
    }

    fclose(file);
    fclose(temp);
    remove(USER_FILE);
    rename("temp.csv", USER_FILE);
    printf("User name changed to %s\n", new_name);
}

void edit_user_key(const char *username, const char *new_key) {
    FILE *file = fopen(USER_FILE, "r");
    FILE *temp = fopen("temp.csv", "w");
    if (!file || !temp) {
        printf("Error opening user file.\n");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char stored_username[100], stored_password[100], stored_role[10];
        int id;
        sscanf(line, "%d,%[^,],%s,%s", &id, stored_username, stored_password, stored_role);
        if (strcmp(stored_username, username) == 0) {
            fprintf(temp, "%d,%s,%s,%s\n", id, stored_username, new_key, stored_role);
        } else {
            fprintf(temp, "%d,%s,%s,%s\n", id, stored_username, stored_password, stored_role);
        }
    }

    fclose(file);
    fclose(temp);
    remove(USER_FILE);
    rename("temp.csv", USER_FILE);
    printf("User key changed\n");
}

void delete_user(const char *username) {
    FILE *file = fopen(USER_FILE, "r");
    FILE *temp = fopen("temp.csv", "w");
    if (!file || !temp) {
        printf("Error opening user file.\n");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char stored_username[100], stored_password[100], stored_role[10];
        int id;
        sscanf(line, "%d,%[^,],%s,%s", &id, stored_username, stored_password, stored_role);
        if (strcmp(stored_username, username) == 0) {
            printf("User %s deleted\n", username);
            continue;
        }
        fprintf(temp, "%d,%s,%s,%s\n", id, stored_username, stored_password, stored_role);
    }

    fclose(file);
    fclose(temp);
    remove(USER_FILE);
    rename("temp.csv", USER_FILE);
}


bool channel_exists(const char *channelname) {
    FILE *file = fopen(CHANNEL_FILE, "r");
    if (!file) return false;

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char stored_channelname[100];
        sscanf(line, "%*d,%[^,]", stored_channelname);
        if (strcmp(stored_channelname, channelname) == 0) {
            fclose(file);
            return true;
        }
    }

    fclose(file);
    return false;
}

int get_next_id_channel() {
    FILE *file = fopen(CHANNEL_FILE, "r");
    if (!file) {
        return 1; // If file doesn't exist, start from ID 1
    }

    int max_id = 0;
    char line[256];

    while (fgets(line, sizeof(line), file)) {
        int id;
        sscanf(line, "%d", &id);
        if (id > max_id) {
            max_id = id;
        }
    }

    fclose(file);
    return max_id + 1;
}

void editchannelname(const char *old_name, const char *new_name) {
    if (channel_exists(new_name)) {
        printf("Channel %s already exists\n", new_name);
        return;
    }

    FILE *file = fopen(CHANNEL_FILE, "r");
    FILE *temp = fopen("temp.csv", "w");
    if (!file || !temp) {
        printf("Error opening channel file.\n");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char stored_channelname[100], stored_key[100];
        int id;
        sscanf(line, "%d,%[^,],%s", &id, stored_channelname, stored_key);
        if (strcmp(stored_channelname, old_name) == 0) {
            fprintf(temp, "%d,%s,%s\n", id, new_name, stored_key);
        } else {
            fprintf(temp, "%d,%s,%s\n", id, stored_channelname, stored_key);
        }
    }

    fclose(file);
    fclose(temp);
    remove(CHANNEL_FILE);
    rename("temp.csv", CHANNEL_FILE);
    printf("Channel name changed to %s\n", new_name);

    char old_path[256], new_path[256];
    snprintf(old_path, sizeof(old_path), "./discorit/%s", old_name);
    snprintf(new_path, sizeof(new_path), "./discorit/%s", new_name);
    rename(old_path, new_path);
}

void delete_directory(const char *path) {
    DIR *d = opendir(path);
    if (!d) return;

    struct dirent *entry;
    char file_path[512];
    while ((entry = readdir(d)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
        snprintf(file_path, sizeof(file_path), "%s/%s", path, entry->d_name);
        struct stat statbuf;
        if (stat(file_path, &statbuf) == 0 && S_ISDIR(statbuf.st_mode)) {
            delete_directory(file_path);
        } else {
            remove(file_path);
        }
    }
    closedir(d);
    rmdir(path);
}

void deletechannel(const char *channelname) {
    FILE *file = fopen(CHANNEL_FILE, "r");
    FILE *temp = fopen("temp.csv", "w");
    if (!file || !temp) {
        printf("Error opening channel file.\n");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char stored_channelname[100], stored_key[100];
        int id;
        sscanf(line, "%d,%[^,],%s", &id, stored_channelname, stored_key);
        if (strcmp(stored_channelname, channelname) == 0) {
            printf("Channel %s deleted\n", channelname);
            continue;
        }
        fprintf(temp, "%d,%s,%s\n", id, stored_channelname, stored_key);
    }

    fclose(file);
    fclose(temp);
    remove(CHANNEL_FILE);
    rename("temp.csv", CHANNEL_FILE);

    char path[256];
    snprintf(path, sizeof(path), "./discorit/%s", channelname);
    delete_directory(path);
}


int main(int argc, char const *argv[]) {
    while (1) {
        int server_fd, new_socket, valread;
        struct sockaddr_in address;
        int opt = 1;
        int addrlen = sizeof(address);
        char buffer[1024] = {0};

        if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
            perror("socket failed");
            exit(EXIT_FAILURE);
        }

        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
            perror("setsockopt");
            exit(EXIT_FAILURE);
        }

        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(PORT);

        if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
            perror("bind failed");
            exit(EXIT_FAILURE);
        }

        if (listen(server_fd, 3) < 0) {
            perror("listen");
            exit(EXIT_FAILURE);
        }

        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        valread = read(new_socket, buffer, 1024);
        printf("Received: %s\n", buffer);

        if (buffer[strlen(buffer) - 1] == '\n')
            buffer[strlen(buffer) - 1] = '\0';

        char *command = strtok(buffer, " ");
        if (command == NULL) {
            printf("Invalid command\n");
            close(new_socket);
            continue;
        }

        if (strcmp(command, "LIST") == 0) {
            char *type = strtok(NULL, " ");
            if (type == NULL) {
                printf("Invalid command\n");
            } else if (strcmp(type, "USER") == 0) {
                list_user(new_socket);
            } else {
                printf("Invalid command\n");
            }
        } else if (strcmp(command, "EDIT") == 0) {
            char *target = strtok(NULL, " ");
            char *name = strtok(NULL, " ");
            char *flag = strtok(NULL, " ");
            char *new_value = strtok(NULL, "");

            if (target == NULL || name == NULL || flag == NULL || new_value == NULL) {
                printf("Invalid command\n");
            } else if (strcmp(target, "WHERE") == 0) {
                if (strcmp(flag, "-u") == 0) {
                    edit_user_name(name, new_value);
                } else if (strcmp(flag, "-p") == 0) {
                    edit_user_key(name, new_value);
                } else {
                    printf("Invalid flag\n");
                }
            } else if (strcmp(target, "CHANNEL") == 0) {
                if (strcmp(flag, "TO") == 0) {
                    editchannelname(name, new_value);
                } else {
                    printf("Invalid flag\n");
                }
            } else {
                printf("Invalid target\n");
            }
        } else if (strcmp(command, "DEL") == 0) {
            char *target = strtok(NULL, " ");
            if (target == NULL) {
                printf("Invalid command\n");
            }else if (strcmp(target, "CHANNEL") == 0) {
                char *channel_name = strtok(NULL, " ");
                if (channel_name == NULL) {
                    printf("Invalid command\n");
                } else {
                    deletechannel(channel_name);
                }
            } else {
                printf("Invalid target\n");
            }
        } else if (strcmp(command, "REMOVE") == 0) {
            char *target = strtok(NULL, " ");
            delete_user(target);
        } else if (strcmp(command, "EXIT") == 0) {
            break;
        } else {
            printf("Invalid command\n");
        }
        
       
        memset(buffer, 0, sizeof(buffer));
        close(new_socket);
    }

    return 0;
}

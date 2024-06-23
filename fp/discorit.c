#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

#define CHANNEL_FILE "./discorit/channels.csv"
#define USER_FILE "./discorit/users.csv"
#define PORT 8080

void list_channel() {
    FILE *file = fopen(CHANNEL_FILE, "r");
    if (!file) {
        printf("Error opening channel file.\n");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char channelname[100], key[100];
        int id;
        sscanf(line, "%d,%[^,],%s", &id, channelname, key);
        printf("%s  ", channelname);
    }

    fclose(file);
}



/*
check global role
*/
int check_role_userchannel(const char *channelname, const char *username) {
    char path[256];
    snprintf(path, sizeof(path), "./discorit/%s/auth.csv", channelname);
    FILE *auth_file = fopen(path, "r");
    if (!auth_file) {
        printf("Error opening auth file.\n");
        return 1;
    }

    char line[256];
    while (fgets(line, sizeof(line), auth_file)) {
        char stored_username[100], stored_role[10];
        int id;
        sscanf(line, "%d,%[^,],%s", &id, stored_username, stored_role);
        if (strcmp(stored_username, username) == 0) {
            if (strcmp(stored_role, "ADMIN") == 0) {
                fclose(auth_file);
                return 0;
            } else if (strcmp(stored_role, "USER") == 0) {
                fclose(auth_file);
                return 1;
            } else if (strcmp(stored_role, "BANNED") == 0) {
                fclose(auth_file);
                return 2;
            }
        }
    }

    fclose(auth_file);
    return 3;
}

int check_role_userfile(const char *username) {
    FILE *file = fopen(USER_FILE, "r");
    if (!file) {
        printf("Error opening user file.\n");
        return 1;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char stored_username[100], stored_password[100], stored_role[10];
        int id;
        sscanf(line, "%d,%[^,],%s,%s", &id, stored_username, stored_password, stored_role);
        if (strcmp(stored_username, username) == 0) {
            if (strcmp(stored_role, "ROOT") == 0) {
                fclose(file);
                return 0;
            } else if (strcmp(stored_role, "USER") == 0) {
                fclose(file);
                return 1;
            }
        }
    }

    fclose(file);
    return 2;
}
/*
get date
*/

char *get_current_date() {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char* current_date = (char*) malloc(20);
    strftime(current_date, 20, "%Y-%m-%d %H:%M:%S", t);
    return current_date;
}

/*
send message
*/

int getnextid(const char *channelname, const char *roomname) {
    char path[256];
    snprintf(path, sizeof(path), "./discorit/%s/%s/chat.csv", channelname, roomname);
    FILE *chat_file = fopen(path, "r");
    if (!chat_file) {
        printf("Error opening chat file.\n");
        return 1; // Start from ID 1 if the file doesn't exist
    }

    int max_id = 0;
    char line[256];
    while (fgets(line, sizeof(line), chat_file)) {
        int id;
        sscanf(line, "[%*[^][]][%d]", &id);
        if (id > max_id) {
            max_id = id;
        }
    }

    fclose(chat_file);
    return max_id + 1;
}

void send_message(const char *channelname, const char *roomname, const char *username, const char *message) {
    char path[256];
    snprintf(path, sizeof(path), "./discorit/%s/%s/chat.csv", channelname, roomname);
    FILE *chat_file = fopen(path, "a+");
    if (chat_file) {
        char *current_date = get_current_date();
        fprintf(chat_file, "[%s][%d][%s] \"%s\"\n", current_date, getnextid(channelname, roomname), username, message);
        free(current_date);
        fclose(chat_file);
    } else {
        printf("Error opening chat file.\n");
    }
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

void edit_message(const char *username, const char *channelname, const char *roomname, int id, const char *new_message) {
    char path[256];
    snprintf(path, sizeof(path), "./discorit/%s/%s/chat.csv", channelname, roomname);
    FILE *chat_file = fopen(path, "r");
    FILE *temp = fopen("temp.csv", "w");
    if (!chat_file || !temp) {
        printf("Error opening chat file.\n");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), chat_file)) {
        int stored_id;
        sscanf(line, "[%*[^][]][%d]", &stored_id);
        if (stored_id == id) {
            char *current_date = get_current_date();
            fprintf(temp, "[%s][%d][%s] \"%s\"\n", current_date, id, username, new_message);
            free(current_date);
        } else {
            fprintf(temp, "%s", line);
        }
    }

    fclose(chat_file);
    fclose(temp);
    remove(path);
    rename("temp.csv", path);
}

void delete_message(const char *channelname, const char *roomname, int id) {
    char path[256];
    snprintf(path, sizeof(path), "./discorit/%s/%s/chat.csv", channelname, roomname);
    FILE *chat_file = fopen(path, "r");
    FILE *temp = fopen("temp.csv", "w");
    if (!chat_file || !temp) {
        printf("Error opening chat file.\n");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), chat_file)) {
        int stored_id;
        sscanf(line, "[%*[^][]][%d]", &stored_id);
        if (stored_id != id) {
            fprintf(temp, "%s", line);
        }
    }

    fclose(chat_file);
    fclose(temp);
    remove(path);
    rename("temp.csv", path);
}

/*
JOIN CHANNEL AND ROOM
*/
void join_room(const char *channelname, const char *roomname) {
    char path[256];
    snprintf(path, sizeof(path), "./discorit/%s/%s", channelname, roomname);
    if (access(path, F_OK) != -1) {
        printf("Joined room %s\n", roomname);
    } else {
        printf("Room not found\n");
    }
}

int get_next_id_auth(const char *channelname) {
    char path[256];
    snprintf(path, sizeof(path), "./discorit/%s/auth.csv", channelname);
    FILE *auth_file = fopen(path, "r");
    if (!auth_file) {
        return 1; // Start from ID 1 if the file doesn't exist
    }

    int max_id = 0;
    char line[256];
    while (fgets(line, sizeof(line), auth_file)) {
        int id;
        sscanf(line, "%d", &id);
        if (id > max_id) {
            max_id = id;
        }
    }

    fclose(auth_file);
    return max_id + 1;
}

bool no_same_name_auth(const char *username){
    FILE *file = fopen(USER_FILE, "r");
    if (!file) return false;

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char stored_username[100];
        sscanf(line, "%*d,%[^,]", stored_username);
        if (strcmp(stored_username, username) == 0) {
            fclose(file);
            return false;
        }
    }

    fclose(file);
    return true;

}

void join_channel(const char *username, const char *channelname, const char *key) {
    FILE *file = fopen(CHANNEL_FILE, "r");
    if (!file) {
        printf("Error opening channel file.\n");
        return;
    }

    char line[256];
    int channel_found = 0;
    while (fgets(line, sizeof(line), file)) {
        char stored_channelname[100], stored_key[100];
        int id;
        sscanf(line, "%d,%[^,],%s", &id, stored_channelname, stored_key);

        if (strcmp(stored_channelname, channelname) == 0) {
            channel_found = 1;

            if (check_role_userchannel(channelname, username) == 2) {
                printf("You are banned from this channel\n");
                fclose(file);
                return;
            }

            if (strcmp(stored_key, key) == 0) {
                printf("Joined channel %s\n", channelname);

                if (no_same_name_auth(username) == true) {
                    char path[256];
                    snprintf(path, sizeof(path), "./discorit/%s/auth.csv", channelname);
                    FILE *auth_file = fopen(path, "a+");
                    if (auth_file) {
                        fprintf(auth_file, "%d,%s,%s\n", get_next_id_auth(stored_channelname), username, "USER");
                        fclose(auth_file);
                    }
                    else {
                        printf("Error opening auth file.\n");
                    }
                }
                fclose(file);
                return;
            } else {
                printf("Wrong key\n");
                fclose(file);
                return;
            }
        }
    }

    if (!channel_found) {
        printf("Channel not found\n");
    }

    fclose(file);
}




/*
LIST ROOM
*/


void list_room(const char *channelname) {
    char path[256];
    snprintf(path, sizeof(path), "./discorit/%s", channelname);
    DIR *dir = opendir(path);
    if (!dir) {
        printf("Error opening directory.\n");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            printf("%s\n", entry->d_name);
        }
    }

    closedir(dir);
}

/*
CREATE, EDIT, DELETE ROOM
*/
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

void create_channel(const char *channel_name, const char *key, const char *username) {
    if (!channel_exists(channel_name)) {
        FILE *file = fopen(CHANNEL_FILE, "a+");
        if (!file) {
            printf("Error opening channel file.\n");
            return;
        }

        int id = get_next_id_channel();

        fprintf(file, "%d,%s,%s\n", id, channel_name, key);
        fclose(file);
        printf("Channel %s created\n", channel_name);

        char path[256];
        snprintf(path, sizeof(path), "./discorit/%s", channel_name);
        mkdir(path, 0777);
        snprintf(path, sizeof(path), "./discorit/%s/auth.csv", channel_name);
        FILE *auth_file = fopen(path, "w");
        if (auth_file) {
            fprintf(auth_file, "1,%s,%s\n", username, "ADMIN");
            fclose(auth_file);
        }
    } else {
        printf("Channel %s already exists\n", channel_name);
    }
}

void create_room(const char *channelname, const char *roomname) {
    char path[256];
    snprintf(path, sizeof(path), "./discorit/%s/%s", channelname, roomname);
    mkdir(path, 0777);

    snprintf(path, sizeof(path), "./discorit/%s/%s/chat.csv", channelname, roomname);
    FILE *chat_file = fopen(path, "w");
    if (chat_file) {
        fclose(chat_file);
    }
}

void edit_room(const char *channelname, const char *roomname, const char *newname) {
    char old_path[256], new_path[256];
    snprintf(old_path, sizeof(old_path), "./discorit/%s/%s", channelname, roomname);
    snprintf(new_path, sizeof(new_path), "./discorit/%s/%s", channelname, newname);
    rename(old_path, new_path);
}

void delete_room(const char *channelname, const char *roomname) {
    char path[256];
    snprintf(path, sizeof(path), "./discorit/%s/%s", channelname, roomname);
    remove(path);

}

/*
LIST,EDIT, DELETE USER
*/
void ban_user(const char *channelname, const char *username) {
    char path[256];
    snprintf(path, sizeof(path), "./discorit/%s/auth.csv", channelname);
    FILE *auth_file = fopen(path, "r");
    FILE *temp = fopen("temp.csv", "w");
    if (!auth_file || !temp) {
        printf("Error opening auth file.\n");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), auth_file)) {
        char stored_username[100], stored_role[10];
        int id;
        sscanf(line, "%d,%[^,],%s", &id, stored_username, stored_role);
        if (strcmp(stored_username, username) == 0) {
            fprintf(temp, "%d,%s,%s\n", id, stored_username, "BANNED");
        } else {
            fprintf(temp, "%d,%s,%s\n", id, stored_username, stored_role);
        }
    }

    fclose(auth_file);
    fclose(temp);
    remove(path);
    rename("temp.csv", path);
    printf("User %s banned\n", username);
}

void unban_user(const char *channelname, const char *username) {
    char path[256];
    snprintf(path, sizeof(path), "./discorit/%s/auth.csv", channelname);
    FILE *auth_file = fopen(path, "r");
    FILE *temp = fopen("temp.csv", "w");
    if (!auth_file || !temp) {
        printf("Error opening auth file.\n");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), auth_file)) {
        char stored_username[100], stored_role[10];
        int id;
        sscanf(line, "%d,%[^,],%s", &id, stored_username, stored_role);
        if (strcmp(stored_username, username) == 0) {
            fprintf(temp, "%d,%s,%s\n", id, stored_username, "USER");
        } else {
            fprintf(temp, "%d,%s,%s\n", id, stored_username, stored_role);
        }
    }

    fclose(auth_file);
    fclose(temp);
    remove(path);
    rename("temp.csv", path);
    printf("User %s unbanned\n", username);
}

void kick_user_fromchannel(const char *channelname, const char *username) {
    char path[256];
    snprintf(path, sizeof(path), "./discorit/%s/auth.csv", channelname);
    FILE *auth_file = fopen(path, "r");
    FILE *temp = fopen("temp.csv", "w");
    if (!auth_file || !temp) {
        printf("Error opening auth file.\n");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), auth_file)) {
        char stored_username[100], stored_role[10];
        int id;
        sscanf(line, "%d,%[^,],%s", &id, stored_username, stored_role);
        if (strcmp(stored_username, username) == 0) {
            fprintf(temp, "%d,%s,%s\n", id, stored_username, "USER");
        } else {
            fprintf(temp, "%d,%s,%s\n", id, stored_username, stored_role);
        }
    }

    fclose(auth_file);
    fclose(temp);
    remove(path);
    rename("temp.csv", path);
    printf("User %s kicked from channel\n", username);
}

/*
Edit own profile
*/

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
}
/*
AUTHORIZE, DEAUTHORIZE
*/

int get_next_id() {
    FILE *file = fopen(USER_FILE, "r");
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

bool user_exists(const char *username) {
    FILE *file = fopen(USER_FILE, "r");
    if (!file) return false;

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char stored_username[100];
        sscanf(line, "%*d,%[^,]", stored_username);
        if (strcmp(stored_username, username) == 0) {
            fclose(file);
            return true;
        }
    }

    fclose(file);
    return false;
}

void register_user(const char *username, const char *password) {
    if (!user_exists(username)) {
        FILE *file = fopen(USER_FILE, "a+");
        if (!file) {
            printf("Error opening user file.\n");
            return;
        }

        int id = get_next_id();
        bool is_root = (id == 1);

        fprintf(file, "%d,%s,%s,%s\n", id, username, password, is_root ? "ROOT" : "USER");
        fclose(file);
        printf("%s registered successfully\n", username);
    } else {
        printf("%s is already registered\n", username);
    }
}

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

// Handle command
void handle_command(const char *input) {
    struct sockaddr_in address;
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return;
    }

    send(sock, input, strlen(input), 0);
    valread = read(sock, buffer, 1024);
    printf("%s\n", buffer);
    close(sock);
}

int main(int argc, char const *argv[]) {
    if (argc < 2) {
        printf("Usage: ./discorit <command> <args>\n");
        return 1;
    }

    const char *command = argv[1];

    if (strcmp(command, "REGISTER") == 0 && argc == 5 && strcmp(argv[3], "-p") == 0) {
        const char *username = argv[2];
        const char *password = argv[4];
        register_user(username, password);
        return 0;
    } else if (strcmp(command, "LOGIN") == 0 && argc == 5 && strcmp(argv[3], "-p") == 0) {
        const char *username = argv[2];
        const char *password = argv[4];
        if (login_user(username, password)) {
            char channelname[100] = "";
            char roomname[100] = "";

            while (1) {
                char input[100];
                printf("[%s] ", username);
                fgets(input, sizeof(input), stdin);
                input[strcspn(input, "\n")] = 0; // Remove trailing newline

                char original_input[100];
                strcpy(original_input, input); // Make a copy of the input

                char *cmd = strtok(input, " ");

                if (strcmp(cmd, "JOIN") == 0 && strlen(channelname) == 0) {
                    char *channel = strtok(NULL, " ");
                    printf("key: ");
                    char key[100];
                    scanf("%s", key);
                    getchar(); // Consume the newline character left by scanf
                    join_channel(username, channel, key);
                    strncpy(channelname, channel, sizeof(channelname));
                    while (1) {
                        printf("[%s/%s] ", username, channelname);
                        fgets(input, sizeof(input), stdin);
                        input[strcspn(input, "\n")] = 0;
                        strcpy(original_input, input); // Make a copy of the input

                        char *cmd = strtok(input, " ");
                        if (strcmp(cmd, "JOIN") == 0 && strlen(channelname) > 0) {
                            char *room = strtok(NULL, " ");
                            join_room(channelname, room);
                            strncpy(roomname, room, sizeof(roomname));
                            while (1) {
                                printf("[%s/%s/%s] ", username, channelname, roomname);
                                fgets(input, sizeof(input), stdin);
                                input[strcspn(input, "\n")] = 0; // Remove trailing newline
                                strcpy(original_input, input); // Make a copy of the input

                                char *chat_cmd = strtok(input, " ");
                                if (strcmp(chat_cmd, "CHAT") == 0) {
                                    char *message = strtok(NULL, "\n");
                                    send_message(channelname, roomname, username, message);
                                } else if (strcmp(chat_cmd, "SEE") == 0) {
                                    see_chat(channelname, roomname);
                                } else if (strcmp(chat_cmd, "EDIT") == 0) {
                                    char *str = strtok(NULL, " ");
                                    if (strcmp(str, "CHAT") == 0) {
                                        int id = atoi(strtok(NULL, " "));
                                        char *new_message = strtok(NULL, "\n");
                                        edit_message(username, channelname, roomname, id, new_message);
                                    }
                                } else if (strcmp(chat_cmd, "DEL") == 0) {
                                    char *str = strtok(NULL, " ");
                                    if (strcmp(str, "CHAT") == 0) {
                                        int id = atoi(strtok(NULL, " "));
                                        delete_message(channelname, roomname, id);
                                    }
                                } else if (strcmp(chat_cmd, "EXIT") == 0) {
                                    roomname[0] = '\0'; // Exit room
                                    break;
                                }
                            }
                        } else if (strcmp(cmd, "LIST") == 0 && strlen(channelname) > 0) {
                            list_room(channelname);
                        } else if(check_role_userchannel(channelname, username) == 0 ) {
                        if (strcmp(cmd, "CREATE") == 0 && strlen(channelname) > 0) {
                            char *entity = strtok(NULL, " ");
                            if (strcmp(entity, "ROOM") == 0) {
                                char *room = strtok(NULL, " ");
                                create_room(channelname, room);
                            }
                        } else if (strcmp(cmd, "EDIT") == 0 && strlen(channelname) > 0) {
                            char *entity = strtok(NULL, " ");
                            if (strcmp(entity, "ROOM") == 0) {
                                char *room = strtok(NULL, " ");
                                char *to = strtok(NULL, " ");
                                char *newname = strtok(NULL, " ");
                                if(strcmp(to, "TO") == 0){
                                    edit_room(channelname, room, newname);
                                }
                            }
                        } else if (strcmp(cmd, "DEL") == 0 && strlen(channelname) > 0) {
                            char *entity = strtok(NULL, " ");
                            if (strcmp(entity, "ROOM") == 0) {
                                char *room = strtok(NULL, " ");
                                delete_room(channelname, room);
                            }
                        } else if (strcmp(cmd, "BAN") == 0 && strlen(channelname) > 0) {
                            char *entity = strtok(NULL, " ");
                            if (strcmp(entity, "USER") == 0) {
                                char *user = strtok(NULL, " ");
                                ban_user(channelname, user);
                            }
                        } else if (strcmp(cmd, "UNBAN") == 0 && strlen(channelname) > 0) {
                            char *entity = strtok(NULL, " ");
                            if (strcmp(entity, "USER") == 0) {
                                char *user = strtok(NULL, " ");
                                unban_user(channelname, user);
                            }
                        } else if (strcmp(cmd, "KICK") == 0 && strlen(channelname) > 0) {
                            char *entity = strtok(NULL, " ");
                            if (strcmp(entity, "USER") == 0) {
                                char *user = strtok(NULL, " ");
                                kick_user_fromchannel(channelname, user);
                            }
                        } } else if (strcmp(cmd, "EXIT") == 0) {
                            channelname[0] = '\0'; // Exit channel
                            break;
                        }
                    }
                } else if (strcmp(cmd, "CREATE") == 0 && strlen(channelname) == 0) {
                    char *entity = strtok(NULL, " ");
                    if (strcmp(entity, "CHANNEL") == 0) {
                        char *channel = strtok(NULL, " ");
                        char *parameter = strtok(NULL, " ");
                        char *key = strtok(NULL, " ");
                        create_channel(channel, key, username);
                    }
                } else if (strcmp(cmd, "EXIT") == 0) {
                    if (strlen(roomname) > 0) {
                        roomname[0] = '\0'; // Exit room
                    } else if (strlen(channelname) > 0) {
                        channelname[0] = '\0'; // Exit channel
                    } else {
                        break; // Exit application
                    }
                } else if (strcmp(original_input, "LIST CHANNEL") == 0) {
                    list_channel();
                } else if (strcmp(cmd, "EDIT") == 0 && strlen(channelname) == 0) {
                    char *entity = strtok(NULL, " ");
                    if (strcmp(entity, "PROFILE") == 0) {
                        char *option = strtok(NULL, " ");
                        if (strcmp(option, "-u") == 0) {
                            char *new_name = strtok(NULL, " ");
                            edit_user_name(username, new_name);
                        } else if (strcmp(option, "-p") == 0) {
                            char *new_password = strtok(NULL, " ");
                            edit_user_key(username, new_password);
                        }
                    } else {
                        handle_command(original_input);
                    }
                }else{
                        handle_command(original_input);
                }
            }
        }
    } else {
        printf("Invalid command or arguments\n");
    }

    return 0;
}

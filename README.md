# Sisop-FP-2024-MH-IT29

## Anggota Kelompok:
- Daffa Rajendra Priyatama 5027231009
- Nicholas Arya Krisnugroho Rerangin 5027231058
- Johanes Edward Nathanael 5027231067
  
# Pendahuluan
Dalam final project praktikum DiscorIT, kami diminta untuk menyelesaikan implementasi sebuah sistem chat berbasis socket yang terdiri dari tiga file utama yaitu discorit.c (client untuk mengirim request), server.c (server yang menerima dan merespon request), dan monitor.c (client untuk menampilkan chat secara real-time).

Program ini memungkinkan user untuk berkomunikasi secara real-time melalui channel dan room yang dapat dikelola oleh user dengan peran tertentu. User harus melakukan autentikasi sebelum dapat mengakses fitur-fitur yang ada. Keamanan juga dijamin dengan menggunakan bcrypt untuk enkripsi password dan key channel.

### Tree
![alt text](https://media.discordapp.net/attachments/1256171847284953088/1256172451142963303/Screenshot_1970.png?ex=667fcd42&is=667e7bc2&hm=d495f45000b7043dbf785212aaff8bc655e57391623ec388db5340a0f382f399&=&format=webp&quality=lossless&width=718&height=656)

### Keterangan Setiap File
![alt text](https://media.discordapp.net/attachments/1256171847284953088/1256172451428171947/Screenshot_1971.png?ex=667fcd42&is=667e7bc2&hm=a6bea88f079dc47b94bb9ee4fe0ad4180a1f7d29cf8c5219be8fb77a16eef59e&=&format=webp&quality=lossless&width=584&height=656)

### Start Server
Fungsi `start_server` bertanggung jawab untuk menginisialisasi server dengan beberapa langkah penting. Pertama, fungsi ini membuat sebuah socket yang akan digunakan untuk komunikasi jaringan. Jika pembuatan socket gagal, program akan menampilkan pesan kesalahan dan keluar. Selanjutnya, socket tersebut diikat (bind) ke alamat dan port tertentu, memungkinkan server untuk menerima koneksi dari klien melalui port tersebut. Jika proses pengikatan gagal, program akan menampilkan pesan kesalahan dan keluar. Setelah socket berhasil diikat, server akan mendengarkan koneksi masuk dari klien, siap dalam menerima sejumlah koneksi yang telah ditentukan oleh `MAX_CLIENTS`. Jika proses ini gagal, program akan menampilkan pesan kesalahan dan keluar. Sebagai tambahan, server akan menampilkan pesan debug untuk menunjukkan bahwa server telah dimulai dan mendengarkan pada port yang ditentukan.
**Kode**:
<details>
<summary><h3>Detail Kode </h3></summary>

```c
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
```
</details>

## Login/Register 
### Fungsi Register 

Fungsi ini merupakan fungsi untuk melakukan register suatu user. Pertama fungsi akan memeriksa apakah username sudah terdaftar dan membukauka file penyimpanan data pengguna. Lalu, akan mengambil ID unik untuk pengguna baru dan juga menetapkan pengguna pertama sebagai admin (ROOT). Setelah itu fungsi akan menulis data pengguna (ID, username, password, status) ke file dan akan menutup file setelah penulisan selesai.

### Fungsi Login

Fungsi ini akan membuka file penyimpanan data pengguna. Lalu akan membaca file dan cocokkan username dan password yang diberikan dengan yang ada di file. Serta akan mengembalikan nilai true jika login berhasil, false jika tidak.
<details>
<summary><h3>Detail Kode</h3></summary>

```c
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
```
</details>


## Join

### Join Channel
Fungsi Join Channel akan dimulai dengan membuka file yang menyimpan data channel serta mencari kanal yang sesuai dengan channelname. Setelah itu, akan memeriksa apakah pengguna dilarang bergabung ke channel dan mencocokan channel key. Jika kunci cocok dan pengguna tidak dilarang, tambahkan pengguna ke file otorisasi channelnya. File akan ditutup setelah operasinya selesai dan akan menampilkan pesan bergabung berhasil, salah kunci, atau channel tidak ditemukan.

**Kode**:
<details>
<summary><h3>Detail Kode</h3></summary>

```c

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
```
</details>

### Join Room
Proses Join akan dimulai dengan Buat path untuk direktori room dalam channel. Setelah path terbentuk, fungsi akan memeriksa direktori room dan menampilkan pesan berhasil bergabung atau jika tidak ada pathnya maka akan mencul pesan room tidak ditemukan. Lalu akan dilakukan fungsi untuk mengecek file otorisasi Channelnya dan mencari ID terbesar/maksimal dan mengembalikan id terbesar ditambah satu. Namun jika filenya tidak ada, maka akan dimulai dari ID 1. Fungsi ini juga meyimpan data pengguna dan mencari username yang sama. Jika username ditemukan maka akan dikembalikan dengan false dan true jika tidak ditemukan.

<details>
<summary><h3>Detail Kode</h3></summary>

```c
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

```
</details>

## Chatting

### Time Stamp pada Chat
Fungsi `get_timestamp()` adalah fungsi pendukung yang esensial dalam sistem chat karena menghasilkan timestamp yang digunakan untuk merekam waktu pengiriman chat dalam room chat. Saat digunakan dalam konteks aplikasi chat, timestamp ini bertindak sebagai penanda waktu yang menunjukkan kapan chat dikirim. Prosesnya dimulai dengan memanggil fungsi `time` yang akan mendapatkan waktu saat ini lalu akan dilakukan koversi waktu ke waktu dengan struktur `tm` local dan mengformat ke string dengan struktur "YYYY-MM-DD HH:MM". Akhirnya, akan mengembalikan dengan string yang berisi waktu yang sekarang yang telah diformat.

<details>
<summary><h3>Detail Kode</h3></summary>

```c
  char *get_current_date() {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char* current_date = (char*) malloc(20);
    strftime(current_date, 20, "%Y-%m-%d %H:%M:%S", t);
    return current_date;
}
```
</details>

### Send Chat
Proses send chat dimulai dengan pengecekan apakah user berada dalam sebuah room. Setelah pengecekan, sistem akan membuat path untuk file chat di dalam room dan membukan file chat untuk mulai melakukan chat. Lalu, Fungsi akan memanggil fungsi `timestamp` unruk mengabil waktu dan tanggal saat ini. Fungsi selanjutnya akan mendaptkan id pesan berikutnya dengan fungsi `getnextid`. Selanjutnya, Fungsi akan menulis pesan beserta tanggal, id, username ke file dan membebaskan memori yang dialokasikan sekarang.

<details>
<summary><h3>Kode Chat</h3></summary>

```c
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
```
</details>

### See Chat
Proses See Chat lebih simple, pertama fungsi akan membuat path untuk file chat ke dalam room. Lalu akan membuka file chat dan juga menampilkan setiap baris chat ke dalam file chat. Setelah selesai sistem akan menutup filenya.

<details>
<summary><h3>Detail Kode</h3></summary>

```c
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
```
</details>

### Edit Chat
Dalam fungsi ini, Sistem akan membuat path untuk file chat ke dalam roomnya. Setelah itu akan membuka file chat untuk read dan temp file untuk write sementara. Setelah itu fungsi akan memeriksa apakah file chat atau temp file gagal dibuka. Jika dibuka dengan lancar, sistem akan membaca setiap line dalam file chat dan memulai fungsi edit messagenya. Edit Message dimulai dengan mencari id yang cocok. Jika id sudah cocok, menulis pesan baru dengan waktu sekarang ke temp file. Jika tidak, maka akan menyalin baris asli. Setelah sudah melakukan edit, file chat dan temp file akan di tutup dan file chat lama akan dihapus serta digantkan dengan temp file yang berisi pesan yang baru/sudah diedit.

<details>
<summary><h3>Detail Kode</h3></summary>

```c
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

```
</details>

### Delete Chat
Fungsi `del_chat` digunakan untuk menghapus chat dalam sebuah room chat berdasarkan ID tertentu. Pertama, fungsi akan membuat path file chat di dalam room. Lalu, fungsi akan membuka file chat untuk read dan temp file untuk write. Akan ada error-checking untuk memeriksa apakah ada file yang gagal dibuka. Setelah itu, baca setiap line dalam file chat. Fungsi juga akan mengecek ID Messagenya, jika IDnya cocok makan line akan diabaikan namun jika IDnya tidak cocok makan akan menyalin line ke temp file. Setelah selesai, fungsi akan menutup file chat dan temp file serta mengganti file lama dengan yang baru dengan cara menghapus file chat lama dan menggantukannya dengan temp file yang sudah difilter.

<details>
<summary><h3>Detail Kode</h3></summary>

```c
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

```
</details>

![alt text](![image](https://github.com/DaffaEA/Sisop-FP-2024-MH-IT06/assets/142997842/3048bb4c-e024-46b6-8923-39584eafecef))

![alt text](![image](https://github.com/DaffaEA/Sisop-FP-2024-MH-IT06/assets/142997842/faf6e36f-1dde-4f2f-b6d0-60c5e3fc259a))

![alt text](![image](https://github.com/DaffaEA/Sisop-FP-2024-MH-IT06/assets/142997842/3607877c-8318-4f03-ae3f-98a39cba9729))





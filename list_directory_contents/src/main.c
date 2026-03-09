#include <dirent.h>
#include <grp.h>
#include <limits.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

struct directory;
struct file;
struct link;
struct contents;

enum link_target_type {
    LINK_TARGET_TYPE_FILE,
    LINK_TARGET_TYPE_DIRECTORY,
    LINK_TARGET_TYPE_LINK,
};

struct shared_attributes {
    char *path;
    char *name;
    unsigned short permissions;
    char *owner_str;
    unsigned long owner_id;
    char *group_str;
    unsigned long group_id;
    time_t modified_at;
};

struct file {
    char *path;
    char *name;
    unsigned short permissions;
    char *owner_str;
    unsigned long owner_id;
    char *group_str;
    unsigned long group_id;
    time_t modified_at;
    unsigned long long size;
};

struct link {
    char *path;
    char *name;
    unsigned short permissions;
    char *owner_str;
    unsigned long owner_id;
    char *group_str;
    unsigned long group_id;
    time_t modified_at;
    char *target_path;
    enum link_target_type target_type;
};

struct contents {
    unsigned short directory_count;
    unsigned short directory_capacity;
    struct directory *directories;
    unsigned short file_count;
    unsigned short file_capacity;
    struct file *files;
    unsigned short link_count;
    unsigned short link_capacity;
    struct link *links;
};

struct directory {
    char *path;
    char *name;
    unsigned short permissions;
    char *owner_str;
    unsigned long owner_id;
    char *group_str;
    unsigned long group_id;
    time_t modified_at;
    bool has_contents;
    struct contents *contents;
};

struct directory root_directory;

// utilities

char *duplicate_string(const char *src) {
    size_t len = strlen(src) + 1;
    char *dest = malloc(len);
    if (!dest)
        return NULL;
    memcpy(dest, src, len);
    return dest;
}

bool ensure_capacity(void **items, unsigned short *capacity, unsigned short required, size_t item_size) {
    if (required <= *capacity)
        return true;

    unsigned short new_capacity = (*capacity == 0) ? 16 : *capacity;
    while (new_capacity < required) {
        if (new_capacity > USHRT_MAX / 2) {
            new_capacity = USHRT_MAX;
            break;
        }
        new_capacity *= 2;
    }
    if (new_capacity < required)
        return false;

    void *new_items = realloc(*items, item_size * new_capacity);
    if (!new_items)
        return false;

    *items = new_items;
    *capacity = new_capacity;
    return true;
}

// data collection

bool shared_attributes_from_stat(struct shared_attributes *restrict shared_attributes,
    const char *restrict path,
    const struct stat *restrict statbuf) {
    shared_attributes->path = duplicate_string(path);
    if (!shared_attributes->path)
        return false;
    char *last_slash = strrchr(shared_attributes->path, '/');
    if (last_slash != NULL)
        shared_attributes->name = last_slash + 1;
    else
        shared_attributes->name = shared_attributes->path;
    shared_attributes->permissions = statbuf->st_mode & 0777;
    struct passwd *pwd = getpwuid(statbuf->st_uid);
    if (pwd != NULL) {
        shared_attributes->owner_str = duplicate_string(pwd->pw_name);
        if (!shared_attributes->owner_str)
            return false;
    }
    else
        shared_attributes->owner_id = statbuf->st_uid;
    struct group *grp = getgrgid(statbuf->st_gid);
    if (grp != NULL) {
        shared_attributes->group_str = duplicate_string(grp->gr_name);
        if (!shared_attributes->group_str)
            return false;
    }
    else
        shared_attributes->group_id = statbuf->st_gid;
    shared_attributes->modified_at = statbuf->st_mtime;
    return true;
}

bool shared_attributes(struct shared_attributes *restrict shared_attributes, const char *restrict path) {
    struct stat statbuf;
    if (lstat(path, &statbuf) != 0)
        return false;
    return shared_attributes_from_stat(shared_attributes, path, &statbuf);
}

bool walk_directory(struct directory *directory, const char *restrict path, bool recursive, bool is_root);

bool walk_directory(struct directory *directory, const char *restrict path, bool recursive, bool is_root) {
    struct directory *current_directory = directory;
    if (!shared_attributes((struct shared_attributes *)current_directory, path))
        return false;
    DIR *dir = opendir(path);
    if (!dir)
        return false;
    if (recursive || is_root) {
        struct contents *contents = calloc(1, sizeof(struct contents));
        if (!contents) {
            closedir(dir);
            return false;
        }
        current_directory->contents = contents;
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            char entry_path[PATH_MAX];
            size_t path_len_src = strlen(path);
            bool has_trailing_slash = (path_len_src > 0 && path[path_len_src - 1] == '/');
            int path_len = snprintf(entry_path, sizeof entry_path, "%s%s%s", path, has_trailing_slash ? "" : "/", entry->d_name);
            if (path_len <= 0 || path_len >= (int)sizeof entry_path)
                continue;

            struct stat statbuf;
            if (lstat(entry_path, &statbuf) != 0)
                continue;

            if (S_ISDIR(statbuf.st_mode)) {
                unsigned short next_count = contents->directory_count + 1;
                if (!ensure_capacity((void **)&contents->directories,
                        &contents->directory_capacity,
                        next_count,
                        sizeof(struct directory))) {
                    closedir(dir);
                    return false;
                }
                struct directory *child = &contents->directories[contents->directory_count];
                memset(child, 0, sizeof *child);
                contents->directory_count = next_count;
                if (!walk_directory(child, entry_path, recursive, false)) {
                    closedir(dir);
                    return false;
                }
                continue;
            }

            if (S_ISREG(statbuf.st_mode)) {
                unsigned short next_count = contents->file_count + 1;
                if (!ensure_capacity((void **)&contents->files,
                        &contents->file_capacity,
                        next_count,
                        sizeof(struct file))) {
                    closedir(dir);
                    return false;
                }
                struct file *child = &contents->files[contents->file_count];
                memset(child, 0, sizeof *child);
                contents->file_count = next_count;
                if (!shared_attributes_from_stat((struct shared_attributes *)child, entry_path, &statbuf)) {
                    closedir(dir);
                    return false;
                }
                child->size = statbuf.st_size;
                continue;
            }

            if (S_ISLNK(statbuf.st_mode)) {
                unsigned short next_count = contents->link_count + 1;
                if (!ensure_capacity((void **)&contents->links,
                        &contents->link_capacity,
                        next_count,
                        sizeof(struct link))) {
                    closedir(dir);
                    return false;
                }
                struct link *child = &contents->links[contents->link_count];
                memset(child, 0, sizeof *child);
                contents->link_count = next_count;
                if (!shared_attributes_from_stat((struct shared_attributes *)child, entry_path, &statbuf)) {
                    closedir(dir);
                    return false;
                }

                size_t target_buffer_size = (statbuf.st_size > 0) ? (size_t)statbuf.st_size + 1 : PATH_MAX;
                char *target_buffer = malloc(target_buffer_size);
                if (!target_buffer) {
                    closedir(dir);
                    return false;
                }
                ssize_t target_len = readlink(entry_path, target_buffer, target_buffer_size - 1);
                if (target_len >= 0) {
                    target_buffer[target_len] = '\0';
                    child->target_path = target_buffer;
                }
                else {
                    target_buffer[0] = '\0';
                    child->target_path = target_buffer;
                }

                struct stat target_statbuf;
                if (stat(entry_path, &target_statbuf) == 0) {
                    if (S_ISREG(target_statbuf.st_mode))
                        child->target_type = LINK_TARGET_TYPE_FILE;
                    else if (S_ISDIR(target_statbuf.st_mode))
                        child->target_type = LINK_TARGET_TYPE_DIRECTORY;
                    else if (S_ISLNK(target_statbuf.st_mode))
                        child->target_type = LINK_TARGET_TYPE_LINK;
                }
                continue;
            }
        }
    }
    closedir(dir);
    return true;
}

// json serialization

int json_string(char *dest, const char *src) {
    char *dest_start = dest;
    *dest++ = '"';
    while (*src) {
        switch (*src) {
        case '"':
            *dest++ = '\\';
            *dest++ = *src;
            break;
        case '\\':
            *dest++ = '\\';
            *dest++ = *src;
            break;
        case '\b':
            *dest++ = '\\';
            *dest++ = 'b';
            break;
        case '\f':
            *dest++ = '\\';
            *dest++ = 'f';
            break;
        case '\n':
            *dest++ = '\\';
            *dest++ = 'n';
            break;
        case '\r':
            *dest++ = '\\';
            *dest++ = 'r';
            break;
        case '\t':
            *dest++ = '\\';
            *dest++ = 't';
            break;
        default:
            *dest++ = *src;
        }
        src++;
    }
    *dest++ = '"';
    return dest - dest_start;
}

size_t json_string_capacity(const char *src) {
    // Worst-case all characters are escaped: 2x growth + surrounding quotes.
    return (strlen(src) * 2) + 2;
}

size_t decimal_capacity_unsigned_long(unsigned long value) {
    size_t digits = 1;
    while (value >= 10) {
        value /= 10;
        digits++;
    }
    return digits;
}

size_t decimal_capacity_unsigned_long_long(unsigned long long value) {
    size_t digits = 1;
    while (value >= 10) {
        value /= 10;
        digits++;
    }
    return digits;
}

int json_object_items_from_shared_attributes(char *json_cur, struct shared_attributes *restrict shared_attributes) {
    char _permissions_str[10];
    struct tm *tm;
    char time_str[22];
    char *json_start = json_cur;
    json_cur += sprintf(json_cur, "\"path\":");
    json_cur += json_string(json_cur, shared_attributes->path);
    json_cur += sprintf(json_cur, ",\"name\":");
    json_cur += json_string(json_cur, shared_attributes->name);
    json_cur += sprintf(json_cur, ",\"permissions\":");
    snprintf(_permissions_str, sizeof _permissions_str, "%c%c%c%c%c%c%c%c%c",
        (shared_attributes->permissions & S_IRUSR) ? 'r' : '-',
        (shared_attributes->permissions & S_IWUSR) ? 'w' : '-',
        (shared_attributes->permissions & S_IXUSR) ? 'x' : '-',
        (shared_attributes->permissions & S_IRGRP) ? 'r' : '-',
        (shared_attributes->permissions & S_IWGRP) ? 'w' : '-',
        (shared_attributes->permissions & S_IXGRP) ? 'x' : '-',
        (shared_attributes->permissions & S_IROTH) ? 'r' : '-',
        (shared_attributes->permissions & S_IWOTH) ? 'w' : '-',
        (shared_attributes->permissions & S_IXOTH) ? 'x' : '-'
    );
    json_cur += json_string(json_cur, _permissions_str);
    if (shared_attributes->owner_str != NULL) {
        json_cur += sprintf(json_cur, ",\"owner\":");
        json_cur += json_string(json_cur, shared_attributes->owner_str);
    }
    else
        json_cur += sprintf(json_cur, ",\"owner\":%lu", shared_attributes->owner_id);
    if (shared_attributes->group_str != NULL) {
        json_cur += sprintf(json_cur, ",\"group\":");
        json_cur += json_string(json_cur, shared_attributes->group_str);
    }
    else
        json_cur += sprintf(json_cur, ",\"group\":%lu", shared_attributes->group_id);
    json_cur += sprintf(json_cur, ",\"modified_at\":");
    tm = gmtime(&shared_attributes->modified_at);
    strftime(time_str, sizeof time_str, "%Y-%m-%dT%H:%M:%SZ", tm);
    json_cur += json_string(json_cur, time_str);
    return json_cur - json_start;
}

int json_from_directory(char *, struct directory *restrict);
int json_from_file(char *, struct file *restrict);
int json_from_link(char *, struct link *restrict);

int json_from_file(char *json_cur, struct file *restrict file) {
    char *json_start = json_cur;
    *json_cur++ = '{';
    json_cur += json_object_items_from_shared_attributes(json_cur, (struct shared_attributes *)file);
    json_cur += sprintf(json_cur, ",\"size\":%llu", file->size);
    *json_cur++ = '}';
    return json_cur - json_start;
}

int json_from_link(char *json_cur, struct link *restrict link) {
    const char *target_type_str;
    char *json_start = json_cur;
    *json_cur++ = '{';
    json_cur += json_object_items_from_shared_attributes(json_cur, (struct shared_attributes *)link);
    json_cur += sprintf(json_cur, ",\"target_path\":");
    json_cur += json_string(json_cur, link->target_path);
    switch (link->target_type) {
    case LINK_TARGET_TYPE_FILE:
        target_type_str = "file";
        break;
    case LINK_TARGET_TYPE_DIRECTORY:
        target_type_str = "directory";
        break;
    case LINK_TARGET_TYPE_LINK:
        target_type_str = "link";
        break;
    default:
        target_type_str = NULL;
    }
    if (target_type_str != NULL) {
        json_cur += sprintf(json_cur, ",\"target_type\":");
        json_cur += json_string(json_cur, target_type_str);
    }
    else
        json_cur += sprintf(json_cur, ",\"target_type\":null");
    *json_cur++ = '}';
    return json_cur - json_start;
}

int json_from_directory(char *json_cur, struct directory *restrict directory) {
    char *json_start = json_cur;
    *json_cur++ = '{';
    json_cur += json_object_items_from_shared_attributes(json_cur, (struct shared_attributes *)directory);
    if (directory->contents != NULL) {
        json_cur += sprintf(json_cur, ",\"contents\":{\"directories\":[");
        for (unsigned short i = 0; i < directory->contents->directory_count; i++) {
            if (i > 0)
                *json_cur++ = ',';
            json_cur += json_from_directory(json_cur, &directory->contents->directories[i]);
        }
        json_cur += sprintf(json_cur, "],\"files\":[");
        for (unsigned short i = 0; i < directory->contents->file_count; i++) {
            if (i > 0)
                *json_cur++ = ',';
            json_cur += json_from_file(json_cur, &directory->contents->files[i]);
        }
        json_cur += sprintf(json_cur, "],\"links\":[");
        for (unsigned short i = 0; i < directory->contents->link_count; i++) {
            if (i > 0)
                *json_cur++ = ',';
            json_cur += json_from_link(json_cur, &directory->contents->links[i]);
        }
        json_cur += sprintf(json_cur, "]}");
    }
    *json_cur++ = '}';
    return json_cur - json_start;
}

// json capacity estimation

size_t json_capacity_shared_attributes(struct shared_attributes *restrict shared_attributes) {
    size_t size = 0;
    size += strlen("\"path\":") + json_string_capacity(shared_attributes->path);
    size += strlen(",\"name\":") + json_string_capacity(shared_attributes->name);
    size += strlen(",\"permissions\":") + json_string_capacity("rwxrwxrwx");
    if (shared_attributes->owner_str != NULL)
        size += strlen(",\"owner\":") + json_string_capacity(shared_attributes->owner_str);
    else
        size += strlen(",\"owner\":") + decimal_capacity_unsigned_long(shared_attributes->owner_id);
    if (shared_attributes->group_str != NULL)
        size += strlen(",\"group\":") + json_string_capacity(shared_attributes->group_str);
    else
        size += strlen(",\"group\":") + decimal_capacity_unsigned_long(shared_attributes->group_id);
    size += strlen(",\"modified_at\":") + json_string_capacity("1970-01-01T00:00:00Z");
    return size;
}

size_t json_capacity_directory(struct directory *restrict directory);
size_t json_capacity_file(struct file *restrict file);
size_t json_capacity_link(struct link *restrict link);

size_t json_capacity_file(struct file *restrict file) {
    size_t size = 2 + json_capacity_shared_attributes((struct shared_attributes *)file);
    size += strlen(",\"size\":") + decimal_capacity_unsigned_long_long(file->size);
    return size;
}

size_t json_capacity_link(struct link *restrict link) {
    size_t size = 2 + json_capacity_shared_attributes((struct shared_attributes *)link);
    size += strlen(",\"target_path\":") + json_string_capacity(link->target_path);
    size += strlen(",\"target_type\":") + json_string_capacity("directory");
    return size;
}

size_t json_capacity_directory(struct directory *restrict directory) {
    size_t size = 2 + json_capacity_shared_attributes((struct shared_attributes *)directory);
    if (directory->contents != NULL) {
        size += strlen(",\"contents\":{\"directories\":[");
        for (unsigned short i = 0; i < directory->contents->directory_count; i++) {
            if (i > 0)
                size += 1;
            size += json_capacity_directory(&directory->contents->directories[i]);
        }
        size += strlen("],\"files\":[");
        for (unsigned short i = 0; i < directory->contents->file_count; i++) {
            if (i > 0)
                size += 1;
            size += json_capacity_file(&directory->contents->files[i]);
        }
        size += strlen("],\"links\":[");
        for (unsigned short i = 0; i < directory->contents->link_count; i++) {
            if (i > 0)
                size += 1;
            size += json_capacity_link(&directory->contents->links[i]);
        }
        size += strlen("]}");
    }
    return size;
}

int main(void) {
    char *directory_path = getenv("MEARIE__directory_path");
    if (!directory_path)
        return EXIT_FAILURE;
    if (directory_path[0] != '/')
        return EXIT_FAILURE;
    bool recursive = false;
    char *recursive_str = getenv("MEARIE__recursive");
    if (recursive_str && (strcmp(recursive_str, "true") == 0))
        recursive = true;

    walk_directory(&root_directory, directory_path, recursive, true);

    size_t json_capacity = json_capacity_directory(&root_directory) + 1;
    char *json_buffer = calloc(1, json_capacity);
    if (!json_buffer)
        return EXIT_FAILURE;
    json_from_directory(json_buffer, &root_directory);
    printf("%s\n", json_buffer);
    return EXIT_SUCCESS;
}

#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/statvfs.h>
#include <sys/utsname.h>
#include <unistd.h>

#define BUFFER_SIZE 256

char sysname[BUFFER_SIZE];
char nodename[BUFFER_SIZE];
char release[BUFFER_SIZE];
char version[BUFFER_SIZE];
char machine[BUFFER_SIZE];
char hostname[BUFFER_SIZE];
char pw_name[BUFFER_SIZE];
char pw_dir[BUFFER_SIZE];
char pw_shell[BUFFER_SIZE];

void escape(char *dest, const char *src) {
    while (*src) {
        if (*src == '"')
            *dest++ = '\\';
        *dest++ = *src++;
    }
}

int main(void) {
    char _buf[BUFFER_SIZE];
    bool _comma_needed = false;
    struct utsname _utsname;
    char _hostname[sizeof hostname];
    uid_t _uid;
    struct passwd _passwd;
    struct passwd *_passwd_p;
    printf("{");
    if (uname(&_utsname) == 0) {
        escape(sysname, _utsname.sysname);
        escape(nodename, _utsname.nodename);
        escape(release, _utsname.release);
        escape(version, _utsname.version);
        escape(machine, _utsname.machine);
        if (_comma_needed)
            printf(",");
        printf(
            "\"system_name\":\"%s\",\"node_name\":\"%s\",\"release\":\"%s\",\"version\":\"%s\",\"machine\":\"%s\"",
            sysname, nodename, release, version, machine
        );
        _comma_needed = true;
    }
    if (gethostname(_hostname, sizeof _hostname) == 0) {
        escape(hostname, _hostname);
        if (_comma_needed)
            printf(",");
        printf("\"host_name\":\"%s\"", hostname);
        _comma_needed = true;
    }
    if (getpwuid_r((_uid = getuid()), &_passwd, _buf, sizeof _buf, &_passwd_p) == 0) {
        escape(pw_name, _passwd.pw_name);
        escape(pw_dir, _passwd.pw_dir);
        escape(pw_shell, _passwd.pw_shell);
        if (_comma_needed)
            printf(",");
        printf(
            "\"user_uid\":%d,\"user_effective_uid\":%d,\"user_gid\":%d,\"user_effective_gid\":%d,\"user_name\":\"%s\",\"user_home\":\"%s\",\"user_shell\":\"%s\"",
            _passwd.pw_uid, geteuid(), _passwd.pw_gid, getegid(), pw_name, pw_dir, pw_shell
        );
    }
    else {
        if (_comma_needed)
            printf(",");
        printf(
            "\"user_uid\":%d,\"user_effective_uid\":%d,\"user_gid\":%d,\"user_effective_gid\":%d",
            _uid, geteuid(), getgid(), getegid()
        );
    }
    printf("}\n");
    return EXIT_SUCCESS;
}

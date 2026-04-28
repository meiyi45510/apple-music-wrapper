#define _GNU_SOURCE

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "logger.h"

static pid_t child_proc = -1;
extern char **environ;

static void handle_shutdown_signal(int signum) {
    (void)signum;
    if (child_proc != -1) {
        kill(child_proc, SIGKILL);
    }
}

int main(int argc, char *argv[], char *envp[]) {
    int status = 0;

    (void)argc;
    (void)envp;
    if (signal(SIGINT, handle_shutdown_signal) == SIG_ERR) {
        log_errno("signal");
        return 1;
    }
    if (signal(SIGTERM, handle_shutdown_signal) == SIG_ERR) {
        log_errno("signal");
        return 1;
    }

    if (chdir("./rootfs") != 0) {
        log_errno("chdir");
        return 1;
    }
    if (chroot("./") != 0) {
        log_errno("chroot");
        return 1;
    }
    if (chdir("/") != 0) {
        log_errno("chdir");
        return 1;
    }
    mknod("/dev/urandom", S_IFCHR | 0666, makedev(0x1, 0x9));
    chmod("/system/bin/linker64", 0755);
    chmod("/system/bin/wrapper-core", 0755);

    child_proc = fork();
    if (child_proc == -1) {
        log_errno("fork");
        return 1;
    }

    if (child_proc > 0) {
        close(STDOUT_FILENO);
        if (waitpid(child_proc, &status, 0) == -1) {
            log_errno("waitpid");
            return 1;
        }
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        }
        if (WIFSIGNALED(status)) {
            return 128 + WTERMSIG(status);
        }
        return 1;
    }

    setenv("ANDROID_ROOT", "/system", 1);
    setenv("ANDROID_DATA", "/data", 1);
    setenv("HOME", "/data/data/com.apple.android.music", 1);
    setenv("TMPDIR", "/data/local/tmp", 1);
    setenv("TZ", "UTC0", 1);
    mkdir("/data/data/com.apple.android.music/files", 0777);
    mkdir("/data/data/com.apple.android.music/files/mpl_db", 0777);
    mkdir("/data/local", 0777);
    mkdir("/data/local/tmp", 0777);
    execve("/system/bin/wrapper-core", argv, environ);
    log_errno("execve");
    return 1;
}

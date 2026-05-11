/* ═══════════════════════════════════════════════════════════════
   aleena.c  –  System Integration
   Responsibilities:
     • Install .service file into /etc/systemd/system/
     • Reload systemd daemon
     • Enable / Start / Stop / Status  via  fork() + exec()
     • Robust error handling for every system call
   NOTE: All functions use fork()+execvp() – NOT system()
   System Service Maker | Team Ahmad
   ═══════════════════════════════════════════════════════════════ */

#include "utils.h"

/* ──────────────────────────────────────────────────────────────
   run_command()  [internal helper]
   Forks a child, exec's argv[0] with the given args.
   Waits for child to finish.
   Returns child exit status  (0 = success, non-zero = failure)
   ────────────────────────────────────────────────────────────── */
static int run_command(char *const argv[])
{
    pid_t pid = fork();

    if (pid < 0) {
        /* fork() failed */
        printf(CLR_RED "[ERROR]   " CLR_RESET
               "fork() failed: %s\n", strerror(errno));
        return -1;
    }

    if (pid == 0) {
        /* ── Child process ── */
        execvp(argv[0], argv);
        /* execvp() only returns on error */
        printf(CLR_RED "[ERROR]   " CLR_RESET
               "execvp('%s') failed: %s\n", argv[0], strerror(errno));
        _exit(127);   /* Use _exit() in child – never return to caller's stack */
    }

    /* ── Parent: wait for child ── */
    int status;
    if (waitpid(pid, &status, 0) < 0) {
        printf(CLR_RED "[ERROR]   " CLR_RESET
               "waitpid() failed: %s\n", strerror(errno));
        return -1;
    }

    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);   /* 0 on success */
    }

    return -1;
}

/* ──────────────────────────────────────────────────────────────
   install_service()
   Copies  <name>.service  →  /etc/systemd/system/
   Returns  1  →  success  |  0  →  failure
   ────────────────────────────────────────────────────────────── */
int install_service(const char *name)
{
    char src[MAX_PATH];
    char dst[MAX_PATH];

    snprintf(src, sizeof(src), "%s.service", name);
    snprintf(dst, sizeof(dst), "%s%s.service", SYSTEMD_DIR, name);

    /* Make sure local .service file exists first */
    if (access(src, F_OK) != 0) {
        printf(CLR_RED "[ERROR]   " CLR_RESET
               "Local service file not found: %s\n", src);
        return 0;
    }

    /* cp <src> <dst> */
    char *argv[] = { "cp", src, dst, NULL };
    int ret = run_command(argv);

    if (ret != 0) {
        printf(CLR_RED "[ERROR]   " CLR_RESET
               "Failed to install service (exit %d). "
               "Are you running as root?\n", ret);
        return 0;
    }

    printf(CLR_GREEN "[SUCCESS] " CLR_RESET
           "Service file installed: %s\n", dst);
    return 1;
}

/* ──────────────────────────────────────────────────────────────
   reload_daemon()
   Runs: systemctl daemon-reexec
   Returns  1  →  success  |  0  →  failure
   ────────────────────────────────────────────────────────────── */
int reload_daemon(void)
{
    PRINT_INFO("Reloading systemd daemon...");

    char *argv[] = { "systemctl", "daemon-reexec", NULL };
    int ret = run_command(argv);

    if (ret != 0) {
        printf(CLR_RED "[ERROR]   " CLR_RESET
               "daemon-reexec failed (exit %d).\n", ret);
        return 0;
    }

    PRINT_OK("systemd daemon reloaded.");
    return 1;
}

/* ──────────────────────────────────────────────────────────────
   enable_service()
   Runs: systemctl enable <name>
   Returns  1  →  success  |  0  →  failure
   ────────────────────────────────────────────────────────────── */
int enable_service(const char *name)
{
    printf(CLR_CYAN "[INFO]    " CLR_RESET
           "Enabling service: %s\n", name);

    char *argv[] = { "systemctl", "enable", (char *)name, NULL };
    int ret = run_command(argv);

    if (ret != 0) {
        printf(CLR_RED "[ERROR]   " CLR_RESET
               "Failed to enable '%s' (exit %d).\n", name, ret);
        return 0;
    }

    printf(CLR_GREEN "[SUCCESS] " CLR_RESET
           "Service '%s' enabled.\n", name);
    return 1;
}

/* ──────────────────────────────────────────────────────────────
   start_service()
   Runs: systemctl start <name>
   Returns  1  →  success  |  0  →  failure
   ────────────────────────────────────────────────────────────── */
int start_service(const char *name)
{
    printf(CLR_CYAN "[INFO]    " CLR_RESET
           "Starting service: %s\n", name);

    char *argv[] = { "systemctl", "start", (char *)name, NULL };
    int ret = run_command(argv);

    if (ret != 0) {
        printf(CLR_RED "[ERROR]   " CLR_RESET
               "Failed to start '%s' (exit %d).\n", name, ret);
        return 0;
    }

    printf(CLR_GREEN "[SUCCESS] " CLR_RESET
           "Service '%s' started.\n", name);
    return 1;
}

/* ──────────────────────────────────────────────────────────────
   stop_service()
   Runs: systemctl stop <name>
   Returns  1  →  success  |  0  →  failure
   ────────────────────────────────────────────────────────────── */
int stop_service(const char *name)
{
    printf(CLR_CYAN "[INFO]    " CLR_RESET
           "Stopping service: %s\n", name);

    char *argv[] = { "systemctl", "stop", (char *)name, NULL };
    int ret = run_command(argv);

    if (ret != 0) {
        printf(CLR_RED "[ERROR]   " CLR_RESET
               "Failed to stop '%s' (exit %d).\n", name, ret);
        return 0;
    }

    printf(CLR_GREEN "[SUCCESS] " CLR_RESET
           "Service '%s' stopped.\n", name);
    return 1;
}

/* ──────────────────────────────────────────────────────────────
   status_service()
   Runs: systemctl status <name>
   Returns  1  →  active  |  0  →  not active / error
   ────────────────────────────────────────────────────────────── */
int status_service(const char *name)
{
    printf(CLR_CYAN "[INFO]    " CLR_RESET
           "Checking status: %s\n\n", name);

    char *argv[] = { "systemctl", "status", (char *)name, NULL };
    int ret = run_command(argv);

    /* systemctl status returns 0 if active, 3 if inactive */
    if (ret == 0) {
        printf("\n" CLR_GREEN "[SUCCESS] " CLR_RESET
               "Service '%s' is ACTIVE.\n", name);
        return 1;
    } else {
        printf("\n" CLR_YELLOW "[WARN]    " CLR_RESET
               "Service '%s' is NOT active (exit %d).\n", name, ret);
        return 0;
    }
}

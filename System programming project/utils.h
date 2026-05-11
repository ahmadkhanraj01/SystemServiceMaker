#ifndef UTILS_H
#define UTILS_H

/* ─────────────────────────────────────────────
   utils.h  –  Shared definitions & prototypes
   System Service Maker | Team Ahmad
   ───────────────────────────────────────────── */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>

/* ── Colour macros ── */
#define CLR_RESET   "\033[0m"
#define CLR_RED     "\033[0;31m"
#define CLR_GREEN   "\033[0;32m"
#define CLR_YELLOW  "\033[0;33m"
#define CLR_CYAN    "\033[0;36m"
#define CLR_BOLD    "\033[1m"

/* ── Convenience print macros ── */
#define PRINT_OK(msg)    printf(CLR_GREEN  "[SUCCESS] " CLR_RESET "%s\n", msg)
#define PRINT_ERR(msg)   printf(CLR_RED    "[ERROR]   " CLR_RESET "%s\n", msg)
#define PRINT_INFO(msg)  printf(CLR_CYAN   "[INFO]    " CLR_RESET "%s\n", msg)
#define PRINT_WARN(msg)  printf(CLR_YELLOW "[WARN]    " CLR_RESET "%s\n", msg)

/* ── Paths ── */
#define SYSTEMD_DIR   "/etc/systemd/system/"
#define LOG_FILE      "log.txt"
#define MAX_NAME      128
#define MAX_PATH      512
#define MAX_CMD       1024

/* ────────────────────────────────
   marwan.c  – Core Logic
   ──────────────────────────────── */
int validate_service_name(const char *name);
int validate_path(const char *path);
int create_service_file(const char *name, const char *path);

/* ────────────────────────────────
   aleena.c  – System Integration
   ──────────────────────────────── */
int install_service(const char *name);
int reload_daemon(void);
int enable_service(const char *name);
int start_service(const char *name);
int stop_service(const char *name);
int status_service(const char *name);

/* ────────────────────────────────
   main.c  – Logging
   ──────────────────────────────── */
void write_log(const char *level, const char *message);

#endif /* UTILS_H */

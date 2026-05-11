/* ═══════════════════════════════════════════════════════════════
   marwan.c  –  Core Logic
   Responsibilities:
     • Validate service name  (no spaces / special chars / empty)
     • Validate executable path  (must exist & be executable)
     • Generate .service file with correct systemd unit syntax
     • Duplicate-service check before writing
   System Service Maker | Team Ahmad
   ═══════════════════════════════════════════════════════════════ */

#include "utils.h"

/* ──────────────────────────────────────────────────────────────
   validate_service_name()
   Returns  1  →  name is valid
            0  →  name is invalid (prints reason)
   ────────────────────────────────────────────────────────────── */
int validate_service_name(const char *name)
{
    if (name == NULL || name[0] == '\0') {
        PRINT_ERR("Service name cannot be empty.");
        return 0;
    }

    if (strlen(name) >= MAX_NAME) {
        PRINT_ERR("Service name is too long (max 127 characters).");
        return 0;
    }

    for (int i = 0; name[i] != '\0'; i++) {
        char c = name[i];
        /* Allow: letters, digits, hyphen, underscore, dot */
        if (!isalnum(c) && c != '-' && c != '_' && c != '.') {
            printf(CLR_RED "[ERROR]   " CLR_RESET
                   "Invalid character '%c' in service name. "
                   "Use only letters, digits, '-', '_', '.'.\n", c);
            return 0;
        }
        /* No leading dot */
        if (i == 0 && c == '.') {
            PRINT_ERR("Service name must not start with a dot.");
            return 0;
        }
    }

    return 1;
}

/* ──────────────────────────────────────────────────────────────
   validate_path()
   Returns  1  →  path exists and is executable
            0  →  path is invalid
   ────────────────────────────────────────────────────────────── */
int validate_path(const char *path)
{
    if (path == NULL || path[0] == '\0') {
        PRINT_ERR("Executable path cannot be empty.");
        return 0;
    }

    if (strlen(path) >= MAX_PATH) {
        PRINT_ERR("Executable path is too long.");
        return 0;
    }

    /* Check existence */
    if (access(path, F_OK) != 0) {
        printf(CLR_RED "[ERROR]   " CLR_RESET
               "Path does not exist: %s\n", path);
        return 0;
    }

    /* Check execute permission */
    if (access(path, X_OK) != 0) {
        printf(CLR_RED "[ERROR]   " CLR_RESET
               "File is not executable: %s\n"
               "          Run: chmod +x %s\n", path, path);
        return 0;
    }

    return 1;
}

/* ──────────────────────────────────────────────────────────────
   create_service_file()
   Writes  <name>.service  in the current directory.
   Returns  1  →  success
            0  →  failure
   ────────────────────────────────────────────────────────────── */
int create_service_file(const char *name, const char *path)
{
    char filename[MAX_NAME + 9];   /* name + ".service\0" */
    snprintf(filename, sizeof(filename), "%s.service", name);

    /* ── Duplicate check ── */
    char systemd_path[MAX_PATH];
    snprintf(systemd_path, sizeof(systemd_path), "%s%s.service",
             SYSTEMD_DIR, name);

    if (access(systemd_path, F_OK) == 0) {
        printf(CLR_YELLOW "[WARN]    " CLR_RESET
               "Service '%s' already exists in %s.\n"
               "          Remove it first or choose a different name.\n",
               name, SYSTEMD_DIR);
        return 0;
    }

    /* ── Also check in current directory ── */
    if (access(filename, F_OK) == 0) {
        printf(CLR_YELLOW "[WARN]    " CLR_RESET
               "Local file '%s' already exists. Overwriting...\n", filename);
    }

    /* ── Write the unit file ── */
    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        printf(CLR_RED "[ERROR]   " CLR_RESET
               "Cannot create service file '%s': %s\n",
               filename, strerror(errno));
        return 0;
    }

    fprintf(fp,
        "[Unit]\n"
        "Description=Auto Generated Service – %s\n"
        "After=network.target\n"
        "\n"
        "[Service]\n"
        "Type=simple\n"
        "ExecStart=%s\n"
        "Restart=always\n"
        "RestartSec=5\n"
        "StandardOutput=journal\n"
        "StandardError=journal\n"
        "\n"
        "[Install]\n"
        "WantedBy=multi-user.target\n",
        name, path);

    fclose(fp);

    printf(CLR_GREEN "[SUCCESS] " CLR_RESET
           "Service file created: %s\n", filename);
    return 1;
}

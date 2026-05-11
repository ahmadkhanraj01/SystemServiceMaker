/* ═══════════════════════════════════════════════════════════════
   main.c  –  CLI Controller + Logging
   Responsibilities:
     • Parse command-line arguments
     • Orchestrate marwan.c and aleena.c calls
     • Root privilege check
     • write_log() – timestamped append-mode logger
     • Interactive menu mode (if no args given)
   
   Usage:
     sudo ./service_maker create <name> <exec_path>
     sudo ./service_maker start  <name>
     sudo ./service_maker stop   <name>
     sudo ./service_maker status <name>

   System Service Maker | Team Ahmad
   ═══════════════════════════════════════════════════════════════ */

#include "utils.h"

/* ──────────────────────────────────────────────────────────────
   write_log()
   Appends a timestamped entry to LOG_FILE.
   level   : "INFO" | "SUCCESS" | "ERROR" | "WARN"
   message : free-form description
   ────────────────────────────────────────────────────────────── */
void write_log(const char *level, const char *message)
{
    FILE *fp = fopen(LOG_FILE, "a");
    if (fp == NULL) {
        /* Non-fatal: just warn on screen */
        fprintf(stderr,
                CLR_YELLOW "[WARN]    " CLR_RESET
                "Could not open log file '%s': %s\n",
                LOG_FILE, strerror(errno));
        return;
    }

    /* Timestamp: [YYYY-MM-DD HH:MM:SS] */
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char timestamp[32];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", t);

    fprintf(fp, "[%s] [%-7s] %s\n", timestamp, level, message);
    fclose(fp);
}

/* ──────────────────────────────────────────────────────────────
   check_root()
   Exits with a helpful message if not running as root.
   ────────────────────────────────────────────────────────────── */
static void check_root(void)
{
    if (getuid() != 0) {
        PRINT_ERR("This tool requires root privileges.");
        printf("       Run:  " CLR_BOLD "sudo %s <command>\n" CLR_RESET,
               "service_maker");
        exit(EXIT_FAILURE);
    }
}

/* ──────────────────────────────────────────────────────────────
   print_banner()
   ────────────────────────────────────────────────────────────── */
static void print_banner(void)
{
    printf(CLR_BOLD CLR_CYAN
           "\n"
           "  ╔══════════════════════════════════════╗\n"
           "  ║      System Service Maker v1.0       ║\n"
           "  ║         Team Ahmad – CS-411          ║\n"
           "  ║            Aleena Khan               ║\n"
           "  ║             Marwan Rahim             ║\n"           
           "  ╚══════════════════════════════════════╝\n"
           CLR_RESET "\n");
}

/* ──────────────────────────────────────────────────────────────
   print_usage()
   ────────────────────────────────────────────────────────────── */
static void print_usage(const char *prog)
{
    printf(CLR_BOLD "Usage:\n" CLR_RESET);
    printf("  sudo %s create <name> <exec_path>\n", prog);
    printf("  sudo %s start  <name>\n", prog);
    printf("  sudo %s stop   <name>\n", prog);
    printf("  sudo %s status <name>\n\n", prog);
    printf(CLR_BOLD "Examples:\n" CLR_RESET);
    printf("  sudo %s create myapp /usr/local/bin/myapp\n", prog);
    printf("  sudo %s start  myapp\n", prog);
    printf("  sudo %s status myapp\n", prog);
    printf("  sudo %s stop   myapp\n\n", prog);
}

/* ──────────────────────────────────────────────────────────────
   do_create()
   Full pipeline: validate → generate → install → reload → enable
   ────────────────────────────────────────────────────────────── */
static int do_create(const char *name, const char *path)
{
    char log_buf[MAX_CMD];

    printf(CLR_BOLD "\n── CREATE ──────────────────────────────\n" CLR_RESET);

    /* 1. Validate service name */
    snprintf(log_buf, sizeof(log_buf), "Validating service name: %s", name);
    write_log("INFO", log_buf);
    PRINT_INFO(log_buf);

    if (!validate_service_name(name)) {
        write_log("ERROR", "Service name validation failed.");
        return 0;
    }
    write_log("SUCCESS", "Service name is valid.");

    /* 2. Validate executable path */
    snprintf(log_buf, sizeof(log_buf), "Validating executable path: %s", path);
    write_log("INFO", log_buf);
    PRINT_INFO(log_buf);

    if (!validate_path(path)) {
        write_log("ERROR", "Executable path validation failed.");
        return 0;
    }
    write_log("SUCCESS", "Executable path is valid.");

    /* 3. Generate .service file */
    snprintf(log_buf, sizeof(log_buf), "Creating service file for: %s", name);
    write_log("INFO", log_buf);

    if (!create_service_file(name, path)) {
        write_log("ERROR", "Service file creation failed.");
        return 0;
    }
    write_log("SUCCESS", "Service file created.");

    /* 4. Install into /etc/systemd/system/ */
    snprintf(log_buf, sizeof(log_buf), "Installing service: %s", name);
    write_log("INFO", log_buf);
    PRINT_INFO(log_buf);

    if (!install_service(name)) {
        write_log("ERROR", "Service installation failed.");
        return 0;
    }
    write_log("SUCCESS", "Service installed to systemd.");

    /* 5. Reload daemon */
    write_log("INFO", "Reloading systemd daemon.");
    if (!reload_daemon()) {
        write_log("ERROR", "daemon-reexec failed.");
        return 0;
    }
    write_log("SUCCESS", "systemd daemon reloaded.");

    /* 6. Enable service */
    snprintf(log_buf, sizeof(log_buf), "Enabling service: %s", name);
    write_log("INFO", log_buf);

    if (!enable_service(name)) {
        write_log("ERROR", "Service enable failed.");
        return 0;
    }
    snprintf(log_buf, sizeof(log_buf), "Service '%s' enabled successfully.", name);
    write_log("SUCCESS", log_buf);

    printf(CLR_GREEN CLR_BOLD
           "\n✔  Service '%s' is ready. "
           "Use 'start' to launch it.\n" CLR_RESET "\n", name);
    return 1;
}

/* ──────────────────────────────────────────────────────────────
   do_start()
   ────────────────────────────────────────────────────────────── */
static int do_start(const char *name)
{
    char log_buf[MAX_CMD];
    printf(CLR_BOLD "\n── START ───────────────────────────────\n" CLR_RESET);

    snprintf(log_buf, sizeof(log_buf), "Starting service: %s", name);
    write_log("INFO", log_buf);

    int ok = start_service(name);
    snprintf(log_buf, sizeof(log_buf),
             ok ? "Service '%s' started." : "Failed to start service '%s'.", name);
    write_log(ok ? "SUCCESS" : "ERROR", log_buf);
    return ok;
}

/* ──────────────────────────────────────────────────────────────
   do_stop()
   ────────────────────────────────────────────────────────────── */
static int do_stop(const char *name)
{
    char log_buf[MAX_CMD];
    printf(CLR_BOLD "\n── STOP ────────────────────────────────\n" CLR_RESET);

    snprintf(log_buf, sizeof(log_buf), "Stopping service: %s", name);
    write_log("INFO", log_buf);

    int ok = stop_service(name);
    snprintf(log_buf, sizeof(log_buf),
             ok ? "Service '%s' stopped." : "Failed to stop service '%s'.", name);
    write_log(ok ? "SUCCESS" : "ERROR", log_buf);
    return ok;
}

/* ──────────────────────────────────────────────────────────────
   do_status()
   ────────────────────────────────────────────────────────────── */
static int do_status(const char *name)
{
    char log_buf[MAX_CMD];
    printf(CLR_BOLD "\n── STATUS ──────────────────────────────\n" CLR_RESET);

    snprintf(log_buf, sizeof(log_buf), "Checking status of service: %s", name);
    write_log("INFO", log_buf);

    int ok = status_service(name);
    snprintf(log_buf, sizeof(log_buf),
             ok ? "Service '%s' is active." : "Service '%s' is NOT active.", name);
    write_log(ok ? "INFO" : "WARN", log_buf);
    return ok;
}

/* ──────────────────────────────────────────────────────────────
   interactive_menu()
   Shown when the user runs ./service_maker with no arguments.
   ────────────────────────────────────────────────────────────── */
static void interactive_menu(void)
{
    char choice[8];
    char name[MAX_NAME];
    char path[MAX_PATH];

    printf(CLR_BOLD
           "  Select an operation:\n"
           CLR_RESET
           "  1. Create service\n"
           "  2. Start service\n"
           "  3. Stop service\n"
           "  4. Status of service\n"
           "  5. Exit\n\n"
           "  Choice: ");
    fflush(stdout);

    if (fgets(choice, sizeof(choice), stdin) == NULL) return;
    int c = atoi(choice);

    if (c == 5 || c == 0) {
        printf("  Bye!\n\n");
        return;
    }

    if (c < 1 || c > 4) {
        PRINT_ERR("Invalid choice.");
        return;
    }

    printf("  Service name: ");
    fflush(stdout);
    if (fgets(name, sizeof(name), stdin) == NULL) return;
    name[strcspn(name, "\n")] = '\0';   /* strip newline */

    if (c == 1) {
        printf("  Executable path: ");
        fflush(stdout);
        if (fgets(path, sizeof(path), stdin) == NULL) return;
        path[strcspn(path, "\n")] = '\0';
        do_create(name, path);
    } else if (c == 2) {
        do_start(name);
    } else if (c == 3) {
        do_stop(name);
    } else if (c == 4) {
        do_status(name);
    }
}

/* ──────────────────────────────────────────────────────────────
   main()
   ────────────────────────────────────────────────────────────── */
int main(int argc, char *argv[])
{
    print_banner();
    check_root();

    write_log("INFO", "=== service_maker started ===");

    /* ── No arguments: show interactive menu ── */
    if (argc == 1) {
        print_usage(argv[0]);
        interactive_menu();
        write_log("INFO", "=== service_maker exited ===");
        return EXIT_SUCCESS;
    }

    /* ── Parse command ── */
    const char *cmd = argv[1];

    /* ── CREATE: needs 4 args total ── */
    if (strcmp(cmd, "create") == 0) {
        if (argc != 4) {
            PRINT_ERR("'create' requires <name> and <exec_path>.");
            print_usage(argv[0]);
            write_log("ERROR", "create: wrong number of arguments.");
            return EXIT_FAILURE;
        }
        int ok = do_create(argv[2], argv[3]);
        write_log("INFO", "=== service_maker exited ===");
        return ok ? EXIT_SUCCESS : EXIT_FAILURE;
    }

    /* ── START / STOP / STATUS: need 3 args ── */
    if (strcmp(cmd, "start")  == 0 ||
        strcmp(cmd, "stop")   == 0 ||
        strcmp(cmd, "status") == 0)
    {
        if (argc != 3) {
            printf(CLR_RED "[ERROR]   " CLR_RESET
                   "'%s' requires <name>.\n", cmd);
            print_usage(argv[0]);
            write_log("ERROR", "wrong number of arguments.");
            return EXIT_FAILURE;
        }

        /* Validate name before calling aleena */
        if (!validate_service_name(argv[2])) {
            write_log("ERROR", "Invalid service name supplied.");
            return EXIT_FAILURE;
        }

        int ok = 0;
        if      (strcmp(cmd, "start")  == 0) ok = do_start(argv[2]);
        else if (strcmp(cmd, "stop")   == 0) ok = do_stop(argv[2]);
        else if (strcmp(cmd, "status") == 0) ok = do_status(argv[2]);

        write_log("INFO", "=== service_maker exited ===");
        return ok ? EXIT_SUCCESS : EXIT_FAILURE;
    }

    /* ── Unknown command ── */
    printf(CLR_RED "[ERROR]   " CLR_RESET
           "Unknown command: '%s'\n\n", cmd);
    print_usage(argv[0]);
    write_log("ERROR", "Unknown command.");
    return EXIT_FAILURE;
}

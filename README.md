# ⚙️ SystemServiceMaker

> A C-based Linux command-line utility that automates the creation and management of systemd services — replacing 5 manual error-prone steps with a single command.

![Platform](https://img.shields.io/badge/Platform-Linux-blue)
![Language](https://img.shields.io/badge/Language-C-orange)
![License](https://img.shields.io/badge/License-MIT-green)
![Course](https://img.shields.io/badge/Course-CS--411%20System%20Programming-navy)
![Status](https://img.shields.io/badge/Status-Working-brightgreen)

---

## 📸 Screenshots

### Tool Banner & CREATE Command
<!-- Screenshot: sudo ./service_maker create myapp /usr/local/bin/myservice.sh -->
<img width="1150" height="669" alt="Screenshot 2026-04-25 132629" src="https://github.com/user-attachments/assets/44cd0ba1-099e-4a45-9fa0-bb855baa026a" />

---

### Start Service
<!-- Screenshot: sudo ./service_maker start myapp  -->
<img width="953" height="766" alt="Screenshot 2026-04-25 132734" src="https://github.com/user-attachments/assets/bd6f066d-b0d0-4e92-a54c-0f185cbfa967" />



---

### Service Running — STATUS Output
<!-- Screenshot: sudo ./service_maker status myapp showing Active: active (running) -->
<img width="1919" height="790" alt="Screenshot 2026-04-25 132949 - Copy" src="https://github.com/user-attachments/assets/70707072-8a75-4e75-b1bb-3fc5c928171c" />


---

## 📌 What is This?

Managing Linux background services with systemd requires writing unit files by hand, copying them to system directories, reloading the daemon, enabling, and starting — all manually. One typo breaks everything silently.

**SystemServiceMaker** automates the entire lifecycle with one command:

```bash
sudo ./service_maker create myapp /usr/local/bin/myservice.sh
```

This single command:
- ✅ Validates the service name and executable path
- ✅ Generates a correct `.service` unit file
- ✅ Installs it to `/etc/systemd/system/`
- ✅ Reloads the systemd daemon
- ✅ Enables the service to auto-start on boot
- ✅ Logs every operation with a timestamp to `log.txt`

---

## 🏗️ Architecture

```
project/
├── main.c       →  CLI controller + logging system
├── marwan.c     →  Input validation + .service file generation
├── aleena.c     →  System integration via fork() + execvp()
├── utils.h      →  Shared macros, constants, prototypes
├── Makefile     →  Build configuration
└── log.txt      →  Auto-created runtime log
```

### Module Responsibilities

| File | Owner | Role |
|---|---|---|
| `main.c` | Muhammad Ahmad | CLI parsing, `check_root()`, `write_log()`, pipeline orchestration |
| `marwan.c` | Marwan Rahim | `validate_service_name()`, `validate_path()`, `create_service_file()` |
| `aleena.c` | Aleena Khan | `install_service()`, `reload_daemon()`, `enable/start/stop/status` |
| `utils.h` | Shared | All `#include`s, colour macros, `MAX` constants, function prototypes |

---

## 🔧 POSIX System Calls Used

| System Call | File | Purpose |
|---|---|---|
| `getuid()` | `main.c` | Verify root (UID == 0) before any privileged operation |
| `fork()` | `aleena.c` | Create child process to run system commands |
| `execvp()` | `aleena.c` | Replace child image with `cp` / `systemctl` |
| `waitpid()` | `aleena.c` | Parent waits for child; extracts exit code via `WEXITSTATUS()` |
| `access()` | `marwan.c` | `F_OK`: file exists? `X_OK`: is executable? |
| `fopen()` | `main.c`, `marwan.c` | Open `log.txt` (append) and `.service` file (write) |
| `fprintf()` | `main.c`, `marwan.c` | Write unit file content and log entries |
| `time()` / `strftime()` | `main.c` | Generate `[YYYY-MM-DD HH:MM:SS]` timestamps |
| `strerror()` | All files | Human-readable error messages from `errno` |

---

## 🚀 Getting Started

### Prerequisites

```bash
sudo apt update && sudo apt upgrade -y
sudo apt install build-essential -y    # GCC + Make
sudo apt install systemd -y            # systemd (pre-installed on Ubuntu)
```

### Verify Installation

```bash
gcc --version
make --version
systemctl --version
```

### Clone & Build

```bash
git clone https://github.com/YOUR_USERNAME/SystemServiceMaker.git
cd SystemServiceMaker
make
```

Expected output:
```
gcc -Wall -Wextra -Wpedantic -std=c11 -g -c main.c -o main.o
gcc -Wall -Wextra -Wpedantic -std=c11 -g -c marwan.c -o marwan.o
gcc -Wall -Wextra -Wpedantic -std=c11 -g -c aleena.c -o aleena.o
gcc -Wall -Wextra -Wpedantic -std=c11 -g -o service_maker main.o ...

  Build complete →  ./service_maker
```

---

## 📖 Usage

### CLI Mode

```bash
sudo ./service_maker create <name> <exec_path>
sudo ./service_maker start  <name>
sudo ./service_maker stop   <name>
sudo ./service_maker status <name>
```

### Interactive Menu Mode

Run with no arguments to get a numbered menu:

```bash
sudo ./service_maker
```

```
  Select an operation:
  1. Create service
  2. Start service
  3. Stop service
  4. Status of service
  5. Exit
```

---

## 🎬 Full Demo — Real Working Service

**Step 1 — Create a heartbeat script:**

```bash
sudo nano /usr/local/bin/myservice.sh
```

Paste this inside:

```bash
#!/bin/bash
while true; do
    echo "Service is alive — $(date)" >> /var/log/myservice.log
    sleep 5
done
```

```bash
sudo chmod +x /usr/local/bin/myservice.sh
```

**Step 2 — Register it as a service:**

```bash
sudo ./service_maker create myapp /usr/local/bin/myservice.sh
```

Expected output:
```
[INFO]    Validating service name: myapp
[INFO]    Validating executable path: /usr/local/bin/myservice.sh
[SUCCESS] Service file created: myapp.service
[SUCCESS] Service file installed: /etc/systemd/system/myapp.service
[SUCCESS] systemd daemon reloaded.
[SUCCESS] Service 'myapp' enabled.
✔  Service 'myapp' is ready. Use 'start' to launch it.
```

**Step 3 — Start it:**

```bash
sudo ./service_maker start myapp
```

**Step 4 — Watch it work:**

```bash
tail -f /var/log/myservice.log
```

```
Service is alive — Sat Apr 25 01:27:47 AM PDT 2026
Service is alive — Sat Apr 25 01:27:52 AM PDT 2026
Service is alive — Sat Apr 25 01:27:57 AM PDT 2026
```

**Step 5 — Stop it:**

```bash
sudo ./service_maker stop myapp
```

---

## ❌ Error Handling

| Command | Error | Output |
|---|---|---|
| `create myapp /usr/bin/fake` | Path does not exist | `[ERROR] Path does not exist: /usr/bin/fake` |
| `create "my app" /usr/bin/python3` | Space in name | `[ERROR] Invalid character ' ' in service name.` |
| `create my@pp /usr/bin/python3` | Special character | `[ERROR] Invalid character '@' in service name.` |
| `create myapp ...` (2nd time) | Duplicate service | `[WARN] Service 'myapp' already exists in /etc/systemd/system/` |
| `start ghostservice` | Service not found | `[ERROR] Failed to start 'ghostservice' (exit 5).` |
| `./service_maker` (no sudo) | Not root | `[ERROR] This tool requires root privileges.` |

---

## 📋 Log File

Every operation is automatically recorded to `log.txt` with a full timestamp:

```
[2026-04-25 01:27:44] [INFO   ] === service_maker started ===
[2026-04-25 01:27:44] [INFO   ] Validating service name: myapp
[2026-04-25 01:27:44] [SUCCESS] Service name is valid.
[2026-04-25 01:27:44] [SUCCESS] Executable path is valid.
[2026-04-25 01:27:44] [SUCCESS] Service file created.
[2026-04-25 01:27:44] [SUCCESS] Service installed to systemd.
[2026-04-25 01:27:44] [SUCCESS] systemd daemon reloaded.
[2026-04-25 01:27:44] [SUCCESS] Service myapp enabled.
[2026-04-25 01:27:49] [SUCCESS] Service myapp started.
[2026-04-25 01:27:57] [SUCCESS] Service myapp stopped.
[2026-04-25 01:27:57] [INFO   ] === service_maker exited ===
```

---

## 🧪 Test Cases

| Test ID | Description | Expected Result | Status |
|---|---|---|---|
| TC-01 | Valid service creation | File created + installed + enabled | ✅ PASS |
| TC-02 | Invalid executable path | `[ERROR]` Path does not exist | ✅ PASS |
| TC-03 | Space in service name | `[ERROR]` Invalid character | ✅ PASS |
| TC-04 | Special char in name | `[ERROR]` Invalid character | ✅ PASS |
| TC-05 | Duplicate service | `[WARN]` Already exists | ✅ PASS |
| TC-06 | Start valid service | `[SUCCESS]` Service started | ✅ PASS |
| TC-07 | Status — active service | `Active: active (running)` | ✅ PASS |
| TC-08 | Stop running service | `[SUCCESS]` Service stopped | ✅ PASS |
| TC-09 | Status — after stop | `Active: inactive (dead)` | ✅ PASS |
| TC-10 | Run without sudo | `[ERROR]` Requires root | ✅ PASS |
| TC-11 | log.txt entries | Timestamped entries appended | ✅ PASS |

---

## 🧹 Cleanup

Remove the service completely:

```bash
sudo systemctl stop myapp
sudo systemctl disable myapp
sudo rm /etc/systemd/system/myapp.service
sudo systemctl daemon-reexec
rm myapp.service
rm log.txt
make clean
```

---

## 👥 Team

| Name | Role | Contributions |
|---|---|---|
| **Muhammad Ahmad** | Testing & Documentation | `main.c`, logging system, CLI controller, test cases, report, presentation |
| **Marwan Rahim** | Core Logic | `marwan.c` — all validation and service file generation |
| **Aleena Khan** | System Integration | `aleena.c` — all `fork()+execvp()` system calls and service control |

---

## 📚 Course

**CS-411 — System Programming**
University Project | April 2026

---

## 📄 License

This project is licensed under the MIT License.

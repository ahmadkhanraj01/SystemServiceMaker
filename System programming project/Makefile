# ═══════════════════════════════════════════════════
#  Makefile  –  System Service Maker
#  Team Ahmad | CS-411
# ═══════════════════════════════════════════════════

CC      = gcc
CFLAGS  = -Wall -Wextra -Wpedantic -std=c11 -g
TARGET  = service_maker
SRCS    = main.c marwan.c aleena.c
OBJS    = $(SRCS:.c=.o)

# ── Default target ──────────────────────────────────
all: $(TARGET)
	@echo ""
	@echo "  Build complete →  ./$(TARGET)"
	@echo "  Run:  sudo ./$(TARGET) --help"
	@echo ""

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c utils.h
	$(CC) $(CFLAGS) -c $< -o $@

# ── Convenience targets ─────────────────────────────
run: all
	sudo ./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET) *.service log.txt
	@echo "  Cleaned."

# ── Install / Uninstall (copies binary to /usr/local/bin) ──
install: all
	sudo cp $(TARGET) /usr/local/bin/$(TARGET)
	@echo "  Installed to /usr/local/bin/$(TARGET)"

uninstall:
	sudo rm -f /usr/local/bin/$(TARGET)
	@echo "  Uninstalled."

.PHONY: all run clean install uninstall

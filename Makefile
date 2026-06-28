# Makefile — Ready Player Pi (CMake-wrapper)
#
# Wrapper omkring CMake. Qt6 + AUTOMOC styres af CMake; denne Makefile
# eksponerer simple targets for at bygge, køre og teste.
#
# Krav: sudo apt install nlohmann-json3-dev libasound2-dev \
#                       qt6-base-dev qt6-virtualkeyboard-dev cmake
#
# Mål:
#   make                → bygger server + client (Qt6 + NotePi)
#   make server         → kun server
#   make client         → kun client
#   make client-hw      → client med BMI160 over SPI (kræver Raspberry Pi)
#   make tests          → bygger module_test + notepi_end_to_end_test
#   make run            → starter server + 2 client-hw (live BMI160) i terminaler
#   make run-stub       → som make run, men med stub-client (hardcoded tilt)
#   make run-server     → kun server (i forgrunden)
#   make run-client     → kun client-hw (forbinder til 127.0.0.1:9000, live IMU)
#   make e2e            → kører end-to-end protokol-test (server + 2 sockets)
#   make module-test    → kører DSP-pipeline-test mod laptop-mic (10 sek)
#   make clean          → slet build-mapper + scoreboard-filer
#
# Bemærk: Qt-builds kræver CMake. Denne Makefile delegerer til cmake under
# hætten — ingen manuel MOC-håndtering.

BUILD_DIR       := build
TESTS_BUILD_DIR := Tests/build
JOBS            := $(shell nproc 2>/dev/null || echo 4)

# Auto-detekt terminal-emulator. Kan overskrives: `make run TERMINAL=xterm`
# Rækkefølge: kitty → gnome-terminal → konsole → terminator → xfce4 → xterm
TERMINAL ?= $(shell \
    for t in kitty gnome-terminal konsole terminator xfce4-terminal xterm; do \
        command -v $$t >/dev/null 2>&1 && echo $$t && break; \
    done)

.PHONY: all configure server client client-hw tests \
        run run-stub run-server run-client \
        e2e module-test mic-fix mic-status \
        clean

# ── Build ─────────────────────────────────────────────────────────────────────

all: server client

configure: $(BUILD_DIR)/CMakeCache.txt

$(BUILD_DIR)/CMakeCache.txt:
	cmake -B $(BUILD_DIR)

server: configure
	cmake --build $(BUILD_DIR) --target server -j$(JOBS)

client: configure
	cmake --build $(BUILD_DIR) --target client -j$(JOBS)

client-hw:
	cmake -B $(BUILD_DIR) -DBUILD_HARDWARE_CLIENT=ON
	cmake --build $(BUILD_DIR) --target client-hw -j$(JOBS)

tests: $(TESTS_BUILD_DIR)/CMakeCache.txt
	cmake --build $(TESTS_BUILD_DIR) -j$(JOBS)

$(TESTS_BUILD_DIR)/CMakeCache.txt:
	cmake -B $(TESTS_BUILD_DIR) -S Tests

# ── Kør (kræver build færdig først) ───────────────────────────────────────────

# Mic-device override: ALSA "default" picker ikke altid den rigtige mic.
# Sæt RPP_MIC_DEVICE her for laptop-test. Override på CLI:
#   make run RPP_MIC_DEVICE='default'
# DSP-tuning gøres direkte i ClientApp/NotePi/domain/NotePiConfig.h
# (MIN_ENERGY, DOMINANCE_THRESHOLD, RECENT_DETECTIONS_SIZE).
RPP_MIC_DEVICE ?= plughw:CARD=Generic_1,DEV=0

run: server client-hw
	@if [ -z "$(TERMINAL)" ]; then \
	    echo "Ingen kendt terminal-emulator fundet."; \
	    echo "Sæt TERMINAL=<navn> manuelt eller kør i tre vinduer:"; \
	    echo "  ./$(BUILD_DIR)/server"; \
	    echo "  ./$(BUILD_DIR)/client-hw 127.0.0.1"; \
	    echo "  ./$(BUILD_DIR)/client-hw 127.0.0.1"; \
	    exit 1; \
	fi
	@if [ ! -e /dev/spidev0.0 ]; then \
	    echo "ADVARSEL: /dev/spidev0.0 findes ikke — Performer-rolle vil fejle på SPI."; \
	    echo "          Kør evt. 'make run-stub' for at bruge hardcoded tilt-path."; \
	fi
	@echo "Starter server + 2 client-hw (live IMU) i $(TERMINAL)..."
	@$(MAKE) -s _spawn CMD="./$(BUILD_DIR)/server"
	@sleep 1
	@$(MAKE) -s _spawn CMD="./$(BUILD_DIR)/client-hw 127.0.0.1"
	@sleep 1
	@$(MAKE) -s _spawn CMD="./$(BUILD_DIR)/client-hw 127.0.0.1"

run-stub: all
	@if [ -z "$(TERMINAL)" ]; then \
	    echo "Ingen kendt terminal-emulator fundet."; \
	    echo "Sæt TERMINAL=<navn> manuelt eller kør i tre vinduer:"; \
	    echo "  ./$(BUILD_DIR)/server"; \
	    echo "  ./$(BUILD_DIR)/client 127.0.0.1"; \
	    echo "  ./$(BUILD_DIR)/client 127.0.0.1"; \
	    exit 1; \
	fi
	@echo "Starter server + 2 stub-clients (hardcoded tilt) i $(TERMINAL)..."
	@$(MAKE) -s _spawn CMD="./$(BUILD_DIR)/server"
	@sleep 1
	@$(MAKE) -s _spawn CMD="./$(BUILD_DIR)/client 127.0.0.1"
	@sleep 1
	@$(MAKE) -s _spawn CMD="./$(BUILD_DIR)/client 127.0.0.1"

# Intern target: spawn $(CMD) i ny terminal med RPP_MIC_DEVICE i env.
.PHONY: _spawn
_spawn:
	@ENV='RPP_MIC_DEVICE=$(RPP_MIC_DEVICE)'; \
	case "$(TERMINAL)" in \
	    kitty)             setsid kitty bash -c "env $$ENV $(CMD); read -p 'Press enter'" >/dev/null 2>&1 & ;; \
	    gnome-terminal)    setsid gnome-terminal -- bash -c "env $$ENV $(CMD); read -p 'Press enter'" >/dev/null 2>&1 & ;; \
	    konsole)           setsid konsole -e bash -c "env $$ENV $(CMD); read -p 'Press enter'" >/dev/null 2>&1 & ;; \
	    terminator)        setsid terminator -x bash -c "env $$ENV $(CMD); read -p 'Press enter'" >/dev/null 2>&1 & ;; \
	    xfce4-terminal)    setsid xfce4-terminal --command="bash -c \"env $$ENV $(CMD); read -p 'Press enter'\"" >/dev/null 2>&1 & ;; \
	    xterm)             setsid xterm -e bash -c "env $$ENV $(CMD); read -p 'Press enter'" >/dev/null 2>&1 & ;; \
	    *)                 echo "Ukendt TERMINAL=$(TERMINAL)"; exit 1 ;; \
	esac

run-server: server
	./$(BUILD_DIR)/server

run-client: client-hw
	./$(BUILD_DIR)/client-hw 127.0.0.1

# ── Test-kørsel ───────────────────────────────────────────────────────────────

# End-to-end testen forventer ./build/server findes — bygger først.
e2e: server tests
	./$(TESTS_BUILD_DIR)/notepi_end_to_end_test

module-test: tests
	./$(TESTS_BUILD_DIR)/module_test

# ── ALSA mic-volumen ──────────────────────────────────────────────────────────
# Card "Generic_1" er Frederiks laptop-mic. Default capture-niveau er +30dB
# (100%) hvilket clipper signalet. Sæt til ~50% (+6dB) for nyttig DSP-input.
ALSA_CARD ?= Generic_1
MIC_LEVEL ?= 50%

mic-fix:
	@echo "Sætter ALSA capture på $(ALSA_CARD) til $(MIC_LEVEL)..."
	@amixer -c $(ALSA_CARD) sset Capture $(MIC_LEVEL) | tail -3

mic-status:
	@amixer -c $(ALSA_CARD) sget Capture | tail -5

# ── Clean ─────────────────────────────────────────────────────────────────────

clean:
	rm -rf $(BUILD_DIR) $(TESTS_BUILD_DIR)
	rm -f scoreboard_*.txt

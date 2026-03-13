# REAL-TIME-VEHICLE-STATUS-MONITORING-USING-CAN

> A real-time multi-node vehicle status monitoring built from scratch on bare-metal ARM7 microcontrollers — no RTOS, no HAL, just pure embedded C.

---

## 🌟 What This Project Does

This system simulates a vehicle's dashboard electronics using **4 independent MCU nodes** communicating over a **CAN bus** — the same protocol used in real cars (ISO 11898).

The central display node renders a live dashboard on a **20×4 character LCD**, showing:
- ⛽ Real-time fuel level as an animated bar + percentage
- 🔁 Left / right turn indicator arrows that blink
- 💥 Airbag deployment detection using a 3-axis accelerometer

---

## 🎥 Dashboard Preview

```
     ┌────────────────────┐
L1   │  VEHICLE DASHBOARD │
L2   │FUEL [████████░░]75%│  ← live fuel bar
L3   │IND  <<<   ◆   >>> │  ← indicator blink
L4   │AIRBAG  STATUS: OK! │  ← airbag monitor
     └────────────────────┘
        JHD 204A  20×4 LCD
```

---

## 🏗️ System Architecture

```
                        ┌─────────────────────────────────┐
  ⛽ FUEL NODE          │           🖥️  MAIN NODE          |
  ┌──────────────┐      │        (MAIN_CAN_TX.c)          │
  │ FUEL_MAIN.c  │      │                                 │
  │              │      │  • Right/Left Button press      │
  │ ADC sensor   │─────►│  • Drives 20×4 LCD dashboard    │
  │ → % value    │      │  • Monitors accelerometer       │
  │ every 300ms  │      │  • Controls airbag output       │
  └──────────────┘      └───────────────┬─────────────────┘
                                        │
                                        │ CAN Bus
                                        │ (125 kbps)
                                        ▼
                          ┌─────────────────────────┐
                          │   💡 INDICATOR RX NODE  │
                          │     (MAIN_CAN_RX.c)     │
                          │  8-LED sweep animation  │
                          └─────────────────────────┘
```

---

## ✨ Key Features

| Feature | Details |
|---|---|
| 🔌 **Multi-node CAN bus** | 4 MCUs on a shared 125 kbps CAN network |
| ⛽ **Real-time fuel gauge** | 10-segment animated bar updates every 300 ms |
| 🔁 **Turn indicators** | Blinking arrow animation with toggle logic |
| 💥 **Airbag detection** | 3-axis accelerometer threshold detection (±1.2g) |
| 🖥️ **Custom LCD characters** | 8 CGRAM characters: arrows, blocks, diamond, border |
| ⚡ **Interrupt-driven buttons** | EINT0/EINT2 with VIC for zero-latency response |
| 🛠️ **Zero dependencies** | No RTOS, no HAL — every driver written from scratch |

---

## 🧠 Skills Demonstrated

```
Embedded C           ████████████████████  Expert
CAN Bus Protocol     ████████████████░░░░  Strong
I2C Communication    ███████████████░░░░░  Strong
ADC / Sensors        ██████████████░░░░░░  Strong
Interrupt Handling   ████████████████░░░░  Strong
LCD Driver (HD44780) ████████████████████  Expert
Bit Manipulation     ████████████████████  Expert
Hardware Debugging   ███████████████░░░░░  Strong
```

---

## 🗂️ Project Structure

```
vehicle-dashboard/
│
├── 📡 CAN Driver
│   ├── CAN.c              # init, TX (blocking + timeout), RX (non-blocking)
│   ├── can.h              # CANF frame struct, function prototypes
│   └── can_defines.h      # BTR timing macros (BRP, TSEG1, TSEG2, SJW)
│
├── ⛽ Fuel Node
│   ├── FUEL_MAIN.c        # main: ADC read → CAN transmit loop
│   ├── FUEL.c             # ADC init + single-shot blocking conversion
│   ├── fuel.h             # ADC interface
│   └── fuel_defnes.h      # ADC clock, pin, and result bit macros
│
├── 🔗 I2C Driver
│   ├── I2C.c              # start/restart/stop/write/nack/mack
│   ├── i2c.h              # I2C interface
│   └── i2c_defines.h      # 100 kHz divider, pin and bit macros
│
├── 📐 Accelerometer (MMA7660FC)
│   ├── MMA_7660.c         # read/write registers, sign-extend 6-bit data
│   ├── mma_7660.h         # register map, I2C addresses, prototypes
│   └── MAIN_MMA_7660.c    # airbag_trigger() — impact detection logic
│
├── 🖥️ LCD Driver (JHD 204A 20×4)
│   ├── LCD.c              # full HD44780 driver + CGRAM custom chars
│   ├── lcd.h              # LCD interface (str, int, float, hex, CGRAM)
│   └── lcd_defines.h      # pin map, DDRAM line addresses
│
├── 🔁 Indicator System
│   ├── INDICATOR.c        # 8-LED left/right sweep animation
│   ├── INDICATOR_GEN.c    # TX node: button ISRs + CAN TX + blink flags
│   ├── indicator.h        # shared interface (ISRs, LED steps, airbag)
│   └── EXT_INT.c          # simpler EINT variant (single-press, no toggle)
│
├── 🖥️ Main Node
│   ├── MAIN_CAN_TX.c      # dashboard: CAN RX, LCD update, airbag, blink
│   └── MAIN_CAN_RX.c      # indicator RX: state machine + LED sweep
│
└── 🔧 Utilities
    ├── defines.h           # SETBIT / CLRBIT / READBIT / WRITEBYTE macros
    ├── delay.c             # us / ms / s busy-wait delays @ 60 MHz
    └── delay.h             # delay interface
```

---

## 📡 CAN Message Protocol

| CAN ID | Sender | Byte | Value | Meaning |
|:---:|---|:---:|:---:|---|
| `0x01` | Indicator Node | DATA1 | `0x01` | ◄ Left button pressed |
| `0x01` | Indicator Node | DATA1 | `0x02` | ► Right button pressed |
| `0x02` | Fuel Node | DATA2 | `0–100` | ⛽ Fuel level percentage |

---

## 🔩 Hardware Used

| Component | Part Number | Interface |
|---|---|---|
| Microcontroller | LPC2129 (ARM7TDMI-S) | — |
| CAN Transceiver | MCP2551 / SN65HVD230 | P0.0 RD, P0.1 TD |
| LCD Display | JHD 204A — 20×4 HD44780 | 8-bit parallel P0.10–P0.20 |
| Accelerometer | MMA7660FC | I2C — P0.2 SCL, P0.3 SDA |
| Fuel Sensor | FUEL GUAGE | AIN1 — P0.28 (ADC) |
| Indicator LEDs | ×8 generic | P0.0–P0.7 |
| Push buttons | ×2 momentary | EINT0 (P0.0), EINT2 (P0.7) |

---

## ⚙️ Clock Configuration

```
🔵 Crystal (FOSC)    =  12 MHz
🟢 CPU clock (CCLK)  =  60 MHz    ← PLL × 5
🟡 Peripheral (PCLK) =  15 MHz    ← ÷ 4
📡 CAN bit rate      = 125 kbps   ← 15 MHz / (8 BRP × 15 quanta)
🔗 I2C speed         = 100 kHz    ← standard mode
⚡ ADC clock         =   3 MHz    ← ≤ 4.5 MHz max (datasheet)
```

---

## 🐛 Bugs Found & Fixed

| File | Bug | Fix Applied |
|---|---|---|
| `MMA_7660.c` | `#include` statement **inside** a function body | Moved to top of file |
| `INDICATOR.c` | Used undeclared variable `position` instead of `pos` | Unified to `pos` throughout |
| `MAIN_CAN_TX.c` | Labels redrawn every loop — caused LCD flicker | Labels written **once** before `while(1)` |
| `MAIN_CAN_TX.c` | Percentage digits shifted left/right (1→2→3 digits) | Fixed-width with space padding |

---

## 🚀 How to Build

**Toolchain:** Keil µVision 5 or ARM-GCC
**Target device:** LPC2129 (ARM7TDMI-S — 64 kB RAM, 256 kB Flash)

```bash
# ARM-GCC example
arm-none-eabi-gcc -mcpu=arm7tdmi -mthumb-interwork \
  -O1 -Wall \
  CAN.c FUEL.c LCD.c I2C.c MMA_7660.c INDICATOR.c \
  MAIN_CAN_TX.c MAIN_MMA_7660.c INDICATOR_GEN.c delay.c \
  -o dashboard.elf

arm-none-eabi-objcopy -O binary dashboard.elf dashboard.bin
```

> **Note:** A vendor startup file (`startup_LPC21xx.s`) is required for vector table and stack init. Not included — obtain from NXP or Keil device pack.

---

## 📸 What I Learned

- Designing a **multi-node embedded system** from scratch with no OS
- Writing **bare-metal peripheral drivers** (CAN, I2C, ADC, LCD) by reading datasheets directly
- Handling **hardware interrupts** (VIC, EINT) with real-time responsiveness
- Debugging **register-level issues** on physical hardware with LEDs and an oscilloscope
- Structuring **embedded C code** cleanly across multiple files for readability and maintainability

---

## 👨‍💻 Author

Built as a major embedded systems project demonstrating end-to-end hardware bring-up, driver development, and system integration on ARM7 architecture.

---

> ⭐ *If you found this project interesting, feel free to star the repo!*

# 🏎️ Real Time Vehicle Status Monitoring Using CAN

> An embedded systems project that enhances **vehicle safety and monitoring** using the **Controller Area Network (CAN) protocol** — built on bare-metal LPC2129 ARM7 microcontrollers with no RTOS, no HAL, just pure Embedded C.

---

## 🎯 Aim

The aim of this project is to enhance vehicle safety and monitoring by using the **Controller Area Network (CAN) protocol**. The system is designed to display critical vehicle parameters such as:

- ⛽ **Fuel percentage** — read from an ADC-connected fuel gauge sensor
- 🔁 **Indicator status** — left / right turn signal control via CAN
- 💥 **Airbag status / activation** — impact detection using an accelerometer

All of this is displayed in **real time** on a 20×4 LCD.

---

## 🧠 Insight — What You Need to Know

- 📝 Knowledge of **Embedded-C programming**
- 🔬 Thorough understanding of **LPC2129 Architecture** — GPIO, ADC, Interrupts, and CAN interface
- 📡 Understanding of the **CAN protocol** (Controller Area Network, ISO 11898)

---

## 🏗️ Block Diagram

```
                    ┌──────────────────────────────────────────┐
                    │               🖥️  MAIN NODE              |
                    │                                          │
                    │  ┌──────────────────┐   ──────────►  LCD │
                    │  │  ON-CHIP CAN     │   ──────────►  BUZZER/LED
                    │  │  CONTROLLER      │   ◄────────►  MMA7660
                    │  │    LPC2129       │   ◄──  L.I SW (EINT0)
          MCP2551 ◄─┤  └──────────────────┘   ◄──  R.I SW (EINT2)
                    └───────────┬──────────────────────────────┘
                                │                    
          ══════════════════════╪══════════════════════ CAN BUS
               CANH / CANL      │          CANH / CANL
          ══════════════════════╪══════════════════════
                    ┌───────────┴────────┐    ┌────────────────────────┐
                    │  💡 INDICATOR NODE │    │     ⛽ FUEL NODE       │
                    │                   │    │                        │
                    │  ON-CHIP CAN      │    │  ON-CHIP CAN           │
                    │  CONTROLLER       │    │  CONTROLLER            │
                    │  LPC2129          │    │  LPC2129               │
                    │  MCP2551          │    │  MCP2551               │
                    │       │           │    │       │                │
                    │  8 LEDs(Indicator)│    │ADC(P0.28) ◄── FUEL GAUGE│
                    └───────────────────┘    └────────────────────────┘

  LEFT  INDICATION: LEDs scroll one by one from RIGHT to LEFT
  RIGHT INDICATION: LEDs scroll one by one from LEFT  to RIGHT
```

---

## 🔧 Hardware Requirements

| # | Component | Purpose |
|---|---|---|
| 1 | **LPC2129** | ARM7TDMI-S MCU — main processing unit for all 3 nodes |
| 2 | **MCP2551** | CAN transceiver — interfaces MCU to CAN bus (CANH/CANL) |
| 3 | **LEDs (×8)** | Indicator node — visual turn-signal sweep animation |
| 4 | **LCD (20×4)** | Main node — real-time dashboard display |
| 5 | **MMA7660** | 3-axis accelerometer — airbag impact detection via I2C |
| 6 | **Fuel Gauge** | Resistive sensor — read via on-chip ADC (AIN1) |
| 7 | **Switches (×2)** | L.I SW / R.I SW — trigger EINT0 / EINT2 interrupts |
| 8 | **USB to UART Converter** | Flash firmware and debug via serial port |

---

## 💻 Software Requirements

| Tool | Purpose |
|---|---|
| **Embedded C** | Programming language for all firmware |
| **Keil µVision** | IDE and ARM compiler for LPC2129 |
| **Flash Magic** | Firmware flashing tool over UART/USB |

---

## 📡 CAN Message Protocol

| CAN ID | Node | Byte | Value | Meaning |
|:---:|---|:---:|:---:|---|
| `0x01` | Main Node → Indicator Node | DATA1 | `0x01` | ◄ Left  indicator triggered |
| `0x01` | Main Node → Indicator Node | DATA1 | `0x02` | ► Right indicator triggered |
| `0x02` | Fuel Node → Main Node | DATA2 | `0–100` | ⛽ Fuel level percentage |

---

## 🎥 LCD Dashboard Layout (20×4)

```
     ┌────────────────────┐
L1   │  VEHICLE DASHBOARD │   ← static title
L2   │FUEL [████████░░]75%│   ← real-time fuel bar + percentage
L3   │IND  <<<   ◆   >>> │   ← indicator arrows (blink on press)
L4   │AIRBAG  STATUS: OK! │   ← airbag: OK / BOOM on impact
     └────────────────────┘
```

---

## 🔄 Node Descriptions

### 🖥️ Main Node (`MAIN_CAN_TX.c`)
- Continuously reads **MMA7660 accelerometer** data over I2C
- Displays real-time X / Y / Z acceleration values on LCD
- Analyses acceleration against a **predefined safety threshold**
- If threshold exceeded → identifies as **accident event** → displays airbag deployed message
- When **SW1 / SW2** are pressed (via EINT0 / EINT2 external interrupts) → sends indicator control signal to Indicator Node via CAN
- Receives **fuel percentage** from Fuel Node via CAN and displays it on LCD

> ⚠️ SW1 and SW2 must be handled using **external interrupts** (not polling)

### 💡 Indicator Node (`MAIN_CAN_RX.c`)
- Continuously **waits for data** from Main Node on the CAN bus
- On receiving a frame with ID=1:
  - `DATA1 = 0x01` → Left  indication: LEDs scroll **right to left** one by one
  - `DATA1 = 0x02` → Right indication: LEDs scroll **left to right** one by one
- Uses 8 LEDs on P0.0–P0.7 for the sweep animation

### ⛽ Fuel Node (`FUEL_MAIN.c`)
- Continuously reads the **fuel gauge sensor** using the on-chip ADC
- Converts raw 10-bit ADC value (0–1023) to a percentage (0–100%)
- Sends the fuel percentage to the Main Node via CAN every **300 ms**

---

## 📋 Implementation Sequence

Follow these steps in order when building and testing the project:

```
Step 1  ✅  Create a new project folder and name it appropriately
Step 2  ✅  Test LCD — display character, string, and integer constants
Step 3  ✅  Test on-chip ADC — connect potentiometer, display value on LCD
Step 4  ✅  Develop fuel percentage logic — read ADC, show % on LCD
Step 5  ✅  Test external interrupts — count presses, display count on LCD
Step 6  ✅  Read accelerometer (MMA7660) via I2C — display X/Y/Z on LCD
Step 7  ✅  Download and test CAN basic code — verify TX/RX on hardware
Step 8  ✅  Integrate all modules — build full Main, Indicator, Fuel nodes
```

---

## 🗂️ File Structure

```
vehicle-dashboard/
│
├── 📡 CAN Driver
│   ├── CAN.c              # init, TX (blocking + timeout), RX (non-blocking)
│   ├── can.h              # CANF frame struct + function prototypes
│   └── can_defines.h      # BTR timing macros (BRP, TSEG1, TSEG2, SJW)
│
├── ⛽ Fuel Node
│   ├── FUEL_MAIN.c        # main: ADC read → CAN transmit every 300 ms
│   ├── FUEL.c             # ADC init + single-shot blocking conversion
│   ├── fuel.h             # ADC interface
│   └── fuel_defnes.h      # ADC clock, pin, and result bit macros
│
├── 🔗 I2C Driver
│   ├── I2C.c              # start / restart / stop / write / read
│   ├── i2c.h              # I2C interface
│   └── i2c_defines.h      # 100 kHz divider, pin and bit macros
│
├── 📐 Accelerometer (MMA7660)
│   ├── MMA_7660.c         # register read/write, 6-bit sign-extension
│   ├── mma_7660.h         # register map, I2C address, prototypes
│   └── MAIN_MMA_7660.c    # airbag_trigger() — threshold impact detection
│
├── 🖥️ LCD Driver (JHD 204A 20×4)
│   ├── LCD.c              # HD44780 init + CGRAM custom characters
│   ├── lcd.h              # lcd_str / lcd_int / lcd_char / lcd_cmd
│   └── lcd_defines.h      # pin map, DDRAM line addresses
│
├── 🔁 Indicator System
│   ├── INDICATOR.c        # 8-LED left/right sweep animation
│   ├── INDICATOR_GEN.c    # TX node: EINT ISRs + CAN TX + blink flags
│   ├── indicator.h        # shared interface (ISRs, LED steps, airbag)
│
├── 🖥️ Main Node
│   ├── MAIN_CAN_TX.c      # dashboard: CAN RX, LCD update, airbag, blink
│   └── MAIN_CAN_RX.c      # indicator RX: state machine + LED sweep
│
└── 🔧 Utilities
    ├── defines.h           # SETBIT / CLRBIT / READBIT / WRITEBYTE macros
    ├── delay.c             # us / ms / s busy-wait delays @ 60 MHz CCLK
    └── delay.h             # delay interface
```

---

## ⚙️ Clock & Peripheral Configuration

```
🔵 Crystal (FOSC)    =  12 MHz
🟢 CPU clock (CCLK)  =  60 MHz    ← PLL × 5
🟡 Peripheral (PCLK) =  15 MHz    ← ÷ 4
📡 CAN bit rate      = 125 kbps   ← 15 MHz / (8 BRP × 15 quanta)
🔗 I2C speed         = 100 kHz    ← standard mode (MMA7660)
⚡ ADC clock         =   3 MHz    ← ≤ 4.5 MHz datasheet limit
```

---

## 🐛 Bugs Found & Fixed

| File | Bug | Fix Applied |
|---|---|---|
| `MAIN_CAN_TX.c` | Static labels redrawn every loop — caused LCD **flicker** | Labels written **once** before `while(1)` |
| `MAIN_CAN_TX.c` | Percentage digits shifted position for 1/2/3-digit values | Fixed-width right-aligned with space padding |

---

## ✅ Expected Output

If the project is implemented correctly:

- 🖥️ **LCD** shows live fuel %, blinking indicator arrows, and airbag status
- 💡 **8 LEDs** on the Indicator Node sweep left or right based on button press
- 💥 **Airbag message** appears on LCD when MMA7660 threshold is breached
- 📡 **CAN bus** carries all data between the three independent MCU boards

---

## 👨‍💻 Skills Demonstrated

```
Embedded C Programming    ████████████████████  Core language
CAN Bus Protocol          ████████████████░░░░  Multi-node
I2C Communication         ███████████████░░░░░  MMA7660 accelerometer
ADC / Sensor Reading      ██████████████░░░░░░  Fuel gauge, 10-bit result
External Interrupts (VIC) ████████████████░░░░  EINT0 / EINT2, ISR
LCD Driver (HD44780)      ████████████████████  CGRAM, DDRAM, 8-bit mode
Bit-level Register Access ████████████████████  Direct SFR writes
Hardware Debugging        ███████████████░░░░░  LED indicators + UART
```

---
## 👨‍💻 Author

Built as a major embedded systems project demonstrating end-to-end hardware bring-up, driver development, and system integration on ARM7 architecture.

---

> ⭐ *If you found this project interesting, feel free to star the repo!*

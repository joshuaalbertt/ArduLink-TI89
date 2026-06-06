# ArduLink-TI89 🚀

An Arduino Nano-based hardware emulator designed specifically to connect the classic **Texas Instruments TI-89 (Non-Titanium)** calculator to PC connectivity software such as TiLP.

This project functions as a clone of the **GrayLink** cable, using an Arduino Nano to handle the translation layer natively without requiring the obsolete DB9 serial port.

Source Code $\rightarrow$ https://github.com/jw0k/serial2ti83/blob/master/serial2ti83.ino
Source Code Optimization (Me + Claude, haha) $\rightarrow$ `ti_link_bridge.ino`

---

## 📐 Mathematical Model & Logical Transparency

To prevent data corruption during large FLASH ROM dumps or application transfers, we must manipulate the hardware serial core constraints.

### 1. The Buffer Overflow Constraint
Let $B_{rx}$ be the hardware receive buffer size allocated in SRAM (defined by `SERIAL_RX_BUFFER_SIZE`), and let $P_{size}$ be the incoming burst packet size from TiLP. The default Arduino architecture enforces:
$$B_{rx} = 64 \text{ bytes}$$

When sending advanced 68k architecture variables, the packet length routinely exceeds this limit ($P_{size} > 64$). The number of lost bytes $D_{lost}$ is defined by the boundary condition:
$$D_{lost} = \max(0, P_{size} - B_{rx})$$

If $D_{lost} > 0$, the communication integrity drops to zero, forcing TiLP to throw a Timeout Error $(-1)$.

### 2. Explicit Substitution Justification
To ensure zero data loss ($\forall P_{size} \le 255$), we must apply an explicit substitution inside the `HardwareSerial.h` file of the Arduino core:
$$B_{rx} \leftarrow 256$$

### 3. Forced Variables Range
Why must the variable range be forced exactly to $256$?
On the 8-bit AVR architecture (ATmega328P), the ring buffer index pointer variable $I$ is stored as an unsigned 8-bit integer (`uint8_t`). This architecture forces the mathematical range of the pointer to lock strictly within:
$$I \in [0, 255]$$

By setting the buffer size to $256$ ($2^8$), any value of $I$ that increments past $255$ automatically overflows back to $0$ via native hardware clock cycles ($I \pmod{256}$). This eliminates the need for expensive conditional `if` checks in the interrupt service routine, optimizing execution speed to meet the microsecond timing windows required by TiLP.

---

## 🛠️ Mandatory Local Pre-Compilation Steps

Before clicking **Upload** in the Arduino IDE, you must modify your local compiler files:

1. Open your file explorer and locate the file: `HardwareSerial.h`
   *(Usually found under: `C:\Users\<Username>\AppData\Local\Arduino15\packages\arduino\hardware\avr\<version>\cores\arduino\HardwareSerial.h`)*
2. Search for the line containing `#define SERIAL_RX_BUFFER_SIZE 64` & `#define SERIAL_TX_BUFFER_SIZE 64`
3. Change `64` to `256`.
4. Save the file and restart your Arduino IDE.

---

## 🔌 Hardware Setup & Physical Layer Constraints

Standard jumper wires loosely wrapped around the audio jack terminal will cause **contact chattering** (microsecond signal drops), causing TiLP to reject the handshake.

* **Electrical Constraint:** Contact Resistance ($R_{\text{contact}}$) must be locked at zero ($R_{\text{contact}} \equiv 0$). **Soldering the wires to the 2.5 mm jack terminal is highly mandatory.**
* **Isolation Constraint:** Inter-line resistance must approach infinity ($R_{\text{isolation}} \to \infty$). Wrap each terminal pin individually with electrical tape before bundling them together to prevent Bus Errors (`code -6`).

### Pin Mapping:
* **Arduino Pin D2** $\rightarrow$ Jack TIP
* **Arduino Pin D3** $\rightarrow$ Jack RING
* **Arduino Pin GND** $\rightarrow$ Jack SLEEVE

$\rightarrow$ Visit https://github.com/jw0k/serial2ti83 for more detailed documentation.

---

## 🔍 Troubleshooting & Diagnostic Protocol (The ArTICL Test)

If TiLP throws a `Cable Error` or refuses to connect, **do not panic**. You need to isolate whether the fault lies within your software layer or the physical copper connections. Follow this baseline diagnostic test using the **ArTICL library**:

### Visual Metaphor: The Do-It-Yourself Doorknob Test
Running TiLP directly without physical testing is like guessing whether a phone line is down in the middle of a storm. Using the ArTICL library for unit testing acts as a simple doorbell tester. We just want to make sure that when a button on the calculator is pressed, the alarm on the Arduino responds instantly without any PC intervention.

### Diagnostic Steps:
1. Download the **ArTICL** library by Christopher Mitchell from GitHub (as a `.zip` file).
$\rightarrow$ Visit https://github.com/KermMartian/ArTICL
2. Import it into Arduino IDE: **Sketch** $\rightarrow$ **Include Library** $\rightarrow$ **Add .ZIP Library...**
3. Open the basic communication sketch: **File** $\rightarrow$ **Examples** $\rightarrow$ **ArTICL** $\rightarrow$ **ControlLED**.
4. Open the code and uncomment the verbosity toggle to unlock the raw signal lens:
   `cbl.setVerbosity(true, &Serial);`
5. Upload the code and open the **Serial Monitor** at **9600 baud**.

### Log Interpretation & Boundary Conditions:
Observe the terminal behavior during the *idle state* (when no buttons are pressed) to pinpoint your physical wiring condition:

* **Case 1: Screen floods violently with `died waiting for bit ack 0` or freezes at `code -6` / `Not Connected`**
  $$\text{Condition: } \quad \text{digitalRead}(2) == \text{LOW} \quad \lor \quad \text{digitalRead}(3) == \text{LOW}$$
  * *Diagnosis:* Permanent short circuit. Your exposed copper strands are touching each other or leaking to the GND sleeve. Cut any loose ties, unwrap the tape, and separate the raw wires to clear the bus.
* **Case 2: Screen stays dead silent, but running a `Send` command on the TI-89 triggers nothing**
  $$\text{Condition: } \quad \forall t, \quad V_{D2} \equiv 5\text{V} \quad \land \quad V_{D3} \equiv 5\text{V}$$
  * *Diagnosis:* Open circuit (broken line). Your wires are floating because they aren't making actual mechanical contact with the jack terminals. Pressing down on the wires with your hands or resoldering will drop the contact resistance back to zero ($R_{\text{contact}} \equiv 0$), immediately triggering valid byte streams (`Got byte 0, 2, 68...`).

Once the Serial Monitor falls into a perfect **silent idle state**, and reacts **only** when the TI-89 transmits data, your physical hardware layer is officially certified. You can now safely flash the main `serial2ti89.ino` firmware.

---

## 💻 Software Configuration (TiLP)

1. Connect your freshly soldered ArduLink hardware to the TI-89 and your PC.
2. Launch **TiLP**.
3. Navigate to **Setup** $\rightarrow$ **Change Device/Cable**.
4. Set **Cable** to `GrayLink`.
5. Set **Port** to the exact virtual `COM Port` allocated to your Arduino Nano by the CH340 driver.
6. Set **Device** to `TI-89`.
7. Test the connection by hitting **Dirlist** or dumping a backup!

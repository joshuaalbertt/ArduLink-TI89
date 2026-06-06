# ArduLink-TI89

An Arduino Nano-based hardware emulator designed specifically to connect the classic **Texas Instruments TI-89 (Non-Titanium)** calculator to PC connectivity software such as **TiLP.

This project functions as a clone of the **GrayLink** cable, using an Arduino Nano to handle the translation layer natively without requiring the obsolete DB9 serial port.

---

| Jack port     | Arduino       |
|:-------------:|:-------------:|
| sleeve        | GND pin       |
| tip           | pin 2         |
| ring          | pin 3         | 

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
2. Search for the line containing `#define SERIAL_RX_BUFFER_SIZE 64`.
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

---

## 💻 Software Configuration (TiLP)

1. Connect your freshly soldered ArduLink hardware to the TI-89 and your PC.
2. Launch **TiLP**.
3. Navigate to **Setup** $\rightarrow$ **Change Device/Cable**.
4. Set **Cable** to `GrayLink`.
5. Set **Port** to the exact virtual `COM Port` allocated to your Arduino Nano by the CH340 driver.
6. Set **Device** to `TI-89`.
7. Test the connection by hitting **Dirlist** or dumping a backup!

/**
 * TI Link Protocol Bridge — Arduino Uno
 *
 * Wiring:
 *   Pin 2 (PD2) → TIP
 *   Pin 3 (PD3) → RING
 *   GND         → GND
 *
 * Protocol:
 *   Idle: both lines HIGH. Bits transmitted LSb-first.
 *   Bit 0: sender pulls TIP LOW,  receiver acks by pulling RING LOW.
 *   Bit 1: sender pulls RING LOW, receiver acks by pulling TIP LOW.
 */

#define MASK_TIP   (1 << 2)  // PD2, 0x04
#define MASK_RING  (1 << 3)  // PD3, 0x08

#define TXTIMEOUT  50UL  // ms
#define RXTIMEOUT   5UL  // ms — keep short so TX isn't starved; raise to 15-20 if RX drops

// Drive a line LOW (set OUTPUT before asserting to avoid a HIGH glitch)
static inline void pullLow(uint8_t mask)
{
    PORTD &= ~mask;
    DDRD  |=  mask;
}

// Return a line to INPUT_PULLUP
static inline void release(uint8_t mask)
{
    DDRD  &= ~mask;
    PORTD |=  mask;
}

bool sendByte(uint8_t data)
{
    for (uint8_t i = 0; i < 8; ++i)
    {
        bool bit = data & 0x01;
        data >>= 1;

        uint8_t ourMask = bit ? MASK_RING : MASK_TIP;
        uint8_t oppMask = bit ? MASK_TIP  : MASK_RING;

        // Wait for idle (both lines HIGH)
        unsigned long t = millis();
        while (!(PIND & MASK_TIP) || !(PIND & MASK_RING))
        {
            if (millis() - t > TXTIMEOUT) return false;
        }

        // Assert bit
        pullLow(ourMask);

        // Wait for calculator to acknowledge
        t = millis();
        while (PIND & oppMask)
        {
            if (millis() - t > TXTIMEOUT)
            {
                release(ourMask);
                return false;
            }
        }

        release(ourMask);

        // Wait for calculator to release its line
        t = millis();
        while (!(PIND & oppMask))
        {
            if (millis() - t > TXTIMEOUT) return false;
        }
    }

    return true;
}

bool getByte(uint8_t& byte)
{
    uint8_t result = 0;

    for (uint8_t i = 0; i < 8; ++i)
    {
        // Wait for calculator to pull a line LOW.
        // Snapshot PIND atomically to avoid a race condition between detection and bit sampling.
        uint8_t snapshot;
        unsigned long t = millis();
        while (true)
        {
            snapshot = PIND;
            if (!((snapshot & MASK_TIP) && (snapshot & MASK_RING))) break;
            if (millis() - t > RXTIMEOUT) return false;
        }

        // RING LOW = 1, TIP LOW = 0
        bool bit = !(snapshot & MASK_RING);

        result >>= 1;
        if (bit) result |= 0x80;  // assemble LSb-first

        uint8_t ourMask = bit ? MASK_TIP  : MASK_RING;
        uint8_t oppMask = bit ? MASK_RING : MASK_TIP;

        // Acknowledge
        pullLow(ourMask);

        // Wait for calculator to release its line
        t = millis();
        while (!(PIND & oppMask))
        {
            if (millis() - t > RXTIMEOUT)
            {
                release(ourMask);
                return false;
            }
        }

        release(ourMask);
    }

    byte = result;
    return true;
}

void setup()
{
    // Turn off built-in LED (PB5 = D13)
    DDRB  |=  (1 << 5);
    PORTB &= ~(1 << 5);

    Serial.begin(115200);

    // TIP and RING as INPUT_PULLUP
    DDRD  &= ~(MASK_TIP | MASK_RING);
    PORTD |=  (MASK_TIP | MASK_RING);
}

void loop()
{
    // PC → Calculator
    while (Serial.available() > 0)
    {
        sendByte((uint8_t)Serial.read());
    }

    // Calculator → PC
    uint8_t received;
    if (getByte(received))
    {
        Serial.write(received);
    }
}

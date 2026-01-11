# OLED Love Quotes Display for Riya ❤️

A romantic Arduino sketch that displays animated love quotes and hearts on a 0.94" OLED display using ESP32.

## 🎯 Features

- ✨ **Animated Heart Icons** - Beating hearts in all corners
- 💕 **9 Romantic Quotes** - Beautiful messages that rotate automatically
- 🎬 **Smooth Transitions** - Professional animations
- 💌 **Personal Dedication** - Final message: "I Love You Riya - Tanveer"
- 🔄 **Auto-Rotating Display** - Quotes change every 4 seconds
- 💗 **Startup Animation** - Heart growing effect on startup

## 📋 Hardware Requirements

### Components Needed

1. **ESP32 Development Board** - Any ESP32 board will work
2. **0.94" OLED Display** - SSD1306 based (128x64 pixels, I2C interface)
3. **Jumper Wires** - For connections

### Wiring Diagram

```
OLED Display (SSD1306) → ESP32
====================================
VCC  → 3.3V
GND  → GND
SDA  → GPIO 21
SCL  → GPIO 22
```

**Important Notes:**
- Some OLED displays work with 5V, but 3.3V is safer for ESP32
- Most 0.94" OLED displays use I2C address 0x3C (default in code)
- If display doesn't work, try address 0x3D

## 📚 Required Libraries

Install these libraries via Arduino IDE Library Manager:

1. **Adafruit GFX Library** (by Adafruit)
2. **Adafruit SSD1306** (by Adafruit)
3. **Wire** (Built-in with Arduino)

### Installation Steps:

1. Open Arduino IDE
2. Go to **Sketch → Include Library → Manage Libraries**
3. Search for "Adafruit SSD1306" and install
4. Search for "Adafruit GFX" and install
5. Done!

## 🚀 Quick Start Guide

### Step 1: Hardware Setup

1. Connect the OLED display to ESP32 as shown in wiring diagram above
2. Ensure all connections are secure
3. Connect ESP32 to computer via USB cable

### Step 2: Software Setup

1. Install Arduino IDE (if not already installed)
2. Install ESP32 board support:
   - Go to **File → Preferences**
   - Add this URL to "Additional Board Manager URLs":
     ```
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
     ```
   - Go to **Tools → Board → Boards Manager**
   - Search for "esp32" and install

3. Install required libraries (see above)

### Step 3: Upload the Sketch

1. Open `oled-love-quotes-riya.ino` in Arduino IDE
2. Select your board: **Tools → Board → ESP32 Arduino → ESP32 Dev Module**
3. Select correct COM port: **Tools → Port → (your ESP32 port)**
4. Click **Upload** button (→)
5. Wait for upload to complete

### Step 4: Enjoy!

- The display will show a startup animation
- Then it will cycle through 9 romantic quotes
- Each quote displays for 4 seconds
- Hearts animate in all corners
- Finally, it shows the dedication message: "I Love You Riya - Tanveer"

## 💝 The Quotes

The display shows these romantic messages:

1. "Every moment with you is a treasure"
2. "You are my sunshine on a cloudy day"
3. "In your smile, I find my happiness"
4. "You make my heart skip a beat"
5. "Together is my favorite place to be"
6. "You are the reason I smile daily"
7. "My love for you grows stronger every day"
8. "You complete me in every way possible"
9. "Forever isn't long enough with you"

**Final Message:** "I Love You Riya - Tanveer" ❤️

## ⚙️ Customization

### Change Quote Display Time

Edit this line in the code:
```cpp
#define QUOTE_DISPLAY_TIME 4000   // Time in milliseconds (4000 = 4 seconds)
```

### Change Heart Animation Speed

Edit this line:
```cpp
#define HEART_ANIMATION_SPEED 100 // Time in milliseconds (100 = 0.1 seconds)
```

### Add More Quotes

Find this section in the code:
```cpp
const char* quotes[] = {
  "Every moment\nwith you is\na treasure",
  // Add your own quotes here!
  "Your custom\nquote goes\nhere"
};
```

**Format Tips:**
- Use `\n` for line breaks
- Keep lines short (max ~12 characters per line)
- Use 3 lines maximum per quote for best display

### Change Final Message

Find this section:
```cpp
display.print(F("I Love"));
display.print(F("You"));
display.print(F("Riya - Tanveer"));
```

Replace with your own names!

### Change I2C Address

If your display uses address 0x3D instead of 0x3C, change this line:
```cpp
#define SCREEN_ADDRESS 0x3C  // Change to 0x3D if needed
```

## 🔧 Troubleshooting

### Display Not Working

**Problem:** Blank screen or no response

**Solutions:**
1. Check all wiring connections
2. Verify OLED is getting 3.3V power
3. Try different I2C address (0x3D instead of 0x3C)
4. Run I2C scanner sketch to find correct address
5. Check if display is 128x64 pixels (not 128x32)

### Upload Errors

**Problem:** "Failed to connect to ESP32"

**Solutions:**
1. Hold BOOT button while uploading
2. Try different USB cable
3. Install CH340/CP2102 drivers for your ESP32
4. Select correct COM port

### Display Shows Garbage

**Problem:** Random pixels or strange characters

**Solutions:**
1. Check I2C address (try 0x3C or 0x3D)
2. Verify display is SSD1306 compatible
3. Check for loose connections
4. Try lowering I2C speed in code

## 📊 Technical Specifications

- **Display Type:** Monochrome OLED (White on Black)
- **Resolution:** 128x64 pixels
- **Interface:** I2C (Two-wire)
- **I2C Address:** 0x3C (default) or 0x3D
- **Refresh Rate:** ~20 FPS (50ms delay)
- **Heart Animation:** 10 FPS (100ms per frame)
- **Quote Rotation:** 4 seconds per quote
- **Total Runtime:** ~36 seconds for all quotes + final message

## 🎨 Display Layout

### Quote Display Mode
```
╔════════════════════════════╗
║ ❤️                      ❤️ ║
║                            ║
║      Quote Text Line 1     ║
║      Quote Text Line 2     ║
║      Quote Text Line 3     ║
║                            ║
║ ❤️                      ❤️ ║
╚════════════════════════════╝
```

### Final Message Mode
```
╔════════════════════════════╗
║     ❤️         ❤️          ║
║                            ║
║        I Love              ║
║          You               ║
║                            ║
║      Riya - Tanveer        ║
║ ❤️                      ❤️ ║
╚════════════════════════════╝
```

## 💡 Power Consumption

- **ESP32:** ~80mA (active with WiFi off)
- **OLED Display:** ~15mA (typical)
- **Total:** ~95mA
- **Battery Life (with 2000mAh battery):** ~21 hours

**Power Saving Tips:**
- Use deep sleep between quote changes
- Lower display brightness (if supported)
- Use USB power bank for extended use

## 🔋 Portable Setup Ideas

1. **USB Power Bank** - Easy and reliable
2. **3.7V LiPo Battery** - With ESP32 battery management
3. **AA Battery Holder** - 3x AA (4.5V) with regulator
4. **Coin Cell** - CR2032 holder (not recommended, too low power)

## 📝 Serial Monitor Output

When running, you'll see:
```
=== OLED Love Quotes Display ===
Initializing display...
Display initialized successfully!
Showing quote #1
Showing quote #2
...
Displaying final message!
```

## 🎁 Gift Ideas

This project makes a perfect gift! Suggestions:
- Mount in a picture frame
- Add to a decorative box with clear window
- Build into a desk ornament
- Create a bedside night display
- Make a wearable badge/pin

## 📸 Taking Photos/Videos

To capture the display:
- Use low-light camera settings
- Slow shutter speed (1/30s)
- Turn off flash
- Record video to show animation
- Use tripod for stability

## 🤝 Contributing

Feel free to:
- Add more romantic quotes
- Create different animation patterns
- Add sound effects (buzzer)
- Implement different heart designs
- Add more visual effects

## 📄 License

This is a personal project created with love for Riya by Tanveer ❤️

Feel free to use and modify for your own romantic projects!

## 💌 Credits

- **Created by:** Tanveer
- **Dedicated to:** Riya
- **Date:** January 2026
- **Libraries:** Adafruit GFX & SSD1306
- **Hardware:** ESP32 & SSD1306 OLED

---

Made with ❤️ by Tanveer for Riya

**Remember:** The best gift is not the code, but the thought behind it! 💝

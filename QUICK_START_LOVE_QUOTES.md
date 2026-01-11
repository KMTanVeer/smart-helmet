# Quick Start - OLED Love Quotes Display ⚡

Get your romantic OLED display running in 5 minutes!

## ✅ What You Need

- ESP32 board
- 0.94" OLED display (SSD1306, I2C)
- 4 jumper wires
- USB cable
- Computer with Arduino IDE

## 🔌 Step 1: Connect Hardware (2 minutes)

### Simple 4-Wire Connection:

```
OLED → ESP32
=============
VCC  → 3.3V
GND  → GND
SDA  → GPIO 21
SCL  → GPIO 22
```

**That's it!** Just 4 wires to connect.

## 💻 Step 2: Install Software (2 minutes)

### A. Install Arduino IDE (if needed)
Download from: https://www.arduino.cc/en/software

### B. Add ESP32 Support
1. Open Arduino IDE
2. Go to **File → Preferences**
3. In "Additional Board Manager URLs" paste:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
4. Click OK
5. Go to **Tools → Board → Boards Manager**
6. Search "esp32" → Install "esp32 by Espressif Systems"

### C. Install Libraries
1. Go to **Sketch → Include Library → Manage Libraries**
2. Search "Adafruit SSD1306" → Install
3. Search "Adafruit GFX" → Install
4. Done!

## 📤 Step 3: Upload Code (1 minute)

1. Open `oled-love-quotes-riya.ino`
2. Select board: **Tools → Board → ESP32 Dev Module**
3. Select port: **Tools → Port → (your ESP32 port)**
4. Click Upload button (→)
5. Wait for "Done uploading"

**Tip:** If upload fails, hold the BOOT button on ESP32 while uploading.

## 🎉 Step 4: Enjoy!

The display will show:
1. ❤️ Startup animation
2. 💕 9 romantic quotes (4 seconds each)
3. 💌 Final message: "I Love You Riya - Tanveer"

## 🆘 Not Working?

### Blank Screen?
- ✅ Check VCC is connected to 3.3V (not 5V)
- ✅ Check SDA → GPIO 21, SCL → GPIO 22
- ✅ Try changing `SCREEN_ADDRESS` from `0x3C` to `0x3D` in code

### Upload Failed?
- ✅ Hold BOOT button during upload
- ✅ Try different USB cable
- ✅ Check correct COM port selected

### Garbage on Screen?
- ✅ Check I2C address (0x3C or 0x3D)
- ✅ Verify all connections are secure
- ✅ Try restarting ESP32

## 🔧 Quick Customization

### Change Your Names
Find this in the code (around line 286):
```cpp
display.print(F("Riya - Tanveer"));
```
Replace with your names!

### Add Your Own Quote
Find the `quotes[]` array (around line 53) and add:
```cpp
"Your custom\nquote goes\nhere"
```

### Change Quote Time
Find this (around line 46):
```cpp
#define QUOTE_DISPLAY_TIME 4000   // 4 seconds
```
Change 4000 to any value (in milliseconds)

## 📊 What to Expect

- **Total quotes:** 9
- **Display time per quote:** 4 seconds
- **Heart animation:** Pulsing effect
- **Total cycle time:** ~36 seconds
- **Final message:** Stays on screen forever

## 🎥 Capture the Moment

Want to record it?
- Use your phone camera
- Record video (not photo) to show animation
- Use low light settings
- Turn off flash
- Keep camera steady

## 💡 Power Options

### Option 1: USB Cable (Easiest)
Just plug into computer or USB charger

### Option 2: Power Bank (Portable)
Connect ESP32 to any USB power bank

### Option 3: Battery (Advanced)
Use 3.7V LiPo battery with ESP32 battery connector

## 🎁 Make it Special

### Gift Ideas:
- Mount in a photo frame
- Put in decorative box with clear top
- Create desk ornament
- Make bedside display
- Add LED lights around it

### Optional Upgrades:
- Add buzzer for love songs
- Add button to cycle quotes manually
- Connect to battery for portable use
- Add acrylic case for protection

## 📱 Next Steps

Once working:
- Show it to Riya! 💝
- Take photos/videos
- Customize the quotes
- Add your own romantic messages
- Share your creation!

## 💌 Need Help?

Check the full documentation: `OLED_LOVE_QUOTES_README.md`

---

**Made with ❤️ for Riya by Tanveer**

Enjoy your romantic display! 💕

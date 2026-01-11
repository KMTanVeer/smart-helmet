/* 
 * =====================================================
 * OLED LOVE QUOTES ANIMATION FOR RIYA
 * =====================================================
 * 
 * A romantic display program that shows animated love quotes
 * and hearts on a 0.94" OLED display using ESP32
 * 
 * HARDWARE COMPONENTS:
 * --------------------
 * 1. ESP32 Development Board
 * 2. 0.94" OLED Display SSD1306 (128x64 pixels, I2C)
 * 
 * WIRING CONNECTIONS:
 * -------------------
 * OLED Display (SSD1306):
 *   - VCC  -> 3.3V
 *   - GND  -> GND
 *   - SDA  -> GPIO 21
 *   - SCL  -> GPIO 22
 * 
 * FEATURES:
 * ---------
 * - Animated heart icons
 * - Rotating romantic quotes
 * - Smooth text transitions
 * - Final dedication message
 * 
 * Author: Tanveer
 * For: Riya ❤️
 * Date: 2026
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

/* ================= OLED DISPLAY CONFIGURATION ================= */
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1  // Reset pin (-1 if sharing ESP32 reset pin)
#define SCREEN_ADDRESS 0x3C  // I2C address for 0.94" OLED

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

/* ================= TIMING CONFIGURATION ================= */
#define QUOTE_DISPLAY_TIME 4000   // Time to display each quote (ms)
#define HEART_ANIMATION_SPEED 100 // Heart beat animation speed (ms)

/* ================= ROMANTIC QUOTES FOR RIYA ================= */
const char* quotes[] = {
  "Every moment\nwith you is\na treasure",
  "You are my\nsunshine on\na cloudy day",
  "In your smile,\nI find my\nhappiness",
  "You make my\nheart skip\na beat",
  "Together is my\nfavorite place\nto be",
  "You are the\nreason I\nsmile daily",
  "My love for you\ngrows stronger\nevery day",
  "You complete me\nin every way\npossible",
  "Forever isn't\nlong enough\nwith you"
};

const int NUM_QUOTES = sizeof(quotes) / sizeof(quotes[0]);

/* ================= GLOBAL VARIABLES ================= */
int currentQuote = 0;
unsigned long lastQuoteChange = 0;
unsigned long lastHeartBeat = 0;
bool heartBig = true;
int animationPhase = 0;  // 0 = quote display, 1 = final message

/* ================= HEART BITMAP (8x8 pixels) ================= */
const unsigned char PROGMEM heartSmall[] = {
  0b00000000,
  0b01100110,
  0b11111111,
  0b11111111,
  0b01111110,
  0b00111100,
  0b00011000,
  0b00000000
};

const unsigned char PROGMEM heartBig[] = {
  0b01100110,
  0b11111111,
  0b11111111,
  0b11111111,
  0b11111111,
  0b01111110,
  0b00111100,
  0b00011000
};

void setup() {
  Serial.begin(115200);
  Serial.println(F("\n=== OLED Love Quotes Display ==="));
  Serial.println(F("Initializing display..."));
  
  // Initialize I2C
  Wire.begin();
  
  // Initialize OLED display
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed!"));
    Serial.println(F("Check connections:"));
    Serial.println(F("  SDA -> GPIO 21"));
    Serial.println(F("  SCL -> GPIO 22"));
    Serial.println(F("  VCC -> 3.3V"));
    Serial.println(F("  GND -> GND"));
    for(;;); // Don't proceed, loop forever
  }
  
  Serial.println(F("Display initialized successfully!"));
  
  // Clear the buffer
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  
  // Show startup animation
  showStartupAnimation();
  
  delay(2000);
  lastQuoteChange = millis();
}

void loop() {
  unsigned long currentTime = millis();
  
  if (animationPhase == 0) {
    // Quote display phase
    
    // Check if it's time to change quote
    if (currentTime - lastQuoteChange >= QUOTE_DISPLAY_TIME) {
      currentQuote++;
      
      // After showing all quotes, move to final message
      if (currentQuote >= NUM_QUOTES) {
        animationPhase = 1;
        showFinalMessage();
        return;
      }
      
      lastQuoteChange = currentTime;
      Serial.print(F("Showing quote #"));
      Serial.println(currentQuote + 1);
    }
    
    // Animate heart beat
    if (currentTime - lastHeartBeat >= HEART_ANIMATION_SPEED) {
      heartBig = !heartBig;
      lastHeartBeat = currentTime;
    }
    
    // Display current quote with animated hearts
    displayQuoteWithHearts(currentQuote);
    
  } else {
    // Final message phase - keep showing the final message
    
    // Animate heart beat on final message
    if (currentTime - lastHeartBeat >= HEART_ANIMATION_SPEED) {
      heartBig = !heartBig;
      lastHeartBeat = currentTime;
      displayFinalMessage();
    }
  }
  
  delay(50); // Small delay for stability
}

/* ================= DISPLAY FUNCTIONS ================= */

void showStartupAnimation() {
  // Show a heart growing animation
  for (int i = 0; i < 3; i++) {
    display.clearDisplay();
    
    // Draw expanding heart
    int size = 8 + (i * 4);
    int x = (SCREEN_WIDTH - size) / 2;
    int y = (SCREEN_HEIGHT - size) / 2;
    
    drawHeart(x, y, i == 2);
    
    // Show "For Riya" text
    display.setTextSize(1);
    display.setCursor(40, 50);
    display.print(F("For Riya"));
    
    display.display();
    delay(400);
  }
  
  delay(1000);
}

void displayQuoteWithHearts(int quoteIndex) {
  display.clearDisplay();
  
  // Draw animated hearts in corners
  drawHeart(5, 5, heartBig);           // Top left
  drawHeart(SCREEN_WIDTH - 13, 5, !heartBig);    // Top right
  drawHeart(5, SCREEN_HEIGHT - 13, !heartBig);   // Bottom left
  drawHeart(SCREEN_WIDTH - 13, SCREEN_HEIGHT - 13, heartBig); // Bottom right
  
  // Display quote text in center
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  // Calculate text position to center it
  int textY = 18;
  
  // Parse and display multi-line quote
  String quote = String(quotes[quoteIndex]);
  int lineNum = 0;
  int startPos = 0;
  
  while (startPos < quote.length()) {
    int newlinePos = quote.indexOf('\n', startPos);
    String line;
    
    if (newlinePos == -1) {
      line = quote.substring(startPos);
      startPos = quote.length();
    } else {
      line = quote.substring(startPos, newlinePos);
      startPos = newlinePos + 1;
    }
    
    // Center the text
    int textX = (SCREEN_WIDTH - (line.length() * 6)) / 2;
    display.setCursor(textX, textY + (lineNum * 10));
    display.print(line);
    lineNum++;
  }
  
  display.display();
}

void showFinalMessage() {
  Serial.println(F("Displaying final message!"));
  displayFinalMessage();
}

void displayFinalMessage() {
  display.clearDisplay();
  
  // Draw multiple hearts around the screen
  drawHeart(10, 8, heartBig);
  drawHeart(SCREEN_WIDTH - 18, 8, !heartBig);
  drawHeart(10, SCREEN_HEIGHT - 16, !heartBig);
  drawHeart(SCREEN_WIDTH - 18, SCREEN_HEIGHT - 16, heartBig);
  
  // Additional hearts
  drawHeart(SCREEN_WIDTH/2 - 4, 2, heartBig);
  drawHeart(SCREEN_WIDTH/2 - 4, SCREEN_HEIGHT - 10, !heartBig);
  
  // Display final message
  display.setTextSize(2);
  display.setCursor(10, 20);
  display.print(F("I Love"));
  
  display.setCursor(28, 38);
  display.print(F("You"));
  
  display.setTextSize(1);
  display.setCursor(10, 54);
  display.print(F("Riya - Tanveer"));
  
  display.display();
}

void drawHeart(int x, int y, bool big) {
  // Draw heart bitmap
  if (big) {
    display.drawBitmap(x, y, heartBig, 8, 8, SSD1306_WHITE);
  } else {
    display.drawBitmap(x, y, heartSmall, 8, 8, SSD1306_WHITE);
  }
}

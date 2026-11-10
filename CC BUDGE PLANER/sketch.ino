
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDR 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define LED_R 25
#define LED_G 26
#define LED_B 27

float budget = 0;
float expense = 0;

// Common-anode RGB LED: LOW means ON
void turnOffLED() {
  digitalWrite(LED_R, HIGH);
  digitalWrite(LED_G, HIGH);
  digitalWrite(LED_B, HIGH);
}

void setLED(bool red, bool green, bool blue) {
  digitalWrite(LED_R, red ? LOW : HIGH);
  digitalWrite(LED_G, green ? LOW : HIGH);
  digitalWrite(LED_B, blue ? LOW : HIGH);
}

void showMessage(const char* line1, const char* line2) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("BUDGET PLANNER");
  display.println("----------------");
  display.println(line1);
  display.println(line2);
  display.display();
}

void showBudget() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("BUDGET PLANNER");
  display.println("----------------");
  display.println("INPUT BUDGET:");
  display.print("Rs ");
  display.println(budget, 2);
  display.println("Enter expense...");
  display.display();
}

void showResult(const char* status, float balance) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  display.println("BUDGET PLANNER");
  display.println("----------------");
  display.print("Budget: Rs ");
  display.println(budget, 2);
  display.print("Expense: Rs ");
  display.println(expense, 2);
  display.print("Balance: Rs ");
  display.println(balance, 2);
  display.print("Status: ");
  display.println(status);
  display.display();
}

// Read one complete line from Serial Monitor.
// Reject letters, mixed text, empty input and invalid numbers.
bool readNumber(const char* prompt, float &value) {
  char input[40];
  size_t length = 0;
  bool tooLong = false;

  Serial.println(prompt);

  while (true) {
    while (Serial.available() > 0) {
      char c = Serial.read();

      if (c == '\r' || c == '\n') {
        if (length == 0 && !tooLong) continue;

        input[length] = '\0';

        if (tooLong) {
          Serial.println("Invalid input! Enter a number.");
          showMessage("Invalid input!", "Enter a number");
          return false;
        }

        char* endPointer;
        double parsed = strtod(input, &endPointer);

        while (isspace((unsigned char)*endPointer)) {
          endPointer++;
        }

        if (endPointer == input || *endPointer != '\0' ||
            !isfinite(parsed) || parsed > 1000000000.0 ||
            parsed < -1000000000.0) {
          Serial.println("Invalid input! Enter a number.");
          showMessage("Invalid input!", "Enter a number");
          return false;
        }

        value = (float)parsed;
        return true;
      }

      if (length < sizeof(input) - 1) {
        input[length++] = c;
      } else {
        tooLong = true;
      }
    }

    delay(5);
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
  turnOffLED();

  Wire.begin(21, 22);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED not found. Check wiring.");
    while (true) delay(100);
  }

  showMessage("System Ready", "Enter budget");
  Serial.println("\n=== BUDGET PLANNER ===");
}

void loop() {
  turnOffLED();

  // Step 1: Enter budget
  if (!readNumber("Enter budget (Rs):", budget)) {
    delay(1500);
    return;
  }

  if (budget <= 0) {
    Serial.println("Invalid budget! Enter an amount above zero.");
    showMessage("Invalid budget!", "Amount must be > 0");
    delay(1500);
    return;
  }

  // Immediately show the entered budget on OLED
  showBudget();

  Serial.print("Entered budget: Rs ");
  Serial.println(budget, 2);

  // Step 2: Enter expense
  if (!readNumber("Enter expense (Rs):", expense)) {
    delay(1500);
    showBudget();
    return;
  }

  if (expense < 0) {
    Serial.println("Invalid expense! Enter zero or more.");
    showMessage("Invalid expense!", "Enter amount >= 0");
    delay(1500);
    showBudget();
    return;
  }

  float balance = budget - expense;
  const char* status;

  if (expense > budget) {
    status = "OVER BUDGET";
    setLED(true, false, false);      // RED
  } else if (expense == budget) {
    status = "BUDGET REACHED";
    setLED(true, true, false);       // YELLOW
  } else if (expense >= budget * 0.8) {
    status = "NEAR LIMIT";
    setLED(true, true, false);       // YELLOW
  } else {
    status = "WITHIN LIMIT";
    setLED(false, true, false);      // GREEN
  }

  // Show all entered amounts and result
  showResult(status, balance);

  Serial.println("\n--- BUDGET RESULT ---");
  Serial.print("Input budget: Rs ");
  Serial.println(budget, 2);
  Serial.print("Input expense: Rs ");
  Serial.println(expense, 2);
  Serial.print("Remaining balance: Rs ");
  Serial.println(balance, 2);
  Serial.print("Status: ");
  Serial.println(status);

  // LED glows for 5 seconds, then switches off
  delay(5000);
  turnOffLED();

  Serial.println("LED OFF. Ready for next entry.");
}

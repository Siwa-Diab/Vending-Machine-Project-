#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
#include <Keypad.h>

#define buzzer 22
#define led1 30
#define led2 31
#define led3 32
#define led4 33
#define greenLed 27
#define redLed 28

// RFID
#define SS_PIN 53
#define RST_PIN 5
MFRC522 mfrc522(SS_PIN, RST_PIN);

// LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Adjust address as necessary

// Servos
Servo servo1;
Servo servo2;
Servo servo3;
Servo servo4;
Servo lockServo;


const byte ROWS = 4; // Four rows
const byte COLS = 4; // Four columns

// Define the keymap
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

// Connect the keypad ROW0, ROW1, ROW2, ROW3 to these Arduino pins
byte rowPins[ROWS] = {2, 3, 4, A1}; 
// Connect the keypad COL0, COL1, COL2, COL3 to these Arduino pins
byte colPins[COLS] = {6, 7, 8, 9}; 

// Create the Keypad object
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Variables
int cardBalance = 1000; // Initial balance for demonstration purposes
int item1Price = 100;
int item2Price = 125;
int item3Price = 75;
int item4Price = 150;
int newPrice = 0;

String password = "123456A";
String password_entered;
String symbol_entered;


void setup() {
  // Initialize serial communication for debugging
  Serial.begin(9600);
  Serial.println("Initializing...");

  // RFID setup
  SPI.begin();
  mfrc522.PCD_Init(SS_PIN, RST_PIN);
  Serial.println("RFID initialized");

  // Servo setup
  servo1.attach(A0);
  servo2.attach(11);
  servo3.attach(12);
  servo4.attach(13);
  lockServo.attach(10);
  Serial.println("Servos initialized");

  // LCD setup
  lcd.init();
  lcd.backlight();
  Serial.println("LCD initialized");


  pinMode(buzzer, OUTPUT);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(redLed, OUTPUT);

  // Turn on the always-on LEDs
  digitalWrite(led1, HIGH);
  digitalWrite(led2, HIGH);
  digitalWrite(led3, HIGH);
  digitalWrite(led4, HIGH);
  
  digitalWrite(greenLed, LOW);
  digitalWrite(redLed, LOW);
  Serial.println("Setup completed");

  // Initial display
  resetDisplay();

}

void loop() {
  char key = keypad.getKey();
  if (key != NO_KEY) {
    if (key == '*') {
      startDispensing();
    } else if (key == '#') {
      refillProcess();
    }
  }

}



void startDispensing() {
  lcd.clear();
  lcd.print("Scan your card");
  lcd.setCursor(0, 1);
  lcd.print("D-> Exit");

  while (true) {
    char key = keypad.getKey();
    if (key == 'D') {
      resetDisplay();
      return;
    }
    if (checkRFID()) {
      break;
    }
  }

  lcd.clear();
  lcd.print("Card read");
  lcd.setCursor(0, 1);
  lcd.print("Successfully");
  delay(3000);

  // Display initial balance
  lcd.clear();
  lcd.print("Balance:");
  lcd.print(cardBalance);
  lcd.print(" lbp ");
  delay(3000);

  while (true) {
    lcd.clear();
    lcd.print("Select item:");
    lcd.setCursor(0, 1);
    lcd.print("1 2 3 4  D:Exit");
    char key = keypad.getKey();
    if (key != NO_KEY) {
      switch (key) {
        case '1':
          processItemDispense(servo1, A0, item1Price);
          return; // Return to main loop to reset the display
        case '2':
          processItemDispense(servo2, 11, item2Price);
          return; // Return to main loop to reset the display
        case '3':
          processItemDispense(servo3, 12, item3Price);
          return; // Return to main loop to reset the display
        case '4':
          processItemDispense(servo4, 13, item4Price);
          return; // Return to main loop to reset the display
        case 'D':
          resetDisplay();
          return;
      }
    }
  }
}

void processItemDispense(Servo &servo, int pin, int itemPrice) {
  if (cardBalance >= itemPrice) {
    lcd.clear();
    lcd.print("price: ");
    lcd.print(itemPrice);
    lcd.print(" lbp ");
    cardBalance -= itemPrice;
    lcd.setCursor(0, 1);
    lcd.print("Balance: ");
    lcd.print(cardBalance);
    lcd.print(" lbp ");
    delay(3000);
    controlServo(servo, pin); // Use helper function to control the servo
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Receive your");
    lcd.setCursor(1, 1);
    lcd.print("item.");
    delay(5000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("SA7TEN :)");
    delay(3000);
  } else {
    lcd.clear();
    lcd.print("Insufficient");
    lcd.setCursor(0, 1);
    lcd.print("balance");
    delay(3000);
  }
  resetDisplay(); // Always reset display at the end
}

void controlServo(Servo &servo, int pin) {
  if (!servo.attached()) {
    servo.attach(pin);
  }
  servo.write(-180); // Rotate servo to 180 degrees
  delay(2000);
  servo.detach();
}

bool checkRFID() {
  if (!mfrc522.PICC_IsNewCardPresent()) {
    Serial.println("No new card present");
    delay(50);
    return false;
  }
  if (!mfrc522.PICC_ReadCardSerial()) {
    Serial.println("Error reading card serial");
    delay(50);
    return false;
  }

  String cardUID = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    cardUID.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""));
    cardUID.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.print("Card UID: ");
  Serial.println(cardUID);

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  return true;
}


void resetDisplay() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("*-Start");
  lcd.setCursor(0, 1);
  lcd.print("#-Refill /Price");

}

void refillProcess() {
  enum RefillState { SHOW_OPTIONS, ENTER_PASSWORD, CHANGE_PASSWORD, RFID_SCAN, RFID_MENU, SCAN_RFID,CHANGE_PRICES };
  RefillState currentState = SHOW_OPTIONS;
  RefillState previousState = SHOW_OPTIONS;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("By Pass:A");
  lcd.setCursor(0, 1);
  lcd.print("By Tag:C");
  lcd.setCursor(10, 1);
  lcd.print("D:Exit");

  while (true) {
    char key = keypad.getKey();
    if (key != NO_KEY) {

      switch (currentState) {
        case SHOW_OPTIONS:
          if (key == 'D') {
            resetDisplay();
            return;
          } else if (key == 'A') {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Enter pass->#");
            lcd.setCursor(0, 1);
            lcd.print("Change->*");
            lcd.setCursor(10, 1);
            lcd.print("B:Back");
            previousState = SHOW_OPTIONS;
            currentState = ENTER_PASSWORD;
          }else if (key == 'C') {
             lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Press each key");
            lcd.setCursor(0, 1);
            lcd.print("3 times");
            delay(3000);
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("1-Refill");
            lcd.setCursor(0, 1);
            lcd.print("2-Change Prices");
            previousState = SHOW_OPTIONS;
            currentState = RFID_MENU;
          }
          break;

    case ENTER_PASSWORD:
    if (key == 'B') {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("By Pass:A");
        lcd.setCursor(0, 1);
        lcd.print("By Tag:C");
        lcd.setCursor(10, 1);
        lcd.print("D:Exit");
        currentState = SHOW_OPTIONS;
    } else if (key == '#') {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("7 digit: ");
        lcd.setCursor(10, 0);
        lcd.print(" *:Del ");
        password_entered = "";
      
        while (password_entered.length() < 7) {
    char key = keypad.getKey();
    if (key != NO_KEY) {
        if (key == '*' && password_entered.length() > 0) {
            password_entered.remove(password_entered.length() - 1);
            lcd.setCursor(password_entered.length(), 1);
            lcd.print(' ');
            lcd.setCursor(password_entered.length(), 1);
        } else if (key != '*') {
            password_entered += key;
            lcd.setCursor(password_entered.length() - 1, 1);
            lcd.print(key);
        }
        Serial.print("Key pressed: ");
        Serial.println(key);
    }
}


if (password_entered == password) {
 digitalWrite(greenLed, HIGH); // Turn on the green LED for success
  for (int i = 0; i < 15; i++) { // Flash the buzzer for 3 seconds
    tone(buzzer, 2000);
  delay(100);
  noTone(buzzer);
  delay(100);
  } digitalWrite(greenLed, LOW); // Turn off the green LED
  lcd.setCursor(0, 0);
  lcd.print("CORRECT PASSWORD");
  lcd.setCursor(0, 1);
  lcd.print("DOOR OPENED");
  lockServo.write(180);
  delay(3000);
  lcd.clear();
    
  lcd.setCursor(0,0);
  lcd.print("1->4 To refill");
  lcd.setCursor(0,1);
  lcd.print("###->lock again");

while (true) {
        char key = keypad.getKey();
        if (key != NO_KEY) {
            if (key == '1') {
                controlServoReverse(servo1, A0);
            } else if (key == '2') {
                controlServoReverse(servo2, 11);
            } else if (key == '3') {
                controlServoReverse(servo3, 12);
            } else if (key == '4') {
                controlServoReverse(servo4, 13);
            } else if (key == '#') {
                // Check if the user wants to lock the door again
                symbol_entered = "";
                while (symbol_entered.length() < 2) {
                    char key = keypad.getKey();
                    if (key != NO_KEY) {
                        symbol_entered += key;
                    }
                }

                if (symbol_entered == "##") {
                    lockServo.write(90);
                    resetDisplay();
                    return;
                }
            }
        }
    } 
}
else {
            digitalWrite(redLed, HIGH); // Turn on the red LED for failure
          for (int i = 0; i < 6; i++) { // Flash the buzzer and red LED simultaneously for 3 seconds
            tone(buzzer, 2000);
            delay(250);
            noTone(buzzer);
            delay(250);
            }
          digitalWrite(redLed, LOW); // Turn off the red LED
            lcd.setCursor(0, 0);
            lcd.print("WRONG PASSWORD!");
            lcd.setCursor(0, 1);
            lcd.print("PLEASE TRY AGAIN");
            delay(3000);
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Enter pass->#");
            lcd.setCursor(0, 1);
            lcd.print("Change->*");
            lcd.setCursor(10, 1);
            lcd.print("B:Back");
        }
    } else if (key == '*') {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("old pass:");
        lcd.setCursor(0, 1);
        lcd.print(" *->Del ");
        lcd.setCursor(9, 1);
        lcd.print("B->Back");
        currentState = CHANGE_PASSWORD;
    }
    break;

case CHANGE_PASSWORD:
    if (key == 'B') {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Enter pass->#");
        lcd.setCursor(0, 1);
        lcd.print("Change->*");
        lcd.setCursor(10, 1);
        lcd.print("B:Back");
        currentState = ENTER_PASSWORD;
    } else if (key != NO_KEY) {
        password_entered = "";
        int pos = 9;  // Initial position after "old pass: "
        while (password_entered.length() < 7) {
            char key = keypad.getKey();
            if (key != NO_KEY) {
                if (key == '*' && password_entered.length() > 0) {
                    password_entered.remove(password_entered.length() - 1);
                    lcd.setCursor(pos - 1, 0);
                    lcd.print(' ');
                    pos--;
                    lcd.setCursor(pos, 0);
                } else if (key != '*') {
                    password_entered += key;
                    lcd.setCursor(pos++, 0);
                    lcd.print(key);
                }
            }
        }

        if (password_entered == password) {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Enter new pass:");
            lcd.setCursor(9, 1);
            lcd.print(" *:Del ");
            password_entered = "";

            while (password_entered.length() < 7) {
    char key = keypad.getKey();
    if (key != NO_KEY) {
        if (key == '*' && password_entered.length() > 0) {
            password_entered.remove(password_entered.length() - 1);
            lcd.setCursor(password_entered.length(), 1);
            lcd.print(' ');
            lcd.setCursor(password_entered.length(), 1);
        } else if (key != '*') {
            password_entered += key;
            lcd.setCursor(password_entered.length() - 1, 1);
            lcd.print(key);
        }
        Serial.print("Key pressed: ");
        Serial.println(key);
    }
}


            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Re-enter new pass:");
            lcd.setCursor(9, 1);
            lcd.print(" *:Del ");
            String new_password_confirm = "";
            
while (new_password_confirm.length() < 7) {
    char key = keypad.getKey();
    if (key != NO_KEY) {
        if (key == '*' && new_password_confirm.length() > 0) {
            // Remove the last character from the confirmation password
            new_password_confirm.remove(new_password_confirm.length() - 1);

            // Move cursor back and print space to 'erase' character from LCD
            lcd.setCursor(new_password_confirm.length(), 1);
            lcd.print(' ');

            // Move cursor back again to current position
            lcd.setCursor(new_password_confirm.length(), 1);
        } else if (key != '*') {
            // Add the key to the confirmation password
            new_password_confirm += key;

            // Print the key to the LCD
            lcd.setCursor(new_password_confirm.length() - 1, 1);
            lcd.print(key);
        }
    }
}

            if (password_entered == new_password_confirm) {
                password = new_password_confirm;
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Password changed");
                delay(3000);
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Enter pass->#");
                lcd.setCursor(0, 1);
                lcd.print("Change->*");
                lcd.setCursor(10, 1);
                lcd.print("B:Back");
                currentState = ENTER_PASSWORD;
            } else {
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Passwords don't");
                lcd.setCursor(0, 1);
                lcd.print("match!!");
                delay(3000);
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Enter pass->#");
                lcd.setCursor(0, 1);
                lcd.print("Change->*");
                lcd.setCursor(10, 1);
                lcd.print("B:Back");
                currentState = ENTER_PASSWORD;
            }
        } else {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Incorrect pass");
            delay(3000);
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Enter pass->#");
            lcd.setCursor(0, 1);
            lcd.print("Change->*");
            lcd.setCursor(10, 1);
            lcd.print("B:Back");
            currentState = ENTER_PASSWORD;
        }
    }
    break;


    case RFID_MENU:

    char key;
    while (true) {
        key = keypad.getKey(); // Get the key press
        if (key != NO_KEY) {
          Serial.print("Key pressed: ");
            Serial.println(key); // Check if a key was pressed
            if (key == '1') {
                currentState = RFID_SCAN;
                break; // Exit the loop after state transition
            } else if (key == '2') {
                currentState = SCAN_RFID;
                break; // Exit the loop after state transition
            }
        }
    }
    break;



    case SCAN_RFID:
       lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Scan Card");
          lcd.setCursor(0, 1);
          lcd.print("B->Back");
           previousState = SCAN_RFID; // Save the previous state
           currentState = SCAN_RFID; // Continue scanning

           while (true) {
        char key = keypad.getKey();
        if (key == 'B') {
            // Go back 
            refillProcess();
            return;
        }
        if (checkRFID()) {
           lcd.clear();
            lcd.print("Card Detected");
            delay(2000);
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Press any key");
            lcd.setCursor(0, 1);
            lcd.print("to change price");
          currentState = CHANGE_PRICES;
          break;
        }
           }
           break;

case CHANGE_PRICES: 
 
    while (true) {
        lcd.clear();
        lcd.print("Choose item(1->4)");
        lcd.setCursor(0, 1);
        lcd.print("*: Back");
        previousState = SCAN_RFID;
        currentState = CHANGE_PRICES;
        while (true) {
            char key = keypad.getKey();
            if (key != NO_KEY) {
                if (key == '*') {
                    refillProcess();
                    currentState = previousState;
                    return;
                }
                if (key >= '1' && key <= '4') {
                    int itemNum = key - '0';
                    lcd.clear();
                    lcd.print("Set price:");
                    lcd.setCursor(0, 1);
                    lcd.print("--- #=Done *:Del");

                    String newPriceStr = ""; // Start with an empty string for dynamic length
                    int cursorPos = 0; // Position of the cursor on LCD

                    while (true) {
                        char key = keypad.getKey();
                        if (key != NO_KEY) {
                            if (key == '#') {
                                int newPrice = newPriceStr.toInt();
                                switch (itemNum) {
                                    case 1:
                                        item1Price = newPrice;
                                        break;
                                    case 2:
                                        item2Price = newPrice;
                                        break;
                                    case 3:
                                        item3Price = newPrice;
                                        break;
                                    case 4:
                                        item4Price = newPrice;
                                        break;
                                }
                                lcd.clear();
                                lcd.print("Price updated");
                                delay(2000);
                                currentState = SCAN_RFID;
                                break;  // Exit the inner while loop after updating price
                            } else if (key == '0' && newPriceStr.toInt() == 0 && cursorPos == 0) {
                                // Skip leading zeros if entered (optional)
                                continue;
                            } else if (key == '*' && cursorPos > 0) {
                                // Backspace if there are characters to delete
                                lcd.setCursor(--cursorPos, 1);
                                lcd.print(' ');
                                newPriceStr.remove(newPriceStr.length() - 1); // Remove last character
                            } else if (cursorPos < 3) {
                                // Allow up to 3 digits
                                lcd.setCursor(cursorPos++, 1);
                                lcd.print(key);
                                newPriceStr += key; // Append entered key to string
                            }
                        }
                    }
                    break;
                }
            }
        }
    }
    break;

          


  case RFID_SCAN:
    lcd.clear();
    lcd.print("Scan refill card");
    lcd.setCursor(0, 1);
    lcd.print("B-> Back");
    previousState = RFID_SCAN; // Save the previous state
    currentState = RFID_SCAN; // Continue scanning

    while (true) {
        char key = keypad.getKey();
        if (key == 'B') {
            // Go back
            refillProcess();;
            return;
        }
        if (checkRFID()) {
            // If an RFID card is scanned
            lcd.clear();
            lcd.print("DOOR OPENED");
            lockServo.write(180); // Open the door
            delay(3000);
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("1->4 To refill");
            lcd.setCursor(0,1);
            lcd.print("Scan to lock");

            // Loop to allow the user to refill items
            while (true) {
                char key = keypad.getKey();
                if (key != NO_KEY) {
                    if (key == '1') {
                        controlServoReverse(servo1, A0);
                    } else if (key == '2') {
                        controlServoReverse(servo2, 11);
                    } else if (key == '3') {
                        controlServoReverse(servo3, 12);
                    } else if (key == '4') {
                        controlServoReverse(servo4, 13);
                    }
                }
                // Check if RFID card is scanned again to lock the door
                if (checkRFID()) {
                    lockServo.write(90); // Lock the door
                    lcd.clear();
                    lcd.print("DOOR LOCKED");
                    delay(2000);
                    resetDisplay();
                    return;
                }
            }
        }
    }
    break;


default:
    resetDisplay();
    return;
}

    }
  }
}

void controlServoReverse(Servo &servo, int pin) {
  if (!servo.attached()) {
    servo.attach(pin);
  }
  servo.write(180); // Rotate servo to 0 degrees
  delay(2000);
  servo.detach();
}
/**************************************************************************
StepVorschub

An electronic feed for my manual lathe.

My lathe have an feed and thread gearbox. But I have to change the gears
manualy, there is no Norton gearbox. So every time when I want to change the
feed rate or the thread pitch I have to disasseble the current gears
and mount the new ones. Maybe only for a short movement. Thats a lot of work!

Now I have replaced these gears by this electronic feed and thread controller.
It consists of:
- An Arduino Nano with an 0.9" GLCD and a manual rotary encoder for human interface.
- An quadrature encoder with high resolution to track the position of the main spindle.
- An stepper motor and the driver attached to the original feed / thread gearbox.

The handling is now much easier!
I only have to select the needed feed or thread pitch value on the controller.
And of course the right settings on the lathe. (Bed slide engagement, leadscrew or towbar, ...)
It also adds a motion limit function. So after setup the motion will stop
always at the same position.

Written by Dirk Radloff, with contributions from the open source community.
GNU license, check license.txt for more information. All text above must be
included in any redistribution.

Hosted on Github:
  https://github.com/NixToDo/StepVorschub

This software uses following libraries:
- For the display: Adafruit SSD1306 and the top level GFX library
  https://github.com/adafruit/Adafruit_SSD1306
  https://github.com/adafruit/Adafruit-GFX-Library
- For the manual quadrature encoder:
  https://github.com/MajicDesigns/MD_REncoder
- For debouncing the manual encoder push button:
  https://github.com/thomasfredericks/Bounce2
**************************************************************************/
#include <avr/interrupt.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MD_REncoder.h>
#include <Bounce2.h>

// Pins
#define MENC_A_PIN 14 // Manual encoder
#define MENC_B_PIN 15
#define BUTTON_PIN 16
#define SENC_A_PIN 4  // Spindle encoder
#define SENC_B_PIN 5
#define SENC_Z_PIN 3
#define STEP_PIN   9  // Stepper driver
#define DIR_PIN    10
#define EN_PIN     11

// Const
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define SENC_PULSES_PER_TURN 25000 // Spindle encoder counts per turn
#define STEPS_MM_TOWBAR ((200 * 8 * 2 * 2) / 1.36) // Steps needed per 1 mm travel
#define STEPS_MM_LEADSCREW ((200 * 8 * 2 * 2) / 3.0) // Steps needed per 1 mm/U
#define RESTFACTOR 32768 // Fixed point arithmetic factor

// Limits
#define MIN_STEIGUNG 0.0 // mm / U
#define MAX_STEIGUNG 5.0 // mm / U
#define MIN_TPI      0  // Turns per inch
#define MAX_TPI      60 // Turns per inch

// Button states
#define EB_NORMAL   0
#define EB_PRESSED  1
#define EB_DOWN     2
#define EB_RELEASED 3
#define EB_LONG     4
#define EB_DOUBLE   5

#define EB_LONG_TIME   1000
#define EB_DOUBLE_TIME 500

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Set up encoder object
MD_REncoder Encoder = MD_REncoder(MENC_A_PIN, MENC_B_PIN);

// Instantiate a Bounce object
Bounce Debouncer = Bounce(); 

// Direction
typedef enum {
  Dir_Unknown,
  Dir_Left,
  Dir_Right
} Direction_t;

// Global variables
float Vorschub = 0.1;
float Gewinde = 1.0;
uint8_t TPI = 20;
float CntPos = 1.0;
volatile uint16_t TargetValue = 0, TargetRest = 0, ReloadRest = 0;
volatile Direction_t Cur_Direction = Dir_Unknown, Old_Direction = Dir_Unknown;
volatile bool UseStartPosition = false;
volatile bool UseStopPosition = false;
volatile int32_t StopPosition = 0;
volatile int32_t Position = 0;


void Timer1_Off (void){
  TCCR1B = 0; // STOP Timer1
  EIMSK &= ~(1<<INT1); // Disable INT1
  PCICR &= ~(1<<PCIE2); // Disable pin change
}

void Timer1_On (void){
  TCCR1B = (1<<WGM12) | (1<<WGM13); // STOP Timer1, CTC mode 12
  TCCR1A = (1<<COM1A0); // OCR1A toggle mode
  TCNT1  = 0;
  ICR1   = SENC_PULSES_PER_TURN;
  OCR1A  = TargetValue;
  ReloadRest = 0;
  TIFR1  = (1<<OCF1A); // OCIE cleared
  TIMSK1 = (1<<OCIE1A); // OCIE enabled
  TCCR1B |= (1<<CS12) | (1<<CS11) | (1<<CS10); // Start Timer1 ext. Clk
  Cur_Direction = Dir_Unknown;
  Direction_detection_enable();
  UseStartPosition = false;
  UseStopPosition = false;
}

void Direction_detection_enable (void){
  PCMSK2 |= (1<<PCINT20) | (1<<PCINT21); // Enable pin change interrupt on PD4 & PD5 (SENC_A_PIN & SENC_B_PIN)
  PCIFR |= (1<<PCIF2);
  PCICR |= (1<<PCIE2);
}

ISR(PCINT2_vect){
  // Direction detection
  static uint8_t state = 0x01;
  state = state | (PIND & 0x30);
  
  switch (state){
    case 0x20:
    case 0xB0:
    case 0xD0:
    case 0x40:
      // Set stepper DIR_PIN
      PORTB |= (1<<PB2);
      PCICR &= ~(1<<PCIE2); // Disable INT
      state = 0x01;
      Old_Direction = Cur_Direction;
      Cur_Direction = Dir_Right;
      break;
    
    case 0x10:
    case 0x70:
    case 0xE0:
    case 0x80:
      // Clear stepper DIR_PIN
      PORTB &= ~(1<<PB2);
      PCICR &= ~(1<<PCIE2); // Disable INT
      state = 0x01;
      Old_Direction = Cur_Direction;
      Cur_Direction = Dir_Left;
      break;
    
    default:
      // First edge will end here
      state = ((state<<2) & 0xC0);
      break;
  }
}

ISR(TIMER1_COMPA_vect){
  // Step output OCR1A reloader
  if ((Old_Direction != Dir_Unknown) && (Old_Direction != Cur_Direction)){ // Correct counting value because an direction change occur
    int16_t tmp = TCNT1;
    int16_t dup = OCR1A - tmp; // Delta to next step
    int16_t ddn = tmp - (OCR1A - (2 * TargetValue)); // Delta to last step
    int16_t delta = dup - ddn; // Delta between last and next step
    tmp += delta; // Add delta to correct directional change
    
    if(tmp >= SENC_PULSES_PER_TURN){ // Correct overflow
      tmp -= (65535 - SENC_PULSES_PER_TURN);
    }

    TCNT1 = tmp;
    // BUG: ReloadRest correction needed?
    Old_Direction = Cur_Direction; // Mark as done
  }

  // Calculate next step puls target
  OCR1A += TargetValue;
  ReloadRest += TargetRest;
  
  if (ReloadRest >= RESTFACTOR){ // Rest bigger or equal than one step?
    OCR1A++;
    ReloadRest -= RESTFACTOR;
  }
  
  if (OCR1A >= SENC_PULSES_PER_TURN) // Reload bigger or equal than encoder counts?
    OCR1A -= SENC_PULSES_PER_TURN;
  
  if (Cur_Direction == Dir_Right) // Count current position
    Position++;
  else
    Position--;
  
  if (UseStartPosition == true){ // 1.Limit
    if (Position == 0){
      if ((TCCR1A & (1<<COM1A0)))
        TCCR1A &= ~(1<<COM1A0);
      else
        TCCR1A |= (1<<COM1A0);
    }
  }
  
  if (UseStopPosition == true){ // 2.Limit
    if (Position == StopPosition){
      if ((TCCR1A & (1<<COM1A0)))
        TCCR1A &= ~(1<<COM1A0);
      else
        TCCR1A |= (1<<COM1A0);
    }
  }
  
  delayMicroseconds(5);
  TCCR1C |= (1<<FOC1A); // Force compare match to clear step pin again
}

uint8_t Standstill (void){
  
  if ((PCICR & (1<<PCIE2)) != 0) // DIR detection active
    return(2);
  
  uint16_t tmp = TCNT1;
  delay(100);
  
  if (tmp == TCNT1) // Change in Timer 1 value?
    return(1);
  else
    return(0); // Spindel is running
}

void Calc_Steps (float Steigung, float Steps_MM){
  float pulsStep = (float)SENC_PULSES_PER_TURN / (Steps_MM * Steigung);
  TargetValue = (uint16_t)pulsStep;
  TargetRest  = (uint16_t)((pulsStep - (float)TargetValue) * RESTFACTOR) + 0.5;
}

uint8_t Encoder_Button_Readout (int8_t *cnt){
  static uint32_t lasthightime = 0;
  
	switch (Encoder.read())
	{
		case DIR_NONE:
			break;
		
		case DIR_CW:
			(*cnt)++;
			break;
		
		case DIR_CCW:
			(*cnt)--;
			break;
	}
  
  Debouncer.update();
  
  if (Debouncer.fell() == true){ // Button goes down detected
    lasthightime = Debouncer.previousDuration();
		return (EB_PRESSED);
  }
  
	if (Debouncer.rose() == true){ // Button goes up
    if (lasthightime < EB_DOUBLE_TIME) // Double click?
      return (EB_DOUBLE);
    
    return (EB_RELEASED);
	}
  
	if (Debouncer.read() == LOW){ // Button down
    if (Debouncer.duration() >= EB_LONG_TIME) // Long press?
      return (EB_LONG);
    
		return (EB_DOWN);
	}
  
	return (EB_NORMAL); // Button untouched
}

uint8_t Main_Menu (bool draw, int8_t *enccnt, uint8_t bclick, uint8_t rtnmenu){
  static uint8_t menupos = 0;
  
  if (bclick == EB_PRESSED){ // Go to selected menu item
    *enccnt = 0;
    return(menupos);
  }
  
	if (*enccnt < 1) // Limit encoder
		*enccnt = 1;
	else if (*enccnt > 3)
		*enccnt = 3;
	
	if (draw == true || *enccnt != menupos){ // Draw it only if needed
    menupos = *enccnt;
    display.clearDisplay();
		display.setCursor(0, 0);
		display.println(F("Main menu:"));
		display.println(F(" 1 Feed"));
		display.println(F(" 2 Metric"));
		display.println(F(" 3 TPI"));
		display.setCursor(0, (menupos * 16));
		display.print(F(">"));
		display.display();
	}
	
  return(0);
}

uint8_t Feed_Screen (bool draw, int8_t *enccnt, uint8_t bclick, uint8_t rtnmenu){
  if (bclick == EB_LONG){ // Return to main menu
    PORTB &= ~(1<<PB3); // Disable stepper
    return (0);
  }
  
  if (bclick == EB_DOUBLE){ // Go to limit menu
    return (4);
  }
  
  if (bclick == EB_PRESSED){ // Change rotay resolution
    if (CntPos > 0.01)
      CntPos /= 10.0;
    else
      CntPos = 1.0;
    
    return (1);
  }
  
  if (draw == true) // Set it at menu entry
    CntPos = 1.0;
  
  if (draw == true || *enccnt != 0){ // Draw it only if needed
		if (rtnmenu != 4){ // Don't change value after return from limit menu
		  Vorschub += (*enccnt * CntPos);
      *enccnt = 0;
      
      if (Vorschub > MAX_STEIGUNG) // Limit value
        Vorschub = MAX_STEIGUNG;
		  else if (Vorschub < MIN_STEIGUNG)
        Vorschub = MIN_STEIGUNG;
      
      if (Vorschub != 0.0)
        PORTB |= (1<<PB3); // Enable stepper
      else
        PORTB &= ~(1<<PB3); // Disable stepper at 0.0 mm/U
      
      // Calculate new value
      Timer1_Off();
      Calc_Steps(Vorschub, STEPS_MM_TOWBAR);
      Timer1_On();
    }
    
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(F("Feed:"));
    display.print(Vorschub, 2);
    display.println(F(" mm/U"));

    if (UseStartPosition == true)
      display.print(F("Limit 1"));
    if (UseStopPosition == true)
      display.print(F("&2"));
    
    display.display();
  }
  
  return (1);
}

uint8_t Thread_MM_Screen (bool draw, int8_t *enccnt, uint8_t bclick, uint8_t rtnmenu){
  if (bclick == EB_LONG){ // Return to main menu
    PORTB &= ~(1<<PB3); // Disable stepper
    return (0);
  }
  
  if (bclick == EB_DOUBLE){ // Go to limit menu
    return (4);
  }
  
  if (bclick == EB_PRESSED){ // Change rotay resolution
    if (CntPos > 0.01)
      CntPos /= 10.0;
    else
      CntPos = 1.0;
    
    return (2);
  }
  
  if (draw == true) // Set it at menu entry
    CntPos = 1.0;
  
  if (draw == true || *enccnt != 0){ // Draw it only if needed
    if (rtnmenu != 4){ // Don't change value after return from limit menu
      Gewinde += (*enccnt * CntPos);
      *enccnt = 0;
      
      if (Gewinde > MAX_STEIGUNG) // Limit value
        Gewinde = MAX_STEIGUNG;
      else if (Gewinde < MIN_STEIGUNG)
        Gewinde = MIN_STEIGUNG;
      
      if (Gewinde != 0.0)
        PORTB |= (1<<PB3); // Enable stepper
      else
        PORTB &= ~(1<<PB3); // Disable stepper at 0.0 mm/U
      
      // Calculate new value
      Timer1_Off();
      Calc_Steps(Gewinde, STEPS_MM_LEADSCREW);
      Timer1_On();
    }
    
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(F("Thread:"));
    display.print(Gewinde, 2);
    display.println(F(" mm/U"));
    
    if (UseStartPosition == true)
      display.print(F("Limit 1"));
    if (UseStopPosition == true)
      display.print(F("&2"));
    
    display.display();
  }
  
  return (2);
}

uint8_t Thread_TPI_Screen (bool draw, int8_t *enccnt, uint8_t bclick, uint8_t rtnmenu){
  if (bclick == EB_LONG){ // Return to main menu
    PORTB &= ~(1<<PB3); // Disable stepper
    return (0);
  }
  
  if (bclick == EB_DOUBLE){ // Go to limit menu
    return (4);
  }
  
  if (bclick == EB_PRESSED){ // Change rotay resolution
    if (CntPos > 1.0)
      CntPos /= 10.0;
    else
      CntPos = 10.0;
    
    return (3);
  }
  
  if (draw == true) // Set it at menu entry
    CntPos = 10.0;
  
  if (draw == true || *enccnt != 0){ // Draw it only if needed
    if (rtnmenu != 4){ // Don't change value after return from limit menu
      TPI += (*enccnt * (int8_t)CntPos);
      *enccnt = 0;
      
      if (TPI > MAX_TPI) // Limit value
        TPI = MAX_TPI;
      else if (TPI < MIN_TPI)
        TPI = MIN_TPI;
      
      if (TPI != 0.0)
        PORTB |= (1<<PB3); // Enable stepper
      else
        PORTB &= ~(1<<PB3); // Disable stepper at 0.0 mm/U
      
      // Calculate new value
      Timer1_Off();
      Calc_Steps((25.4 / TPI), STEPS_MM_LEADSCREW);
      Timer1_On();
    }
    
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(F("Thread:"));
    display.print(TPI);
    display.println(F(" TPI"));
    
    if (UseStartPosition == true)
      display.print(F("Limit 1"));
    if (UseStopPosition == true)
      display.print(F("&2"));
    
    display.display();
  }
  
  return (3);
}

uint8_t Sub_Menu (bool draw, int8_t *enccnt, uint8_t bclick, uint8_t rtnmenu){
  static uint8_t menupos = 0, rtnm = 0;
  
  if (bclick == EB_PRESSED){ // Menu item selected
    if (Standstill() > 0){ // Only if spindle does not rotate
      switch (menupos){
        case 1: // 1. Limit
          UseStartPosition = true;
          UseStopPosition = false;
          Position = 0;
          break;
          
        case 2: // 2. Limit
          if (UseStartPosition == true){
            UseStopPosition = true;
            StopPosition = Position;
          }
          break;
          
        default: // Return
          break;
      }
    }
    
    *enccnt = 0;
    return(rtnm);
  }
  
  if (*enccnt < 1) // Limit encoder
    *enccnt = 1;
  else if (*enccnt > 3)
    *enccnt = 3;
  
  if (draw == true){ // Draw display anyway
    rtnm = rtnmenu;
    
    if (UseStartPosition == true && UseStopPosition == false) // Second call: directly to second limit
      *enccnt = 2;
    else if (UseStartPosition == true && UseStopPosition == true) // Thrid call: directly to return
      *enccnt = 3;
  }
  
  if (draw == true || *enccnt != menupos){ // Draw it only if needed
    menupos = *enccnt;
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(F("Set:"));
    display.println(F(" 1. Limit"));
    display.println(F(" 2. Limit"));
    display.println(F(" Return"));
    display.setCursor(0, (menupos * 16));
    display.print(F(">"));
    display.display();
  }
  
  return (4);
}

void Menu_Selector (void){
  static uint8_t curmenu = 0, oldmenu = 0xFF, lastmenu = 0;
	static int8_t enccnt = 0;
	uint8_t bt = Encoder_Button_Readout(&enccnt);
  bool menuchanged;
  
	if (curmenu != oldmenu){ // Record of last selected menus
    menuchanged = true;
    lastmenu = oldmenu;
    oldmenu = curmenu;
  }
  else
    menuchanged = false;
  
  switch (curmenu){
    case 0:
    default:
      curmenu = Main_Menu(menuchanged, &enccnt, bt, lastmenu);
      break;
    
    case 1:
      curmenu = Feed_Screen(menuchanged, &enccnt, bt, lastmenu);
      break;
    
    case 2:
      curmenu = Thread_MM_Screen(menuchanged, &enccnt, bt, lastmenu);
      break;
    
    case 3:
      curmenu = Thread_TPI_Screen(menuchanged, &enccnt, bt, lastmenu);
      break;
    
    case 4:
      curmenu = Sub_Menu(menuchanged, &enccnt, bt, lastmenu);
      break;
  }
}

void setup() {
  Serial.begin(57600);
  Serial.print(F("Init "));
  
  // Setup pins
  pinMode(SENC_A_PIN, INPUT);
  pinMode(SENC_B_PIN, INPUT);
  pinMode(SENC_Z_PIN, INPUT);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(EN_PIN, OUTPUT);
  
  // Disable stepper
  PORTB &= ~(1<<PB3);
  
  // Encoder
  Encoder.begin();
  
  // After setting up the button, setup the Bounce instance :
  Debouncer.attach(BUTTON_PIN, INPUT_PULLUP);
  Debouncer.interval(10); // interval in ms
  
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  
  // Clear the buffer
  display.clearDisplay();
  display.setTextSize(2); // 2x pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, 0); // Start at top-left corner
  display.cp437(true); // Use full 256 char 'Code Page 437' font
  
  Serial.println(F("done"));
}

void loop() {
  Menu_Selector();
  
  if (Standstill() == 1){ // At standstill, start DIR detection again
    Direction_detection_enable();
  }
}

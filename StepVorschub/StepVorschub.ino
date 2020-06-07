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
#define RESTFACTOR 32768

// Limits
#define MIN_STEIGUNG 0.0 // mm / U
#define MAX_STEIGUNG 5.0 // mm / U

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Set up encoder object
MD_REncoder R = MD_REncoder(MENC_A_PIN, MENC_B_PIN);

// Instantiate a Bounce object
Bounce debouncer = Bounce(); 

// Direction
typedef enum {
  Dir_Unknown,
  Dir_Left,
  Dir_Right
} Direction_t;

// Global variables
bool Mode = false;
float Steps_MM = STEPS_MM_TOWBAR;
float Vorschub = 0.1;
float Gewinde = 1.0;
float Steigung = Vorschub;
float CntPos = 1.0;
volatile uint16_t TargetValue = 0, TargetRest = 0, ReloadRest = 0;
volatile Direction_t Cur_Direction = Dir_Unknown, Old_Direction = Dir_Unknown;
volatile bool Stop_Position = false;
volatile int32_t Position = 0;


void Timer1_Off (void)
{
  TCCR1B = 0; // STOP Timer1
  EIMSK &= ~(1<<INT1); // Disable INT1
  PCICR &= ~(1<<PCIE2); // Disable pin change
}

void Timer1_On (void)
{
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
  Stop_Position = false;
}
/*
void Timer1_On_Sync (void)
{
  TCCR1B = (1<<WGM12) | (1<<WGM13); // STOP Timer1, CTC mode 12
  TCCR1A = (1<<COM1A0); // OCR1A toggle mode
  TCNT1  = 0;
  ICR1   = SENC_PULSES_PER_TURN;
  OCR1A  = TargetValue;
  ReloadRest = 0;
  TIFR1  = (1<<OCF1A); // OCIE cleared
  TIMSK1 = (1<<OCIE1A); // OCIE enabled
  EICRA |= (1<<ISC11) | (1<<ISC10); // INT1 Spindle index signal at rising edge
  EIFR  |= (1<<INTF1); // Clear flag
  EIMSK |= (1<<INT1); // Enable INT1
  Cur_Direction = Dir_Unknown;
  Direction_detection_enable();
  Stop_Position = false;
}
*/
// Enable pin change interrupt on PD4 & PD5 (SENC_A_PIN & SENC_B_PIN)
void Direction_detection_enable (void)
{
  PCMSK2 |= (1<<PCINT20) | (1<<PCINT21);
  PCIFR |= (1<<PCIF2);
  PCICR |= (1<<PCIE2);
}

// Direction detection
ISR(PCINT2_vect)
{
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
/*
// Start detection by Index signal
ISR(INT1_vect)
{
  TCCR1B |= (1<<CS12) | (1<<CS11) | (1<<CS10); // Start Timer1 ext. Clk
  EIMSK  &= ~(1<<INT1); // Disable INT1
}
*/
// Step output OCR1A reloader
ISR(TIMER1_COMPA_vect)
{
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

  if (Stop_Position == true){
    if (Cur_Direction == Dir_Right)
      Position++;
    else
      Position--;
    
    if (Position == 0){
      if ((TCCR1A & (1<<COM1A0)))
        TCCR1A &= ~(1<<COM1A0);
      else
        TCCR1A |= (1<<COM1A0);
    }
  }
  
  delayMicroseconds(5);
  TCCR1C |= (1<<FOC1A); // Force compare match to clear step pin again
}

uint8_t Standstill (void)
{
  if ((PCICR & (1<<PCIE2)) != 0)
    return(2);
  
  uint16_t tmp = TCNT1;
  delay(100);
  
  if (tmp == TCNT1)
    return(1);
  else
    return(0);
}

void Calc_Steps (void)
{
  float pulsStep;
  
  pulsStep = (float)SENC_PULSES_PER_TURN / (Steps_MM * Steigung);
  TargetValue = (uint16_t)pulsStep;
  TargetRest  = (uint16_t)((pulsStep - (float)TargetValue) * RESTFACTOR) + 0.5;
}

void setup() {
  Serial.begin(57600);
  Serial.print(F("Init "));
  R.begin();

  // Setup pins
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(SENC_A_PIN, INPUT);
  pinMode(SENC_B_PIN, INPUT);
  pinMode(SENC_Z_PIN, INPUT);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(EN_PIN, OUTPUT);
  
  // After setting up the button, setup the Bounce instance :
  debouncer.attach(BUTTON_PIN);
  debouncer.interval(5); // interval in ms
  
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
  
  display.println(F("Vorschub:"));
  display.print(Steigung, 2);
  display.println(F(" mm/U"));
  display.display();

  Timer1_Off();
  Calc_Steps();
  PORTB |= (1<<PB3); // Enable stepper
  Timer1_On();
  
  Serial.println(F("done"));
}

void loop() {
  debouncer.update();
  uint8_t x = R.read();
  bool btpress = debouncer.fell();
  bool btlow = debouncer.read();
    
  // Check for button press
  if (btpress == true){
    if (CntPos > 0.01)
      CntPos /= 10.0;
    else
      CntPos = 1.0;
  }
  
  // Check for manual encoder turn
  if (btlow == HIGH){ // Encoder button not pressed
    if (x == DIR_CW){
      Steigung += CntPos;
      
      if (Steigung > MAX_STEIGUNG)
        Steigung = MAX_STEIGUNG;
    }
    else if (x == DIR_CCW){
      Steigung -= CntPos;
      
      if (Steigung < MIN_STEIGUNG)
        Steigung = MIN_STEIGUNG;
    }
  }
  else { // Encoder button pressed
    if (x == DIR_CW){
      Gewinde = Steigung;
      Steigung = Vorschub;
      Steps_MM = STEPS_MM_TOWBAR;
      Mode = false;
    }
    else if (x == DIR_CCW){
      Vorschub = Steigung;
      Steigung = Gewinde;
      Steps_MM = STEPS_MM_LEADSCREW;
      Mode = true;
    }
    else if ((x == DIR_NONE) && (Standstill() > 0)){
      Stop_Position = true;
      Position = 0;
      display.setCursor(40, 32);
      display.print(F("STOP"));
      display.setCursor(16, 49);
      display.print(F("gesetzt!"));
      display.display();
    }
  }
  
  if (x != DIR_NONE){
    Timer1_Off();
    Calc_Steps();
    Timer1_On();
    display.clearDisplay();
    display.setCursor(0, 0);
    
    if (Mode == false){
      display.println(F("Vorschub:"));
      //Timer1_On_Now();
    }
    else{
      display.println(F("Gewinde:"));
      //Timer1_On_Sync();
    }
    
    display.print(Steigung, 2);
    display.println(F(" mm/U"));
    display.display();
    
    if (Steigung != 0.0)
      PORTB |= (1<<PB3); // Enable stepper
    else
      PORTB &= ~(1<<PB3); // Disable stepper at 0.0 mm/U
  }
  
  if (Standstill() == 1){
      Direction_detection_enable();
  }
}

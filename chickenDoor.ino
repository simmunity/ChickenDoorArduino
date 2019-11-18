// Chicken Door light meter and timer
// Shannon Bailey Nov 17 7:20PM 2019
// Includes DS3231M RTC code with control through joystick with select on depress

#include <DS3231M.h>
//#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

#define LIGHT_PIN A0
#define ANALOG_LR A1  // L=0, R=1023
#define ANALOG_UD A2  // D=0, U=1023
#define SELECT_PIN 4  // Select=0
#define INDICATOR_PIN 3
#define RELAY_OPEN_PIN 7
#define RELAY_CLOSE_PIN 8

#define POWER_ON 1
#define POWER_OFF 0
#define OPEN 2
#define CLOSE 3
#define DELAY_MS 1000
#define CHANGE_DELAY 10
#define LIGHT_SAMPLES 20
#define SPRINTF_BUFFER_SIZE 32
#define JOY_MAX 950
#define JOY_MIN 50
#define EEPROM_MORNING 1022
#define EEPROM_NIGHT 1023

int8_t  nightHour;
int8_t  morningHour;
uint8_t  powerState;
uint8_t  openClose;
uint8_t  selected;
uint8_t  manual;
uint32_t powerOnTime;
uint32_t powerOffTime;
int8_t   sampleIndex;
uint16_t levels[ LIGHT_SAMPLES + 2 ];
char     outputBuffer[ SPRINTF_BUFFER_SIZE ];

// hour, minute, second, year, month, day, morning, night
const uint8_t set_row[] PROGMEM = {1, 1, 1, 1, 1,  1,  3, 3};
const uint8_t set_col[] PROGMEM = {0, 3, 6, 9, 14, 17, 8, 17};
const uint8_t set_len[] PROGMEM = {2, 2, 2, 4, 2,  2,  2, 2};
const uint16_t set_upper_limit[] PROGMEM = {23, 59, 59, 2050, 12, 31, 23, 23};
const uint16_t set_lower_limit[] PROGMEM = {0,  0,  0,  2018, 1,  1,  0,  0};

DS3231M_Class DS3231M;
DateTime now;
LiquidCrystal_I2C lcd( 0x27, 20, 4 );  // set the LCD address to 0x27 for a 20 chars and 4 line display

void setup() {
  Serial.begin( 9600 );

  lcd.init();       // initialize the lcd
  lcd.backlight();  // turn on the backlight
   
  lcd.setCursor( 0, 0 );            // go to the top left corner
  lcd.print( F("Chicken Door Control") );
  lcd.setCursor( 0, 1 );            // go to the 2nd row, left edge
  lcd.print( F("S Bailey Nov 17 2019") );

  byte box[8] = {
    B11111,
    B10101,
    B11011,
    B10101,
    B11011,
    B10101,
    B11111
  };
  delay( 100 );
  lcd.createChar( 0, box );

  delay( 3000 );
  
  lcd.init();  //initialize the lcd again to blank the screen

  while( !DS3231M.begin() ) {
    Serial.println( F("Unable to find DS3231MM. Checking again in 3 seconds.") );
    delay( 3000 );
  }
  DS3231M.pinSquareWave();
  // DS3231M.adjust(); // Adjust() sets date and time to compiler __DATE__ __TIME__
  
  now = DS3231M.now();

  pinMode( SELECT_PIN, INPUT );
  pinMode( INDICATOR_PIN, OUTPUT );
  pinMode( RELAY_OPEN_PIN, OUTPUT );
  pinMode( RELAY_CLOSE_PIN, OUTPUT );

  morningHour  = EEPROM.read( EEPROM_MORNING );
  nightHour    = EEPROM.read( EEPROM_NIGHT );
  powerState   = POWER_OFF;
  openClose    = OPEN;
  manual       = false;
  powerOnTime  = 0;
  powerOffTime = CHANGE_DELAY;

  for( sampleIndex = 0; sampleIndex < LIGHT_SAMPLES; sampleIndex++ ) {
    levels[sampleIndex] = analogRead( LIGHT_PIN );
  }
}

int16_t adjust( int16_t value, uint8_t cursor, int8_t direction ) {
  value += direction;
  if ( value > (int16_t)pgm_read_word( &set_upper_limit[ cursor ] ) ) {
    value = pgm_read_word( &set_lower_limit[ cursor ] );
  } else if ( value < (int16_t)pgm_read_word( &set_lower_limit[ cursor ] ) ) {
    value = pgm_read_word( &set_upper_limit[ cursor ] );
  }
  return value;
}

void settings() {
  lcd.clear();
  lcd.setCursor( 0, 0 );    // go to the 1st row, left edge
  lcd.print( F("Set Time and Date   ") );

  lcd.setCursor( 0, 2 );    // go to the 3rd row, left edge
  lcd.print( F("Door Open Close Time") );

  lcd.setCursor( 0, 3 );    // go to the 4th row, left edge
  lcd.print( F("Morning=   Night=   ") );

  uint8_t cursor = 0;
  uint8_t row, col, len;
  int16_t hour, minute, second, year, month, day;
  uint16_t up_down, left_right;
  int8_t direction;
  
  do {
    row = pgm_read_byte( &set_row[cursor] );
    col = pgm_read_byte( &set_col[cursor] );
    len = pgm_read_byte( &set_len[cursor] );
    lcd.setCursor( col, row );
    for ( ; len > 0; len-- )
      lcd.write( byte(0));
    
    delay( 500 );

    DateTime now = DS3231M.now();

    hour   = now.hour();
    minute = now.minute();
    second = now.second();
    year   = now.year();
    month  = now.month();
    day    = now.day();
  
    sprintf( outputBuffer, "%02d:%02d:%02d %04d-%02d-%02d",
      hour, minute, second, year, month, day );
  
    lcd.setCursor( 0, 1 );    // go to the 2nd row, left edge
    lcd.print( outputBuffer );

    sprintf( outputBuffer, "%02d", morningHour );
    lcd.setCursor( 8, 3 );    // go to the 2nd row, 8 characters from left edge
    lcd.print( outputBuffer );
  
    sprintf( outputBuffer, "%02d", nightHour );
    lcd.setCursor ( 17, 3 );
    lcd.print( outputBuffer );

    up_down = analogRead( ANALOG_UD );
    direction = 0;
    if ( up_down > JOY_MAX ) {
      direction = 1;
    } else if ( up_down < JOY_MIN ) {
      direction = -1;
    }

    if ( direction != 0 ) {
      switch ( cursor ) {
        case 0:
          hour = adjust( hour, cursor, direction );
          break;
        case 1:
          minute = adjust( minute, cursor, direction );
          break;
        case 2:
          second = adjust( second, cursor, direction );
          break;
        case 3:
          year = adjust( year, cursor, direction );
          break;
        case 4:
          month = adjust( month, cursor, direction );
          break;
        case 5:
          day = adjust( day, cursor, direction );
          break;
        case 6:
          morningHour = adjust( morningHour, cursor, direction );
          break;
        case 7:
          nightHour = adjust( nightHour, cursor, direction );
          break;         
      }
      DS3231M.adjust( DateTime(year, month, day, hour, minute, second) );
    }
    
    if ( !digitalRead( SELECT_PIN ) ) {
      cursor++;
    }

    left_right = analogRead( ANALOG_LR );
    if ( left_right > JOY_MAX ) {
      cursor++;
    } else if ( left_right < JOY_MIN && cursor > 0 ) {
      cursor--;
    }
    
    delay( 500 );
    
  } while ( cursor < 8 );

  if ( morningHour != EEPROM.read( EEPROM_MORNING ) )
    EEPROM.write( EEPROM_MORNING, morningHour );
  if ( nightHour != EEPROM.read( EEPROM_NIGHT ) )
    EEPROM.write( EEPROM_NIGHT, nightHour);
    
  lcd.clear();
  selected = false;
}

void loop() {
  uint16_t lightLevel;
  uint32_t lightAverage;
  uint8_t  index;
  uint8_t  overRide;
  uint8_t  night;
  uint16_t left_right;

  if ( selected ) {
    settings();
  }

  // Delay between measurements.
  delay( DELAY_MS );
  
  // Sample light and create an average
  lightLevel  = analogRead( LIGHT_PIN );
  sampleIndex = (sampleIndex + 1) % LIGHT_SAMPLES;
  levels[sampleIndex] = lightLevel;
  lightAverage = 0;
  for (index = 0; index < LIGHT_SAMPLES; index++) {
    lightAverage += levels[index];
  }
  lightAverage = lightAverage / LIGHT_SAMPLES;
  lightAverage = lightAverage < 1000 ? lightAverage : 999;

  selected = !digitalRead( SELECT_PIN );

  DateTime now = DS3231M.now();

  night = (now.hour() >= nightHour || now.hour() <= morningHour);
//  night = !night; // STB enable for testing
  
  sprintf( outputBuffer, "%02d:%02d:%02d %04d-%02d-%02d", now.hour(),
    now.minute(), now.second(), now.year(), now.month(), now.day() );

  lcd.setCursor( 0, 0 );    // go to the 1st row, left edge
  lcd.print( outputBuffer );

  lcd.setCursor( 0, 1 );    // go to the 2nd row, left edge
  lcd.print( F("Morning=   Night=   ") );

  sprintf( outputBuffer, "%02d", morningHour );
  lcd.setCursor( 8, 1 );    // go to the 2nd row, 8 characters from left edge
  lcd.print( outputBuffer );
  
  sprintf( outputBuffer, "%02d", nightHour );
  lcd.setCursor ( 17, 1 );
  lcd.print( outputBuffer );

  lcd.setCursor( 0, 2 );    // go to the 3rd row
  lcd.print( F("Light=    Temp=") );

  sprintf( outputBuffer, "%03d", lightAverage );
  lcd.setCursor( 6, 2 );
  lcd.print( outputBuffer );

  // Convert Centigrade to Fahrenheit and move decimal point right as needed by device
  dtostrf( (1.8 * (DS3231M.temperature()/100.0)) + 32.0, 5, 1, outputBuffer );
  lcd.setCursor( 15, 2 );
  lcd.print( outputBuffer );
  
  lcd.setCursor( 0, 3 );
  if ( night ) {
    lcd.print( F("NIGHT ") ); 
  } else {
    lcd.print( F("DAY   ") );
  }
  
  left_right = analogRead( ANALOG_LR );
  if ( left_right > JOY_MAX ) {
    manual = true;
  } else if ( left_right < JOY_MIN ) {
    manual = false;
  }

  if ( manual ) {
    lcd.setCursor( 0, 3 );
    lcd.print( F("MANUAL") );
  }
  
  lcd.setCursor( 10, 3 );
  if ( openClose == OPEN ) {
    lcd.print( F("DOOR OPEN ") );
  } else {
     lcd.print( F("DOOR CLOSE") );
  }
  
  if ( powerState == POWER_ON ) {
    powerOnTime++;
    if ( powerOnTime > CHANGE_DELAY ) {
      MoveActuator( POWER_OFF );
    }
  }

  if ( powerState == POWER_OFF ) {
    powerOffTime++;
    if ( powerOffTime > CHANGE_DELAY ) {
      if ( manual ) {
        if (openClose != OPEN) {
          MoveActuator( OPEN );
        }
      } else {
        if (night) {
          if ( openClose != CLOSE ) {
            MoveActuator( CLOSE );
          }
        } else {
          if ( openClose != OPEN && (lightAverage > 100) ) {
            MoveActuator(OPEN);
          } else {
            if ( openClose != CLOSE && (lightAverage <= 100) ) {
              MoveActuator(CLOSE);
            }
          }
        }
      }
    }
  }
}

void MoveActuator( uint8_t state ) {
  switch( state ) {
    case POWER_OFF:
      digitalWrite( RELAY_OPEN_PIN, LOW );
      digitalWrite( RELAY_CLOSE_PIN, LOW );
      powerState = POWER_OFF;
      break;
    case OPEN:
      digitalWrite( INDICATOR_PIN, LOW );
      digitalWrite( RELAY_OPEN_PIN, HIGH );
      digitalWrite( RELAY_CLOSE_PIN, LOW );
      openClose  = OPEN;
      powerState = POWER_ON;
      break;
    case CLOSE:
    default:
      digitalWrite( INDICATOR_PIN, HIGH );
      digitalWrite( RELAY_OPEN_PIN, LOW );
      digitalWrite( RELAY_CLOSE_PIN, HIGH );
      openClose  = CLOSE;
      powerState = POWER_ON;
      break;
  }
  powerOnTime  = 0;
  powerOffTime = 0;
}

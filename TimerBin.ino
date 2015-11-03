#include <Arduino.h>

#include <Wire.h>
#include <Time.h>
#include <ST7032.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>

#include "J1ClockKit.h"
#include "J1RX8025RTC.h"

// type definitions
// http://keitetsu.blogspot.jp/2014/11/arduinotypedef.html

#include "typedef.h"

// static variables
ST7032 lcd;

/* Assign a unique ID to this sensor at the same time */
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

static J1ClockKit::Crown *_crown;

static state_t _state = stateIdle;

static J1ClockKit::ElapsedTimer *sTimer = NULL;

static time_t sUpsideDownTime = 0;

// function definitions
const char* eventToString( const event_t event ) {
    static const char* str[] = {
        "eventIdle",
        "singleTapped",
        "doubleTapped",
        "tiltLeft",
        "tiltRight",
        "standLeft",
        "stnandRight",
        "tiltBack",
        "tiltFront",
        "timedOut",
        "evnetUpsideDown",
    };
    return ( (int)event < sizeof(str) ) ? str[ (int)event ] : NULL;
}

const char* actionToString( const action_t action ) {
    static const char* str[] = {
        "actionIdle",
        "forward",
        "forwardFast",
        "backward",
        "backwardFast",
        "selectNext",
        "selectPrev",
        "toAdjusting",
        "toClock",
        "CancelAdjusting",
        "ToSleep",
        "actionUpsideDown",
        "actionDisplayOpend",
    };
    return ( (int)action < sizeof(str) ) ? str[ (int)action ] : NULL;
}

const char* stateToString( const state_t state ) {
    static const char* str[] = {
        "stateIdle",
        "stateClock",
        "stateAdjusting",
        "stateSleep",
        "stateUpsideDown",
        "actionDisplayOpend",
    };
    return ( (int)state < sizeof(str) ) ? str[ (int)state ] : NULL;
}

event_t getEvent( void ) {
    static event_t prevEvent = eventIdle;
    
    // Get a sensor event
    sensors_event_t sensorEvent;
    accel.getEvent(&sensorEvent);
    sensorEvent.timestamp = millis();
    
    // Get a sensor register value
    uint8_t reg = accel.readRegister(ADXL345_REG_INT_SOURCE);
    
    event_t event = eventIdle;
    
    // analyze an event
    if      ((reg & 0x20) != 0) {
        event = eventDoubleTapped;
    }
    else if ((reg & 0x40) != 0) {
        event = eventSingleTapped;
    }
    else if (sensorEvent.acceleration.z < -7.0) {
        Serial.print("accel.z = ");
        Serial.print(sensorEvent.acceleration.z, 2);
        Serial.print( " " );

        if (sTimer -> elapsed() > 2 * 1000) {
            event = eventUpsideDown;
        }
    }
    else if (sensorEvent.acceleration.y < -7.0) {
        event = eventStandLeft;
    }
    else if (sensorEvent.acceleration.y < -4.0) {
        event = eventTiltLeft;
    }
    else if (sensorEvent.acceleration.y > +7.0) {
        event = eventStandRight;
    }
    else if (sensorEvent.acceleration.y > +4.0) {
        event = eventTiltRight;
    }
    else if (sensorEvent.acceleration.x < -7.0) {
        event = eventTiltBack;
    }
    else if (sensorEvent.acceleration.x > +7.0) {
        event = eventTiltFront;
    }
    else if (sTimer->elapsed() > 30 * 1000) {
        event = eventTimedOut;
    }
    else {
        event = eventIdle;
    }
    
    prevEvent = event;
    return event;
}

action_t selectAction( const event_t event, const state_t state ) {
    action_t action = actionIdle;
    
    // make an action from the event
    switch (state) {
        case stateSleep:
            switch (event) {
                case eventSingleTapped:
                    action = actionDisplayOpend;
                    break;
                case eventUpsideDown:
                    action = actionUpsideDown;
                    break;
            }
            break;
        case stateIdle:
            switch (event) {
                case eventSingleTapped:
                    action = actionDisplayOpend;
                    break;
                case eventUpsideDown:
                    action = actionUpsideDown;
                    break;
            }
            break;
        case stateUpsideDown:
            switch (event) {
                case eventSingleTapped:
                    action = actionToClock;
                    break;
                case eventTimedOut:
                    action = actionToSleep;
                    break;
                default:
                    action = actionDisplayOpend;
                    break;
            }
            break;
        case stateClock:
            switch (event) {
                case eventDoubleTapped:
                    action = actionToAdjusting;
                    break;
                case eventSingleTapped:
                case eventTimedOut:
                    action = actionToSleep;
                    break;
                case eventUpsideDown:
                    action = actionUpsideDown;
                    break;
            }
            break;
        case stateAdjusting:
            switch (event) {
                case eventDoubleTapped:
                    action = actionToClock;
                    break;
                case eventTiltRight:
                    action = actionForward;
                    break;
                case eventStandRight:
                    action = actionForwardFast;
                    break;
                case eventTiltLeft:
                    action = actionBackward;
                    break;
                case eventStandLeft:
                    action = actionBackwardFast;
                    break;
                case eventTiltBack:
                    action = actionSelectPrev;
                    break;
                case eventTiltFront:
                    action = actionSelectNext;
                    break;
                case eventTimedOut:
                    action = actionCancelAdjusting;
                    break;
                default:
                    action = actionIdle;
                    break;
            }
            break;
    }
    
    return action;
}

state_t doAction( const action_t action, const state_t state, tmElements_t &tm ) {
    state_t nextState = state;
    
    if (action != actionIdle) {
        sTimer -> start();
    }
    
    // action
    switch (action) {
        case actionIdle:
            break;
        case actionForward:
            _crown->forward();
            break;
        case actionForwardFast:
            _crown->forward( 10 );
            break;
        case actionBackward:
            _crown->backward();
            break;
        case actionBackwardFast:
            _crown->backward( 10 );
            break;
        case actionSelectNext:
            _crown->move( -1 );
            break;
        case actionSelectPrev:
            _crown->move( +1 );
            break;
        case actionToAdjusting:
            // load from the timeElements to a crown
            _crown->load( tm );
            nextState = stateAdjusting;
            break;
        case actionToClock:
            if (state == stateAdjusting) {
                // write back to the timeElements from a crown
                _crown->save( tm );
                tm.Second = 0;
                if (RX8025RTC.write( tm )) {
                    Serial.println("RTC has been saved");
                }
                else {
                    Serial.println("RTC is unable to save");
                }
            }
            nextState = stateClock;
            break;
        case actionCancelAdjusting:
            nextState = stateClock;
            break;
        case actionToSleep:
            nextState = stateSleep;
            break;
        case actionUpsideDown:
            sUpsideDownTime = now();
            nextState = stateUpsideDown;
            break;
        case actionDisplayOpend:
            nextState = stateUpsideDown;
            break;
        default:
            break;
    }
    return nextState;
}

void displaySensorDetails(void) {
    sensor_t sensor;
    accel.getSensor(&sensor);
    Serial.print  ("Sensor:       "); Serial.println(sensor.name);
    Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
    Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
    Serial.print  ("Max Value:    "); Serial.println(sensor.max_value);
    Serial.print  ("Min Value:    "); Serial.println(sensor.min_value);
    Serial.print  ("Resolution:   "); Serial.println(sensor.resolution);
    delay(500);
}

void displayClock( const tmElements_t &tm ) {
    char s[16];
    lcd.display();
    lcd.noBlink();
    
    // display yeaer, month, day on LCD
    sprintf( s, "%02d-%02d-%02d", (tm.Year + 1970) % 100, tm.Month, tm.Day );
    lcd.setCursor(0,0);
    lcd.print(s);
    
//    Serial.print( s   );
//    Serial.print( " " );
    
    // display hour, minute, second on LCD
    sprintf( s, "%02d:%02d:%02d", tm.Hour, tm.Minute , tm.Second );
    lcd.setCursor(0,1);
    lcd.print(s);
    
//    Serial.println( s );
}

void displayAdjusting( const action_t action ) {
    // display the action
    char *tilt = NULL;
    switch (action) {
        case actionForward:
            tilt = " + ";
            break;
        case actionForwardFast:
            tilt = " ++";
            break;
        case actionBackward:
            tilt = " - ";
            break;
        case actionBackwardFast:
            tilt = " --";
            break;
        case actionSelectPrev:
            tilt = " < ";
            break;
        case actionSelectNext:
            tilt = " > ";
            break;
        default:
            tilt = "   ";
            break;
    }
    lcd.setCursor(5,1);
    lcd.print(tilt);
    
    int x = 0;
    int y = 0;
    tmByteFields field = _crown->getField();
    switch(field) {
        case tmYear:    // year
            x = 0;
            y = 0;
            break;
        case tmMonth:   // month
            x = 3;
            y = 0;
            break;
        case tmDay:     // day
            x = 6;
            y = 0;
            break;
        case tmHour:    // hour
            x = 0;
            y = 1;
            break;
        case tmMinute:  // minute
            x = 3;
            y = 1;
            break;
        default:        // error
            break;
    }
    
    // display the value and blink the cursor
    char s[16];
    int value = _crown->getValue();
    sprintf(s, "%02d", value);
    lcd.display();
    lcd.setCursor(x, y);
    lcd.print(s);
    lcd.setCursor(x + 1, y);
    lcd.blink();
}

void displayUpsideDown( const time_t time ) {
    tmElements_t tm;
    breakTime( time, tm );
    
    lcd.display();
    lcd.noBlink();
    lcd.clear();

    lcd.setCursor(0,0);
    lcd.print("Opend at");

    char s[16];
    sprintf( s, "%02d:%02d:%02d", tm.Hour, tm.Minute , tm.Second );
    lcd.setCursor(0,1);
    lcd.print(s);
    
    Serial.print( "Upside Down = " );
    Serial.println( s );
}

void displayOff( void ) {
    lcd.clear();
    delay(5);
    lcd.noDisplay();
}

#pragma mark -

void setup() {
    Serial.begin(9600);
    Serial.println(F("Hello World!"));
    
    lcd.begin(8,2);
    lcd.setContrast(30);
    lcd.display();
    lcd.setCursor(0,0);
    lcd.print("Hello");
    lcd.setCursor(2,1);
    lcd.print("World!");
    
    delay(1000);
    lcd.clear();
    
    /* Initialise the sensor */
    if(!accel.begin())
    {
        /* There was a problem detecting the ADXL345 ... check your connections */
        Serial.println("Ooops, no ADXL345 detected ... Check your wiring!");
        while(1);
    }
    
    /* Set the range to whatever is appropriate for your project */
    // accel.setRange(ADXL345_RANGE_16_G);
    // displaySetRange(ADXL345_RANGE_8_G);
    // displaySetRange(ADXL345_RANGE_4_G);
    accel.setRange(ADXL345_RANGE_2_G);
    
    accel.writeRegister(ADXL345_REG_THRESH_TAP, 50);  // tap threshold 62.5mg
    accel.writeRegister(ADXL345_REG_DUR, 15);         // tap duration 625usec
    accel.writeRegister(ADXL345_REG_LATENT, 80);      // tap latency 1.25msec
    accel.writeRegister(ADXL345_REG_WINDOW, 200);     // tap windows 1.25msec
    
    // tap x, y, z enable
    accel.writeRegister(ADXL345_REG_TAP_AXES, 0x07);
    
    accel.writeRegister(ADXL345_REG_INT_MAP,0); // all interrupts map to INT1
    
    uint8_t reg = accel.readRegister(ADXL345_REG_INT_ENABLE);
    reg |= 0x60; // single and double tap enabled
    accel.writeRegister(ADXL345_REG_INT_ENABLE, reg);
    
    /* Display some basic information on this sensor */
    displaySensorDetails();
    
    // initialize Real Time Controller RX8025
    RX8025RTC.init();
    setSyncProvider(RX8025RTC.get);
    
    tmElements_t tm;
    tm.Year   = 2015 - 1970;
    tm.Month  = 10;
    tm.Day    = 25;
    tm.Wday   =  1; // Sunday
    tm.Hour   = 21;
    tm.Minute =  0;
    tm.Second =  0;
    
    setSyncInterval( 1 ); // sync interval = 1 sec
    if (RX8025RTC.write(tm)) {
        Serial.println("RTC has been wrote");
    }
    else {
        Serial.println("Unable to write");
    }
    
    _crown = new J1ClockKit::Crown;
    _state = stateClock;
    sTimer = new J1ClockKit::ElapsedTimer();
    sTimer -> start();
}

void loop() {
    // Get a current time
    tmElements_t timeElements;
    time_t time = now();
    breakTime( time, timeElements );
    
    char s[16];
    sprintf( s, "%02d:%02d:%02d", timeElements.Hour, timeElements.Minute , timeElements.Second );
    Serial.print( s );
    Serial.print( " " );

    // Get an event
    event_t event = getEvent();
    Serial.print( eventToString( event ) );
    Serial.print( " " );
    
    // Select an action from the event in the state
    action_t action = selectAction( event, _state );
    Serial.print( actionToString( action ) );
    Serial.print( " " );
    
    // Do the action in the state and get a next state
    _state = doAction( action, _state, timeElements );
    Serial.println( stateToString(_state) );
    
    // display
    switch (_state) {
        case stateClock:
            displayClock( timeElements );
            break;
        case stateAdjusting:
            displayAdjusting( action );
            break;
        case stateUpsideDown:
            displayUpsideDown( sUpsideDownTime );
            break;
        case stateSleep:
            displayOff();
            break;
    }
    delay(500);
}


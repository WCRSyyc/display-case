/*
WCRS Display Case

This sketch controls power to the display modules based on input from motion sensors
using relays.  Actually heat sensors.  If there is no one around, the displays
(except for the motion sensors) are powered down, then started up again when a
warm body is detected.

The display case contains a (set of) presentation to be watched, and potentially
interacted with.  To reflect that, theatre concepts are used to name variables
and functions.

Relay Wiring:
Digital Pin 4 - LEDs
 - this is an LED strip around inside bottom of the clear display cover.  It serves
   to attract attention, and somewhat illuminate the display contents.
Digital Pin 5 - Xylophone
Digital Pin 6 - Line Follower
Digital Pin 7 - Spatula Robot

Except for the LEDs, each of the display sections is run by it's own Arduino and sketch.

Motion Sensor Wiring:
Digital Pin 2 - Port Motion Sensor (right when looking at the front of the display) (
Digital Pin 3 - Starboard Motion Sensor (left when looking at the front of the display)

Written and tested with:
  Environment: Arduino 1.0.6 on Fedora 21
  Hardware: Arduino Uno
*/

// Constants, never modified while the sketch is running

unsigned int DBG_STATE = 0;// 0 turns debug messages off, higher increases verbosity

// Currently there are only 2 sensors, but use an array to make it easy to handle more
int motionPin[] = {2, 3};// Port, Starboard sensors
int relayPin[] = {4, 5, 6, 7};// Output pins to control power relays
// DEBUG NOTE: When testing sensors and short interval power cycling, only use the LED
// sub-system
//int relayPin = {4};

unsigned const int LED_PIN = 13;// current audience detected indicator
unsigned const int PIR_COUNT = sizeof(motionPin) / sizeof(int);// Number of Passive Infrared Sensors
unsigned const int STAGE_COUNT = sizeof(relayPin) / sizeof(int);// Number of acts/displays/features
// The length of time to continue a presentation after the current audience leaves
unsigned const long CONTINUE_TIME = 30000;// milliseconds (30 seconds)
// The time to wait after applying power to the PIR sensors before taking actual
// readings
unsigned const int SETTLE_TIME = 30; // seconds

// Global variables

unsigned long curtainCall = 0;// Scheduled time to power down
boolean actInProgress = false;// displays are [not] powered up


void setup() {
  if (DBG_STATE > 0) {
    Serial.begin(9600); // Only used for logging debug messages
  }
  pinMode(LED_PIN, OUTPUT);
  setupRelays();
  setupSensors();
}

void loop() {
  unsigned long now = millis();

  checkAudience(now);// updates curtainCall when an audience is present

  if (actInProgress && now > curtainCall) {
    // There has been no audience for awhile
    ringDownCurtain(now);
  } else if (!actInProgress && now < curtainCall) {
    // An audience just arrived: start the show
    raiseCurtain(now);
  }
}

// Power up the displays
void raiseCurtain(unsigned long refTime) {// time only used for debug logging
  unsigned int idx;
  for (idx = 0; idx < STAGE_COUNT; idx++) {
    // Power up each of the feature presentations
    digitalWrite(relayPin[idx], HIGH);
  }
  if (DBG_STATE > 2) {
    Serial.print("Starting presentations at ");
    Serial.println(refTime, DEC);
  }
}

// Power down the active displays
void ringDownCurtain(unsigned long refTime) {// time only used for debug logging
  unsigned int idx;

  for (idx = 0; idx < STAGE_COUNT; idx++) {
    digitalWrite(relayPin[idx], LOW); // Turn power off
  }
  if (DBG_STATE > 2) {
    Serial.print("End of the current act at ");
    Serial.println(refTime, DEC);
  }
}

// Check the heat / motion sensors, to see if there is any audience in range.
// Update the future (global) curtain call time when an audience is present
void checkAudience(unsigned long refTime) {
  unsigned int idx;
  boolean liveAudience = false;

  for (idx = 0; idx < PIR_COUNT; idx++) {
    if (digitalRead(motionPin[idx])) {
      liveAudience = true;// Current sensor detected heat/motion
      if (DBG_STATE > 8) {
        Serial.print("Sensor on pin ");
        Serial.print(motionPin[idx], DEC);
        Serial.print(" detected audience at ");
        Serial.println(refTime, DEC);
      }
    }
  }

  if (liveAudience) {
    // Currently have an audience: push the curtain call further into the future
    curtainCall = refTime + CONTINUE_TIME;// When to shut off, if no one shows up sooner
  }
  digitalWrite(LED_PIN, liveAudience);// Show when an audience is present

  if (DBG_STATE > 2) {
    if (!actInProgress && liveAudience) {
      Serial.print("New audience detected at time ");
      Serial.println(refTime, DEC);
      Serial.print("Next curtain call at ");
      Serial.println(curtainCall, DEC);
    }
  }
}

void setupRelays() {
  unsigned int idx;
  for (idx = 0; idx < STAGE_COUNT; idx++) {
    pinMode(relayPin[idx], OUTPUT);
    digitalWrite(relayPin[idx], LOW); // Make sure power is off to start with
    if (DBG_STATE > 3) {
      Serial.print("Relay ");
      Serial.print(idx);
      Serial.print(" on pin ");
      Serial.print(relayPin[idx]);
      Serial.println(" initialized and off");
    }
  }
  if (DBG_STATE > 1) {
    Serial.println("Stage control relays initialized");
  }
}

void setupSensors() {
  unsigned int idx;
  for (idx = 0; idx < PIR_COUNT; idx++) {
    pinMode(motionPin[idx], INPUT);
    digitalWrite(motionPin[idx], LOW); // Turn off internal pull up resistor
  }

  // What is this actually doing?  There is nothing here that does anything with the
  // just initialized sensor pins.  It is **just** waiting.  Is this left over from
  // some removed calbration code?  Is it not needed?  Do the sensors actually need
  // a delay time in order to stabilize readings?
  if (DBG_STATE > 0) {
    Serial.print("calibrating sensor ");
    for(int i = 0; i < SETTLE_TIME; i++){
      Serial.print(".");
      delay(1000);// one second
    }
    Serial.println(" done");
    Serial.println("SENSOR ACTIVE");
  } else {
    delay (SETTLE_TIME * 1000);
  }
}

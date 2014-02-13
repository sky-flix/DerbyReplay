#include <Narcoleptic.h>
#include <IRremote.h>

/* 
Pinewood Derby Automated Instant Replay
John Lever
February 9th, 2014

This code sends automated infrared commands to a Sony Handycam CX-500V video camera in order to record
the finish of a Pinewood Derby Race, and then automatically play the footage back onto a connected display
or projector, over the HDMI output of the camera.

One particular nicety about this camera is that it offers 3 seconds of high speed recording which is more
than enough for a derby race. The camera, when triggered by a break of a laser line, records high speed
video for 3 seconds, then switches to playback mode, plays the video in slow motion mode, then switches
back to Smooth Slow Recording.

Hardware couldn't be simpler:
Two LEDs used for status updates. I chose Red (Recording or Playback) and Green (ready for race),
An infrared LED
A photoresistor
A Laser
And, if you need to decode an existing IR remote, an Infrared Receiver Module
various resistors
Arduino Pro Mini, 8Mhz, 3.3V (This is a small, power efficient unit, suitable for battery use, but any Arduino will suffice)

Total maximum power consumption (while the laser is active), is less than 40mA, so a 1000mAh battery will
power the system for practically an entire day.

This could be easily modified for use with any IR controlled camera, even still cameras.

Many thanks to Ken Shirriff and his IR library, for which without, this project would not have been possible:
http://www.righto.com/2009/08/multi-protocol-infrared-remote-library.html

To Do: Switch while loop for photoresistor reading to a hardware interrupt
*/


const int sensorPin = A0;
const byte ledPinIR = 3; //this can't be changed without modifying IR library
const byte ledPinRed = 4;
const byte ledPinGreen = 5;
const byte laserPin = 6;
const byte photoPin = 7;
const byte irRepeat = 2; // number of times each IR function must be sent for a single command
const int interIRDelay = 40; // Delay in milliseconds between required repeated IR functions
const int irDelay=200; // Delay in milliseconds between disparate IR commands
const byte videoLoopCount = 1; // Number of times to loop through slow scan playback
const int secondsToPlayback=30; // 3 seconds of recording takes 60s to play back in slow scan

IRsend irsend;

int sensorValue = 0;

void setup() {
  
  pinMode(ledPinRed, OUTPUT);
  pinMode(ledPinGreen, OUTPUT);
  pinMode(laserPin, OUTPUT); // My laser is driven directly off a digital output, since it uses only 10mA
  pinMode(ledPinIR, OUTPUT);  
  pinMode(photoPin, OUTPUT);
  
  prepForRace();
  
  while (true) {
    
    if (analogRead(sensorPin) < 900) {
      
      digitalWrite(ledPinGreen, LOW);
      digitalWrite(ledPinRed, HIGH);
      digitalWrite(laserPin, LOW); // Turn off laser to save power, as minimal as it might be
      digitalWrite(photoPin, LOW); // Turn off the photo sensor to save power as well
    
      record(); // Initiate 3 second Smooth Slow Rec
    
      // Camera takes 20 seconds to write the 3s of Smooth record to memory. Since we dont' need to do anything else
      // during this time, we use Peter Knight's Narcoleptic library to put the unit to sleep to conserve power
    
      Narcoleptic.delay(20000);
    
      recToPlayback(); // Switch from record to playback mode
    
      for (int i=0; i < videoLoopCount; i++) {
        playLastVideo(); // Select the last video from the index and play it
      }

      prepForRace();
    }
  }
}

void loop() {
}

boolean prepForRace() {
  modeSlowRec();
  digitalWrite(ledPinRed, LOW);
  digitalWrite(photoPin, HIGH); // Turn the photo sensor on
  digitalWrite(laserPin, HIGH); // Turn the laser on
    
  while(analogRead(sensorPin) < 900) {
      digitalWrite(ledPinGreen, HIGH);
      delay(200);
      digitalWrite(ledPinGreen, LOW);
      delay(200);
  }
  
  digitalWrite(ledPinGreen, HIGH);
  return true;
}

void modeSlowRec() {
  for (int x = 0; x < 3; x++) {
    up();
  }
  for (int x = 0; x < 3; x++) {
    left();
  }
  enter();
  down();
  enter();
}

void recToPlayback() {
  up();
  up();
  enter();
  down();
  enter();
  Narcoleptic.delay(2000); // Give the camera some time to catch up
}

void playLastVideo() {
  // move to last image in index
  for (int x = 0; x < 5; x++) {
    right();
  }
  for (int y = 0; y < 3; y++) {
    down();
  }
  enter();
  enter();
  // rewind to beginning, in case we are playing this video for the second time and it wasn't watched all the way through the first time
  skipBack();
  slow();
  Narcoleptic.delay(secondsToPlayback * 1000);
  stopPlayback();
}

/*
Begin specific IR commands, for which the other functions are built upon. My camera requires that it
receive the IR command twice in order to accept it. YMMV.
*/

void up() {
  delay(irDelay);
  for (int i = 0; i < irRepeat; i++) {
      irsend.sendSony(0x9EB44, 20);
      delay(interIRDelay);
  }
}

void down() {
  delay(irDelay);
  for (int i = 0; i < irRepeat; i++) {
      irsend.sendSony(0x5EB44, 20);
      delay(interIRDelay);
  }
}

void left() {
  delay(irDelay);
  for (int i = 0; i < irRepeat; i++) {
      irsend.sendSony(0xDEB44, 20);
      delay(interIRDelay);
  }
}

void right() {
  delay(irDelay);
  for (int i = 0; i < irRepeat; i++) {
      irsend.sendSony(0x3EB44, 20);
      delay(interIRDelay);
  }
}

void enter() {
  delay(irDelay);
  for (int i = 0; i < irRepeat; i++) {
      irsend.sendSony(0xD0B44, 20);
      delay(interIRDelay);
  }
}

void pause() {
  delay(irDelay);
  for (int i = 0; i < irRepeat; i++) {
      irsend.sendSony(0x9CB44, 20);
      delay(interIRDelay);
  }
}

void stopPlayback() {
  delay(irDelay);
  for (int i = 0; i < irRepeat; i++) {
      irsend.sendSony(0x1CB44, 20);
      delay(interIRDelay);
  }
}

void index() {
  delay(irDelay);
  for (int i = 0; i < irRepeat; i++) {
      irsend.sendSony(0x2CBC0, 20);
      delay(interIRDelay);
  }
}


void record() {
  delay(irDelay);
  for (int i = 0; i < irRepeat; i++) {
      irsend.sendSony(0xCBC0, 20);
      delay(interIRDelay);
  }
}

void slow() {
  delay(irDelay);
  for (int i = 0; i < irRepeat; i++) {
      irsend.sendSony(0xC4B44, 20);
      delay(interIRDelay);
  }
}

void skipBack() {
  delay(irDelay);
  for (int i = 0; i < irRepeat; i++) {
      irsend.sendSony(0xCB44, 20);
      delay(interIRDelay);
  }
}

void skipForward() {
  delay(irDelay);
  for (int i = 0; i < irRepeat; i++) {
      irsend.sendSony(0x8CB44, 20);
      delay(interIRDelay);
  }
}

void play() {
  delay(irDelay);
  for (int i = 0; i < irRepeat; i++) {
      irsend.sendSony(0xC4B44, 20);
      delay(interIRDelay);
  }
}

void menuDisplay() {
  delay(irDelay);
  for (int i = 0; i < irRepeat; i++) {
      irsend.sendSony(0x2AB44, 20);
      delay(interIRDelay);
  }
}

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
Arduino Pro Mini, 8Mhz, 3.3V (This is a very small unit which can run off of a single cell LiPo for hours,
however any Arduino should work just fine)

Total maximum power consumption (while the laser is active), is approximately 40mA, so a 1000mAh battery will
power the system for practically an entire day.

This could could be easily modified for use with any IR controlled camera, even still cameras.

Many thanks to Ken Shirriff and his IR library, for which without, this project would not have been possible:
http://www.righto.com/2009/08/multi-protocol-infrared-remote-library.html

To Do: Switch while loop for photoresistor reading to a hardware interrupt
*/


const int sensorPin = A0;

const int ledPinIR = 3; //this can't be changed without modifying IR library
const int ledPinRed = 4;
const int ledPinGreen = 5;
const int laserPin = 6;

IRsend irsend;

int irRepeat = 2; // number of times each IR function must be sent for a single command
int interIRDelay = 40; // Delay in milliseconds between required repeated IR functions
int irDelay=200; // Delay in milliseconds between disparate IR commands
int sensorValue = 0;
int videoLoopCount = 1; // Number of times to loop through slow scan playback
int secondsToPlayback=30; // 3 seconds of recording takes 60s to play back in slow scan

void setup()
{
  pinMode(ledPinRed, OUTPUT);
  pinMode(ledPinGreen, OUTPUT);
  pinMode(laserPin, OUTPUT); // My laser is driven directly off a digital output, since it uses only 10mA
  pinMode(ledPinIR, OUTPUT);  
  
  ModeSlowRec(); // put the camera in Smooth Slow Record mode

  digitalWrite(laserPin, HIGH); // Turn the laser on
    
  CheckForBeam();
}

void loop()
{
  if (analogRead(sensorPin) > 900)
  {
    digitalWrite(ledPinGreen, HIGH);
  }
  else
  {
    // Begin Main Work Loop
    digitalWrite(ledPinGreen, LOW);
    digitalWrite(ledPinRed, HIGH);
    digitalWrite(laserPin, LOW); // Turn off laser to save power, as minimal as it might be
    
    Record(); // Initiate 3 second Smooth Slow Rec
    
    // Camera takes 20 seconds to write the 3s of Smooth record to memory. Since we dont' need to do anything else
    // at this point, we use Peter Knight's Narcoleptic library to put the unit to sleep to conserve power
    
    Narcoleptic.delay(20000);
    
    RecToPlayback(); // Switch from record to playback mode
    
    delay(1000); // Give the camera some time to catch up
    
    for (int i=0; i < videoLoopCount; i++)
    {
      PlayLastVideo(); // Select the last video from the index and play it
    }

    ModeSlowRec(); // Switch back to Smooth Slow Record in preparation of next race

    // End Main Work Loop
    
    // Prep for next race
    digitalWrite(ledPinRed, LOW);
    digitalWrite(laserPin, HIGH); // Turn the laser on
    
    CheckForBeam();
    
  }
}

boolean CheckForBeam()
{
  while(analogRead(sensorPin) < 900) // Depending on model of photoresistor and ambient light, this value may need to be adjusted.
    {
      digitalWrite(ledPinGreen, HIGH);
      delay(200);
      digitalWrite(ledPinGreen, LOW);
      delay(200);
    }
    return true;
}

void ModeSlowRec()
{
  for (int x = 0; x < 3; x++)
  {
    Up();
    delay(irDelay);
  }
  for (int x = 0; x < 3; x++)
  {
    Left();
    delay(irDelay);
  }
  Enter();
  delay(irDelay);
  Down();
  delay(irDelay);
  Enter();
  delay(irDelay);
}

void RecToPlayback()
{
  Up();
  delay(irDelay);
  Up();
  delay(irDelay);
  Enter();
  delay(2000);
  Down();
  delay(irDelay);
  Enter();
  delay(irDelay);
}

void PlayLastVideo()
{
  // move to last image in index
  for (int x = 0; x < 5; x++)
  {
    Right();
    delay(irDelay);
  }
  for (int y = 0; y < 4; y++)
  {
    Down();
    delay(irDelay);
  }
  Enter();
  delay(irDelay);
  Enter();
  delay(irDelay);
  // rewind to beginning, in case we are playing this video for the second time and it wasn't watched all the way through the first time
  SkipBack();
  delay(irDelay);
  Slow();
  Narcoleptic.delay(secondsToPlayback * 1000);
  StopPlayback();
}

/*
Begin specific IR commands, for which the other functions are built upon. My camera requires that it
receive the IR command twice in order to accept it. YMMV.
*/

void Up()
{
  for (int i = 0; i < irRepeat; i++) {
      irsend.sendSony(0x9EB44, 20);
      delay(interIRDelay);
    }
}

void Down()
{
  for (int i = 0; i < irRepeat; i++) {
      irsend.sendSony(0x5EB44, 20);
      delay(interIRDelay);
    }
}

void Left()
{
  for (int i = 0; i < irRepeat; i++) {
      irsend.sendSony(0xDEB44, 20);
      delay(interIRDelay);
    }
}

void Right()
{
  for (int i = 0; i < irRepeat; i++) {
      irsend.sendSony(0x3EB44, 20);
      delay(interIRDelay);
    }
}

void Enter()
{
  for (int i = 0; i < irRepeat; i++) {
      irsend.sendSony(0xD0B44, 20);
      delay(interIRDelay);
    }
}

void Pause()
{
  for (int i = 0; i < irRepeat; i++)
  {
      irsend.sendSony(0x9CB44, 20);
      delay(interIRDelay);
   }
}

void StopPlayback()
{
  for (int i = 0; i < irRepeat; i++)
  {
      irsend.sendSony(0x1CB44, 20);
      delay(interIRDelay);
   }
}

void Index()
{
  for (int i = 0; i < irRepeat; i++)
  {
      irsend.sendSony(0x2CBC0, 20);
      delay(interIRDelay);
  }
}


void Record()
{
  for (int i = 0; i < irRepeat; i++)
  {
      irsend.sendSony(0xCBC0, 20);
      delay(interIRDelay);
  }
}

void Slow()
{
  for (int i = 0; i < irRepeat; i++)
  {
      irsend.sendSony(0xC4B44, 20);
      delay(interIRDelay);
  }
}

void SkipBack()
{
  for (int i = 0; i < irRepeat; i++)
  {
      irsend.sendSony(0xCB44, 20);
      delay(interIRDelay);
  }
}

void SkipForward() 
{
  for (int i = 0; i < irRepeat; i++)
  {
      irsend.sendSony(0x8CB44, 20);
      delay(interIRDelay);
  }
}

void Play()
{
  for (int i = 0; i < irRepeat; i++)
  {
      irsend.sendSony(0xC4B44, 20);
      delay(interIRDelay);
  }
}

void Display()
{
  for (int i = 0; i < irRepeat; i++)
  {
      irsend.sendSony(0x2AB44, 20);
      delay(interIRDelay);
  }
}

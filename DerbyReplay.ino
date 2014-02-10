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

This could could be easily modified for use with other cameras by obtaining the IR codes or
experimenting with an existing IR remote in order to determine the proceduries for your specific model.

Many thanks to Ken Shirriff and his IR library, for which this project would have been impossible without:
http://www.righto.com/2009/08/multi-protocol-infrared-remote-library.html
*/

#include <IRremote.h>

const int sensorPin = A0;

const int ledPinRed = 4;
const int ledPinGreen = 5;
const int laserPin = 6;

const int IRledPin = 3; //this can't be changed without modifying IR library

IRsend irsend;

int IRDelay=300; // Delay in milliseconds between IR commands
int sensorValue = 0;
int VideoLoopCount = 2; // Number of times
int SecondsToPlayback=40; // How long to play the recorded video back. 3 seconds of recording takes 60s to play

void setup()
{
  pinMode(ledPinRed, OUTPUT);
  pinMode(ledPinGreen, OUTPUT);
  pinMode(laserPin, OUTPUT); // My laser is driven directly off a digital output, since it uses only 10mA
  pinMode(IRledPin, OUTPUT);  
  
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
    
    Record(); // Initiate Recording
    
    delay(20000); // Camera takes 20 seconds to write the 3s of Smooth record to memory
    
    RecToPlayback(); // Switch from record to playback mode
    
    delay(2000); // Give the camera some time to catch up
    
    for (int i=0; i < VideoLoopCount; i++)
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

int ModeSlowRec()
{
  for (int x = 0; x < 3; x++)
  {
    Up();
    delay(IRDelay);
  }
  for (int x = 0; x < 3; x++)
  {
    Left();
    delay(IRDelay);
  }
  Enter();
  delay(IRDelay);
  Down();
  delay(IRDelay);
  Enter();
  delay(IRDelay);
}

int RecToPlayback()
{
  Up();
  delay(IRDelay);
  Up();
  delay(IRDelay);
  Enter();
  delay(2000);
  Down();
  delay(IRDelay);
  Enter();
  delay(IRDelay);
}

int PlayLastVideo()
{
  // move to last image in index
  for (int x = 0; x < 5; x++)
  {
    Right();
    delay(IRDelay);
  }
  for (int y = 0; y < 4; y++)
  {
    Down();
    delay(IRDelay);
  }
  Enter();
  delay(IRDelay);
  Enter();
  delay(IRDelay);
  // rewind to beginning, in case we are playing this video for the second time and it wasn't watched all the way through the first time
  SkipBack();
  delay(IRDelay);
  Slow();
  delay(SecondsToPlayback * 1000);
  Stop();
  delay(IRDelay);
}

/*
Begin specific IR commands, for which the other functions are built upon. My camera requires that it
receive the IR command twice in order to accept it. YMMV. Perhaps replace the integer with a variable
and globally make the change in order to experiment
*/

int Up()
{
  for (int i = 0; i < 2; i++) {
      irsend.sendSony(0x9EB44, 20);
      delay(40);
    }
}

int Down()
{
  for (int i = 0; i < 2; i++) {
      irsend.sendSony(0x5EB44, 20);
      delay(40);
    }
}

int Left()
{
  for (int i = 0; i < 2; i++) {
      irsend.sendSony(0xDEB44, 20);
      delay(40);
    }
}

int Right()
{
  for (int i = 0; i < 2; i++) {
      irsend.sendSony(0x3EB44, 20);
      delay(40);
    }
}

int Enter()
{
  for (int i = 0; i < 2; i++) {
      irsend.sendSony(0xD0B44, 20);
      delay(40);
    }
}

int Pause()
{
  for (int i = 0; i < 2; i++)
  {
      irsend.sendSony(0x9CB44, 20);
      delay(40);
   }
}

int Stop()
{
  for (int i = 0; i < 2; i++)
  {
      irsend.sendSony(0x1CB44, 20);
      delay(40);
   }
}

int Index()
{
  for (int i = 0; i < 2; i++)
  {
      irsend.sendSony(0x2CBC0, 20);
      delay(40);
  }
}


int Record()
{
  for (int i = 0; i < 2; i++)
  {
      irsend.sendSony(0xCBC0, 20);
      delay(40);
  }
}

int Slow()
{
  for (int i = 0; i < 2; i++)
  {
      irsend.sendSony(0xC4B44, 20);
      delay(40);
  }
}

int SkipBack()
{
  for (int i = 0; i < 2; i++)
  {
      irsend.sendSony(0xCB44, 20);
      delay(40);
  }
}

int SkipForward() 
{
  for (int i = 0; i < 2; i++)
  {
      irsend.sendSony(0x8CB44, 20);
      delay(40);
  }
}

int Play()
{
  for (int i = 0; i < 2; i++)
  {
      irsend.sendSony(0xC4B44, 20);
      delay(40);
  }
}

int Display()
{
  for (int i = 0; i < 2; i++)
  {
      irsend.sendSony(0x2AB44, 20);
      delay(40);
  }
}

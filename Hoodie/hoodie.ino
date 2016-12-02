


// We'll need a few libraries to access all this hardware!

#include <SPI.h>            // To talk to the SD card and MP3 chip
#include <SdFat.h>          // SD card file system
#include <SFEMP3Shield.h>   // MP3 decoder chip

// LilyPad MP3 Player pin definitions:

#define TRIG1 A0
#define ROT_LEDG A1
#define SHDN_GPIO1 A2
#define ROT_B A3
#define TRIG2 A4
#define TRIG3 A5
#define RIGHT A6
#define LEFT A7

#define TRIG5_RXI 0
#define TRIG4_TXO 1
#define MP3_DREQ 2
#define ROT_A 3
#define ROT_LEDB 5
#define MP3_CS 6
#define MP3_DCS 7
#define MP3_RST 8
#define SD_CS 9
#define ROT_LEDR 10
#define MOSI 11
#define MISO 12
#define SCK 13

// Create library objects:

SFEMP3Shield MP3player;
SdFat sd;
SdFile file;
char track[13];

// If you would like debugging information sent to the
// serial port, set debugging = true. This will require
// the use of triggers 4 and 5 (but this sketch doesn't
// need them).

boolean debugging = true;


void setup()
{
  unsigned char result;
  
  // If serial port debugging is inconvenient, you can connect
  // a LED to the red channel of the rotary encoder to see any
  // startup error codes:
  
  pinMode(ROT_LEDR,OUTPUT);
  digitalWrite(ROT_LEDR,HIGH);  // HIGH = off

  // Turn the amplifier chip off and start the MP3 chip
  // in MP3 mode:
  
  pinMode(SHDN_GPIO1,OUTPUT);
  digitalWrite(SHDN_GPIO1,LOW);  // MP3 mode / amp off

  // Keep the VS1053 MP3 chip powered down while we're waiting
  // to save power:

  pinMode(MP3_RST, OUTPUT);
  digitalWrite(MP3_RST,LOW);  // keep VS1053 in reset
  
  if (debugging)
  {
    Serial.begin(9600);
    Serial.println(F("Lilypad MP3 Player prank sketch"));
  }
  
  // Wait until we're ready to play the prank:
  
  if (debugging) Serial.print("waiting... ");
  if (debugging) Serial.println("done");
 
  // Turn on the MP3 chip:

  digitalWrite(MP3_RST,HIGH);  // VS1053 active
  
  // Initialize the SD card:

  if (debugging) Serial.print(F("initialize SD card... "));

  result = sd.begin(SD_CS, SPI_HALF_SPEED); // 1 for success
  
  if (result != 1) // Problem initializing the SD card
  {
    if (debugging) Serial.print(F("error, halting"));
    errorBlink(1); // Halt forever, blink LED if present.
  }
  else
    if (debugging) Serial.println(F("success!"));
  
  // Start up the MP3 library:

  if (debugging) Serial.print(F("initialize MP3 chip... "));

  result = MP3player.begin(); // 0 or 6 for success

  // Check the result, see the library readme for error codes.

  if ((result != 0) && (result != 6)) // Problem starting up
  {
    if (debugging) Serial.print(F("error code "));
    if (debugging) Serial.print(result);
    if (debugging) Serial.print(F(", halting."));
    errorBlink(result); // Halt forever, blink red LED if present.
  }
  else
    if (debugging) Serial.println(F("success!"));

  // Change to the root directory of the SD card:

  sd.chdir("/",true);

  // Set the VS1053 volume. 0 is loudest, 255 is lowest:

  MP3player.setVolume(0, 0);
  
  // Turn on the amplifier chip:
  
  digitalWrite(SHDN_GPIO1,HIGH);
  delay(2);

  pinMode(TRIG1, INPUT);
  pinMode(TRIG2, INPUT);
  pinMode(TRIG3, INPUT);
  pinMode(A1, INPUT);
  pinMode(A3, INPUT);
  randomSeed(0);
}


void loop() {
  // Play the files on the SD card. Over and over and over...
//  if (digitalRead(TRIG4_TXO) == HIGH && !MP3player.isPlaying()) {
//    getNextTrack();
//    startPlaying(); 
//  } else if (digitalRead(TRIG4_TXO) == LOW && MP3player.isPlaying()) {
//    stopPlaying();
//  }



  int xAxisValue = analogRead(A0) - 512;
  int yAxisValue = analogRead(A1) - 512;
  int zAxisValue = analogRead(A3) - 542;

  double magnitude = sqrt(pow(xAxisValue, 2) + pow(yAxisValue, 2) + pow(zAxisValue, 2));

  Serial.print(F("X, Y, Z, Magnitude: "));
  Serial.print(xAxisValue);
  Serial.print(F(" "));
  Serial.print(yAxisValue);
  Serial.print(F(" "));
  Serial.print(zAxisValue);
  Serial.print(F(" "));
  Serial.println(magnitude);

  if (!MP3player.isPlaying() && magnitude > 400) {
    getRandomTrack(20);
    startPlaying();
    delay(1000);
  }
}


void errorBlink(int blinks)
{
  // The following function will blink the red LED in the rotary
  // encoder (optional) a given number of times and repeat forever.
  // This is so you can see any startup error codes without having
  // to use the serial monitor window.

  int x;

  while(true) // Loop forever
  {
    for (x=0; x < blinks; x++) // Blink the given number of times
    {
      digitalWrite(ROT_LEDR,LOW); // Turn LED ON
      delay(250);
      digitalWrite(ROT_LEDR,HIGH); // Turn LED OFF
      delay(250);
    }
    delay(1500); // Longer pause between blink-groups
  }
}


void getRandomTrack(int space) {
    int randomIterations = random(space);
    for (int i = 0; i < randomIterations; i++) {
        getNextTrack();
    }
}

void getNextTrack()
{
  // Get the next playable track (check extension to be
  // sure it's an audio file)
  
  do
    getNextFile();
  while(isPlayable() != true);
}

void getNextFile()
{
  // Get the next file (which may be playable or not)

  int result = (file.openNext(sd.vwd(), O_READ));

  // If we're at the end of the directory,
  // loop around to the beginning:
  
  if (!result)
  {
    sd.chdir("/",true);
    getNextFile();
    return;
  }
  file.getFilename(track);  
  file.close();
}


boolean isPlayable()
{
  // Check to see if a filename has a "playable" extension.
  // This is to keep the VS1053 from locking up if it is sent
  // unplayable data.

  char *extension;
  
  extension = strrchr(track,'.');
  extension++;
  if (
    (strcasecmp(extension,"MP3") == 0) ||
    (strcasecmp(extension,"WAV") == 0) ||
    (strcasecmp(extension,"MID") == 0) ||
    (strcasecmp(extension,"MP4") == 0) ||
    (strcasecmp(extension,"WMA") == 0) ||
    (strcasecmp(extension,"FLA") == 0) ||
    (strcasecmp(extension,"OGG") == 0) ||
    (strcasecmp(extension,"AAC") == 0)
  )
    return true;
  else {
    Serial.print(F("Skipping file "));
    Serial.println(track);
    return false;
  }
    
}


void startPlaying()
{
  int result;
  
  if (debugging)
  {
    Serial.print(F("playing "));
    Serial.print(track);
    Serial.print(F("..."));
  }  

  result = MP3player.playMP3(track);

  if (debugging)
  {
    Serial.print(F(" result "));
    Serial.println(result);  
  }
}


void stopPlaying()
{
  if (debugging) Serial.println(F("stopping playback"));
  MP3player.stopTrack();
}


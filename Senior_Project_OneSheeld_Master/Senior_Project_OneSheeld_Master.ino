/* OneSheeld Arduino Hexapod Master Crode 
* Created by Timothy Ramsey McReynolds on 12/20/18
* Code is based off of Arduino Wire examples to communicate with Hexapod Slave Arduino over I2C 
* Sends inputs from smartphone through OneSheeld as inputs from a PS2 controller to the Slave Arduino
* Instead of moving a joystick, the user rotates their phone to move the robot
* Supports sending comnmands over GamePad and Voice Recognition shields as of now
* Can play music on user's smartphone through Voice Recognition and Music Player shields  
*/ 

//Initializes desired shields
#define CUSTOM_SETTINGS
#define INCLUDE_ACCELEROMETER_SENSOR_SHIELD
#define INCLUDE_GAMEPAD_SHIELD
#define INCLUDE_VOICE_RECOGNIZER_SHIELD
#define INCLUDE_MUSIC_PLAYER_SHIELD
#define INCLUDE_TERMINAL_SHIELD
#include <OneSheeld.h>

#include <Wire.h>

void setup() {
  Wire.begin(); // join i2c bus (address optional for master)
  
  OneSheeld.begin();

  VoiceRecognition.setOnError(error);
  VoiceRecognition.start();
}

//These are the "joystick" values we are sending to the slave Arduino, values range from 0-255
byte LX = 127, LY = 127, RX = 127, RY = 127;

// Voice commands set by the user.
const char playCommand[] = "play";
const char pauseCommand[] = "pause";
const char stopCommand[] = "stop";
const char nextCommand[] = "next";
const char startCommand[] = "start";


//Used to send a voice command during sendGamePad
String voiceCommand = "";

//If a voice command not related to the music player is recongized, set voiceCommand to that command


void sendVoice()
{
  if(VoiceRecognition.isNewCommandReceived())
  {
    if(!strcmp(playCommand,VoiceRecognition.getLastCommand()))
    {
      MusicPlayer.play();
    }
    // Compare the pause command. 
    else if (!strcmp(pauseCommand,VoiceRecognition.getLastCommand()))
    {
      // Pause the track. 
      MusicPlayer.pause();
    }
    // Compare the stop command. 
    else if (!strcmp(stopCommand,VoiceRecognition.getLastCommand()))
    {
      // Stop the track. 
      MusicPlayer.stop();
    }
    // Compare the next command. 
    else if (!strcmp(nextCommand,VoiceRecognition.getLastCommand()))
    {
      // Next track. 
      MusicPlayer.next();
    }
    else if(!strcmp(startCommand,VoiceRecognition.getLastCommand()))
    {
      // Toggle the robot on or off. 
      voiceCommand = "Start";
    }
  }
}

//Sends the values from the GamePad shield as values from a PS2 controller
//If a voiceCommand has been recognized, send that instead of a GamePad button
//If a button is not being used, don't need to worry about checking the value or just send a blank String
void sendGamePad()
{ 
  if(GamePad.isBluePressed() || voiceCommand.equals("Start"))
  {
    voiceCommand = "";
    Wire.write("Start");
    Terminal.println("Start");
  }
  else if(GamePad.isRedPressed())
  {
    Wire.write("L1");
    Terminal.println("L1");
  }
  else if(GamePad.isOrangePressed())
  {
    Wire.write("L2");
    Terminal.println("L2");
  }
  else if(GamePad.isGreenPressed())
  {
    Wire.write("Select");
    Terminal.println("Select");
  }
  /*
  else if(GamePad.isLeftPressed())
  {
    Wire.write("");
  }
  else if(GamePad.isRightPressed())
  {
    Wire.write("");
  }
  else if(GamePad.isUpPressed())
  {
    Wire.write("");
  }
  else if(GamePad.isDownPressed())
  {
    Wire.write("");
  }
  */
  else
  {
    Wire.write("");
  }
}

//Sets the standard minimum movements of accelerometer so the robot is not constantly moving
int minimum = 2;

//If true, hold smartphone in landscape mode. If false, hold smartphone like a remote
boolean GamePadOrientation = true;

//Used to send accelerometer values as PS2 joystick values
//Must always be sent last in loop
void sendAccelerometer()
{
  int accelY = AccelerometerSensor.getY();
  int accelZ = AccelerometerSensor.getZ();
  int accelX = AccelerometerSensor.getX();
  LX = 127;
  LY = 127;
  RX = 127;
  RY = 127;
  if(GamePadOrientation)
  {
    if((accelY > minimum || accelY < -minimum))
    {
      LX = (byte)map(accelY, -10, 10, 0, 255); //Y of accelerometer is used as the x axis of the left joystick
    }
    if(accelZ > minimum || accelZ < -minimum)
    {
      LY = (byte)map(accelZ, -10, 10, 255, 0); //Z of accelerometer is used as the y axis of the left joystick
    }
  }
  else
  {
    if((accelX > minimum || accelX < -minimum))
    {
      LX = (byte)map(accelX, -10, 10, 255, 0); //Y of accelerometer is used as the x axis of the left joystick
    }
    if(accelZ > minimum || accelZ < -minimum)
    {
      LY = (byte)map(accelZ, -10, 10, 255, 0); //Z of accelerometer is used as the y axis of the left joystick
    }
  }
  Wire.write(LX); 
  Wire.write(LY); 
  Wire.write(RX);
  Wire.write(RY);
  Terminal.println("Sending joystick values");
}

//Constantly sends  to the slave.
//If a button is pressed or a voice command is recognized, pause for longer than normal
void loop() 
{
  Wire.beginTransmission(8); // transmit to device #8
  sendVoice();
  sendGamePad();
  sendAccelerometer();
  Wire.endTransmission();    // stop transmitting
  delay(10);
  //Wire.endTransmission();
  //Terminal.println("Working");
}



//Error code for Voice Recognition shield, do not change
void error(byte errorData)
{
  // Switch on error and print it on the terminal. 
  switch(errorData)
  {
    case NETWORK_TIMEOUT_ERROR: Terminal.println("Network timeout");break;
    case NETWORK_ERROR: Terminal.println("Network Error");break;
    case AUDIO_ERROR: Terminal.println("Audio error");break;
    case SERVER_ERROR: Terminal.println("No Server");break;
    case SPEECH_TIMEOUT_ERROR: Terminal.println("Speech timeout");break;
    case NO_MATCH_ERROR: Terminal.println("No match");break;
    case RECOGNIZER_BUSY_ERROR: Terminal.println("Busy");break;
  }
}

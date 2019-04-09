//====================================================================
//Project Lynxmotion Phoenix
//Description: Phoenix, control file.
//The control input subroutine for the phoenix software is placed in this file.
//Can be used with V2.0 and above
//Configuration version: V1.0
//Date: 25-10-2009
//Programmer: Jeroen Janssen (aka Xan)
//             Kurt Eckhardt (aka KurtE) - converted to c ported to Arduino...
//
//Hardware setup: PS2 version
// 
//NEW IN V1.0
//- First Release
//
//Walk method 1:
//- Left StickWalk/Strafe
//- Right StickRotate
//
//Walk method 2:
//- Left StickDisable
//- Right StickWalk/Rotate
//
//
//PS2 CONTROLS:
//[Common Controls]
//- StartTurn on/off the bot
//- L1Toggle Shift mode
//- L2Toggle Rotate mode
//- CircleToggle Single leg mode
//   - Square        Toggle Balance mode
//- TriangleMove body to 35 mm from the ground (walk pos) 
//and back to the ground
//- D-Pad upBody up 10 mm
//- D-Pad downBody down 10 mm
//- D-Pad leftdecrease speed with 50mS
//- D-Pad rightincrease speed with 50mS
//
//[Walk Controls]
//- selectSwitch gaits
//- Left Stick(Walk mode 1) Walk/Strafe
// (Walk mode 2) Disable
//- Right Stick(Walk mode 1) Rotate, 
//(Walk mode 2) Walk/Rotate
//- R1Toggle Double gait travel speed
//- R2Toggle Double gait travel length
//
//[Shift Controls]
//- Left StickShift body X/Z
//- Right StickShift body Y and rotate body Y
//
//[Rotate Controls]
//- Left StickRotate body X/Z
//- Right StickRotate body Y
//
//[Single leg Controls]
//- selectSwitch legs
//- Left StickMove Leg X/Z (relative)
//- Right StickMove Leg Y (absolute)
//- R2Hold/release leg position
//
//[GP PlLXer Controls]
//- selectSwitch Sequences
//- R2Start Sequence
//
//====================================================================
// [Include files]
#if ARDUINO>99
#include <Arduino.h> // Arduino 1.0
#else
#include <Wprogram.h> // Arduino 0022
#endif
#include "Hex_Globals.h"
#ifdef USEPS2
#include <PS2X_lib.h>

//[CONSTANTS]
#define WALKMODE          0
#define TRANSLATEMODE     1
#define ROTATEMODE        2
#define SINGLELEGMODE     3
#define GPPLLXERMODE      4


#define cTravelDeadZone 4      //The deadzone for the analog input from the remote
#define  MAXPS2ERRORCNT  5     // How many times through the loop will we go before shutting off robot?

//=============================================================================
// Global - Local to this file only...
//=============================================================================
PS2X ps2x; // create PS2 Controller Class


// Define an instance of the Input Controller...
InputController  g_InputController;       // Our Input controller 

static short      g_BodyYOffset; 
static short      g_sPS2ErrorCnt;
static short       g_BodyYShift;
static byte        ControlMode;
static bool        DoubleHeightOn;
static bool        DoubleTravelOn;
static bool        WalkMethod;
byte            GPSeq;             //Number of the sequence

// some external or forward function references.
extern void MSound(uint8_t _pin, byte cNotes, ...);
extern void PS2TurnRobotOff(void);

//==============================================================================
// This is The function that is called by the Main program to initialize
//the input controller, which in this case is the PS2 controller
//process any commands.
//==============================================================================

// If both PS2 and XBee are defined then we will become secondary to the xbee
void InputController::Init(void)
{
    int error;
    //error = ps2x.config_gamepad(9, 7, 6, 10);  // Setup gamepad (clock, command, attention, data) pins
    error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT);  // Setup gamepad (clock, command, attention, data) pins

    g_BodyYOffset = 65;  // 0 - Devon wanted...
    g_BodyYShift = 0;
    g_sPS2ErrorCnt = 0;  // error count

    ControlMode = WALKMODE;
    DoubleHeightOn = false;
    DoubleTravelOn = false;
    WalkMethod = false;

    g_InControlState.SpeedControl = 100;    // Sort of migrate stuff in from Devon.
}

//==============================================================================
// This function is called by the main code to tell us when it is about to
// do a lot of bit-bang outputs and it would like us to minimize any interrupts
// that we do while it is active...
//==============================================================================
void InputController::AllowControllerInterrupts(boolean fAllow)
{
// We don't need to do anything...
}

//==============================================================================
// This is The main code to input function to read inputs from the PS2 and then
//process any commands.
//==============================================================================

static String prevCommand = "";

void InputController::ControlInput(String command, int LX, int LY, int RX, int RY)
{
  if(command.equals(prevCommand) && !command.equals(""))
  {
    digitalWrite(13, HIGH);
    return;
  }
  else
  {
    digitalWrite(13, LOW);
    prevCommand = command;
  }
    // Then try to receive a packet of information from the PS2.
    // Then try to receive a packet of information from the PS2.
    //ps2x.read_gamepad();          //read controller and set large motor to spin at 'vibrate' speed

    // Wish the library had a valid way to verify that the read_gamepad succeeded... Will hack for now
    //if ((ps2x.Analog(1) & 0xf0) == 0x70) {
        // In an analog mode so should be OK...
        g_sPS2ErrorCnt = 0;    // clear out error count...
        
        if (command.equals("Start")) {// OK lets try "0" button for Start. 
            if (g_InControlState.fHexOn) {
              delay(250);
                PS2TurnRobotOff();
            } else {
                //Turn on
                g_InControlState.fHexOn = 1;
                g_BodyYOffset = 65;
            }
        }

        if (g_InControlState.fHexOn) {
            // [SWITCH MODES]
    
             //Translate mode
            if (command.equals("L1")) {// L1 Button Test
                MSound(SOUND_PIN, 1, 50, 2000);  //sound SOUND_PIN, [50\4000]
                if (ControlMode != TRANSLATEMODE )
                    ControlMode = TRANSLATEMODE;
                else {
                    if (g_InControlState.SelectedLeg==255) 
                        ControlMode = WALKMODE;
                    else
                        ControlMode = SINGLELEGMODE;
                }
            }
  
            //Rotate mode
            if (command.equals("L2")) {    // L2 Button Test
                MSound(SOUND_PIN, 1, 50, 2000);  //sound SOUND_PIN, [50\4000]
                if (ControlMode != ROTATEMODE)
                    ControlMode = ROTATEMODE;
                else {
                    if (g_InControlState.SelectedLeg == 255) 
                        ControlMode = WALKMODE;
                    else
                        ControlMode = SINGLELEGMODE;
                }
            }
    
            //Single leg mode fNO
            if (command.equals("Circle")) {// O - Circle Button Test
                if (abs(g_InControlState.TravelLength.x)<cTravelDeadZone && abs(g_InControlState.TravelLength.z)<cTravelDeadZone 
                        && abs(g_InControlState.TravelLength.y*2)<cTravelDeadZone )   {
                    //Sound SOUND_PIN,[50\4000]
                    if (ControlMode != SINGLELEGMODE) {
                        ControlMode = SINGLELEGMODE;
                            if (g_InControlState.SelectedLeg == 255)  //Select leg if none is selected
                                g_InControlState.SelectedLeg=cRF; //Startleg
                    } else {
                        ControlMode = WALKMODE;
                        g_InControlState.SelectedLeg=255;
                    }
                }
            }      

#ifdef OPT_GPPLAYER
#endif // OPT_GPPLAYER

            //[Common functions]
            //Switch Balance mode on/off 
            if (command.equals("Square")) { // Square Button Test
                g_InControlState.BalanceMode = !g_InControlState.BalanceMode;
                if (g_InControlState.BalanceMode) {
                    MSound(SOUND_PIN, 1, 250, 1500);  //sound SOUND_PIN, [250\3000]
                } else {
                    MSound(SOUND_PIN, 2, 100, 2000, 50, 4000);
                }
            }

            //Stand up, sit down  
            if (command.equals("Triangle")) { // Triangle - Button Test
                if (g_BodyYOffset>0) 
                    g_BodyYOffset = 0;
                else
                    g_BodyYOffset = 65;
            }

            if (command.equals("Up"))// D-Up - Button Test
                g_BodyYOffset += 10;

            if (command.equals("Down"))// D-Down - Button Test
                g_BodyYOffset -= 10;

            if (command.equals("Right")) { // D-Right - Button Test
                if (g_InControlState.SpeedControl>0) {
                    g_InControlState.SpeedControl = g_InControlState.SpeedControl - 50;
                    MSound(SOUND_PIN, 1, 50, 2000);  //sound SOUND_PIN, [50\4000]
                }
            }

            if (command.equals("Left")) { // D-Left - Button Test
                if (g_InControlState.SpeedControl<2000 ) {
                    g_InControlState.SpeedControl = g_InControlState.SpeedControl + 50;
                    MSound(SOUND_PIN, 1, 50, 2000);  //sound SOUND_PIN, [50\4000]
                }
            }

            //[Walk functions]
            if (ControlMode == WALKMODE) {
                //Switch gates
                if (command.equals("Select")            // Select Button Test
                        && abs(g_InControlState.TravelLength.x)<cTravelDeadZone //No movement
                        && abs(g_InControlState.TravelLength.z)<cTravelDeadZone 
                        && abs(g_InControlState.TravelLength.y*2)<cTravelDeadZone  ) {
                    g_InControlState.GaitType = g_InControlState.GaitType+1;                    // Go to the next gait...
                    if (g_InControlState.GaitType<NUM_GAITS) {                 // Make sure we did not exceed number of gaits...
                        MSound(SOUND_PIN, 1, 50, 2000);  //sound SOUND_PIN, [50\4000]
                    } else {
                        MSound (SOUND_PIN, 2, 50, 2000, 50, 2250); 
                        g_InControlState.GaitType = 0;
                    }
                    GaitSelect();
                }
  
                //Double leg lift height
                if (command.equals("R1")) { // R1 Button Test
                    MSound(SOUND_PIN, 1, 50, 2000);  //sound SOUND_PIN, [50\4000]
                    DoubleHeightOn = !DoubleHeightOn;
                    if (DoubleHeightOn)
                        g_InControlState.LegLiftHeight = 80;
                    else
                        g_InControlState.LegLiftHeight = 50;
                }
  
                //Double Travel Length
                if (command.equals("R2")) {// R2 Button Test
                    MSound (SOUND_PIN, 1, 50, 2000);  //sound SOUND_PIN, [50\4000]
                    DoubleTravelOn = !DoubleTravelOn;
                }
  
                // Switch between Walk method 1 && Walk method 2
                if (command.equals("R3")) { // R3 Button Test
                    MSound (SOUND_PIN, 1, 50, 2000);  //sound SOUND_PIN, [50\4000]
                    WalkMethod = !WalkMethod;
                }
  
                //Walking
                if (WalkMethod)  //(Walk Methode) 
                    g_InControlState.TravelLength.z = ((RY)-128); //Right Stick Up/Down  

                else {
                    g_InControlState.TravelLength.x = -((RX) - 128);
                    g_InControlState.TravelLength.z = ((LY) - 128);
                }

                if (!DoubleTravelOn) {  //(Double travel length)
                    g_InControlState.TravelLength.x = g_InControlState.TravelLength.x/2;
                    g_InControlState.TravelLength.z = g_InControlState.TravelLength.z/2;
                }

                g_InControlState.TravelLength.y = -((LX) - 128)/4; //Right Stick Left/Right 
            }

            //[Translate functions]
            g_BodyYShift = 0;
            if (ControlMode == TRANSLATEMODE) {
                g_InControlState.BodyPos.x = ((LX) - 128)/2;
                g_InControlState.BodyPos.z = -((LY) - 128)/3;
                g_InControlState.BodyRot1.y = (RX - 128)*2;
                g_BodyYShift = (-(RY - 128)/2);
            }

            //[Rotate functions]
            if (ControlMode == ROTATEMODE) {
                g_InControlState.BodyRot1.x = (LY - 128);
                g_InControlState.BodyRot1.y = (RX - 128)*2;
                g_InControlState.BodyRot1.z = (LX - 128);
                g_BodyYShift = (-(RY - 128)/2);
            }

            //[Single leg functions]
            if (ControlMode == SINGLELEGMODE) {
                //Switch leg for single leg control
                if (command.equals("Select")) { // Select Button Test
                    MSound (SOUND_PIN, 1, 50, 2000);  //sound SOUND_PIN, [50\4000]
                    if (g_InControlState.SelectedLeg<5)
                        g_InControlState.SelectedLeg = g_InControlState.SelectedLeg+1;
                    else
                        g_InControlState.SelectedLeg=0;
                }

                g_InControlState.SLLeg.x= (LX - 128)/2; //Left Stick Right/Left
                g_InControlState.SLLeg.y= ((RY) - 128)/10; //Right Stick Up/Down
                g_InControlState.SLLeg.z = ((LY) - 128)/2; //Left Stick Up/Down

                // Hold single leg in place
                if (command.equals("R2")) { // R2 Button Test
                    MSound (SOUND_PIN, 1, 50, 2000);  //sound SOUND_PIN, [50\4000]
                    g_InControlState.fSLHold = !g_InControlState.fSLHold;
                }
            }

#ifdef OPT_GPPLAYER
#endif // OPT_GPPLAYER

            //Calculate walking time delay
            g_InControlState.InputTimeDelay = 128 - max(max(abs((LX) - 128), abs((LY) - 128)), abs((RX) - 128));
        }
  
        //Calculate g_InControlState.BodyPos.y
        g_InControlState.BodyPos.y = max(g_BodyYOffset + g_BodyYShift,  0); 
}

//==============================================================================
// PS2TurnRobotOff - code used couple of places so save a little room...
//==============================================================================
void PS2TurnRobotOff(void)
{
   //Turn off
    g_InControlState.BodyPos.x = 0;
    g_InControlState.BodyPos.y = 0;
    g_InControlState.BodyPos.z = 0;
    g_InControlState.BodyRot1.x = 0;
    g_InControlState.BodyRot1.y = 0;
    g_InControlState.BodyRot1.z = 0;
    g_InControlState.TravelLength.x = 0;
    g_InControlState.TravelLength.z = 0;
    g_InControlState.TravelLength.y = 0;
    g_BodyYOffset = 0;
    g_BodyYShift = 0;
    g_InControlState.SelectedLeg = 255;
    g_InControlState.fHexOn = 0;
}


#endif //USEPS2

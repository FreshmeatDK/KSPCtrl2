// From Demo13

#include <EngNumber.h>
#include <Wire.h>
#include <math.h>
#include <Key.h>
#include <Keypad.h>
#include <TFT.h>
#include <SPI.h>
#include <LedControl.h>
#include <NewliquidCrystal\LiquidCrystal_I2C.h>

//clock address
#define RTCADR 0x68

#define ENGNUM_DIGITS 4

// Comm pins
#define SHFT_LTCH 48
#define MPIN_LOW 23
#define MPIN_HIGH 27
#define TFT_CS 7
#define TFT_DC 8
#define TFT_RST 6
#define MAX_DIN 26
#define MAX_CS 24
#define MAX_CLK 22

//Analog pins
#define RCSXPIN 4
#define RCSYPIN 3
#define THRTPIN 5
#define JOYXPIN 1
#define JOYYPIN 0
#define JOYZPIN 2

//PWM pins
#define MONOPIN 2
#define ELECPIN 3
#define FUELPIN 4
#define RDRPIN 5

//Digtal pins: Port L
#define STAGEPIN 43
#define JOYBTPIN 47
#define RCSBTPIN 42
#define RCSFPIN 46
#define RCSBPIN 44
#define BRKPIN 45

//Input enums
#define SAS 7
#define RCS 6
#define LIGHTS 5
#define GEAR 4
#define BRAKES 3
#define PRECISION 2
#define ABORT 1
#define STAGE 0

//Action group statuses
#define AGSAS      0
#define AGRCS      1
#define AGLight    2
#define AGGear     3
#define AGBrakes   4
#define AGAbort    5
#define AGCustom01 6
#define AGCustom02 7
#define AGCustom03 8
#define AGCustom04 9
#define AGCustom05 10
#define AGCustom06 11
#define AGCustom07 12
#define AGCustom08 13
#define AGCustom09 14
#define AGCustom10 15

//SAS Modes
#define SMOFF           0
#define SMSAS           1
#define SMPrograde      2
#define SMRetroGrade    3
#define SMNormal        4
#define SMAntinormal    5
#define SMRadialIn      6
#define SMRadialOut     7
#define SMTarget        8
#define SMAntiTarget    9
#define SMManeuverNode  10

//Navball Target Modes
#define NAVBallIGNORE   0
#define NAVBallORBIT    1
#define NAVBallSURFACE  2
#define NAVBallTARGET   3

//macro
#define details(name) (uint8_t*)&name,sizeof(name)

//if no message received from KSP for more than 2s, go idle
#define IDLETIMER 2000
#define CONTROLREFRESH 25
#define INDICATORCYCLETIME 250

//warnings
#define GWARN 9                  //9G Warning
#define GCAUTION 5               //5G Caution
#define FUELCAUTION 10.0         //10% Fuel Caution
#define FUELWARN 5.0             //5% Fuel warning

//deadbands for analog joysticks

const int joyXDB[2] = { 480,540 };
const int joyYDB[2] = { 480,560 };
const int joyZDB[2] = { 465,565 };

const int RCSXDB[2] = { 513,529 };
const int RCSYDB[2] = { 485,497 };

const int anMax = 1013; // max value at analog joysticks (common for some reason)

const char prefixB[] = { ' ', 'k','M','G','T' }; // decimal prefixes for EngNumber
const char prefixS[] = { ' ','m', 228,'n','p' };

unsigned long deadtime, deadtimeOld, controlTime, controlTimeOld, indicatorTime, indicatorTimeOld, mainTime, mainTimeOld;
unsigned long now, nowIndic, nowMain;
byte indicatorCycle = 0;

byte inbytes[6], outbytes[3]; // Bytes to hold switch states and temp results
int joyX, joyY, joyZ, rcsX, rcsY, throttle; // analog raw value
int joyXN, joyYN, joyZN, rcsXN, rcsYN; // values with deadband and full range
boolean rcsF, rcsB, rcsBt, joyBt, stage, schemeSelect, invSS; //digital inputs
boolean tglSAS = 0, joyBtPrs = 0, tglRCS = 0, rcsBtPrs = 0;
byte second = 0 , minute, hour = 0, dayOfWeek, dayOfMonth, month, year; // bytes to hold RT clock
char key; // keypress buffer
char cmdStr[19]; // command string to pass
byte cmdStrIndex = 0; //current lenght of cmdStr
byte oldset; //old seetting of selector switch
float slopecalc; // calculated value of current slope of ground, based on vehicle orientation.
boolean autopitch = 1;
float oldAlt, newAlt, VDevNew, VDevOld, dVDev; // for autopilot
int dPitch;
float corr;
boolean longBlink = 0;
boolean shortBlink = 0;

boolean Connected = false;

byte caution = 0, warning = 0, id;
float acc = 1; // default acceleration, can be changed

struct VesselData
{
    byte id;                //1
    float AP;               //2
    float PE;               //3
    float SemiMajorAxis;    //4
    float SemiMinorAxis;    //5
    float VVI;              //6
    float e;                //7
    float inc;              //8
    float G;                //9
    long TAp;               //10
    long TPe;               //11
    float TrueAnomaly;      //12
    float Density;          //13
    long period;            //14
    float RAlt;             //15
    float Alt;              //16
    float Vsurf;            //17
    float Lat;              //18
    float Lon;              //19
    float LiquidFuelTot;    //20
    float LiquidFuel;       //21
    float OxidizerTot;      //22
    float Oxidizer;         //23
    float EChargeTot;       //24
    float ECharge;          //25
    float MonoPropTot;      //26
    float MonoProp;         //27
    float IntakeAirTot;     //28
    float IntakeAir;        //29
    float SolidFuelTot;     //30
    float SolidFuel;        //31
    float XenonGasTot;      //32
    float XenonGas;         //33
    float LiquidFuelTotS;   //34
    float LiquidFuelS;      //35
    float OxidizerTotS;     //36
    float OxidizerS;        //37
    uint32_t MissionTime;   //38
    float deltaTime;        //39
    float VOrbit;           //40
    uint32_t MNTime;        //41
    float MNDeltaV;         //42
    float Pitch;            //43
    float Roll;             //44
    float Heading;          //45
    uint16_t ActionGroups;  //46 status bit order:SAS, RCS, Light, Gear, Brakes, Abort, Custom01 - 10
    byte SOINumber;         //47 SOI Number (decimal format: sun-planet-moon e.g. 130 = kerbin, 131 = mun)
    byte MaxOverHeat;       //48  Max part overheat (% percent)
    float MachNumber;       //49
    float IAS;              //50  Indicated Air Speed
    byte CurrentStage;      //51  Current stage number
    byte TotalStage;        //52  TotalNumber of stages
	float TargetDist;       //53  Distance to targeted vessel (m)
	float TargetV;          //54  Target vessel relative velocity
	byte NavballSASMode;    //55  Combined byte for navball target mode and SAS mode
							// First four bits indicate AutoPilot mode:
							// 0 SAS is off  //1 = Regular Stability Assist //2 = Prograde
							// 3 = RetroGrade //4 = Normal //5 = Antinormal //6 = Radial In
							// 7 = Radial Out //8 = Target //9 = Anti-Target //10 = Maneuver node
							// Last 4 bits set navball mode. (0=ignore,1=ORBIT,2=SURFACE,3=TARGET)

};

struct HandShakePacket
{
  byte id;
  byte M1;
  byte M2;
  byte M3;
};

struct ControlPacket {
  byte id;
  byte MainControls;                  //SAS RCS Lights Gear Brakes Precision Abort Stage
  byte Mode;                          //0 = stage, 1 = docking, 2 = map
  unsigned int ControlGroup;          //control groups 1-10 in 2 bytes
  byte NavballSASMode;                //AutoPilot mode
  byte AdditionalControlByte1;        //other stuff
  int Pitch;                          //-1000 -> 1000
  int Roll;                           //-1000 -> 1000
  int Yaw;                            //-1000 -> 1000
  int TX;                             //-1000 -> 1000
  int TY;                             //-1000 -> 1000
  int TZ;                             //-1000 -> 1000
  int WheelSteer;                     //-1000 -> 1000
  int Throttle;                       //    0 -> 1000
  int WheelThrottle;                  //    0 -> 1000
};

HandShakePacket HPacket;
VesselData VData;
ControlPacket CPacket;
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // set the LCD address to 0x27 
LiquidCrystal_I2C lcd2(0x23, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // set the LCD address to 0x23


// init LED
LedControl lc=LedControl(MAX_DIN, MAX_CLK, MAX_CS, 1);


//init tft
TFT screen = TFT(TFT_CS, TFT_DC, TFT_RST);

  //init keypad
  const byte rows = 4; //four rows
  const byte cols = 3; //three columns
  char keys[rows][cols] = {
    {'*','0','#'},
    {'7','8','9'},
    {'4','5','6'},
    {'1','2','3'}
  };
  byte rowPins[rows] = {28, 30, 32, 34}; //connect to the row pinouts of the keypad
  byte colPins[cols] = {29, 31, 33}; //connect to the column pinouts of the keypad
  Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, rows, cols );
 

void setup() {
	
  Serial.begin(38400);
  lcd.begin(20, 4);
  lcd.backlight();
  lcd2.begin(20, 4);
  lcd2.backlight();

    // Initialize the MAX7219 device
  lc.shutdown(0,false);   // Enable display
  lc.setIntensity(0,10);  // Set brightness level (0 is min, 15 is max)
  lc.clearDisplay(0);     // Clear display register
  
  // initialize the TFT screen
  screen.begin();
  screen.background(0,250,0); // make the background black
  screen.stroke(255,255,255);   // set the stroke color to white

  // init keypad
  keypad.setHoldTime(100);
  keypad.setDebounceTime(50);

    
  //init pwr pins
  for (int i = MPIN_LOW; i < MPIN_HIGH+1; i = i+2){
    pinMode(i,OUTPUT);
    digitalWrite(i,LOW);    
  }
  // init CD4021 pins
  pinMode(SHFT_LTCH, OUTPUT); //latch

  //init guage pins
  for (int i = 2; i < 6; i++) {
	  pinMode(i, OUTPUT);
	  digitalWrite(i, LOW);
  }
  
  // initialize spi communication
  SPI.begin();

  initLEDS();
  InitTxPackets();
  controlsInit();

  LEDSAllOff();

}

void loop()
{
	input();
	output();	
}



















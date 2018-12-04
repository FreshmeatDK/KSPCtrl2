
#include <Keyboard.h>
#include <Keypad.h>
#include <Wire.h>
#include <math.h>
#include <SPI.h>
#include <FastLED.h>
#include <LedControl.h>
#include <NewLiquidCrystal/LiquidCrystal_I2C.h>
#include <avr/dtostrf.h>

//I2C adresses
#define RTCADR 0x68


// pin definitions

// PWR for gauges
#define CHARGE 4
#define MONO 5
#define SPEED 6
#define ALT 7
// 74hc165
#define SS1 46
#define CLKI 48
// CD4021
#define SS2 45
#define CLK 27
#define DTA 25
// FastLed
#define LEDPIN 23
// LedControl
#define LCCLK 43
#define LCCS 39
#define LCDATA 41
//Joystick buttons
#define JOY1BTN 50
#define JOY2BTN 42
#define JOY2FWD 38
#define JOY2BCK 40

// analog pins
#define TRIMYAW 2
#define TRIMPITCH 3
#define TRIMROLL 1
#define TRIMENGINE 0
#define JOY1X 4
#define JOY1Y 5 
#define JOY1Z 6
#define THROTTLE 7
#define JOY2X 8
#define JOY2Y 9 


// number of units attached
#define NUMLC 3
#define NUMLEDS 36
#define NUMIC1 4
#define NUMIC2 1

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


//vars
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

}__attribute__((packed));

struct HandShakePacket
{
	byte id;
	byte M1;
	byte M2;
	byte M3;
}__attribute__((packed));

struct ControlPacket {
	byte id;
	byte MainControls;                  //SAS RCS Lights Gear Brakes Precision Abort Stage
	byte Mode;                          //0 = stage, 1 = docking, 2 = map
	uint16_t ControlGroup;          //control groups 1-10 in 2 bytes
	byte NavballSASMode;                //AutoPilot mode
	byte AdditionalControlByte1;        //other stuff
	int16_t Pitch;                          //-1000 -> 1000
	int16_t Roll;                           //-1000 -> 1000
	int16_t Yaw;                            //-1000 -> 1000
	int16_t TX;                             //-1000 -> 1000
	int16_t TY;                             //-1000 -> 1000
	int16_t TZ;                             //-1000 -> 1000
	int16_t WheelSteer;                     //-1000 -> 1000
	int16_t Throttle;                       //    0 -> 1000
	int16_t WheelThrottle;                  //    0 -> 1000
}__attribute__((packed));

HandShakePacket HPacket;
VesselData VData;
ControlPacket CPacket;

unsigned long deadtime, deadtimeOld, controlTime, controlTimeOld, indicatorTime, indicatorTimeOld, mainTime, mainTimeOld;
unsigned long now, nowIndic, nowMain;
uint32_t SASgrace; //time until we check that SAS is not in agreement and enforce, needed to avoid ping-pong.

byte second = 0, minute, hour = 0, dayOfWeek, dayOfMonth, month, year; // bytes to hold RT clock
char key; // keypress buffer
char cmdStr[19]; // command string to pass
byte cmdStrIndex = 0; //current lenght of cmdStr
int trimY, trimP, trimR, trimE;

long timeout = 0; //timeout counter
bool connected, displayoff; // are we connected and are we in blackout
bool snia; //sas not in agreement

char keys[5][8] = {
	{ '7', '8', '9', '\'', '.', ',', 'S', 'M' },
{ '4', '5', '6', 'c', 'v', 'V', 'P', 'R' },
{ '1', '2', '3', 'u' ,'i' , 'b', 'I', 'O' },
{ '0', '*', '#', 'm', 'T', 'B', 'N', 'A' },
{ 'W', 's', 'x', 'å', ']', 'r', 'g', 'G' }
};
byte rowPins[5] = { 35, 33, 31, 29, 37 };
byte colPins[8] = { 26, 24, 22, 30, 28, 32, 34, 36 };

byte slaveCtrl; //byte to send to slave Arduino to forward to kRPC

// objects
CRGB leds[NUMLEDS], oldLeds[NUMLEDS]; // Array of WS2811
byte dataIn[NUMIC1 + NUMIC2]; // Byte array of spi inputs
byte dataOld[NUMIC1 + NUMIC2]; //testing array
LedControl lc = LedControl(LCDATA, LCCLK, LCCS, NUMLC);
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // set the LCD address to 0x27 
LiquidCrystal_I2C lcd2(0x23, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // set the LCD address to 0x23
Keypad keymain(makeKeymap(keys), rowPins, colPins, 5, 8);


void setup()
{
	Serial.begin(38400);
	Wire.begin();
	Keyboard.begin();

	// Meter init
	pinMode(CHARGE, OUTPUT);
	analogWrite(CHARGE, 0);
	pinMode(MONO, OUTPUT);
	analogWrite(MONO, 0);
	pinMode(SPEED, OUTPUT);
	analogWrite(SPEED, 0);
	pinMode(ALT, OUTPUT);
	analogWrite(ALT, 0);

	//Joystick init
	pinMode(JOY1BTN, INPUT);
	pinMode(JOY2BTN, INPUT);
	pinMode(JOY2FWD, INPUT);
	pinMode(JOY2BCK, INPUT);

	// LCD init
	lcd.begin(20, 4);
	lcd.backlight();

	lcd2.begin(20, 4);
	lcd2.backlight();


	// SPI init
	SPI.begin();
	pinMode(SS1, OUTPUT);
	pinMode(SS2, OUTPUT);

	pinMode(CLKI, OUTPUT);
	pinMode(CLK, OUTPUT);
	pinMode(DTA, INPUT);


	//LED strip init
	FastLED.addLeds<NEOPIXEL, LEDPIN>(leds, NUMLEDS);

	//7seg LED init
	for (int i = 0; i < NUMLC; i++)
	{
		lc.shutdown(i, false);
		lc.setIntensity(i, 1);
		lc.clearDisplay(i);
	}

	//initLEDS();
	InitTxPackets();


	LEDSAllOff();

}

// Add the main program code into the continuous loop() function
void loop()
{
	//input();
	//output();
	testSuite();
}
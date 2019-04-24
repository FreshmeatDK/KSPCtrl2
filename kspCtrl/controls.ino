

void trimmers() 
//analogue inputs
{
	int trYaw = 0, trPitch = 0, trRoll = 0, trEng = 0;

	static int trY0, trP0, trR0, trE0; //previous values of read variables
	static int trY1, trP1, trR1, trE1; //second previous values of read variables, to filter noise
	static unsigned long timer; // time when display started
	bool change;

	trYaw = 50 * nPotJy(analogRead(TRIMYAW), 1023, 500, 454, 4, -20, 20);
	trPitch = 50 * nPotJy(analogRead(TRIMPITCH), 1023, 500, 454, 4, -20, 20);
	trRoll = 50 * nPotJy(analogRead(TRIMROLL), 1023, 545, 501, 2, -20, 20);
	trEng = 5 * (nPotSl(analogRead(TRIMENGINE), 1023, 1, 10, 0, 20));

	if ((g_trimY != trY0) && (g_trimY != trY1))
	{
		g_trimY = trYaw;
		LCPotDisplay(2, g_trimY,'y');
		timer = millis();
	}

	if ((g_trimP != trP0) && (g_trimP != trP1))
	{
		g_trimP = trPitch;
		LCPotDisplay(2, g_trimP,'p');
		timer = millis();
	}

	if ((g_trimR != trR0) && (g_trimR != trR1))
	{
		g_trimR = trRoll;
		LCPotDisplay(2, g_trimR,'r');
		timer = millis();
	}

	if ((g_trimE != trE0) && (g_trimE != trE1))
	{
		g_trimE = trEng;
		LCPotDisplay(2, g_trimE,'e');
		timer = millis();
	}

	trY1 = trY0;
	trP1 = trP0;
	trR1 = trR0;
	trE1 = trE0;

	trY0 = trYaw;
	trP0 = trPitch;
	trR0 = trRoll;
	trE0 = trEng;

	/*Serial.print(g_trimY);
	Serial.print(' ');
	Serial.print(trY0);
	Serial.print(' ');
	Serial.print(trY1);
	Serial.print(' ');
	Serial.println(analogRead(TRIMYAW));*/

	if (millis() - timer > 1000)
	{
		lc.clearDisplay(2);
	}

}

void toggles()
{
	bool change = false;
	SPI.beginTransaction(SPISettings(6000000, MSBFIRST, SPI_MODE2));

	delay(1);
	digitalWrite(SS1, HIGH);

	for (int i = 0; i < NUMIC1; i++)
	{
		digitalWrite(CLKI, LOW);
		dataIn[i] = SPI.transfer(0x00);
		digitalWrite(CLKI, HIGH);

	}

	digitalWrite(SS1, LOW);
	SPI.endTransaction();
	SPI.beginTransaction(SPISettings(3000000, MSBFIRST, SPI_MODE2));
	digitalWrite(SS2, LOW);
	delay(1);

	for (int i = NUMIC1; i < NUMIC1 + NUMIC2; i++)
	{
		dataIn[i] = shiftIn(DTA, CLK, MSBFIRST);
	}

	digitalWrite(SS2, HIGH);
	SPI.endTransaction();
}

void joysticks()
{
	int j1x, j1y, j1z, j2x, j2y, j2z, thr;

	trimmers(); // update trimmers

	j1x = nPotJy(analogRead(JOY1X), 3, 503, 550, 1023, -1000, 1000);
	CPacket.Yaw = constrain(j1x + g_trimY, -1000, 1000);

	j1y = nPotJy(analogRead(JOY1Y), 1023, 500, 540, 3, -1000, 1000);
	CPacket.Pitch = constrain(j1y + g_trimP, -1000, 1000);

	j1z = nPotJy(analogRead(JOY1Z), 3, 480, 550, 1023, -1000, 1000);

	if ((dataIn[0] & B00000111) == 3)
	{
		CPacket.Roll = CPacket.Yaw;
		CPacket.Yaw = constrain(j1z + g_trimR, -1000, 1000);
	}
	else
	{
		CPacket.Roll = constrain(j1z + g_trimR, -1000, 1000);
	}

	j2x = nPotJy(analogRead(JOY2X), 3, 500, 540, 1023, -1000, 1000);
	CPacket.TX = constrain((j2x * g_trimE) / 100, -1000, 1000);

	j2y = nPotJy(analogRead(JOY2Y), 3, 470, 510, 1023, -1000, 1000);
	CPacket.TY = constrain((j2y * g_trimE) / 100, -1000, 1000);

	j2z = (digitalRead(JOY2FWD) - digitalRead(JOY2BCK));
	CPacket.TZ = constrain((j2z*g_trimE) * 10, -1000, 1000);

	thr = nPotSl(analogRead(THROTTLE), 1000, 3, 15, 0, 1000);
	CPacket.Throttle = constrain((thr*g_trimE) / 100, 0, 1000);
}

void chkKeypad() {

	static uint32_t SciGrace = 0; //time until we can redeploy unrepeatable science
	static char cmdStr[19]; // command string to pass

	char *s; // result of search
	char key = keymain.getKey();

	static byte cmdStrIndex = 0; //current lenght of cmdStr

	if (key != NO_KEY) 
	{
		//Serial.println(key);
		if ((key == '#')) {
			cmdStr[cmdStrIndex - 1] = '\0';
			cmdStrIndex--;
			lcd2.setCursor(cmdStrIndex, 3);
			lcd2.print(" ");

		}
		
		s = strchr("0123456789*", key); // Internal control panel codes
		if (s != NULL) 
		{
			cmdStr[cmdStrIndex] = key;
			cmdStrIndex++;
			
		}
		s = strchr(",.-", key); //send to computer as keystrokes
		if (s != NULL)
		{
			Keyboard.print(key);
		
		}

		if (key == 'M') // phys warp increase
		{
			Keyboard.press(130);
			Keyboard.press('.');
			delay(1);
			Keyboard.release('.');
			Keyboard.release(130);
		}

		if (key == 'S') // phys warp decrease
		{
			Keyboard.press(130);
			Keyboard.press(',');
			delay(1);
			Keyboard.release(',');
			Keyboard.release(130);
		}
				

		if (key == 'P') // reaction wheels on
		{
			g_rwheels = true;
		}

		if (key == 'R') // reaction wheels off
		{
			g_rwheels = false;
		}

		if (key == 'c') // navball: surface
		{
			setNavballMode(NAVBallSURFACE);
		}

		if (key == 'v') // navball: orbit
		{
			setNavballMode(NAVBallORBIT);
		}

		if (key == 'V') // navball: target
		{
			setNavballMode(NAVBallTARGET);
		}

		if (key == 'O')// engage parachute
		{
			g_parachute = true;
		}

		if (key == 'A') // repeatable scinece
		{
			g_repscience = true;
		}

		if (key == 'G') // all science
		{
			if ((millis() - SciGrace) > 1000)
			{
				SciGrace = millis();
				g_allscience = true;
			}
		}
				
		if (cmdStrIndex > 18) cmdStrIndex = 18;
	}
	lcd2.setCursor(0, 3);
	lcd2.print(cmdStr);
	if ((cmdStr[cmdStrIndex - 1] == '*') && (cmdStr[cmdStrIndex - 2] == '*')) {
		execCmd(cmdStr, cmdStrIndex);
		lcd2.clear();
		for (int i = 0; i <= 18; i++) {
			cmdStr[i] = '\0';
		}
		cmdStrIndex = 0;
	}

}

void CtlUpdate()
{
	static bool snia; //sas not in agreement
	static uint32_t SASgrace = 0; //time until we check that SAS is not in agreement and enforce, needed to avoid ping-pong.

	byte sasMap[10] = { 9,3,5,7,9,6,4,2,1,10 };
	byte sasVal;
	bool statusRead;
	
	//pack kRPCPacket array to send to kRPC

	kRPCPacket[0] = (dataIn[0] & B11100000); //This holds camera nibble
	sasVal = (dataIn[3] & B00001111); //we borrow this byte for placeholder to make things easier to follow
	kRPCPacket[0] = (kRPCPacket[0] | sasVal); //sasval adds holds  solar, radiator, cargo and reserve battery
	sasVal = (dataIn[3] & B00100000); // engine mode
	kRPCPacket[0] = (kRPCPacket[0] | (g_rwheels << 4));
	kRPCPacket[1] = 0;
	kRPCPacket[1] = (kRPCPacket[1] | (sasVal >> 5)); // add engine mode to last bit
	kRPCPacket[1] = (kRPCPacket[1] | (g_parachute << 1));
	kRPCPacket[1] = (kRPCPacket[1] | (g_repscience << 2));
	kRPCPacket[1] = (kRPCPacket[1] | (g_allscience << 3));
	kRPCPacket[1] = (kRPCPacket[1] | (g_autoland << 4));
	g_autoland = 0;

	
	//set control toggles that does not have LED attached
	sasVal = 15 - (dataIn[1] & B00001111);
	if ((getSASMode() != sasMap[sasVal]) && (millis()-SASgrace > 200 ))
	{
		if (!snia)
		{
			setSASMode(SMSAS);
			snia = true;
		}
		else
		{
			setSASMode(sasMap[sasVal]);
			snia = false;
			SASgrace = millis();
		}
	}
		
	MainControls(STAGE, (dataIn[4] & B00000100));
	MainControls(ABORT, (dataIn[0] & B00010000));

	/*Picks out the relevant toggle of the dataIn bytes and change LED accordingly*/

	//AG 1
	statusRead = (dataIn[1] & B00010000);
	statusLED(19, statusRead);
	ControlGroups(1, statusRead);

	//AG 2
	statusRead = (dataIn[1] & B00100000);
	statusLED(18, statusRead);
	ControlGroups(2, statusRead);

	//AG 3
	statusRead = (dataIn[2] & B00010000);
	statusLED(17, statusRead);
	ControlGroups(3, statusRead);

	//AG 4
	statusRead = (dataIn[2] & B00100000);
	statusLED(16, statusRead);
	ControlGroups(4, statusRead);

	//AG 5
	statusRead = (dataIn[2] & B01000000);
	statusLED(15, statusRead);
	ControlGroups(5, statusRead);

	//AG 6
	statusRead = (dataIn[2] & B10000000);
	statusLED(14, statusRead);
	ControlGroups(6, statusRead);

	//AG 7
	statusRead = (dataIn[2] & B00000001);
	statusLED(13, statusRead);
	ControlGroups(7, statusRead);

	//AG 8
	statusRead = (dataIn[2] & B00000010);
	statusLED(12, statusRead);
	ControlGroups(8, statusRead);

	//AG 9
	statusRead = (dataIn[2] & B00000100);
	statusLED(11, statusRead);
	ControlGroups(9, statusRead);

	//AG 10
	statusRead = (dataIn[2] & B00001000);
	statusLED(10, statusRead);
	ControlGroups(10, statusRead);

	//SAS
	statusRead = (dataIn[1] & B01000000);
	statusLED(0, statusRead);
	MainControls(SAS, statusRead);

	//RCS
	statusRead = (dataIn[1] & B10000000);
	statusLED(1, statusRead);
	MainControls(RCS, statusRead);

	//Gear
	statusRead = (dataIn[3] & B10000000);
	statusLED(2, statusRead);
	MainControls(GEAR, statusRead);

	//Brakes
	statusRead = ((dataIn[3] & B01000000) || (dataIn[4] & B00000010));
	statusLED(3, statusRead);
	MainControls(BRAKES, statusRead);
	

	//Engine mode
	statusRead = (dataIn[3] & B00100000);
	statusLED(4, statusRead);

	//Lights
	statusRead = (dataIn[3] & B00010000);
	statusLED(5, statusRead);
	MainControls(LIGHTS, statusRead);

	//Solar panels
	statusRead = (dataIn[3] & B00001000);
	statusLED(6, statusRead);

	//Radiators
	statusRead = (dataIn[3] & B00000100);
	statusLED(7, statusRead);

	//Cargo bays
	statusRead = (dataIn[3] & B00000010);
	statusLED(8, statusRead);

	//Reserve batteries
	statusRead = !(dataIn[3] & B00000001); // flipped a connection in hardware :-/
	statusLED(9, statusRead);


	FastLED.show();


}

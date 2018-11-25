void joysticks()
{
	int j1x, j1y, j1z, j2x, j2y, j2z, thr;
	
	trimmers(); // update trimmers

	j1x = nPotJy(analogRead(JOY1X), 3, 503, 550, 1023, -1000, 1000);
	CPacket.Yaw = constrain(j1x + trimY, -1000, 1000);

	j1y = nPotJy(analogRead(JOY1Y), 3, 510, 540, 1023, -1000, 1000);
	CPacket.Pitch = constrain(j1y + trimP, -1000, 1000);

	j1z = nPotJy(analogRead(JOY1Z), 3, 500, 550, 1023, -1000, 1000);

	if ((dataIn[0] & B00000111) == 3)
	{
		CPacket.Roll = CPacket.Yaw;
		CPacket.Yaw = constrain(j1z + trimR, -1000, 1000);
	}
	else
	{
		CPacket.Roll = constrain(j1z + trimR, -1000, 1000);
	}
	
	j2x = nPotJy(analogRead(JOY2X), 3, 525, 535, 1023, -1000, 1000);
	CPacket.TX = constrain((j2x * trimE)/100, -1000, 1000);

	j2y = nPotJy(analogRead(JOY2Y), 3, 525, 535, 1023, -1000, 1000);
	CPacket.TY = constrain((j2y * trimE) / 100, -1000, 1000);

	j2z = (digitalRead(JOY2FWD) - digitalRead(JOY2BCK));
	CPacket.TZ = constrain((j2z*trimE)*10, -1000, 1000);

	thr = nPotSl(analogRead(THROTTLE), 1023, 3, 5, 0, 1000);
	CPacket.Throttle = constrain((thr*trimE) / 100, 0, 1000);
}

void trimmers()
{
	int trYaw = 0, trPitch = 0, trRoll = 0, trEng = 0;

	static int trYO, trPO, trRO, trEO; //previous values of read variables
	static unsigned long timer; // time when display started
	bool change;

	trYaw = 50 * nPotJy(analogRead(TRIMYAW), 1004, 490, 454, 4, -20, 20);
	trPitch = 50 * nPotJy(analogRead(TRIMPITCH), 1004, 490, 454, 4, -20, 20);
	trRoll = 50 * nPotJy(analogRead(TRIMROLL), 1002, 545, 501, 2, -20, 20);
	trEng = 5 * (nPotSl(analogRead(TRIMENGINE), 1002, 2, 15, 0, 20));

	if ((trYaw < trYO) || (trYaw > trYO))
	{
		trimY = trYaw;
		LCPotDisplay(2, trimY,'y');
		trYO = trYaw;
		timer = millis();
	}

	if ((trPitch < trPO) || (trPitch > trPO))
	{
		trimP = trPitch;
		LCPotDisplay(2, trimP,'p');
		trPO = trPitch;
		timer = millis();
	}

	if ((trRoll < trRO) || (trRoll > trRO))
	{
		trimR = trRoll;
		LCPotDisplay(2, trimR,'r');
		trRO = trRoll;
		timer = millis();
	}

	if ((trEng < trEO) || (trEng > trEO))
	{
		trimE = trEng;
		LCPotDisplay(2, trimE,'e');
		trEO = trEng;
		timer = millis();
	}



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

void CtlUpdate()
{
	byte sasMap[10] = { 9,3,5,7,9,6,4,2,1,10 };
	byte sasVal;
	bool statusRead;

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
	statusRead = (dataIn[3] & B00000001);
	statusLED(9, statusRead);


	FastLED.show();


}

void joysticks()
{
	int j1x, j1y, j1z, j2x, j2y, j2z, thr;
	
	trimmers(); // update trimmers

	j1x = nPotJy(analogRead(JOY1X), 3, 503, 550, 1023, -1000, 1000);
	CPacket.Yaw = constrain(j1x + trimY, -1000, 1000);

	j1y = nPotJy(analogRead(JOY1Y), 3, 510, 540, 1023, -1000, 1000);
	CPacket.Pitch = constrain(j1y + trimP, -1000, 1000);

	j1z = nPotJy(analogRead(JOY1Z), 3, 500, 550, 1023, -1000, 1000);
	CPacket.Roll = constrain(j1z + trimR, -1000, 1000);

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

	AGandCtlUpdate();
	StatusToggles();

}

void AGandCtlUpdate()
{
	byte sasMap[10] = { 9,2,5,7,9,6,4,2,1,10 };
	byte sasVal;

	sasVal = 15 - (dataIn[1] & B00001111);
	setSASMode(sasMap[sasVal]);
	MainControls(STAGE, (dataIn[4] & B00000100));
	MainControls(SAS, (dataIn[1] & B01000000));
}

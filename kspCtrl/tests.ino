void testSuite()
{
	chkKeypad();

}


void testReset()
{
	for (int i = 0; i < 1000; i++)
	{
		Serial.println(i);
		delay(100);
	}

}

void testKeypad()
{
	char key = keymain.getKey();

	if (key != NO_KEY) {
		Serial.println(key);
	}
}

void testMeters()
{
	int thrRead;
	thrRead = analogRead(THROTTLE) / 4;
	Serial.println(thrRead);
	analogWrite(CHARGE, thrRead);
	analogWrite(MONO, thrRead);
	analogWrite(SPEED, thrRead);
	analogWrite(ALT, thrRead);
}

void testJoy()
{
	int Joy1X, Joy1Y, Joy1Z, Joy2X, Joy2Y;
	Joy1X = analogRead(JOY1X);
	Joy1Y = analogRead(JOY1Y);
	Joy1Z = analogRead(JOY1Z);
	Joy2X = analogRead(JOY2X);
	Joy2Y = analogRead(JOY2Y);
	Serial.print("J1X ");
	Serial.print(Joy1X);
	Serial.print(" J1Y ");
	Serial.print(Joy1Y);
	Serial.print(" J1Z ");
	Serial.print(Joy1Z);
	Serial.print(" J1Btn ");
	Serial.println(digitalRead(JOY1BTN));
	Serial.print("J2X ");
	Serial.print(Joy2X);
	Serial.print(" J2Y ");
	Serial.print(Joy2Y);
	Serial.print(" J2F ");
	Serial.print(digitalRead(JOY2FWD));
	Serial.print(" J2B ");
	Serial.print(digitalRead(JOY2BCK));
	Serial.print(" J2Btn ");
	Serial.println(digitalRead(JOY2BTN));


	Serial.println(analogRead(THROTTLE));
	delay(100);
}

void testLCD()
{

	readTime();
	lcd2.setCursor(0, 2);
	lcd2.print("Time: ");
	lcd2.print(hour);
	lcd2.print(":");
	if (minute < 10) lcd2.print("0");
	lcd2.print(minute);
	lcd2.print(":");
	if (second < 10) lcd2.print("0");
	lcd2.print(second);
	if ((hour > 21) && (minute > 44) || (hour > 22)) {
		lcd2.setCursor(5, 3);
		lcd2.print("Bedtime");
	}
}

void killLC()
{
	for (int i = 0; i < NUMLC; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			lc.setChar(i, j, '.', false);
			delay(50);
		}
	}
}

void testLC()
{
	for (int ad = 0; ad < NUMLC; ad++)
	{
		for (int val = 0; val < 10; val++)
		{
			for (int pos = 0; pos < 8; pos++)
			{
				lc.setDigit(ad, pos, val, false);
				delay(50);
				Serial.println(val);
			}
			delay(500);
		}
	}
}

void testTrim()
{
	int trYaw = 0, trPitch = 0, trRoll = 0, trEng = 0;

	static int trYO, trPO, trRO, trEO; //previous values of read variables
	static unsigned long timer; // time when display started
	bool change;

	trYaw = 5*nPotJy(analogRead(TRIMYAW), 1004, 490, 454, 4, -20, 20);
	trPitch = 5 * nPotJy(analogRead(TRIMPITCH), 1004, 490, 454, 4, -20, 20);
	trRoll = 5 * nPotJy(analogRead(TRIMROLL), 1002, 545, 501, 2, -20, 20);
	trEng = 5*(nPotSl(analogRead(TRIMENGINE), 1002, 2, 5, 0, 20));

	Serial.print(trEng);
	Serial.print(" ");
	Serial.println(nPotSl(trEng, 1002, 2, 5, 0, 100));

	if ((trYaw < trYO) || (trYaw > trYO))
	{
		trimY = trYaw;
		LCNum(2, trimY);
		trYO = trYaw;
		timer = millis();
	}

	if ((trPitch < trPO) || (trPitch > trPO))
	{
		trimP = trPitch;
		LCNum(2, trimP);
		trPO = trPitch;
		timer = millis();
	}

	if ((trRoll < trRO) || (trRoll > trRO))
	{
		trimR = trRoll;
		LCNum(2, trimR);
		trRO = trRoll;
		timer = millis();
	}

	if ((trEng < trEO) || (trEng > trEO ))
	{
		trimE = trEng;
		LCNum(2, trimE);
		trEO = trEng;
		timer = millis();
	}



	if (millis() - timer > 1000)
	{
		lc.clearDisplay(2);
	}

	/*Serial.print("Yaw: ");
	Serial.print(trYaw);
	Serial.print("  Pitch: ");
	Serial.print(trPitch);
	Serial.print("  Roll: ");
	Serial.print(trRoll);
	Serial.print("  Engine: ");
	Serial.println(trEng);
	*/

}

void testPin(int p)
{
	digitalWrite(p, HIGH);
	Serial.println('h');
	delay(2000);

	digitalWrite(p, LOW);
	Serial.println('l');
	delay(2000);

}

void testSPI()
{
	bool change = false;
	SPI.beginTransaction(SPISettings(6000000, MSBFIRST, SPI_MODE2));


	//Serial.println('r');
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

	for (int i = 0; i < NUMIC1 + NUMIC2; i++)
	{
		if (dataIn[i] != dataOld[i])
		{
			change = true;
		}
	}
	if (change == true)
	{
		printAllBytes();
	}

	for (int i = 0; i < NUMIC1 + NUMIC2; i++)
	{
		dataOld[i] = dataIn[i];
	}
	delay(1);
}

void testLED()
{
	for (int i = 0; i < NUMLEDS; i++)
	{
		singleLED(i);
		LCNum(1, i);
		delay(1000);
	
	}

}

void testSerial()
{
	float intalt	= 2;
	
	lcd2.clear();
	lcd2.print(VData.AP);
	lcd2.setCursor(10, 0);
	lcd2.print(VData.PE);
	lcd2.setCursor(0, 1);
	lcd2.print(VData.TAp);
	lcd2.setCursor(10, 1);
	lcd2.print(VData.TPe);
}

void testMiscDisplay()
{
	char pstr[8];
	int select;
	select = (dataIn[0] & B00000111);
	//lcd2.clear();
	//lcd2.print(VData.TAp);
	lcd2.setCursor(0, 0);
	lcd2.print(select);
	lcd2.setCursor(0, 1);
	lcd2.print(dataIn[3], BIN);
	
}

void gaugeCalibration()
{
	int var;

	var = analogRead(THROTTLE) / 4;

	LCNum(1, var);
	analogWrite(CHARGE, var);
	analogWrite(MONO, var);
	analogWrite(ALT, var);
	analogWrite(SPEED, var);

}

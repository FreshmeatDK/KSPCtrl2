void testSuite()
{
	testSPI();
	

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
	lcd2.print(g_hour);
	lcd2.print(":");
	if (g_minute < 10) lcd2.print("0");
	lcd2.print(g_minute);
	lcd2.print(":");
	if (g_second < 10) lcd2.print("0");
	lcd2.print(g_second);
	if ((g_hour > 21) && (g_minute > 44) || (g_hour > 22)) {
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

	trimmers();


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
	lcd2.setCursor(0, 0);
	lcd2.print(kVData.acc);
	lcd2.setCursor(0, 1);
	lcd2.print(kVData.warns);
	lcd2.setCursor(10, 1);
	lcd2.print(g_kconnected);
	
}

void testMiscDisplay()
{
	char pstr[8];
	int select;
	select = (dataIn[0] & B00000111);
	//lcd2.clear();
	//lcd2.print(VData.TAp);
	lcd2.setCursor(0, 0);
	//lcd2.print(select);
	lcd2.setCursor(0, 1);
	//lcd2.print(dataIn[3], BIN);
	
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

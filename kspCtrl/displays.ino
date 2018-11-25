void Indicators()
{
	int select;
	float slope;

	printTime();
	
	testMiscDisplay();
	gauges();

	select = (dataIn[0] & B00000111);

	if (select == 3) LCD1Aircraft();
	
	else if (select == 6)
	{
		slope = acos(cos(VData.Pitch*0.01744)*cos(VData.Roll*0.01744))*57.3;
		LCD1Vehicle(slope);
	}
	else LCD1Rocket();
	
	

}

void printTime()
{
	readTime();
	if (hour > 21)
	{
		if (((second % 4) == 0) || ((second % 4) == 1))
		{
			lc.setChar(0, 7, 'b', false);
		}
		else
		{
			lc.setChar(0, 7, ' ', false);
		}
	}
	if (hour > 9)
	{
		int ones = hour % 10;
		hour = hour / 10;
		lc.setDigit(0, 5, hour, false);
		lc.setDigit(0, 4, ones, true);
	}
	else
	{
		lc.setDigit(0, 5, 0, false);
		lc.setDigit(0, 4, hour, true);
	}
	if (minute > 9)
	{
		int ones = minute % 10;
		minute = minute / 10;
		lc.setDigit(0, 3, minute, false);
		lc.setDigit(0, 2, ones, true);
	}
	else
	{
		lc.setDigit(0, 3, 0, false);
		lc.setDigit(0, 2, minute, true);
	}
	if (second > 9)
	{
		int ones = second % 10;
		second = second / 10;
		lc.setDigit(0, 1, second, false);
		lc.setDigit(0, 0, ones, false);
	}
	else
	{
		lc.setDigit(0, 1, 0, false);
		lc.setDigit(0, 0, second, false);
	}
}

void LCD1Rocket()
{
	// Here we assemble the string to send to LCD display 1 and update it
	char lcd1out[81]; //char buffer to send to display 1
	char pstr[8]; //temp string for f2str
	uint32_t period; 

	// line 0 -----------------------------------------------------------------------
	if (VData.AP > 0)
	{
		charcpypos("AP ", 3, lcd1out, 0);
		f2str(VData.AP, 4, pstr);
		charcpypos(pstr, 6, lcd1out, 3);
		charcpypos("m T", 3, lcd1out, 9);

		if (VData.TAp > VData.TPe)
		{
			charcpypos("p ", 2, lcd1out, 12);
			time2str(VData.TPe, pstr);
			charcpypos(pstr, 6, lcd1out, 14);
		}
		else
		{
			charcpypos("a ", 2, lcd1out, 12);
			time2str(VData.TAp, pstr);
			charcpypos(pstr, 6, lcd1out, 14);
		}
	}
	else
	{
		charcpypos("Escape Trajectory   ", 20, lcd1out, 0);
	}

	// line 2 -----------------------------------------------------------------------
	if (VData.VVI > 20 || VData.RAlt > 10000)
	{
		charcpypos("VO ", 3, lcd1out, 20);
		f2str(VData.VOrbit, 4, pstr);
		charcpypos(pstr, 6, lcd1out, 23);
		charcpypos("% AM ", 5, lcd1out, 29);
		f2str(VData.Alt, 3, pstr);
		charcpypos(pstr, 5, lcd1out, 34);
		charcpypos("m", 1, lcd1out, 39);
	}
	else
	{
		charcpypos("VS ", 3, lcd1out, 20);
		f2str(VData.Vsurf, 4, pstr);
		charcpypos(pstr, 6, lcd1out, 23);
		charcpypos("% AS ", 5, lcd1out, 29);
		f2str(VData.RAlt, 3, pstr);
		charcpypos(pstr, 5, lcd1out, 34);
		charcpypos("m", 1, lcd1out, 39);
	}

	// line 1 -----------------------------------------------------------------------
	if (VData.PE > 0)
	{
		charcpypos("PE ", 3, lcd1out, 40);
		f2str(VData.PE, 4, pstr);
		charcpypos(pstr, 6, lcd1out, 43);
		charcpypos("m P  ", 5, lcd1out, 49);
		period = abs(VData.TPe - VData.TAp)*2;
		time2str(period, pstr);
		charcpypos(pstr, 6, lcd1out, 54);
	}
	else
	{
		charcpypos("Impact Trajectory   ", 20, lcd1out, 40);
	}



	lcd.setCursor(0, 0);
	lcd.print(lcd1out);	

}

void LCD1Aircraft() 
{
	// Here we assemble the string to send to LCD display 1 and update it

	char lcd1out[81]; //char buffer to send to display 1
	char pstr[8]; //temp string for f2str
	float deviation, absDev, approach, lon;
	// ------------------------------ Line 1
	charcpypos("IAS ", 4, lcd1out, 0);
	f2str(VData.IAS, 4, pstr);
	charcpypos(pstr, 6, lcd1out, 4);
	charcpypos("m/s M", 5, lcd1out, 10);
	f2str(VData.MachNumber, 3, pstr);
	charcpypos(pstr, 5, lcd1out, 15);

	// ------------------------------- Line 3

	deviation = (VData.Lat + 0.0486) * 10472;   //  Runway ILS, assumes flat earth
	if (deviation < 0) absDev = -deviation;
	lon = VData.Lon;
	if (lon < 160) lon = lon + 360;
	approach = abs((VData.Lon + 74.72408) * 10472);

	charcpypos("DD ", 3, lcd1out, 20);
	f2str(approach, 3, pstr);
	charcpypos(pstr, 5, lcd1out, 23);
	charcpypos("m / ", 4, lcd1out, 28);
	if (deviation < 0) {
		f2str(absDev, 3, pstr);
		charcpypos(pstr, 5, lcd1out, 32);
		charcpypos("m S", 3, lcd1out, 37);
	}
	else {
		f2str(deviation, 3, pstr);
		charcpypos(pstr, 5, lcd1out, 32);
		charcpypos("m N", 3, lcd1out, 37);
	}

	// ----------------------------- Line 2

	charcpypos("Alt ", 4, lcd1out, 40);
	f2str(VData.Alt, 4, pstr);
	charcpypos(pstr, 6, lcd1out, 44);
	charcpypos("m /", 3, lcd1out, 50);
	f2str(VData.RAlt, 4, pstr);
	charcpypos(pstr, 6, lcd1out, 53);
	charcpypos("m", 3, lcd1out, 59);


	lcd.setCursor(0, 0);
	lcd.print(lcd1out);

}

void LCD1Vehicle(float slope) 
{
	char pstr[8];

	lcd.setCursor(0, 0);
	lcd.print("Hdn:");
	lcd.print("    ");
	lcd.setCursor(4, 0);
	lcd.print(VData.Heading, 0);
	lcd.setCursor(8, 0);
	lcd.print("Alt:");
	lcd.print("        ");
	lcd.setCursor(12, 0);
	lcd.print(VData.Alt, 0);
	lcd.print("m");
	lcd.setCursor(0, 1);
	lcd.print("inc/slope: ");
	lcd.print("          ");
	lcd.setCursor(11, 1);
	lcd.print(VData.Pitch, 0);
	lcd.print("/");
	lcd.print(slope, 0);

	lcd.setCursor(0, 2);
	lcd.print("Lat: ");
	lcd.print("           ");
	lcd.setCursor(5, 2);
	lcd.print(VData.Lat, 5);
	lcd.setCursor(0, 3);
	lcd.print("Lon: ");
	lcd.print("           ");
	lcd.setCursor(5, 3);
	lcd.print(VData.Lon, 5);

}


void gauges()
{
	int alt, speed, mono, charge;
	byte altp, speedp, monop, chargep; //PWM output value

	alt = (int)VData.RAlt;
	if (dataIn[4] & B00100000) alt = alt / 10; // 3 or 30 km range

	if (alt > 3000) alt = 3000; //max out

	if (alt > 1000) altp = 0.031*alt + 31.5;
	else if (alt > 500) altp = 0.034*alt + 28.5;
	else if (alt > 100) altp = 0.0475*alt + 21.75;
	else if (alt > 50) altp = 0.08*alt + 18.5;
	else if (alt > 20) altp = 0.167*alt + 14.2;
	else if (alt > 10) altp = 0.4*alt + 9.5;
	else altp = 1.3*alt + 0.5;

	analogWrite(ALT, altp);

	//--------------------------------

	speed = (int)VData.VVI;

	if (speed > 100) speed = 100; // max out

	if (speed > 50) speedp = 0.32 * speed + 73.5;
	else if (speed > 20) speedp = 0.467 * speed + 66.2;
	else if (speed > 10) speedp = 1.1 * speed + 53.5;
	else if (speed > 5) speedp = 1.6 * speed + 73.5;
	else if (speed > 2) speedp = 3.7 * speed + 38.2;
	else if (speed > -2) speedp = 3 * speed + 39.5;
	else if (speed > -5) speedp = 2 * speed + 37.5;
	else if (speed > -20) speedp = 0.8 * speed + 31.5;
	else if (speed > -50) speedp = 0.2 * speed + 19.5;
	else if (speed > -101) speedp = 0.18 * speed + 18.5;
	else speedp = 0;

	analogWrite(SPEED, speedp);

	//--------------------------------

	charge = (int)(100 * VData.ECharge / VData.EChargeTot + 0.5);
	
	if (charge > 100) charge = 100; //should not happen

	if (charge > 90) chargep = 1.8*charge - 77.5;
	else if (charge > 80) chargep = 1.5*charge - 50.5;
	else if (charge > 70) chargep = 1.1*charge - 18.5;
	else if (charge > 60) chargep = 1*charge - 11.5;
	else if (charge > 50) chargep = 0.9*charge - 5.5;
	else if (charge > 40) chargep = 0.6*charge + 9.5;
	else if (charge > 30) chargep = 0.8*charge + 1.5;
	else if (charge > 20) chargep = 0.7*charge + 4.5;
	else chargep = 0.9*charge + 0.5;

	analogWrite(CHARGE, chargep);

	//--------------------------------

	mono = (int)(100 * VData.MonoProp / VData.MonoPropTot + 0.5);

	if (mono > 100) mono = 100; //should not happen

	if (mono > 90) monop = 2.2*mono - 107.5;
	else if (mono > 80) monop = 1.5*mono - 44.5;
	else if (mono > 70) monop = 1.2*mono - 20.5;
	else if (mono > 60) monop = 1 * mono - 6.5;
	else if (mono > 40) monop = 0.8*mono - 5.5;
	else if (mono > 20) monop = 0.7*mono + 9.5;
	else if (mono > 10) monop = 1 * mono + 3.5;
	else monop = 1.3*mono + 0.5;

	analogWrite(MONO, monop);
}
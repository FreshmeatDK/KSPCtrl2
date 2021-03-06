int nPotJy(int pot, int min, int deadmin, int deadmax, int max, int low, int high)
{
	int retval = 0;
	
	if (max < min)
	{
		pot = min - pot;
		deadmin = min - deadmin;
		deadmax = min - deadmax;
		max = min - max;
		min = 0;
	}

	if (pot < deadmin)
	{
		retval = low-(pot * low/(deadmin - min));
	}
	else if (pot > deadmax)
	{
		retval = (pot - deadmax)*high/(max - deadmax);
	}
	else
	{
		retval = 0;
	}

	return retval;
}

int nPotSl(int pot, int min, int max, int dead, int low, int high)
{
	int retval = 0;

	if (min > max)
	{
		int tmp = min;
		min = max;
		max = tmp;
		pot = max - pot;
	}

	if (pot < dead)
	{
		retval = low;
	}

	if (pot > max - dead)
	{
		retval = high;
	}

	else
	{
		retval = (pot - dead)*(high - low) / (max - min);
	}

	return retval;

}

void blackout()
{
	lcd.noBacklight();
	lcd2.noBacklight();
	lcd.clear();
	lcd2.clear();

	analogWrite(CHARGE, 0);
	analogWrite(MONO, 0);
	analogWrite(ALT, 0);
	analogWrite(SPEED, 0);
	
	for (int i = 0; i < NUMLEDS; i++)
	{
		oldLeds[i] = leds[i];
		leds[i] = CRGB::Black;
	}
	FastLED.show();
}

bool reLight()
{
	lcd.backlight();
	lcd2.backlight();
	for (int i = 0; i < NUMLEDS; i++)
	{
		leds[i] = oldLeds[i];
	}
	FastLED.show();
}

void printByte(byte data)
{
	char bit[8];
	for (int i = 7; i > -1; i--)
	{
		bit[i] = bitRead(data, i);
		Serial.print(bitRead(data, i));
		if (i == 4) Serial.print('|');
	}
}

void printAllBytes()
{
	for (int i = 0; i < NUMIC1 + NUMIC2; i++)
	{
		printByte(dataIn[i]);
		Serial.print(" | | ");
	}
	Serial.println();

}

void rotSelectors()
{
	/*Detects status of rotary selectors for SAS mode, cam mode and panel mode*/

	word SASMode = 0, camMode = 0, panelMode = 0;

	SASMode = dataIn[1] >> 4;
	camMode = dataIn[0] & B00000111;
	panelMode = (dataIn[0] >> 3) & B00000111;

	Serial.print(SASMode);
	Serial.print(' ');
	Serial.print(camMode);
	Serial.print(' ');
	Serial.print(panelMode);
	Serial.println();
}

void singleLED(int lednum)
{
	//Serial.println(lednum);
	for (int i = 0; i < NUMLEDS; i++)
	{
		if (i != lednum) leds[i] = CRGB::Black;
	}
	leds[lednum] = CRGB::Green;
	FastLED.show();
}

void statusLED(int lednum, bool status)
{
	if (status) leds[lednum] = 0x001100; //green
	else leds[lednum] = 0x110000; //red
}

byte warnLvl(byte qty, byte t1, byte t2, byte t3) 
{
	byte wlevel =0;
	// compares the quantity qty against warning thresholds t1 -t3 and returns alarm level
	// goes from high to low, remember to invert high level warnings

	if (qty <= t1) wlevel = 1;
	if (qty <= t2) wlevel = 2;
	if (qty <= t3) wlevel = 3;

	return wlevel;
}


void warnLedSet(uint8_t lednum, uint8_t level)
{
	//colors led according to warnlevel
	// use g_second%2 to create blink

	if (level == 0)
	{
		leds[lednum] = 0x000000;
	}

	if (level == 1) 
	{
		leds[lednum] = 0x100800; //yellow
	}

	else if (level == 2)
	{
		leds[lednum] = 0x110000; //red
	}

	else if (level == 3)
	{
		if (g_second%2 == 0) leds[lednum] = 0x110000; //red blink
		else leds[lednum] = 0x030000;
	}
	
	else if (level == 4) //safe, no two bit value gets this level. Use for parachute
		{
			leds[lednum] = 0x001100; // green
		}

}

byte reqAccPct(int a) {   //returns the required percentage of acceleration
							//to stop at target. a = acceleration * 100
	uint16_t aReqPct;
	float g;

	if (getNavballMode() == 3) { // navball mode target
		if (VData.TargetDist < 3000) {
			aReqPct = (uint16_t)50000*VData.TargetV*VData.TargetV / (VData.TargetDist*a); //a=v^2/2s *100 (pct)*50 (cm/s) 
			if (aReqPct > 255) aReqPct = 255;
		}

	}
	else if ((getNavballMode() == 2) && ((dataIn[3] & B0000111)== B100) && (VData.VVI < 10) && (VData.RAlt < 3000)) 
	{ // rocket in surface mode going down
		g = g_SOIBody(kVData.SOINumber);
		if (g != -1) aReqPct = (uint16_t)10000 * ((VData.VVI*VData.VVI / (2*VData.RAlt) - g )/a );
		else aReqPct = 0;
		if (aReqPct > 255) aReqPct = 255;
	}
	else aReqPct = 0;
	return aReqPct;
}

void execCmd(char cmdStrL[], byte cmdStrIndexL) 
{
	char action[2];
	char value[18];
	char tmpval[8];
	int actionN;

	if (cmdStrL[2] == '*') {                      //if the string is of format xx*, int action = xx
		for (int i = 0; i < 2; i++) {
			action[i] = cmdStrL[i];
		}
		action[2] = '\0';
		for (int i = 3; i < cmdStrIndexL; i++) {   // int value is the value of rest of string
			value[i - 3] = cmdStrL[i];
		}
		value[cmdStrIndexL - 3] = '\0';

		actionN = atoi(action);
		switch (actionN) 
		{
			case 20:
			{
				g_autoland = true;
				break;
			}

			case 91: // set time yy mm dd w hh mm ss
			{
				for (int i = 0; i < 2; i++) {
					tmpval[i] = value[i];
				}
				tmpval[2] = '\0';
				g_year = (byte)atoi(tmpval);
				for (int i = 0; i < 2; i++) {
					tmpval[i] = value[i + 2];
				}
				tmpval[2] = '\0';
				g_month = (byte)atoi(tmpval);
				for (int i = 0; i < 2; i++) {
					tmpval[i] = value[i + 4];
				}
				tmpval[2] = '\0';
				g_dayOfMonth = (byte)atoi(tmpval);
				for (int i = 0; i < 1; i++) {
					tmpval[i] = value[i + 6];
				}
				tmpval[1] = '\0';
				g_dayOfWeek = (byte)atoi(tmpval);
				for (int i = 0; i < 2; i++) {
					tmpval[i] = value[i + 7];
				}
				tmpval[2] = '\0';
				g_hour = (byte)atoi(tmpval);
				for (int i = 0; i < 2; i++) {
					tmpval[i] = value[i + 9];
				}
				tmpval[2] = '\0';
				g_minute = (byte)atoi(tmpval);
				for (int i = 0; i < 2; i++) {
					tmpval[i] = value[i + 11];
				}
				tmpval[2] = '\0';
				g_second = (byte)atoi(tmpval);
				setTime(g_second, g_minute, g_hour, g_dayOfWeek, g_dayOfMonth, g_month, g_year);
				break; }
			case 92: { // clear indicators, todo clear alarms
				lcd.clear();
				lcd2.clear();
				break;
			}

		}


	}
}

void LCPotDisplay(int addr, int16_t num, char pot)
{
	LCNum(addr, num);
	LCChar(addr, 7, pot);
}

void LCChar(int addr, byte pos, char ch)
{
	switch (ch)
	{
		case 'y':
			lc.setRow(addr, pos, B00111011);
			break;
		case 'r':
			lc.setRow(addr, pos, B00000101);
			break;
		case 'p':
			lc.setChar(addr, pos, 'P', false);
			break;
		case 'e':
			lc.setChar(addr, pos, 'E', false);
			break;
		case 'd':
			lc.setChar(addr, pos, 'd', false);
			break;
		case 'h':
			lc.setRow(addr, pos, B00010111);
			break;
		case 'n':
			lc.setRow(addr, pos, B00010101);
			break;
		case 's':
			lc.setRow(addr, pos, B00011001);
			break;

		default:
			lc.setRow(addr, pos, B01001001);
			break;
	}
	
}

void LCNum(int add, int16_t num)
{
	int pos;
	int dig[5];
	bool neg = false;
	bool first = false;
	if (num < 0)
	{
		neg = true;
		num = -1 * num;
	}
	for (char i = 0; i < 5; i++)
	{
		dig[i] = num % 10;
		num = num / 10;
	}
	lc.clearDisplay(add);

	for (int i = 4; i > -1; i--)
	{
		if ((dig[i] == 0) && ((first == true) || (i == 0)))
		{
			lc.setDigit(add, i, dig[i], false);
		}

		if ((dig[i] != 0) && (first == false))
		{
			if (neg == true)
			{
				lc.setChar(add, i + 1, '-', false);
			}

			first = true;
		}
		if ((dig[i] != 0) && (first == true))
		{
			lc.setDigit(add, i, dig[i], false);
		}


	}

}

void LCKerbYDtime(uint32_t time, byte addr)
{
	byte y100, y10, y1;
	byte dayDig[3];
	uint16_t days, years;
	bool firstDigWritten = false;
	
	years = time / 9203545;
	time = time % 9203545;
	days = time / 21600;
	time = time % 21600;

	y1 = years % 10;
	years = years / 10;
	y10 = years % 10;
	y100 = years / 10;

	dayDig[0] = days % 10; //ones
	days = days / 10;
	dayDig[1] = days % 10; //tens
	dayDig[2] = days / 10; //hundreds

	

	lc.clearDisplay(addr);

	if (y100 != 0)
	{
		lc.setDigit(addr, 7, y100, false);
		firstDigWritten = true;
	}
	

	if ((firstDigWritten = true) || (y10 != 0))
	{
		lc.setDigit(addr, 6, y10, false);
		firstDigWritten = true;
	}

	lc.setDigit(addr, 5, y1, false);
	
	LCChar(addr, 4, 'y');
	
	for (int i = 3; i > 0; i--)
	{
		lc.setDigit(addr, i, dayDig[i], false);
	}
	LCChar(addr, 0, 'h');
}

void LCKerbDHMtime(uint32_t time, byte addr)
{
	uint16_t days;
	byte hours, minutes;
	byte dayDig[3];
	byte minDig[2];
	bool firstDigWritten = false;

	days = time / 21600;
	time = time % 21600;
	hours = time / 3600;
	time = time % 3600;
	minutes = time / 60;

	dayDig[0] = days % 10; //ones
	days = days / 10;
	dayDig[1] = days % 10; //tens
	dayDig[2] = days / 10; //hundreds

	minDig[0] = minutes % 10; //ones
	minDig[1] = minutes / 10; // tens


	if (dayDig[2] != 0)
	{
		lc.setDigit(addr, 7, dayDig[2], false);
		firstDigWritten = true;
	}
	if ((dayDig[1] != 0) || (firstDigWritten == true))
	{
		lc.setDigit(addr, 6, dayDig[1], false);
		firstDigWritten = true;
	}
	lc.setDigit(addr, 5, dayDig[0], false);
	lc.setChar(addr, 4, 'd', false);
	lc.setDigit(addr, 3, hours, false);
	LCChar(addr, 2, 'h');
	lc.setDigit(addr, 1, minDig[1], false);
	lc.setDigit(addr, 0, minDig[0], false);
}

void LCKerbHMStime(uint32_t time, byte addr)
{
	byte hours, minutes, seconds;
	byte secDig[2];
	byte minDig[2];
	bool firstDigWritten = false;

	hours = time / 3600;
	time = time % 3600;
	minutes = time / 60;
	seconds = time % 60;

	minDig[0] = minutes % 10; //ones
	minDig[1] = minutes / 10; // tens

	secDig[0] = seconds % 10;
	secDig[1] = seconds / 10;

	
	lc.setDigit(addr, 7, hours, false);
	LCChar(addr, 6, 'h');
	lc.setDigit(addr, 5, minDig[1], false);
	lc.setDigit(addr, 4, minDig[0], false);
	LCChar(addr, 3, 'n');
	lc.setDigit(addr, 2, secDig[1], false);
	lc.setDigit(addr, 1, secDig[0], false);
	LCChar(addr, 0, 's');
}

void setTime(byte ssecond, byte sminute, byte shour, byte sdayOfWeek, byte sdayOfMonth, byte smonth, byte syear)
{
	// sets time and date data to DS3231
	Wire.beginTransmission(RTCADR);
	Wire.write(0); // set next input to start at the seconds register
	Wire.write(decToBcd(ssecond)); // set seconds
	Wire.write(decToBcd(sminute)); // set minutes
	Wire.write(decToBcd(shour)); // set hours
	Wire.write(decToBcd(sdayOfWeek)); // set day of week (1=Sunday, 7=Saturday)
	Wire.write(decToBcd(sdayOfMonth)); // set date (1 to 31)
	Wire.write(decToBcd(smonth)); // set month
	Wire.write(decToBcd(syear)); // set year (0 to 99)
	Wire.endTransmission();
}

void readTime()
{
	Wire.beginTransmission(RTCADR);
	Wire.write(0); // set DS3231 register pointer to 00h
	Wire.endTransmission();
	Wire.requestFrom(RTCADR, 3);
	// request three bytes of data from DS3231 starting from register 00h
	g_second = bcdToDec(Wire.read() & 0x7f);
	g_minute = bcdToDec(Wire.read());
	g_hour = bcdToDec(Wire.read() & 0x3f);
	/*Serial.print(g_hour);
	Serial.print(':');
	Serial.print(g_minute);
	Serial.print(':');
	Serial.println(g_second);
	Serial.println();*/
	//g_dayOfWeek = bcdToDec(Wire.read());
	//g_dayOfMonth = bcdToDec(Wire.read());
	//g_month = bcdToDec(Wire.read());
	//g_year = bcdToDec(Wire.read());
}

byte decToBcd(byte val)
{
	return((val / 10 * 16) + (val % 10));
}
// Convert binary coded decimal to normal decimal numbers
byte bcdToDec(byte val)
{
	return((val / 16 * 10) + (val % 16));
}

void charcpypos(char *in, uint8_t len, char *out, uint8_t pos) {

	// copy chars from in to out, starting at pos

	if (strlen(in) >= len)
	{
		for (int i = 0; i < len + 1; i++) {
			out[pos + i] = in[i];
		}
	}

}

void f2str(float fval, int BC, char *out) {

	/*

	This function converts the float in fval to a string
	in out, containing a rounded result to BC significant
	digits, and an SI-prefix to account for large nnumbers.
	The function will not work generally with negative numbers,
	nor positive values < 1

	fval: value to convert to string, using prefixes
	BC: significant digits
	out: char array. Size at least BC+3

	out will have BC+2 printing chars and a null char.
	*/
	uint32_t pbase = 1UL; //Will be 10^(BC+1), float must have a smalle value than this 
						  // once we have found our prefix. Convert to float if 10^12 values are needed
	
	byte deca = 0; // index variable for prefix
	byte deci; // number of decimals
	char prefix[] = { ' ', 'k','M','G','T' };

	if (BC < 1) out = "error: BC < 1";

	for (int i = 0; i < BC; i++) { //pbase = 10^antal betydende cifre
		pbase = pbase * 10;
	}


	if (fval < 0) {

		if (BC < 2) {
			out[0] = 'e';
			for (int i = 1; i < BC + 2; i++) {
				out[i] = ' ';
			}
		}
		else {
			out[0] = 'e';
			out[1] = 's';
			out[2] = 'c';
			for (int i = 3; i < BC + 2; i++) {
				out[i] = ' ';
			}
		}

		out[BC + 2] = '\0';
	}
	else {
		
		while (fval >= pbase) 
		{
			fval = fval * 0.001;
			deca++;
		}

		if (fval > pbase*0.1) deci = 0;
		else if (fval > pbase*0.01) deci = 1;
		else if (fval > pbase*0.001) deci = 2;
		else deci = 3;

		//if (deci == 2) BC++;

		dtostrf(fval, BC + 1, deci, out);
		out[BC + 1] = prefix[deca];
		out[BC + 2] = '\0';


	}
}

void time2str(uint32_t ltime, char *out) {
	/*
	This function formats a time to a six char format,
	i.e. 00h00m, 0y000d, 00m00s, to get a constant amount
	of significant digits. Breaks above 99 years. Null terminated.
	*/
	int ty, td, th, tm, ts;
	char buf[5];

	ty = ltime / 9203400;
	td = (ltime % 9203400) / (21600);
	th = (ltime % 21600) / 3600;
	tm = (ltime % 3600) / 60;
	ts = (ltime % 60);

	if (ltime < 0) {
		out = "esc trj";
	}
	else {
		if (ty > 9) {
			itoa(ty, buf, 10);
			out[0] = buf[0];
			out[1] = buf[1];
			out[2] = 'Y';
			out[3] = '\0';
		}

		//------------------Y:ddd---------------------

		else if (ty > 0) {
			itoa(ty, buf, 10);
			out[0] = buf[0];
			out[1] = 'Y';

			if (td > 99) {
				itoa(td, buf, 10);
				out[2] = buf[0];
				out[3] = buf[1];
				out[4] = buf[2];
			}
			else if (td > 9) {
				itoa(td, buf, 10);
				out[2] = '0';
				out[3] = buf[0];
				out[4] = buf[1];
			}
			else {
				itoa(td, buf, 10);
				out[2] = '0';
				out[3] = '0';
				out[4] = buf[0];
			}
			out[5] = 'd';
			out[6] = '\0';
		}

		//---------------------ddd:h----------------

		else if (td > 0) {
			if (td > 99) {
				itoa(td, buf, 10);
				out[0] = buf[0];
				out[1] = buf[1];
				out[2] = buf[2];
			}
			else if (td > 9) {
				itoa(td, buf, 10);
				out[0] = '0';
				out[1] = buf[0];
				out[2] = buf[1];
			}
			else {
				itoa(td, buf, 10);
				out[0] = '0';
				out[1] = '0';
				out[2] = buf[0];
			}
			out[3] = 'D';
			itoa(th, buf, 10);
			out[4] = buf[0];
			out[5] = 'h';
			out[6] = '\0';

		}
		//---------------------------hh:mm-----------------    
		else if (th > 0) {
			if (th > 9) {
				itoa(th, buf, 10);
				out[0] = buf[0];
				out[1] = buf[1];
			}
			else {
				itoa(th, buf, 10);
				out[0] = '0';
				out[1] = buf[0];
			}
			out[2] = 'h';
			if (tm > 9) {
				itoa(tm, buf, 10);
				out[3] = buf[0];
				out[4] = buf[1];
			}
			else {
				itoa(tm, buf, 10);
				out[3] = '0';
				out[4] = buf[0];
			}
			out[5] = 'm';
			out[6] = '\0';
		}

		//--------------------mm:ss---------

		else {
			if (tm > 9) {
				itoa(tm, buf, 10);
				out[0] = buf[0];
				out[1] = buf[1];
			}
			else {
				itoa(tm, buf, 10);
				out[0] = '0';
				out[1] = buf[0];
			}
			out[2] = 'm';
			if (ts > 9) {
				itoa(ts, buf, 10);
				out[3] = buf[0];
				out[4] = buf[1];
			}
			else {
				itoa(ts, buf, 10);
				out[3] = '0';
				out[4] = buf[0];
			}
			out[5] = 's';
			out[6] = '\0';
		}

	}

}

void LEDSAllOff() {
	for (int i = 0; i < 8; i++) {
		lc.setRow(0, i, 0);
	}

}

void InitTxPackets() {
	HPacket.id = 0;
	CPacket.id = 101;
}

float dVHohmann(float ap) { // calculates dV to circularize at apsis
	float dV, r, sgp;
	r = r_SOIBody(kVData.SOINumber);
	sgp = sgp_SOIBody(kVData.SOINumber);
	if ((r - ap) > 0 && (r != -1)) 	dV = sqrt(sgp / (ap + r)) - sqrt(sgp*(2 / (ap + r) - 1 / VData.SemiMajorAxis));
	else dV = -1;
	return dV;
}

float r_SOIBody(byte SOI) {
	float r;
	switch (SOI)
	{
	case 100: //kerbol
		r = 261600000;
		break;
	case 110: // moho
		r = 250000;
		break;
	case 120: // eve
		r = 700000;
		break;
	case 121: // gilly
		r = 13000;
		break;
	case 130: // kerbin
		r = 600000;
		break;
	case 131: // mun
		r = 200000;
		break;
	case 132: // minmus
		r = 60000;
		break;
	case 140: // duna
		r = 320000;
		break;
	case 141: // ike
		r = 130000;
		break;
	case 150: //"dres":
		r = 138000;
		break;
	case 160: //"jool":
		r = 6000000;
		break;
	case 161: //"laythe":
		r = 500000;
		break;
	case 162: //"vall":
		r = 300000;
		break;
	case 163: //"tylo":
		r = 600000;
		break;
	case 164: //"bop":
		r = 65000;
		break;
	case 165: //"pol":
		r = 44000;
		break;
	case 170: //"eeloo":
		r = 210000;
		break;

	case 200: //Ciro
		r = 70980000;
		break;
	case 210: // Icarus
		r = 160000;
		break;
	case 220: // Thalia
		r = 270000;
		break;
	case 221: // Eta
		r = 60000;
		break;
	case 230: // Niven
		r = 400000;
		break;
	case 240: // Gael
		r = 600000;
		break;
	case 241: // Iota
		r = 100000;
		break;
	case 242: // Ceti
		r = 150000;
		break;
	case 250: // Tellumo
		r = 1000000;
		break;
	case 251: //"Lili":
		r = 7000;
		break;
	case 260: //"Gratian":
		r = 550000;
		break;
	case 261: //"Geminus":
		r = 230000;
		break;
	case 270: //"Otho":
		r = 3500000;
		break;
	case 271: //"Augustus":
		r = 350000;
		break;
	case 272: //"Hephaestus":
		r = 125000;
		break;
	case 273: //"Jannah":
		r = 105000;
		break;
	case 280: //"Gauss":
		r = 2500000;
		break;
	case 281: // Loki
		r = 180000;
		break;
	case 282: // Catullus
		r = 1200000;
		break;
	case 283: // Tarsiss
		r = 320000;
		break;
	case 290: // Nero
		r = 5000000;
		break;
	case 291: // Hadrian
		r = 300000;
		break;
	case 292: // Narisse
		r = 90000;
		break;
	case 293: //"Muse":
		r = 130000;
		break;
	case 294: //"Minona":
		r = 120000;
		break;
	case 295: //"Agrippina":
		r = 50000;
		break;
	case 296: //"Julia":
		r = 30000;
		break;
	case 310: //"Hox":
		r = 250000;
		break;
	case 311: //"Argo":
		r = 80000;
		break;
	case 320: //"Leto":
		r = 210000;
	case 400: //"Grannus":
		r = 3017000;
		break;

	default:
		r = -1;
		break;
	}
	return r;
}

float sgp_SOIBody(byte SOI) {
	float sgp;
	switch (SOI)
	{
	case 100: //kerbol
		sgp = 1172332800000000000;
		break;
	case 110: // moho
		sgp = 168609380000;
		break;
	case 120: // eve
		sgp = 8171730200000;
		break;
	case 121: // gilly
		sgp = 8289449.8;
		break;
	case 130: // kerbin
		sgp = 3531600000000;
		break;
	case 131: // mun
		sgp = 65138398000;
		break;
	case 132: // minmus
		sgp = 1765800000;
		break;
	case 140: // duna
		sgp = 301363210000;
		break;
	case 141: // ike
		sgp = 18568369000;
		break;
	case 150: //"dres":
		sgp = 21484489000;
		break;
	case 160: //"jool":
		sgp = 282528000000000;
		break;
	case 161: //"laythe":
		sgp = 1962000000000;
		break;
	case 162: //"vall":
		sgp = 207481500000;
		break;
	case 163: //"tylo":
		sgp = 2825280000000;
		break;
	case 164: //"bop":
		sgp = 2486834900;
		break;
	case 165: //"pol":
		sgp = 721702080;
		break;
	case 170: //"eeloo":
		sgp = 74410815000;
		break;

	case 200: //Ciro
		sgp = 1274712872715830000;
		break;
	case 210: // Icarus
		sgp = 40168038400;
		break;
	case 220: // Thalia
		sgp = 214471435500;
		break;
	case 221: // Eta
		sgp = 1765197000;
		break;
	case 230: // Niven
		sgp = 784532000000;
		break;
	case 240: // Gael
		sgp = 3530394000000;
		break;
	case 241: // Iota
		sgp = 8335652500;
		break;
	case 242: // Ceti
		sgp = 29787699375;
		break;
	case 250: // Tellumo
		sgp = 18632635000000;
		break;
	case 251: //"Lili":
		sgp = 7207888;
		break;
	case 260: //"Gratian":
		sgp = 2224883718750;
		break;
	case 261: //"Geminus":
		sgp = 114129792700;
		break;
	case 270: //"Otho":
		sgp = 110520945500000;
		break;
	case 271: //"Augustus":
		sgp = 420460118750;
		break;
	case 272: //"Hephaestus":
		sgp = 12258312500;
		break;
	case 273: //"Jannah":
		sgp = 7027690556;
		break;
	case 280: //"Gauss":
		sgp = 63130309375000;
		break;
	case 281: // Loki
		sgp = 31773546000;
		break;
	case 282: // Catullus
		sgp = 12709418400000;
		break;
	case 283: // Tarsiss
		sgp = 170714163200;
		break;
	case 290: // Nero
		sgp = 237811262500000;
		break;
	case 291: // Hadrian
		sgp = 158867730000;
		break;
	case 292: // Narisse
		sgp = 3177354600;
		break;
	case 293: //"Muse":
		sgp = 13258590800;
		break;
	case 294: //"Minona":
		sgp = 8472945600;
		break;
	case 295: //"Agrippina":
		sgp = 735498750;
		break;
	case 296: //"Julia":
		sgp = 132389775;
		break;
	case 310: //"Hox":
		sgp = 85808187500;
		break;
	case 311: //"Argo":
		sgp = 2196689600;
		break;
	case 320: //"Leto":
		sgp = 51896791800;
	case 400: //"Grannus":
		sgp = 6373375516920090;
		break;

	default:
		sgp = -1;
		break;


	}
	return sgp;
}

float g_SOIBody(byte SOI)
{
	float g;
	switch (SOI)
	{
	case 100: //kerbol
		g = 17.13;
		break;
	case 110: // moho
		g = 2.7;
		break;
	case 120: // eve
		g = 16.68;
		break;
	case 121: // gilly
		g = 0.005;
		break;
	case 130: // kerbin
		g = 9.81;
		break;
	case 131: // mun
		g = 1.63;
		break;
	case 132: // minmus
		g = 0.49;
		break;
	case 140: // duna
		g = 2.94;
		break;
	case 141: // ike
		g = 1.1;
		break;
	case 150: //"dres":
		g = 1.13;
		break;
	case 160: //"jool":
		g = 7.85;
		break;
	case 161: //"laythe":
		g = 7.85;
		break;
	case 162: //"vall":
		g = 2.31;
		break;
	case 163: //"tylo":
		g = 7.85;
		break;
	case 164: //"bop":
		g = 0.59;
		break;
	case 165: //"pol":
		g = 0.37;
		break;
	case 170: //"eeloo":
		g = 1.69;
		break;

	case 200: //Ciro
		g = 253.01157;
		break;
	case 210: // Icarus
		g = 1.569064;
		break;
	case 220: // Thalia
		g = 2.941995;
		break;
	case 221: // Eta
		g = 0.4903325;
		break;
	case 230: // Niven
		g = 4.903325;
		break;
	case 240: // Gael
		g = 9.80665;
		break;
	case 241: // Iota
		g = 0.83356525;
		break;
	case 242: // Ceti
		g = 1.32389775;
		break;
	case 250: // Tellumo
		g = 18.632635;
		break;
	case 251: //"Lili":
		g = 0.14709975;
		break;
	case 260: //"Gratian":
		g = 7.3549875;
		break;
	case 261: //"Geminus":
		g = 2.157463;
		break;
	case 270: //"Otho":
		g = 9.022118;
		break;
	case 271: //"Augustus":
		g = 3.4323275;
		break;
	case 272: //"Hephaestus":
		g = 0.784532;
		break;
	case 273: //"Jannah":
		g = 0.63743225;
		break;
	case 280: //"Gauss":
		g = 10.1008495;
		break;
	case 281: // Loki
		g = 0.980665;
		break;
	case 282: // Catullus
		g = 8.825985;
		break;
	case 283: // Tarsiss
		g = 1.6671305;
		break;
	case 290: // Nero
		g = 9.5124505;
		break;
	case 291: // Hadrian
		g = 1.765197;
		break;
	case 292: // Narisse
		g = 0.392266;
		break;
	case 293: //"Muse":
		g = 0.784532;
		break;
	case 294: //"Minona":
		g = 0.588399;
		break;
	case 295: //"Agrippina":
		g = 0.2941995;
		break;
	case 296: //"Julia":
		g = 0.14709975;
		break;
	case 310: //"Hox":
		g = 1.372931;
		break;
	case 311: //"Argo":
		g = 0.34323275;
		break;
	case 320: //"Leto":
		g = 1.176798;
	case 400: //"Grannus":
		g = 700.19481;
		break;
	default:
		g = -1;
		break;
	}
	return g;
}

int signf(float x) { return (x > 0) - (x < 0); } //returns 1, 0, -1
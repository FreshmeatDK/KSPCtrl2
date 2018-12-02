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
	displayoff = true;
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

void reLight()
{
	displayoff = false;
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

}

void statusLED(int lednum, bool status)
{
	if (status) leds[lednum] = 0x001100;
	else leds[lednum] = 0x110000;
}

void chkKeypad() {

	char key = keymain.getKey();
	if (key != NO_KEY) {
		if ((key == '#')) {
			cmdStr[cmdStrIndex - 1] = '\0';
			cmdStrIndex--;
			lcd2.setCursor(cmdStrIndex, 3);
			lcd2.print(" ");

		}
		if ((key != '#')) {
			cmdStr[cmdStrIndex] = key;
			cmdStrIndex++;
			//Serial.println(key);
		}

		if (cmdStrIndex > 18) cmdStrIndex = 18;

	}
	lcd2.setCursor(0, 3);
	lcd2.print(cmdStr);
	if ((cmdStr[cmdStrIndex - 1] == '*') && (cmdStr[cmdStrIndex - 2] == '*')) {
		execCmd();
		lcd2.clear();
		for (int i = 0; i <= 18; i++) {
			cmdStr[i] = '\0';
		}
		cmdStrIndex = 0;
	}
}

void execCmd() {
	char action[2];
	char value[18];
	char tmpval[8];
	int actionN;

	if (cmdStr[2] == '*') {                      //if the string is of format xx*, int action = xx
		for (int i = 0; i < 2; i++) {
			action[i] = cmdStr[i];
		}
		action[2] = '\0';
		for (int i = 3; i < cmdStrIndex; i++) {   // int value is the value of rest of string
			value[i - 3] = cmdStr[i];
		}
		value[cmdStrIndex - 3] = '\0';

		actionN = atoi(action);
		switch (actionN) {
		case 91: // set time yy mm dd w hh mm ss
		{
			for (int i = 0; i < 2; i++) {
				tmpval[i] = value[i];
			}
			tmpval[2] = '\0';
			year = (byte)atoi(tmpval);
			for (int i = 0; i < 2; i++) {
				tmpval[i] = value[i + 2];
			}
			tmpval[2] = '\0';
			month = (byte)atoi(tmpval);
			for (int i = 0; i < 2; i++) {
				tmpval[i] = value[i + 4];
			}
			tmpval[2] = '\0';
			dayOfMonth = (byte)atoi(tmpval);
			for (int i = 0; i < 1; i++) {
				tmpval[i] = value[i + 6];
			}
			tmpval[1] = '\0';
			dayOfWeek = (byte)atoi(tmpval);
			for (int i = 0; i < 2; i++) {
				tmpval[i] = value[i + 7];
			}
			tmpval[2] = '\0';
			hour = (byte)atoi(tmpval);
			for (int i = 0; i < 2; i++) {
				tmpval[i] = value[i + 9];
			}
			tmpval[2] = '\0';
			minute = (byte)atoi(tmpval);
			for (int i = 0; i < 2; i++) {
				tmpval[i] = value[i + 11];
			}
			tmpval[2] = '\0';
			second = (byte)atoi(tmpval);
			setTime(second, minute, hour, dayOfWeek, dayOfMonth, month, year);
			break; }
		case 92: { // clear indicators, todo clear alarms
			lcd.clear();
			lcd2.clear();
			break;
		}

		}


	}
}

void LCPotDisplay(int add, int16_t num, char pot)
{
	LCNum(add, num);
	LCChar(add, pot);
}

void LCChar(int add, char pot)
{
	switch (pot)
	{
		case 'y':
			lc.setRow(add, 7, B00111011);
			break;
		case 'r':
			lc.setRow(add, 7, B00000101);
			break;
		case 'p':
			lc.setChar(add, 7, 'P', false);
			break;
		case 'e':
			lc.setChar(add, 7, 'E', false);
			break;
		default:
			lc.setRow(add, 7, B01001001);
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
	second = bcdToDec(Wire.read() & 0x7f);
	minute = bcdToDec(Wire.read());
	hour = bcdToDec(Wire.read() & 0x3f);
	/*Serial.print(hour);
	Serial.print(':');
	Serial.print(minute);
	Serial.print(':');
	Serial.println(second);
	Serial.println();*/
	//dayOfWeek = bcdToDec(Wire.read());
	//dayOfMonth = bcdToDec(Wire.read());
	//month = bcdToDec(Wire.read());
	//year = bcdToDec(Wire.read());
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
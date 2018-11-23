// Auxillary functions

void charcpypos(char *in, uint8_t pos, char *out) {

	// copy chars from in to out, starting at pos

	for (int i = 0; i < strlen(in); i++) {
		out[pos + i] = in[i];
	}
}

void engNumF(float val, byte pos, char *out) {
	// fills char array out[pos] to and including out[pos+5] with number val and a prefix
	uint8_t pre;
	EngNumber buf;
	floatToEng(val, &buf);
	for (int i = 0; i <= buf.dp; i++) {
		out[pos + i] = buf.digits[i] + 48;
	}
	out[pos + 1 + buf.dp] = '.';
	for (int i = 0; i < ENGNUM_DIGITS - buf.dp; i++) {
		out[pos + 2 + buf.dp + i] = buf.digits[i + buf.dp + 1] + 48;
	}

	if (buf.exponent < 0) {
		pre = (-buf.exponent / 3);
		out[pos + 5] = prefixS[pre];
	}
	else {
		pre = (buf.exponent / 3);
		out[pos + 5] = prefixB[pre];
	}

}

void debugsetting() {

	lcd2.clear();
	debugLEDs();
}

void debugfindmu() {
	// mu is 228, http://symlink.dk/electro/hd44780/
	char c;
	for (int i = 225; i < 230; i++) {
		lcd2.print(i);
		c = i;
		lcd2.print(c);
		lcd2.print(" ");
		delay(20);
	}
}

void debugEngNumber() {



	int pre;

	char pstr[80];

	charcpypos("Alt: ", 0, pstr);

	engNumF(VData.Alt, 5, pstr);

	charcpypos("abcdefghijklmn", 11, pstr);

	for (int i = 25; i < 80; i++) {
		pstr[i] = 65 + i % 24;
	}
	pstr[80] = '\0';

	lcd2.print(pstr);

}

void debugAutoPilot() {
	lcd2.clear();
	lcd2.print("Pch ");
	lcd2.print(VData.Pitch, 1);
	lcd.setCursor(9, 0);
	lcd2.print("dev ");
	lcd2.print(dPitch);
	lcd2.setCursor(0, 1);
	lcd2.print("Ctl ");
	lcd2.print(CPacket.Pitch);
	lcd2.setCursor(0, 2);
	lcd2.print(" d ");
	lcd2.print(VDevNew, 0);
	lcd2.print(" dd ");
	lcd2.print(corr);

}

void debugSw() {
	lcd2.clear();
	lcd2.print("0:");
	lcd2.print(inbytes[0], BIN);
	lcd2.setCursor(10, 0);
	lcd2.print("1:");
	lcd2.print(inbytes[1], BIN);
	lcd2.setCursor(0, 1);
	lcd2.print("2:");
	lcd2.print(inbytes[2], BIN);
	lcd2.setCursor(10, 1);
	lcd2.print("3:");
	lcd2.print(inbytes[3], BIN);
	lcd2.setCursor(0, 2);
	lcd2.print("4:");
	lcd2.print(inbytes[4], BIN);
	lcd2.setCursor(10, 2);
	lcd2.print("5:");
	lcd2.print(inbytes[5], BIN);

	lcd.clear();
	lcd.print("0:");
	lcd.print(outbytes[0], BIN);
	lcd.setCursor(10, 0);
	lcd.print("1:");
	lcd.print(outbytes[1], BIN);
	lcd.setCursor(0, 1);
	lcd.print("2:");
	lcd.print(outbytes[2], BIN);
	lcd.setCursor(10, 1);
	lcd.print("M:");
	lcd.print(CPacket.MainControls, BIN);
	lcd.setCursor(0, 2);
	lcd.print("AC: ");
	lcd.print(CPacket.ControlGroup, BIN);
	lcd.setCursor(10, 2);

}

void debugGuages() {
	lcd.clear();
	lcd.print(CPacket.Throttle / 5);
	analogWrite(ELECPIN, CPacket.Throttle / 5);

}

void debugLEDs() {
	for (int i = 0; i < 8; i++) {

		lcd2.clear();
		lcd2.print("i: ");
		lcd2.print(i);
		lc.setRow(0, i, 255);
		delay(1000);
		lc.setRow(0, i, 0);
	}

}

void initLEDS() {
	//pinMode(GLED,OUTPUT);
	//digitalWrite(GLED,HIGH);

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
	uint32_t lval; // the value that will hold fval ASAP
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
		while (fval > 4294967295) { // float operation as long as needed
			fval = fval * 0.001;
			deca++;
		}
		lval = fval;
		while (lval >= pbase) {
			lval = lval*0.001;
			deca++;
		}

		if (lval > pbase*0.1) deci = 0;
		else if (lval > pbase*0.01) deci = 1;
		else if (lval > pbase*0.001) deci = 2;
		else deci = 3;

		if (deci == 2) BC++;

		dtostrf(lval, BC + 1, deci, out);
		out[BC + 1] = ' ';
		out[BC + 2] = prefix[deca];
		out[BC + 3] = '\0';


	}
}

void time2str(uint32_t ltime, char *out) {
	/*
	This function formats a time to a six char format,
	i.e. 00h00m, 0y000d, 00m00s, to get a constant amount
	of significant digits. Breaks above 99 years.
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

void time2char(uint32_t ltime, uint8_t pos, char *out) {
	/*
	This function insert a time to a six char format,
	i.e. 00h00m, 0y000d, 00m00s in out[pos] to and including out[pos+5]
	Breaks above 99 years. Not null terminated.
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
			out[0 + pos] = buf[0];
			out[1 + pos] = buf[1];
			out[2 + pos] = 'Y';

		}

		//------------------Y:ddd---------------------

		else if (ty > 0) {
			itoa(ty, buf, 10);
			out[0 + pos] = buf[0];
			out[1 + pos] = 'Y';

			if (td > 99) {
				itoa(td, buf, 10);
				out[2 + pos] = buf[0];
				out[3 + pos] = buf[1];
				out[4 + pos] = buf[2];
			}
			else if (td > 9) {
				itoa(td, buf, 10);
				out[2 + pos] = '0';
				out[3 + pos] = buf[0];
				out[4 + pos] = buf[1];
			}
			else {
				itoa(td, buf, 10);
				out[2 + pos] = '0';
				out[3 + pos] = '0';
				out[4 + pos] = buf[0];
			}
			out[5 + pos] = 'd';

		}

		//---------------------ddd:h----------------

		else if (td > 0) {
			if (td > 99) {
				itoa(td, buf, 10);
				out[0 + pos] = buf[0];
				out[1 + pos] = buf[1];
				out[2 + pos] = buf[2];
			}
			else if (td > 9) {
				itoa(td, buf, 10);
				out[0 + pos] = '0';
				out[1 + pos] = buf[0];
				out[2 + pos] = buf[1];
			}
			else {
				itoa(td, buf, 10);
				out[0 + pos] = '0';
				out[1 + pos] = '0';
				out[2 + pos] = buf[0];
			}
			out[3 + pos] = 'D';
			itoa(th, buf, 10);
			out[4 + pos] = buf[0];
			out[5 + pos] = 'h';


		}
		//---------------------------hh:mm-----------------    
		else if (th > 0) {
			if (th > 9) {
				itoa(th, buf, 10);
				out[0 + pos] = buf[0];
				out[1 + pos] = buf[1];
			}
			else {
				itoa(th, buf, 10);
				out[0 + pos] = '0';
				out[1 + pos] = buf[0];
			}
			out[2 + pos] = 'h';
			if (tm > 9) {
				itoa(tm, buf, 10);
				out[3 + pos] = buf[0];
				out[4 + pos] = buf[1];
			}
			else {
				itoa(tm, buf, 10);
				out[3 + pos] = '0';
				out[4 + pos] = buf[0];
			}
			out[5 + pos] = 'm';

		}

		//--------------------mm:ss---------

		else {
			if (tm > 9) {
				itoa(tm, buf, 10);
				out[0 + pos] = buf[0];
				out[1 + pos] = buf[1];
			}
			else {
				itoa(tm, buf, 10);
				out[0 + pos] = '0';
				out[1 + pos] = buf[0];
			}
			out[2 + pos] = 'm';
			if (ts > 9) {
				itoa(ts, buf, 10);
				out[3 + pos] = buf[0];
				out[4 + pos] = buf[1];
			}
			else {
				itoa(ts, buf, 10);
				out[3 + pos] = '0';
				out[4 + pos] = buf[0];
			}
			out[5 + pos] = 's';

		}

	}

}

int rdrGauge(float input) {

	float ln;
	float res;
	if ((inbytes[2] & B10000000)) {
		input /= 10;
	}
	ln = 10 * log(input);
	res = (0.000018*ln*ln*ln*ln - 0.00205*ln*ln*ln + 0.0749*ln*ln - 0.242*ln + 3.97);
	if (res > 150) res = 150;
	return (int)res;
}

int mnoGauge(int input) {
	float res;
	res = (0.00000331*input*input*input*input - 0.000421*input*input*input + 0.0139*input*input + 0.983*input);
	return (int)res;
}

int elcGuage(int input) {
	int res;
	res = (0.00000224*input*input*input*input - 0.000272*input*input*input + 0.0098*input*input + 1.05*input);
	return res;
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

byte setLB(byte n, byte m) {

	// changes two bits for red/green leds. One is inverse of the other
	// n = bit to change, m = bit from ControlStatus

	byte chbyte;
	bool setbit;

	setbit = (((VData.ActionGroups >> m) & 1) == 1);

	if (setbit) {
		chbyte |= (1 << n);
		chbyte &= ~(1 << n - 1);
	}
	else {
		chbyte &= ~(1 << n);
		chbyte |= (1 << n - 1);
	}
	return chbyte;
}

float dVHohmann(float ap) { // calculates dV to circularize at apsis
	float dV, r, sgp;
	r = r_SOIBody(VData.SOINumber);
	sgp = sgp_SOIBody(VData.SOINumber);
	if ((r - ap) > 0) 	dV = sqrt(sgp / (ap + r)) - sqrt(sgp*(2 / (ap + r) - 1 / VData.SemiMajorAxis));
	else dV = -1;
	return dV;
}


float r_SOIBody(byte SOI) {
	float r;
	switch (SOI)
	{
	case 100: //kerbol
		r = 600000;
		break;
	case 110: // moho
		r = 200000;
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


	}
	return r;
}

float sgp_SOIBody(byte SOI) {
	float sgp;
	switch (SOI)
	{
	case 100: //kerbol
		sgp = 3531600000000;
		break;
	case 110: // moho
		sgp = 65138398000;
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


	}
	return sgp;
}

int signf(float x) { return (x>0) - (x<0); } //returns 1, 0, -1

byte warnLEDseverity(byte qty, byte w1, byte w2, byte w3, byte n, byte LEDcpy) {

	// compares the quantity qty against warning thresholds and activates led
	// blink, blink fast or constant depending on severity

	byte mask;

	mask = ~(1 << n);

	LEDcpy &= mask;

	if ((qty <= w1) && (qty > w2)) {
		LEDcpy |= (longBlink << n);
	}
	if ((qty <= w2) && (qty > w3)) {
		LEDcpy |= (shortBlink << n);
	}
	if (qty <= w3) {
		LEDcpy |= (1 << n);
	}

	return LEDcpy;
}

byte reqAccPct(float a) {   //returns the required percentage of acceleration
							//to stop at target. 
	uint16_t aReqPct;


	if (getNavballMode() == 3) { // navball mode target
		if (VData.TargetDist < 3000) {
			aReqPct =(uint16_t) 50*VData.TargetV*VData.TargetV / (VData.TargetDist*a);
			if (aReqPct > 255) aReqPct = 255;
		}

	}
	else if ((getNavballMode() == 2) && (inbytes[3] == 0) && (VData.VVI < 0) && (VData.RAlt < 3000)) { // rocket in surface mode going down
		aReqPct = (uint16_t)50 * VData.VVI*VData.VVI / (VData.RAlt*3*a);
		if (aReqPct > 255) aReqPct = 255;
	}

}
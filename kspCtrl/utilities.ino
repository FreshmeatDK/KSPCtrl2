// Functions to interact directly with control panel

void Indicators() {
	nowIndic = millis();
	byte delayindicator;
	indicatorTime = nowIndic - indicatorTimeOld;
	if (indicatorTime > INDICATORCYCLETIME) {
		indicatorTimeOld = nowIndic;
		indicatorCycle++;
		warningLEDandGaugesUpdate();

		if (indicatorCycle > 3) indicatorCycle = 0;
		//readTime();
		//printTime();
		if (indicatorCycle == 0) {
		
			shortBlink = !shortBlink;
			longBlink = !longBlink;
			LEDupdate();

		} 
		if (indicatorCycle == 1) {
			shortBlink = !shortBlink;
			
			readTime();
			printTime();
		}

		if (indicatorCycle == 2) {

			shortBlink = !shortBlink;
			longBlink = !longBlink;
			LEDupdate();
		}

		if ((indicatorCycle == 3) && (inbytes[3] == 0)) {
			shortBlink = !shortBlink;
			rocketAwareDisplayChoice();
		}
			
		if ((indicatorCycle == 3) && (inbytes[3] == 1)) {
			shortBlink = !shortBlink;
			slopecalc = acos(cos(VData.Pitch*0.01744)*cos(VData.Roll*0.01744))*57.3;			
		}

	}


    if (inbytes[3] == 0){
      rocketsettingstr();
    }
    else if(inbytes[3] == 2) {
      aircraftsetting();
    }
    else if (inbytes[3] == 1) {
	  vehiclesetting(slopecalc);
    }
    else {
	  debugsetting();
		//rocketsetting();
    }

	if (oldset != inbytes[3]) {
		oldset = inbytes[3];
		lcd2.clear();
	}

    chkKeypad();
	
}

void chkKeypad() {

	key = keypad.getKey();
	if (key != NO_KEY) {
		if ( (key == '#')) {
			cmdStr[cmdStrIndex - 1] = '\0';
			cmdStrIndex--;
			lcd2.setCursor(cmdStrIndex, 3);
			lcd2.print(" ");
		}
		if ((key != '#')) {
			cmdStr[cmdStrIndex] = key;
			cmdStrIndex++;
		}

		if (cmdStrIndex > 19) cmdStrIndex = 19;
	
	}
	lcd2.setCursor(0, 3);
	lcd2.print(cmdStr);
	if ((cmdStr[cmdStrIndex-1] == '*') && (cmdStr[cmdStrIndex - 2] == '*')) {
		execCmd();
		lcd2.clear();
		for (int i = 0; i <= cmdStrIndex; i++) {
			cmdStr[i] = '\0';
		}
		cmdStrIndex = 0;
	}
}

void printTime() {
	lcd2.setCursor(0, 2);
	lcd2.print("Time: ");
	lcd2.print(hour);
	lcd2.print(":");
	if (minute < 10) lcd2.print("0");
	lcd2.print(minute);
	lcd2.print(":");
	if (second < 10) lcd2.print("0");
	lcd2.print(second);
	if ((hour > 21) && (minute > 44) || (hour > 22) ){
		lcd2.setCursor(5, 3);
		lcd2.print("Bedtime");
	}
}

void execCmd() {
	char action[2];
	char value[18];
	char tmpval[8];
	int actionN;

	if (cmdStr[2] = '*') {                      //if the string is of format xx*, int action = xx
		for (int i = 0; i < 2; i++) {
			action[i] = cmdStr[i];
		}
		action[2] = '\0';
		for (int i = 3; i < cmdStrIndex; i++) {   // int value is the value of rest of string
			value[i - 3] = cmdStr[i];
		}
		value[cmdStrIndex-3] = '\0';
		
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
		case 92:{ // clear indicators, todo clear alarms
			lcd.clear();
			lcd2.clear();
			break;
			}
		case 31: {
			autopitch = (bool)atoi(value);
			break;
			}
		case 32:
		{
			setSASMode(atoi(value));
			break;
		}
		case 33:
		{
			setNavballMode(atoi(value));
			break;
		}
		case 41: {
			acc = (float)(atoi(value) / 10);
			break;
		}
		}


	}
}


void rocketsetting(){

	//deprecated in favor of rocketsettingstr

	char pstr[8];
	float dV;
  lcd.setCursor(0,0); //               Apoapsis
  if (VData.AP > 0){
    lcd.print("AP: ");
    f2str(VData.AP,4,pstr);
    lcd.print(pstr);
    lcd.print("m     ");
    lcd.setCursor(14,0);
    if (VData.TAp < VData.TPe){
      time2str(VData.TAp,pstr);
      lcd.print(pstr);
    }
    else lcd.print("      ");
  }
  else {
    lcd.print("Escape Trajectory   ");
  }
  
  lcd.setCursor(0,1); //               Periapsis
  if (VData.PE > 0){
    lcd.print("PE: ");
    f2str(VData.PE, 4, pstr);
    lcd.print(pstr);
    lcd.print("m     ");
    lcd.setCursor(14,1);
    if (VData.TPe < VData.TAp){
      time2str(VData.TPe,pstr);
      lcd.print(pstr);
    }
    else lcd.print("      ");
  }
  else {
    lcd.print("Impact trajectory!  ");
  }
  lcd.setCursor(0,2); //               Altitude
  if (VData.VVI > 20 || VData.Alt > 10000){
    lcd.print("Alt: ");
    f2str(VData.Alt, 4, pstr);
    lcd.print(pstr);
    lcd.print("m   ");
  }
  else{
    lcd.print("Gr. Alt: ");
    f2str(VData.RAlt, 4, pstr);
    lcd.print(pstr);	
    lcd.print("m  ");
  }

  lcd.setCursor(18,3);
  lcd.print("  ");	
  lcd.setCursor(0,3); //               Velocity
  if (VData.VVI > 20 || VData.Alt > 10000){
		lcd.print("Orb. Vel: ");
		f2str(VData.VOrbit, 4, pstr);
                lcd.print(pstr);
		lcd.print("m/s");
	}
	else{
		lcd.print("Sur. Vel: ");
		f2str(VData.Vsurf, 4, pstr);
                lcd.print(pstr);
		lcd.print("m/s");

	}

}

void rocketsettingstr() {
	// assemble the entire string to print at LCD1 and send to display
	char  lcdout1[81]; // char buffer to send to LCD display
	if (VData.AP > 0) {                               //Apoapsis
		charcpypos("AP: ", 0, lcdout1);
		engNumF(VData.AP, 4, lcdout1);
		lcdout1[10] = 'm';
		for (int i = 11; i < 14; i++) {
			lcdout1[i] = ' ';
		}
		time2char(VData.TAp, 14, lcdout1);
	
	}
	else {
		charcpypos("Escape Trajectory", 0, lcdout1);
		for (int i = 17; i < 21; i++) {
			lcdout1[i] = ' ';
		}
	}
	
	if (VData.PE > 0) {                               //Periapsis
		charcpypos("PE: ", 40, lcdout1);
		engNumF(VData.PE, 44, lcdout1);
		lcdout1[50] = 'm';
		for (int i = 51; i < 54; i++) {
			lcdout1[i] = ' ';
		}
		time2char(VData.TPe, 54, lcdout1);

	}
	else {
		charcpypos("Impact Trajectory", 40, lcdout1);
		for (int i = 57; i < 60; i++) {
			lcdout1[i] = ' ';
		}
	}

	if (VData.VVI < 20 && VData.RAlt < 5000) {  // Altitude and velocity
		charcpypos("Gr. Alt: ", 20, lcdout1);
		engNumF(VData.RAlt, 29, lcdout1);
		lcdout1[35] = 'm';
		for (int i = 36; i < 40; i++) {
			lcdout1[i] = ' ';
		}

		charcpypos("Sur. Vel: ", 60, lcdout1);
		engNumF(VData.Vsurf, 70, lcdout1);
		charcpypos("m/s ", 76, lcdout1);
		
	}
	else {
		charcpypos("Alt: ", 20, lcdout1);
		engNumF(VData.Alt, 25, lcdout1);
		lcdout1[31] = 'm';
		for (int i = 31; i < 40; i++) {
			lcdout1[i] = ' ';
		}
		charcpypos("Orb. Vel: ", 60, lcdout1);
		engNumF(VData.VOrbit, 70, lcdout1);
		charcpypos("m/s ", 76, lcdout1);
		
	}
	lcdout1[80] = '\0';
	lcd.setCursor(0, 0);
	lcd.print(lcdout1);
}

void rocketAwareDisplayChoice() {
	if (getNavballMode() == 3) { // if navballmode = target
		targetApproach();
	}
	else {
		transferDV();
	}
}

void transferDV() {
	lcd2.setCursor(0, 0);
	if (VData.e < 1) {
		lcd2.print("Circ at AP: ");
		lcd2.print(dVHohmann(VData.AP), 0);
		lcd2.setCursor(0, 1);
		lcd2.print("Circ at PE: ");
		lcd2.print(-dVHohmann(VData.PE), 0);

	}
	else {
		lcd2.print("               ");
		lcd2.setCursor(0, 1);
		lcd2.print("               ");
	}
}

void targetApproach() {
	//Dispays distance, velocity and time to target
	char lcdout2[21]; //char buffer for half display
	long tgTime; // approximate time to target
	float tgV;

	tgV = abs(VData.TargetV);
	tgTime = (long)(VData.TargetDist / tgV);

	charcpypos("Target:   D: ", 0, lcdout2);
	engNumF(VData.TargetDist, 13, lcdout2);
	charcpypos("m", 19, lcdout2);
	lcdout2[20] = '\0';

	lcd2.setCursor(0, 0);
	lcd2.print(lcdout2);

	charcpypos("t: ", 0, lcdout2);
	time2char(tgTime, 3, lcdout2);
	charcpypos(" V:", 9, lcdout2);
	if (VData.TargetV < 0) {
		lcdout2[12] = '-';
	}
	else {
		lcdout2[12] = ' ';
	}
	engNumF(tgV, 13, lcdout2);
	lcdout2[19] = '%';
	lcdout2[20] = '\0';

	lcd2.setCursor(0, 1);
	lcd2.print(lcdout2);

}

void aircraftsetting(){
	char pstr[8];
	float deviation, absDev, approach;
  lcd.setCursor(0,0);//                 Air Speed
  lcd.print("IAS: ");
  f2str(VData.IAS, 3, pstr);
  lcd.print(pstr);
  lcd.print("m/s ");
  lcd.setCursor(15,0);
  lcd.print("M ");
  lcd.print(VData.MachNumber,1);
  
  lcd.setCursor(0,1); //                Altitude
  lcd.print("Alt: ");
  f2str(VData.Alt, 3, pstr);
  lcd.print(pstr);
  lcd.print("m  ");
  lcd.setCursor(18,1);
  lcd.print("  ");
  lcd.setCursor(13,1);
  f2str(VData.RAlt, 3, pstr);
  lcd.print(pstr);
  lcd.print("m  ");
  
  deviation = (VData.Lat + 0.0486)*10472;   //  Runway ILS
  if (deviation < 0) absDev  = -deviation;
  approach = abs((VData.Lon - 285.28)*10472);
  
  lcd.setCursor(0,2);
  lcd.print("Dev: ");
  if (deviation < 0){
    f2str(absDev, 3, pstr);
    lcd.print(pstr);
    lcd.print("m S  ");
  }
  else {
    f2str(deviation, 3, pstr);
    lcd.print(pstr);
    lcd.print("m N  ");
  }
  

  //lcd.setCursor(18,3);
  //lcd.print("  ");
  lcd.setCursor(5,3);
  f2str(approach, 3, pstr);
  lcd.print(pstr);
  lcd.print("m  ");
  debugAutoPilot();
}

void vehiclesetting(float slope) {
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
	lcd.print(VData.Pitch,0);
	lcd.print("/");
	lcd.print(slope,0);

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

void LEDupdate() {
	byte LEDout[5];
	
	for (int i = 0; i < 5; i++) {
		LEDout[i] = 0;
	}

	LEDout[0] |= setLB(6, 10);
	LEDout[0] |= setLB(4, 9);
	LEDout[0] |= setLB(2, 8);
	LEDout[1] |= setLB(6, 7);
	LEDout[1] |= setLB(4, 6);
	LEDout[1] |= setLB(2, 12);
	LEDout[2] |= setLB(6, 15);
	LEDout[2] |= setLB(4, 14);
	LEDout[2] |= setLB(2, 13);
	LEDout[3] |= setLB(6, 4);
	LEDout[3] |= setLB(4, 3);
	LEDout[3] |= setLB(2, 2);
	LEDout[4] |= setLB(6, 11);
	LEDout[4] |= setLB(4, 1);
	LEDout[4] |= setLB(2, 0);

	for (int i = 0; i < 5; i++) {
		lc.setRow(0, i, LEDout[i]);
	}
	


}

void warningLEDandGaugesUpdate() {
	byte LEDrow = 0;
	boolean crash = 0;
	
	byte mnopct, fuelpct, elecpct, overheat, highv, lowA;
	int elecOld, elecNew;

	elecOld = elecNew;
	elecNew = (int)VData.ECharge;

	mnopct = round(VData.MonoProp / VData.MonoPropTot * 100);
	LEDrow = warnLEDseverity(mnopct, 10, 5, 0, 6, LEDrow);

	if ((inbytes[2] & B1000000)) {
		fuelpct = round(VData.XenonGas / VData.XenonGasTot * 100);
	}
	else {
		fuelpct = round(VData.LiquidFuelS / VData.LiquidFuelTotS * 100);
	}
	LEDrow = warnLEDseverity(fuelpct, 10, 4, 0, 7, LEDrow);

	if ((inbytes[2] & B100000)) {
		elecpct = round(VData.ECharge / VData.EChargeTot * 100);
		LEDrow = warnLEDseverity(elecpct, 25, 10, 0, 0, LEDrow);
	}
	else {
		elecpct = constrain((elecOld - elecNew + 50), 0, 100);
	}

	overheat = 100 - VData.MaxOverHeat;
	LEDrow = warnLEDseverity(overheat, 15, 10, 5, 2, LEDrow);
	
	// check for high velocity
	highv = reqAccPct(1);
	if (highv > 100) crash = 1;
	else highv = 100 - highv;
	LEDrow = warnLEDseverity(highv, 50, 25, 15, 3, LEDrow); 

	if (inbytes[3] == 2) { //aircraft
		if (VData.RAlt < 1000) {
			lowA = (byte)(0.1 * VData.RAlt);
			LEDrow = warnLEDseverity(lowA, 75, 25, 10, 4, LEDrow);
		}
	}

	lc.setRow(0, 6, LEDrow);

	analogWrite(MONOPIN, mnoGauge(mnopct));
	analogWrite(FUELPIN, mnoGauge(fuelpct));
	analogWrite(ELECPIN, elcGuage(elecpct));
	analogWrite(RDRPIN, rdrGauge(VData.RAlt));
	
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
	//dayOfWeek = bcdToDec(Wire.read());
	//dayOfMonth = bcdToDec(Wire.read());
	//month = bcdToDec(Wire.read());
	//year = bcdToDec(Wire.read());
}



void autoPilot() {
		
	float DsPitch = -VDevNew*0.2;
	if (DsPitch < -7) DsPitch = -7;
	if (DsPitch > 7) DsPitch = 7;

	if (VData.Pitch > 7) {
		if (CPacket.Pitch > 0) CPacket.Pitch = 0;
		dPitch -= 5;
	}
	else if (VData.Pitch < -7) {
		if (CPacket.Pitch < 0) CPacket.Pitch = 0;
		dPitch += 5;
	}

	else {
		CPacket.Pitch = (DsPitch-VData.Pitch)*200+200;
		corr -= 0.1 * (signf(VData.VVI) + signf(VDevNew))*abs(VDevNew);
		if (corr < -500) corr = -500;
		if (corr > 500) corr = 500;
		dPitch = corr;

	}
	
	if (dPitch < -500) dPitch = -500;
	if (dPitch > 500) dPitch = 500;

	CPacket.Pitch += dPitch;
	if (CPacket.Pitch > 1000) CPacket.Pitch = 1000;
	if (CPacket.Pitch < -1000) CPacket.Pitch = -1000;

}
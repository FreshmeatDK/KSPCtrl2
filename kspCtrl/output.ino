// Everything back to KSP goes here

void output() {
  now = millis();
  
  controlTime = now - controlTimeOld; 
  if (controlTime > CONTROLREFRESH){
	  controlTimeOld = now;
	  controls();
  } 
 }

void controls() {
      
  if (Connected) {

	joysticks();
	toggles();
	CtlUpdate();

    KSPBoardSendData(details(CPacket));
	SendToSlave(slaveCtrl);
  }
}

byte getSASMode() {
	return VData.NavballSASMode & B00001111; // leaves alone the lower 4 bits of; all higher bits set to 0.
}

byte getNavballMode() {
	return VData.NavballSASMode >> 4; // leaves alone the higher 4 bits of; all lower bits set to 0.
}

void setSASMode(byte m) {
	CPacket.NavballSASMode &= B11110000;
	CPacket.NavballSASMode += m;
}

void setNavballMode(byte m) {
	CPacket.NavballSASMode &= B00001111;
	CPacket.NavballSASMode += m << 4;
}


void MainControls(byte n, boolean s) {
  if (s)
    CPacket.MainControls |= (1 << n);       // forces nth bit of x to be 1.  all other bits left alone.
  else
    CPacket.MainControls &= ~(1 << n);      // forces nth bit of x to be 0.  all other bits left alone.
}

void ControlGroups(byte n, boolean s) {
  if (s)
    CPacket.ControlGroup |= (1 << n);       // forces nth bit of x to be 1.  all other bits left alone.
  else
    CPacket.ControlGroup &= ~(1 << n);      // forces nth bit of x to be 0.  all other bits left alone.
}


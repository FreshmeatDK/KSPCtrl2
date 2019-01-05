int input() 
{
	int returnValue = -1;
	now = millis();

	  if (KSPBoardReceiveData())
	  {
		deadtimeOld = now;
		returnValue = id;
		switch(id) 
		{
			case 0: //Handshake packet
			Handshake();
			break;

			case 1:
			Indicators();
			break;
		}


		//We got some data, set status accordingly

		if (Connected == false) 
		{
			Connected = true;
			reLight();
		}
	
	  }
  else
  { //if no message received for a while, go idle
		deadtime = now - deadtimeOld; 
		if (deadtime > IDLETIMER)
		{
			deadtimeOld = now;
			Connected = false;
			readTime();
			LEDprintTime(hour, minute, second);
			blackout();
			Serial1.print(second);
    }    
  }

  return returnValue;
}

byte ControlStatus(byte n)
{
  return ((VData.ActionGroups >> n) & 1) == 1;
}


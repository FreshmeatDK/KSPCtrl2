int input() 
{
	int returnValue = -1;
	g_now = millis();

	  if (KSPBoardReceiveData())
	  {
		g_deadtimeOld = g_now;
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

		if (g_Connected == false) 
		{
			g_Connected = true;
			reLight();
		}
	
	  }

	  else
	  { //if no message received for a while, go idle
			g_deadtime = g_now - g_deadtimeOld; 
			if (g_deadtime > IDLETIMER)
			{
				g_deadtimeOld = g_now;
				g_Connected = false;
				readTime();
				LCRTC(g_hour, g_minute, g_second,0);
				blackout();

				Serial1.print(g_hour);
				Serial1.print(':');
				Serial1.print(g_minute);
				Serial1.print(':');
				Serial1.println(g_second);

		}    
	  }

	  bool dummy = receivekRPC();

  return returnValue;
}

byte ControlStatus(byte n)
{
  return ((VData.ActionGroups >> n) & 1) == 1;
}


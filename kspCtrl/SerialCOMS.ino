uint8_t rx_len;
uint16_t * address;
byte buffer[256]; //address for temporary storage and parsing buffer
uint8_t structSize;
uint8_t rx_array_inx;  //index for RX parsing buffer
uint8_t calc_CS;	   //calculated Chacksum

//This shit contains stuff borrowed from EasyTransfer lib
boolean KSPBoardReceiveData() {
  if ((rx_len == 0)&&(Serial.available()>3)){
    while(Serial.read()!= 0xBE) {
      if (Serial.available() == 0)
        return false;  
    }
    if (Serial.read() == 0xEF){
      rx_len = Serial.read();   
      id = Serial.read(); 
      rx_array_inx = 1;

      switch(id) {
      case 0:
        structSize = sizeof(HPacket);   
        address = (uint16_t*)&HPacket;     
        break;
      case 1:
        structSize = sizeof(VData);   
        address = (uint16_t*)&VData;     
        break;
      }
    }

    //make sure the binary structs on both Arduinos are the same size.
    if(rx_len != structSize){
      rx_len = 0;
      return false;
    }   
  }

  if(rx_len != 0){
    while(Serial.available() && rx_array_inx <= rx_len){
      buffer[rx_array_inx++] = Serial.read();
    }
    buffer[0] = id;

    if(rx_len == (rx_array_inx-1)){
      //seem to have got whole message
      //last uint8_t is CS
      calc_CS = rx_len;
      for (int i = 0; i<rx_len; i++){
        calc_CS^=buffer[i];
      } 

      if(calc_CS == buffer[rx_array_inx-1]){//CS good
        memcpy(address,buffer,structSize);
        rx_len = 0;
        rx_array_inx = 1;
        return true;
      }
      else{
        //failed checksum, need to clear this out anyway
        rx_len = 0;
        rx_array_inx = 1;
        return false;
      }
    }
  }

  return false;
}

void KSPBoardSendData(uint8_t * address, uint8_t len){
  uint8_t CS = len;
  Serial.write(0xBE);
  Serial.write(0xEF);
  Serial.write(len);
  
  for(int i = 0; i<len; i++){
    CS^=*(address+i);
    Serial.write(*(address+i));
  }
  
  Serial.write(CS);
}


// ----- Send packet to kRPC -----

void sendTokRPC()
{
	byte LRC = 0;
	byte escChar = B00001111;
	Serial1.write(B10101010);

	for (int i = 0; i < NUMSLAVEBYTES; i++)
	{
		if ((kRPCPacket[i] == B11001100) || (kRPCPacket[i] == B00001111) || (kRPCPacket[i] == B10101010))
		{
			Serial1.write((byte*)&escChar, sizeof(escChar));
		}
		Serial1.write((byte*)&kRPCPacket[i], sizeof(kRPCPacket[i]));
		LRC = 0; //LRC^kRPCPacket[i];
	}
	lcd2.println(LRC);
	Serial1.write((byte*)&LRC, sizeof(LRC));
	Serial1.write(B11001100);
}



void CommskRPC()
{
	byte flush;
	byte clearToGo = 0;
	
	if (Serial1.available())
	{
		clearToGo = Serial1.read();
		while (Serial1.available()) Serial1.read();
	}
	if (clearToGo = B01010101)
	{
		sendTokRPC();
	}
	if (Serial1.available() > 80)
	{
		lcd2.clear();
		lcd2.print("Comms error: Overflow");
		while (Serial1.available()) Serial1.read();
		lcd2.clear();
	}
		
}

bool receivekRPC()
{
	const int8_t rx_len = 4;
	uint16_t *adress;
	byte buff[rx_len];
	uint8_t structSize;
	byte tmp;

	structSize = sizeof(kVData);
	adress = (uint16_t*)&kVData;

	if (Serial1.available() > 6)
	{
		tmp = 0;
		while (tmp != 85)
		{
			tmp = Serial1.read();

		}
		if (tmp == 85)
		{

			for (int i = 0; i < rx_len; i++)
			{
				buff[i] = Serial1.read();

			}
			tmp = Serial.read();
			//if (tmp == 170)
			{
				memcpy(adress, buff, structSize);
				return true;
			}
		}
		else return false;
	}
	else return false;
}













#include <RadioLib.h>
#include "Config.h"
#include "KISS.h"
#define BUFFSIZE MTU-64

enum intEvent { Nothing, RxDone, TxDone, CadDone };
volatile enum intEvent waitFor = Nothing;
bool kissMode = false;
char mycall[16];
char line_buffer[BUFFSIZE];
int bufc = 0;
SX1280 LoRa = new Module(pinNSS, pinDIO1, pinNRST, pinBUSY);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(serialBaudRate);
  while (!Serial); // Waiting until LoRa32u4 is ready

  // Buffers
  memset(rxBuffer, 0, sizeof(rxBuffer));
  memset(txBuffer, 0, sizeof(txBuffer));

  startRadio();
}

bool initLoRa() {
  mycall[0] = '\0';
  LoRa.clearDio1Action();
  waitFor = Nothing;
  LoRa.reset();

  int state = LoRa.begin(loraFrequency, bandWidthTable[loraBandwidth],
      loraSpreadingFactor, loraCodingRate, loraTxPower, loraPrlen);
  if (state != ERR_NONE) {
    return false;
  }

  LoRa.setCRC(loraCRC);
  LoRa.setDio1Action(dio1Callback);
  waitFor = RxDone;
  LoRa.startReceive();
  return true;
}

bool startRadio() {
  if (!initLoRa()) {
    kissIndicateError(ERROR_INITRADIO);
    Serial.println("FAIL");
    while(1);
  }
}

void transmit(size_t size) {
  LoRa.standby(); // stop receiver to disable interrupt
  waitFor = TxDone;
  LoRa.startTransmit(txBuffer, (size < MTU) ? size : MTU);
  while (waitFor == TxDone) {
    yield();
  }
  waitFor = RxDone;
  LoRa.startReceive(); // start receiver again
}

void text_transmit(char* buffer) {
  if (strlen(mycall) == 0) {
    mesg("ERR","Set your callsign.");
    return;
  }
  strcpy(txBuffer, mycall);
  strcat(txBuffer, " >: ");
  if (strlen(buffer) < BUFFSIZE) {
    strcat(txBuffer, buffer);
  }
  transmit(strlen(txBuffer));
}

void serialCallback(uint8_t txByte) {
  if (inFrame && txByte == FEND) {
    inFrame = false;
    if ( command == CMD_DATA ) {
      //Serial.println("FULL_KISS");
      if (outboundReady) {
        kissIndicateError(ERROR_QUEUE_FULL);
      } else {
        outboundReady = true;
        //Serial.println("RDY_OUT");
      }
    } else if ( command == CMD_RETURN ) {
      kissMode = false;
    }
  } else if (txByte == FEND) {
    //Serial.println("KISS_FLAG");
    inFrame = true;
    command = CMD_UNKNOWN;
    frameLength = 0;
  } else if (inFrame && frameLength < MTU) {
    // Get command byte
    if (frameLength == 0 && command == CMD_UNKNOWN) {
      //Serial.println("ACQ_CMD");
      command = txByte;
    } else if (command == CMD_DATA) {
      if (txByte == FESC) {
        escape = true;
      } else {
        if (escape) {
          if (txByte == TFEND) {
            txByte = FEND;
          }
          if (txByte == TFESC) {
            txByte = FESC;
          }
          escape = false;
        } else {
          txBuffer[frameLength++] = txByte;
        }
      }
    }
  }
}

bool isOutboundReady() {
  return outboundReady;
}

#if 0 // XXX not implemented
void cadCallback(boolean cadDetected) {
  if (cadDetected) {
    lastHeard = millis();
    channelBusy = true;
    LoRa.startReceive();
  } else {
    LoRa.CAD();
  }
  return;
}
#endif

void dio1Callback() {
  if (waitFor != RxDone) {
    waitFor = Nothing;
    return;
  }

  int packetSize = LoRa.getPacketLength();
  readLength = 0;
  lastRssi = LoRa.getRSSI();
  lastSnr = LoRa.getSNR();
  getPacketData(packetSize);

  if (kissMode) {
    // Send RSSI
    Serial.write(FEND);
    Serial.write(CMD_HARDWARE);
    Serial.print(lastRssi);
    Serial.write(',');
    Serial.print(lastSnr);
    Serial.write(FEND);

    // And then write the entire packet
    Serial.write(FEND);
    Serial.write((uint8_t)CMD_DATA);
    for (int i = 0; i < readLength; i++) {
      uint8_t temp = rxBuffer[i];
      if (temp == FEND) {
        Serial.write(FESC);
        temp = TFEND;
      }
      if (temp == FESC) {
        Serial.write(FESC);
        temp = TFESC;
      }
      Serial.write(temp);
    }
    Serial.write(FEND);
  } else {
    String incoming;
    String rssi = String(lastRssi);
    String snr = String(lastSnr);
    for(int i = 0; i < readLength; i++) {
      incoming += (char)rxBuffer[i];
    }	
    String hiscall = incoming.substring(0,incoming.indexOf(">:"));
    String message = incoming.substring(incoming.indexOf(">:")+2);
    recv_mesg(hiscall,rssi,snr,message);
  }

  readLength = 0;
  lastHeard = millis();
  channelBusy = false;
  waitFor = Nothing;
  // kick the receiver outside interrupt context
}

void escapedSerialWrite (uint8_t bufferByte) {
  switch(bufferByte) {
    case FEND:
      Serial.write(FESC);
      Serial.write(TFEND);
      break;
    case FESC:
      Serial.write(FESC);
      Serial.write(TFESC);
      break;
    default:
      Serial.write(bufferByte);
  }
}

void kissIndicateError(uint8_t errorCode) {
  Serial.write(FEND);
  Serial.write(CMD_ERROR);
  Serial.write(errorCode);
  Serial.write(FEND);
}

void getPacketData(int packetLength) {
  LoRa.readData(rxBuffer, readLength = packetLength);
}

void recv_mesg(String& hiscall, String& rssi, String& snr, String& mesg) {
  Serial.print(hiscall);
  Serial.println("(RSSI="+rssi+",SNR="+snr+"):"+mesg);
}

void mesg(char* prop, uint32_t param) {
  Serial.print(prop);
  Serial.print(' ');
  Serial.println(param);
}

void mesg(char* prop, char* param) {
  Serial.print(prop);
  Serial.print(' ');
  Serial.println(param);
}

void set_Freq(uint32_t f) {
  f = f * 10000;
  mesg("Freq",f);
  if (f < freq_low || f > freq_high) {
    mesg("ERR","Out of band.");
    return;
  }
  loraFrequency = f / 1000000.0F;
  LoRa.setFrequency(loraFrequency);
}

void set_BW(int bw) {
  mesg("BW",bw);
  if (bw < 0 || bw >= (sizeof(bandWidthTable) / sizeof(bandWidthTable[0]))) {
    mesg("ERR","unsupported bandwidth.");
    return;
  }
  loraBandwidth = bw;
  LoRa.setBandwidth(bandWidthTable[loraBandwidth]);
}

void set_SF(int sf) {
  mesg("SF",sf);
  if (sf < 5 || sf > 12) {
    mesg("ERR","unsupported spreding factor.");
    return;
  }
  loraSpreadingFactor = sf;
  LoRa.setSpreadingFactor(loraSpreadingFactor);
}

void set_CR(int cr) {
  mesg("CR",cr);
  if (cr < 5 || cr > 8) {
    mesg("ERR","unsupported coding rate.");
    return;
  }
  loraCodingRate = cr;
  LoRa.setCodingRate(loraCodingRate);
}

void set_Pwr(int pwr) {
  mesg("PWR",pwr);
  if (pwr > 13 || pwr < -18) {
    mesg("ERR","unsupported power.");
    return;
  }
  loraTxPower = pwr;
  LoRa.setOutputPower(loraTxPower);
}

void set_Call(char *call) {
  if (strlen(call) < 12) {
    strcpy(mycall, call);
    mesg("CALL", mycall);
  } else {
    mesg("ERR","callsign too long.");
  }
  return;
}

void set_Backoff(int backoff) {
  mesg("BKOFF",backoff);
  if (backoff < 500) {
    Serial.println("backtime error.");
    return;
  }
  loraMaxBackoff = backoff;
}

void do_command(char buffer[]) {
  char *cmd, *call;
  uint32_t f;
  int bw, sf, cr, pwr, backoff;

  // Calling RadioLib's function at setting Freq/BW/SF/CR turns SX127x
  // stand-by state. To kick receiver later, set "wait for nothing"
  // in advance.
  LoRa.standby();
  waitFor = Nothing;

  cmd = strtok(buffer," ");
  
  if (strcasecmp(cmd, "KISS") == 0) {
    f = atol(strtok(NULL,","));
    bw = atoi(strtok(NULL,","));
    sf = atoi(strtok(NULL,","));
    cr = atoi(strtok(NULL,","));
    backoff = atoi(strtok(NULL,","));

    set_Freq(f);
    set_BW(bw);
    set_SF(sf);
    set_CR(cr);
    set_Backoff(backoff);
    mesg("MSG","TNC MODE.");
    kissMode = true;
    return;
  }

  cmd = strtok(NULL," ");
  if (strcasecmp(cmd , "FREQ") == 0) {
    f = atol(strtok(NULL," "));
    set_Freq(f);
    return;
  }
  if (strcasecmp(cmd, "BW") == 0) {
    bw= atoi(strtok(NULL," "));
    set_BW(bw);
    return;
  }
  if (strcasecmp(cmd, "SF") == 0) {
    sf = atoi(strtok(NULL," "));
    set_SF(sf);
    return;
  }
  if (strcasecmp(cmd, "CR") == 0) {
    cr = atoi(strtok(NULL," "));
    set_CR(cr);
    return;
  }
  if (strcasecmp(cmd, "PWR") == 0) {
    pwr = atoi(strtok(NULL," "));
    set_Pwr(pwr);
    return;
  }
  if (strcasecmp(cmd , "INIT") == 0) {
    initLoRa();
    return;
  }
  if (strcasecmp(cmd , "CALL") == 0) {
    call = strtok(NULL," ");
    set_Call(call);
    return;
  }
  Serial.print(F("Unknown command:"));
  Serial.println(buffer);
}

void loop() {
  char *cmd;
  if (kissMode) {
    kiss_loop();
  } else if(Serial.available()) {
    char txByte = Serial.read();
    if (txByte == '\r') {
      line_buffer[bufc] = '\0';
      if (strncasecmp(line_buffer, "SET", 3) == 0 ||
          strncasecmp(line_buffer, "KISS", 4) == 0) {
        do_command(line_buffer);
      } else {
        text_transmit(line_buffer);
      }
      line_buffer[0] = '\0';
      bufc = 0;
    } else {
      line_buffer[bufc++] =  txByte;
      if (bufc > (MTU - 16)) {
        bufc = MTU - 16;
      }
    }
  }
  if (waitFor == Nothing) {
    waitFor = RxDone;
    LoRa.startReceive();
  }
}

void kiss_loop() {
   uint32_t now = millis();
   if (isOutboundReady() && !SERIAL_READING) {
    if (now - backofft > backoffDuration) {
      if (!channelBusy && (now - lastHeard > lbtDuration)) {
        outboundReady = false;
        transmit(frameLength);
        backofft = 0;
        backoffDuration = 0;
        lastHeard = 0;
      } else {
        backofft = now;
        if (backoffDuration > 0) {
          backoffDuration = backoffDuration /2;
        } else {
          backoffDuration = random(lbtDuration , loraMaxBackoff);
        }
      }
    }
  }
  if (Serial.available()) {
    SERIAL_READING = true;
    char txByte = Serial.read();
    serialCallback(txByte);
    lastSerialRead = millis();
  } else {
    if (SERIAL_READING && millis() - lastSerialRead >= serialReadTimeout) {
      SERIAL_READING = false;
    }
  }
}

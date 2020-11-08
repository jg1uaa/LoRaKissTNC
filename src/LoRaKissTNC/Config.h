
#ifndef CONFIG_H
  #define CONFIG_H

  #define MAJ_VERS 0x01
  #define MIN_VERS 0x00

  #define MCU_328P 0x90
  #define MCU_32U4 0x91

  #if defined(__AVR_ATmega328P__)
    #define MCU_VARIANT MCU_328P
    #warning "Firmware is being compiled for ATmega328p based boards"
  #elif defined(__AVR_ATmega32U4__)
    #define MCU_VARIANT MCU_32U4
    #warning "Firmware is being compiled for ATmega32u4 based boards"
  #else
    #error "The firmware cannot be compiled for the selected MCU variant"
  #endif

  #define MTU  255
  #define CMD_L       4
  int     lastRssi = 0;
  float   lastSnr  = 0;
  uint8_t lastRssiRaw = 0x00;

  size_t  readLength = 0;

  #if MCU_VARIANT == MCU_328P
    const int pinNSS = 10;
    const int pinNRST = 9;
    const int pinDIO0 = 2;
    const int pinDIO1 = 6;
    // const int pinLedRx = 5;
    // const int pinLedTx = 4;

  #endif

  #if MCU_VARIANT == MCU_32U4
    const int pinNSS  = 8;
    const int pinNRST = 4;
    const int pinDIO0 = 7;
    const int pioDIO1 = 6;
  #endif

  const long serialBaudRate   = 9600;

  int loraMaxBackoff = 10000;
  int lbtDuration = 3000;
  uint32_t backofft;
  uint32_t backoffDuration;
  uint32_t lastHeard = 0;
  boolean outboundReady = false;
  boolean channelBusy = false;

  // Default LoRa settings
  int       loraSpreadingFactor = 9;
  int       loraCodingRate      = 8;
  int       loraTxPower         = 20;
  int       loraPrlen = 8;
  bool      loraCRC = false;
  int       loraSyncWord = 0x12;

  float bandWidthTable[] = { 7.8, 10.4, 15.6, 20.8, 31.25, 41.7, 62.5, 125.0, 250.0, 500.0};
  uint32_t  loraBandwidth       = 2;
  const uint32_t freq_low  = 438.0E6;
  const uint32_t freq_high = 439.0E6;
  float loraFrequency = 438.51;

  uint8_t txBuffer[MTU];
  uint8_t rxBuffer[MTU];

  uint32_t statRx = 0;
  uint32_t statTx = 0;


#endif

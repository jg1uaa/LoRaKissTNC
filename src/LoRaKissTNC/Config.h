
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
    const int pinNRST = 3;
    const int pinBUSY = 9;
    const int pinDIO1 = 2;
    // const int pinLedRx = 5;
    // const int pinLedTx = 4;

  #endif

  #if MCU_VARIANT == MCU_32U4
    const int pinNSS  = 8;
    const int pinNRST = 4;
    const int pinBUSY = 7;
    const int pioDIO1 = 6;
  #endif

  const long serialBaudRate   = 38400;

  int loraMaxBackoff = 10000;
  int lbtDuration = 3000;
  uint32_t backofft;
  uint32_t backoffDuration;
  uint32_t lastHeard = 0;
  boolean outboundReady = false;
  boolean channelBusy = false;

  // Default LoRa settings
  int       loraSpreadingFactor = 12;
  int       loraCodingRate      = 5;
  int       loraTxPower         = 13;
  int       loraPrlen = 8;
  bool      loraCRC = false;
  int       loraSyncWord = 0x12;

  float bandWidthTable[] = { 203.125, 406.25, 812.5, 1625.0 };
  uint32_t  loraBandwidth       = 1;
  const uint32_t freq_low  = 2427.0E6;
  const uint32_t freq_high = 2450.0E6;
  float loraFrequency = 2438.50;

  uint8_t txBuffer[MTU];
  uint8_t rxBuffer[MTU];

  uint32_t statRx = 0;
  uint32_t statTx = 0;


#endif

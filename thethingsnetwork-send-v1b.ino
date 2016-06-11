/*
 * TRL  ver 1.0b   10Jun2016
 * 
 * 
 * */
 
 /* *******************************************************************************
 * Copyright (c) 2015 Thomas Telkamp and Matthijs Kooijman
 *
 * Permission is hereby granted, free of charge, to anyone
 * obtaining a copy of this document and accompanying files,
 * to do whatever they want with them without any restriction,
 * including, but not limited to, copying, modification and redistribution.
 * NO WARRANTY OF ANY KIND IS PROVIDED.
 *
 * This example sends a valid LoRaWAN packet with payload "Hello, world!", that
 * will be processed by The Things Network server.
 *
 * Note: LoRaWAN per sub-band duty-cycle limitation is enforced (1% in g1, 
*  0.1% in g2). 
 *
 * Change DEVADDR to a unique address! 
 * See http://thethingsnetwork.org/wiki/AddressSpace
 *
 * Do not forget to define the radio type correctly in config.h, default is:
 *   #define CFG_sx1272_radio 1
 * for SX1272 and RFM92, but change to:
 *   #define CFG_sx1276_radio 1
 * for SX1276 and RFM95.
 *
 *******************************************************************************/

#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>

// LoRaWAN Application identifier (AppEUI)
// Not used in this example
static const u1_t APPEUI[8]  = { 0x70, 0xb3, 0xd5, 0x7e, 0xd0, 0x00, 0x03, 0x8b };

// LoRaWAN DevEUI, unique device ID (LSBF)
// Not used in this example
static const u1_t DEVEUI[8]  = { 0x00, 0x04, 0xa3, 0xff, 0xfe, 0x31, 0x00, 0x02 };

// LoRaWAN NwkSKey, network session key 
// Use this key for The Things Network
//static const u1_t DEVKEY[16] = { 0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C };
static const u1_t DEVKEY[16] = { 0xcb, 0x93, 0x5c, 0x8d, 0xa6, 0x9e, 0x93, 0xAe, 0x0b, 0xa6, 0x42, 0x93, 0xa4, 0xCd, 0x02, 0xe9  };

// LoRaWAN AppSKey, application session key
// Use this key to get your data decrypted by The Things Network
//static const u1_t ARTKEY[16] = { 0xde, 0x3a, 0x15, 0xa3, 0xda, 0x57, 0x7f, 0xb4, 0xf4, 0xc3, 0xd6, 0xe8, 0x9d, 0x8f, 0x5F, 0x81 };
//static const u1_t ARTKEY[16] = { 0x5e, 0x93, 0x5e, 0xb1, 0x93, 0x34, 0xd0, 0xd6, 0xb3, 0x80, 0x32, 0xe9, 0x6c, 0x29, 0x9d, 0xa2 };
static const u1_t ARTKEY[16] = { 0x6b, 0x74, 0xcd, 0x4c, 0x4d, 0xea, 0xcf, 0x3f, 0x42, 0x52, 0x91, 0x1c, 0xf4, 0x56, 0xd7, 0x4b };


// LoRaWAN end-device address (DevAddr)
// See http://thethingsnetwork.org/wiki/AddressSpace
static const u4_t DEVADDR = 0x91042B1F ; // <-- Change this address for every node!

//////////////////////////////////////////////////
// APPLICATION CALLBACKS
//////////////////////////////////////////////////

// provide application router ID (8 bytes, LSBF)
void os_getArtEui (u1_t* buf) {
    memcpy(buf, APPEUI, 8);
}

// provide device ID (8 bytes, LSBF)
void os_getDevEui (u1_t* buf) {
    memcpy(buf, DEVEUI, 8);
}

// provide device key (16 bytes)
void os_getDevKey (u1_t* buf) {
    memcpy(buf, DEVKEY, 16);
}

uint8_t mydata[] = "Hello, World!";
static osjob_t sendjob;
static osjob_t keepalivejob;


#ifdef __AVR_ATmega1284P__      // MoteinoMega LoRa
// Pin mapping
lmic_pinmap pins = 
{
  .nss  = 4,
  .rxtx = 12,    // Not connected on RFM92/RFM95
  .rst  = 13,    // Needed on RFM92/RFM95 ??
  .dio  = {2, 1, 0},
};
#elif __SAMD21G18A__            // RocketScream
// Pin mapping
lmic_pinmap pins = 
{
  .nss  = 5,
  .rxtx = 7,    // Not connected on RFM92/RFM95
  .rst  = 9,    // Needed on RFM92/RFM95 ??
  .dio  = {2, 3, 6},
};
#else
#error Processor Radio pins not defined
#endif


/* ***************************************************** */
void onEvent (ev_t ev) 
{
    //debug_event(ev);

    switch(ev) 
    {
      // scheduled data sent (optionally data received)
      // note: this includes the receive window!
      case EV_TXCOMPLETE:
          // use this event to keep track of actual transmissions
          Serial.print("Event EV_TXCOMPLETE, time: ");
          Serial.println(millis() / 1000);
          
          if(LMIC.dataLen) 
          { // data received in rx slot after tx
//            debug_buf(LMIC.frame+LMIC.dataBeg, LMIC.dataLen);
              Serial.println("Data Received!");
          }
          break;
          
       case EV_JOIN_FAILED:
          Serial.print("Event EV_JOIN_FAILED, time: ");
          Serial.println(millis() / 1000);
          break;

       case EV_RXCOMPLETE:
          Serial.print("Event EV_RXCOMPLETE, time: ");
          Serial.println(millis() / 1000);
          break;

       case EV_LINK_DEAD:
          Serial.print("Event EV_LINK_DEAD, time: ");
          Serial.println(millis() / 1000);
          break;

       case EV_RESET:
          Serial.print("Event EV_RESET, time: ");
          Serial.println(millis() / 1000);
          break;

       case EV_JOINING:
          Serial.print("Event EV_JOINING, time: ");
          Serial.println(millis() / 1000);
          break;

       case EV_REJOIN_FAILED:
          Serial.print("Event EV_REJOIN_FAILED, time: ");
          Serial.println(millis() / 1000);
          break;

       case EV_JOINED:
          Serial.print("Event EV_JOINED, time: ");
          Serial.println(millis() / 1000);
          break;

       case EV_BEACON_FOUND:
          Serial.print("Event EV_BEACON_FOUND, time: ");
          Serial.println(millis() / 1000);
          break;

       case EV_SCAN_TIMEOUT:
          Serial.print("Event EV_SCAN_TIMEOUT, time: ");
          Serial.println(millis() / 1000);
          break;

       case EV_LOST_TSYNC:
          Serial.print("Event EV_LOST_TSYNC, time: ");
          Serial.println(millis() / 1000);
          break;

       case EV_BEACON_TRACKED:
          Serial.print("Event EV_BEACON_TRACKED, time: ");
          Serial.println(millis() / 1000);
          break;

       case EV_BEACON_MISSED:
          Serial.print("Event EV_BEACON_MISSED, time: ");
          Serial.println(millis() / 1000);
          break;

       default:
          Serial.print("Event EV_OTHER, time: ");
          Serial.println(millis() / 1000);
          break;
    }
}


/* ***************************************************** */
#define KeepAliveTime   60      // time is sec to restart this task
void do_keepalive(osjob_t* k)
{
      Serial.println();
      Serial.print("***  Keep-Alive-Time: ");
      Serial.println(millis() / 1000);
 
   // Schedule a timed job to run this task again at the given timestamp (absolute system time)
    os_setTimedCallback(k, os_getTime()+sec2osticks(KeepAliveTime), do_keepalive); 
} 


/* ***************************************************** */
void do_send(osjob_t* j)
{
      Serial.println();
      Serial.print("Time: ");
      Serial.println(millis() / 1000);
    // Show TX channel (channel numbers are local to LMIC)
      Serial.print("Send, txCnhl: ");
      Serial.println(LMIC.txChnl);
      Serial.print("Opmode check: ");
      
    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & (1 << 7)) 
    {
      Serial.println("OP_TXRXPEND, <-- not sending");
    } 
    else 
    {
      Serial.println("ok");
      // Prepare upstream data transmission at the next possible time.
      LMIC_setTxData2(1, mydata, sizeof(mydata)-1, 0);
    }
    
    // Schedule a timed job to run this task again at the given timestamp (absolute system time)
    os_setTimedCallback(j, os_getTime()+sec2osticks(20), do_send);       
}

/* ***************************************************** */
void setup() 
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("*** Starting LMiC ***");
  Serial.println();
  
  // LMIC init
  Serial.println();
  Serial.println("***  LMiC os_init ");
  os_init();
  
  // Reset the MAC state. Session and pending data transfers will be discarded.
  Serial.println();
  Serial.println("***  LMIC_reset ");
  LMIC_reset();
  
  // Set static session parameters. Instead of dynamically establishing a session 
  // by joining the network, precomputed session parameters are provided.
  Serial.println();
  Serial.println("***  LMIC_setSession ");
  LMIC_setSession (0x1, DEVADDR, (uint8_t*)DEVKEY, (uint8_t*)ARTKEY);
  
  // Disable data rate adaptation
  Serial.println();
  Serial.println("***  LMIC_setAdrMode ");
  LMIC_setAdrMode(0);
  
  // Disable link check validation
  Serial.println();
  Serial.println("***  LMIC_setLinkCheckMode ");
  LMIC_setLinkCheckMode(0);
  
  // Disable beacon tracking
  Serial.println();
  Serial.println("***  LMIC_disableTracking ");
  LMIC_disableTracking ();
  
  // Stop listening for downstream data (periodical reception)
  Serial.println();
  Serial.println("***  LMIC_stopPingable ");
  LMIC_stopPingable();
  
  // Set data rate and transmit power (note: txpow seems to be ignored by the library)
  Serial.println();
  Serial.println("***  LMIC_setDrTxpow ");
  LMIC_setDrTxpow(DR_SF7,20);

// for now only TX on ch 0 , disable all others
  Serial.println();
  Serial.println("***  LMIC_disableChannel ");
  for( u1_t i=1; i<64; i++ ) 
    {
      LMIC_disableChannel(i);
    }
  
  Serial.flush();
}

/* ***************************************************** */
void loop() 
{
  do_send(&sendjob);
  do_keepalive(&keepalivejob);
  
  while(1) 
    {
      os_runloop_once();
    }
}


#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>

#define ESP_I2C_ADR                0x41      // Address of ESP
#define I2C_SDAPIN                 D21
#define I2C_SCLPIN                 D22

/* PROTOTYPE OR FORWARD DECLARATION OF FUNCTION AND CLASSES */
typedef struct tagSubsRecMsg {
     char topic[20];
     char msg[75];
} SubsRecMsg;

typedef union {
     uint16_t  uiData[3];
     uint8_t   bytes[6];
} RFDataPacket;

typedef enum
{
     RF_READY, RF_BUSY, RF_EMPTY
} RFDataStatus;

class RFData
{
#define RFDATA_TIMEOUT        100L
public:
     RFData(void);                                               // Constructor
     RFDataPacket   receivedData;                                // Received data (raw) as 6 bytes packet
     void           receiving(uint8_t word);                     // Receiving procedure
     bool           returnData(char* buffer, uint8_t buffSize);  // Returning data and convert it into JSON so that it can be sent to broker
private:
     RFDataStatus   status;
     uint8_t        idx;
     unsigned long  time;
};

void setup_WiFi(void);
void callback(char* topic, byte* payload, unsigned int length);
void reconnect(void);

void onReceive(int len);

// SSID, Password, and brokerhost (MQTT)
const char*         ssid           = "ErliaChiranea";
const char*         pwd            = "RedVelvet0xFF";
const char*         brokerHost     = "test.mosquitto.org";
String              clientId       = "ErliaESP_0x41";

// Object declaration for Network communication via MQTT protocol
WiFiClient          ESPClient;
PubSubClient        client(ESPClient);

#define MSG_SIZE    75
char                msg[MSG_SIZE];
SubsRecMsg          subsMsg;
RFData              myRFData;

unsigned long timeCP;

void setup() {
     timeCP = 0;

     //pinMode(BUILTIN_LED, OUTPUT);
     Wire.onReceive(onReceive);
     Wire.begin((uint8_t)ESP_I2C_ADR);
     // WiFi and MQTT
     setup_WiFi();
     client.setServer(brokerHost, 1883);
     client.setCallback(callback);
}

void loop() {
     // put your main code here, to run repeatedly:
     if (!client.connected())
     {
          reconnect();
     }
     client.loop();

     unsigned long now = millis();
     if (now - timeCP > 200 && myRFData.returnData(msg, MSG_SIZE))
     {
          timeCP = now;
          // Send message via MQTT protocol
          client.publish("Erlia/TrashSkimmer", msg);
     }
}

/* DEFINITIONS OF FUNCTIONS AND CLASSES */
RFData::RFData(void)
{
     // Initialize all variables
     this->status   = RF_EMPTY;
     this->idx      = 0;
     this->time     = millis();
}

void RFData::receiving(uint8_t word)
{
     // Receive data from I2C
     switch (this->status)
     {
     case RF_EMPTY:
          this->status = RF_BUSY;
          this->receivedData.bytes[0] = word;
          this->idx = 1;
          break;
     case RF_BUSY:
          this->receivedData.bytes[this->idx] = word;
          this->idx ++;
          if (this->idx >= 6)
          {
               this->status = RF_READY;
          }
          break;
     case RF_READY:
          // New data has arrived before we take data before. Just 
          // change into BUSY status and proceed to receive data
          this->status = RF_BUSY;
          this->receivedData.bytes[0] = word;
          this->idx = 1;
          break;
     }
}

bool RFData::returnData(char* buffer, uint8_t buffSize)
{
     if (this->status == RF_READY)
     {
          snprintf(
               buffer,
               buffSize,
               "{\"total\":%u,\"lost\":%u,\"corrupted\":%u}",
               this->receivedData.uiData[0],
               this->receivedData.uiData[1],
               this->receivedData.uiData[2]
          );
          this->status = RF_EMPTY;
          return 1;
     } else {
          return 0;
     }
}


void setup_WiFi(void)
{
     delay(10);
     // Connecting to WiFi network
     WiFi.mode(WIFI_STA);
     WiFi.begin(ssid, pwd);
     // Poll until it is connected to the network
     while (WiFi.status() != WL_CONNECTED);    
     // In here, we are connected to the network
     randomSeed(micros());
}

void callback(char* topic, byte* payload, unsigned int length)
{
     // when there is a msg in which the topic is subscribed by ESP    
}

void reconnect(void)
{
     while(!client.connected())
     {
          if(client.connect(clientId.c_str()))
          {
               client.subscribe("Erlia/TrashSkimmer/Control");
          }
          delay(5000);
     }
}

void onReceive(int len)
{
     while(Wire.available())
     {
          uint8_t word = (uint8_t)Wire.read();
          myRFData.receiving(word);
     }
}
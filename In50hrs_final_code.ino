//#include <b64.h>
#include <HttpClient.h>
#include <LTask.h>
#include <LWiFi.h>
#include <LWiFiClient.h>
#include <LDateTime.h>

#define WIFI_AP "Indix-Event"
#define WIFI_PASSWORD "guest@123"
#define WIFI_AUTH LWIFI_WPA 

#define per 50
#define per1 3
#define DEVICEID "DAOb3LDY" // Input your deviceId
#define DEVICEKEY "UZ3aau7J593vcb00" // Input your deviceKey
#define SITE_URL "api.mediatek.com"
unsigned long previousMillis = 0;
const long interval = 15000;
int drip_motor = 4;
#include "DHT.h"
int LED_brightness = 0;
int roof_cooling = 5;
int fog_motor = 8;
int N = 9;
int P = 10;
int K = 11;
int light_trap = 6;
/*int drip_status;
int roof_cooling_status;
int fogger_status;
int n_status;
int p_status;
int k_status;
int light_trap_status;
int LED_status;*/

#define DHTPIN 2     // what pin we're connected to

#define DHTTYPE DHT22   // DHT 22  (AM2302)

DHT dht(DHTPIN, DHTTYPE);

LWiFiClient c;
unsigned int rtc;
unsigned int lrtc;
unsigned int rtc1;
unsigned int lrtc1;
char port[4]={0};
char connection_info[21]={0};
char ip[21]={0};             
int portnum;
int val = 0;
String tcpdata = String(DEVICEID) + "," + String(DEVICEKEY) + ",0";
String upload_led;
String tcpcmd_led_on = "LED_Control,1";
String tcpcmd_led_off = "LED_Control,0";

LWiFiClient c2;
HttpClient http(c2);



void setup()
{
  dht.begin();
  LTask.begin();
  LWiFi.begin();
  Serial.begin(115200);
  while(!Serial) delay(1000); /* comment out this line when Serial is not present, ie. run this demo without connect to PC */
pinMode(drip_motor,OUTPUT);
 pinMode(roof_cooling,OUTPUT);
 pinMode(fog_motor,OUTPUT);
 pinMode(N,OUTPUT);
 pinMode(P,OUTPUT);
 pinMode(K,OUTPUT);
 pinMode(light_trap,OUTPUT);
  Serial.println("Connecting to AP");
  while (0 == LWiFi.connect(WIFI_AP, LWiFiLoginInfo(WIFI_AUTH, WIFI_PASSWORD)))
  {
    delay(1000);
  }
  
  Serial.println("calling connection");

  while (!c2.connect(SITE_URL, 80))
  {
    Serial.println("Re-Connecting to WebSite");
    delay(1000);
  }
  delay(100);

  pinMode(13, OUTPUT);
  getconnectInfo();
  connectTCP();
}

void getconnectInfo(){
  //calling RESTful API to get TCP socket connection
  c2.print("GET /mcs/v2/devices/");
  c2.print(DEVICEID);
  c2.println("/connections.csv HTTP/1.1");
  c2.print("Host: ");
  c2.println(SITE_URL);
  c2.print("deviceKey: ");
  c2.println(DEVICEKEY);
  c2.println("Connection: close");
  c2.println();
  
  delay(500);

  int errorcount = 0;
  while (!c2.available())
  {
    Serial.println("waiting HTTP response: ");
    Serial.println(errorcount);
    errorcount += 1;
    if (errorcount > 10) {
      c2.stop();
      return;
    }
    delay(100);
  }
  int err = http.skipResponseHeaders();

  int bodyLen = http.contentLength();
  Serial.print("Content length is: ");
  Serial.println(bodyLen);
  Serial.println();
  char c;
  int ipcount = 0;
  int count = 0;
  int separater = 0;
  while (c2)
  {
    int v = c2.read();
    if (v != -1)
    {
      c = v;
      Serial.print(c);
      connection_info[ipcount]=c;
      if(c==',')
      separater=ipcount;
      ipcount++;    
    }
    else
    {
      Serial.println("no more content, disconnect");
      c2.stop();

    }
    
  }
  Serial.print("The connection info: ");
  Serial.println(connection_info);
  int i;
  for(i=0;i<separater;i++)
  {  ip[i]=connection_info[i];
  }
  int j=0;
  separater++;
  for(i=separater;i<21 && j<5;i++)
  {  port[j]=connection_info[i];
     j++;
  }
  Serial.println("The TCP Socket connection instructions:");
  Serial.print("IP: ");
  Serial.println(ip);
  Serial.print("Port: ");
  Serial.println(port);
  portnum = atoi (port);
  Serial.println(portnum);

} //getconnectInfo

void uploadstatus(){
  //calling RESTful API to upload datapoint to MCS to report LED status
  Serial.println("calling connection");
  LWiFiClient c2;  

  while (!c2.connect(SITE_URL, 80))
  {
    Serial.println("Re-Connecting to WebSite");
    delay(1000);
  }
  delay(100);
 // if(digitalRead(13)==1)
 // upload_led = "LED_Display,,1";
 // else

   int RoofStatus = 0;
   int val = analogRead(A0);
   int val_con = constrain (val, 0, 500);
  int ledLevel = map(val_con, 0, 500, 255, 0);
  
  analogWrite(3, ledLevel);
  if(ledLevel>200)
  {
    digitalWrite(light_trap,HIGH);
    delay(4000);
    digitalWrite(light_trap,LOW);
  }
  if(ledLevel<200)
  {
    digitalWrite(light_trap,LOW);
  }
  
  Serial.print("ledLevel = ");
  Serial.print(ledLevel);
  Serial.print("  ");
   float t = 0.0;
    float h = 0.0;
    if(dht.readHT(&t, &h))
    {
        Serial.println("------------------------------");
        Serial.print("temperature = ");
        Serial.println(t);
        
        Serial.print("humidity = ");
        Serial.println(h);
    }
    delay(2000);
    if(t>26 && h<62)
    {
     digitalWrite( fog_motor,HIGH);
     digitalWrite( roof_cooling,HIGH);
     Serial.print("Hot & Less Humid");
        Serial.print("  ");
    }
    if(t>26 && h>62)
    {
     digitalWrite( fog_motor,HIGH);
     digitalWrite( roof_cooling,LOW);
     Serial.print("Hot &  Humid");
        Serial.print("  ");
    }
    if(t<26 )
    {
     digitalWrite( fog_motor,LOW);
     digitalWrite( roof_cooling,LOW);
     Serial.print("Temp Under control");
        Serial.print("  ");
    }

    
 
  //upload_led = "";
  upload_led = "Humidity_display,,"+String(h);
  upload_led += "\nTemperature_display,,"+String(t);
  upload_led += "\nLight_sensor,,"+String(val);
  upload_led += "\nFogger_motor,,1";
  upload_led += "\nRoof_motor,,"+String(RoofStatus);
  //upload_led += "\nDrip_motor,,"+String(drip_status);
  int thislength = upload_led.length();
  HttpClient http(c2);
  c2.print("POST /mcs/v2/devices/");
  c2.print(DEVICEID);
  c2.println("/datapoints.csv HTTP/1.1");
  c2.print("Host: ");
  c2.println(SITE_URL);
  c2.print("deviceKey: ");
  c2.println(DEVICEKEY);
  c2.print("Content-Length: ");
  c2.println(thislength);
  c2.println("Content-Type: text/csv");
  c2.println("Connection: close");
  c2.println();
  c2.println(upload_led);
  
  //delay(500);

  int errorcount = 0;
  while (!c2.available())
  {
    Serial.print("waiting HTTP response: ");
    Serial.println(errorcount);
    errorcount += 1;
    if (errorcount > 10) {
      c2.stop();
      return;
    }
    delay(100);
  }
  int err = http.skipResponseHeaders();

  int bodyLen = http.contentLength();
  Serial.print("Content length is: ");
  Serial.println(bodyLen);
  Serial.println();
  while (c2)
  {
    int v = c2.read();
    if (v != -1)
    {
      Serial.print(char(v));
    }
    else
    {
      Serial.println("no more content, disconnect");
      c2.stop();

    }
    
  }
}



void connectTCP(){
  //establish TCP connection with TCP Server with designate IP and Port
  c.stop();
  Serial.println("Connecting to TCP");
  Serial.println(ip);
  Serial.println(portnum);
  while (0 == c.connect(ip, portnum))
  {
    Serial.println("Re-Connecting to TCP");    
    delay(1000);
  }  
  Serial.println("send TCP connect");
  c.println(tcpdata);
  c.println();
  Serial.println("waiting TCP response:");
} //connectTCP

void heartBeat(){
  Serial.println("send TCP heartBeat");
  c.println(tcpdata);
  c.println();
    
} //heartBeat

void loop()
{
  unsigned long currentMillis = millis();
 if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
      digitalWrite(drip_motor,HIGH);
     // drip_status =1;
      delay(2000);
      digitalWrite(N,HIGH);
      digitalWrite(P,HIGH);
      digitalWrite(K,HIGH);
  }
  delay(2000);
  digitalWrite(N,LOW);
      digitalWrite(P,LOW);
      digitalWrite(K,LOW);
      delay(2000);
  digitalWrite(drip_motor,LOW);
  //drip_status =0;
  
  //Check for TCP socket command from MCS Server 
  String tcpcmd="";
  while (c.available())
   {
      int v = c.read();
      if (v != -1)
      {
        Serial.print((char)v);
        tcpcmd += (char)v;
        if (tcpcmd.substring(40).equals(tcpcmd_led_on)){
          digitalWrite(13, HIGH);
          Serial.print("Switch LED ON ");
          tcpcmd="";
        }else if(tcpcmd.substring(40).equals(tcpcmd_led_off)){  
          digitalWrite(13, LOW);
          Serial.print("Switch LED OFF");
          tcpcmd="";
        }
      }
   }

  LDateTime.getRtc(&rtc);
  if ((rtc - lrtc) >= per) {
    heartBeat();
    lrtc = rtc;
  }
  //Check for report datapoint status interval
  LDateTime.getRtc(&rtc1);
  if ((rtc1 - lrtc1) >= per1) {
    uploadstatus();
    lrtc1 = rtc1;
  }
  
}

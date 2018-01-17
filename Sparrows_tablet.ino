#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <Servo.h> 
#include <TimeLib.h>
#include <WiFiUdp.h>

WiFiUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets
static const char ntpServerName[] = "us.pool.ntp.org";
const int timeZone = 3;     // Central European Time
const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets
time_t getNtpTime();
void sendNTPpacket(IPAddress &address);
time_t prevDisplay = 0; // when the digital clock was displayed

const char* ssid = "123";
const char* password = "321";

#define BOTtoken "12345"

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

const int Bot_mtbs = 300;
unsigned long Bot_lasttime;
const int getTime_mtbs = 60 * 1000;
unsigned long getTime_lasttime;

Servo myservo;
bool servoStatus = true;
bool isKeyboardOn = false;
const int SERVO_PIN = 2;

void handleNewMessages(int numNewMessages) 
{
  //Serial.println("handleNewMessages");
  //Serial.println(String(numNewMessages));

  for (int i = 0; i < numNewMessages; i++) 
  {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;
    text.toLowerCase();
    if(isKeyboardOn = false)
    {
      String keyboardJson = "[[\"On\", \"Off\"],[\"Status\"]]";
      bot.sendMessageWithReplyKeyboard(chat_id, "", "", keyboardJson, true);
      isKeyboardOn = true;
    }

    String from_name = bot.messages[i].from_name;
    if (from_name == "") from_name = "Guest";

    if (text == "/on" || text == "on") 
    {
      bot.sendMessage(chat_id, "Воробьев пришел!", "");
      turnOfServo(true);
    }
    else if (text == "/off" || text == "off") 
    {
      bot.sendMessage(chat_id, "Воробьев ушел :(", "");
      turnOfServo(false);
    }
    else if (text == "/status" || text == "status") 
    {
      if(servoStatus){
        bot.sendMessage(chat_id, "Воробьев на биофаке!!!", "");
      } 
      else 
      {
        bot.sendMessage(chat_id, "Воробьева нет. Совсем нет...", "");
      }
    }
    else if(text == "/keyboard" || text == "keyboard") {
      String keyboardJson = "[[\"On\", \"Off\"],[\"Status\"]]";
      bot.sendMessageWithReplyKeyboard(chat_id, "123", "", keyboardJson, true);
    }
    else if (text == "/start") {
      String welcome = "Команды:\n";
      welcome += "/on или on: Изменить статус Воробьева на \"Воробьев пришел\"\n";
      welcome += "/off или off: Изменить статус Воробьева на \"Воробьев ушел\"\n";
      welcome += "/status или status: Узнать есть ли Воробьев на факультете\n";
      welcome += "/keyboard или keyboard: Показать клавиатуру\n";
      bot.sendMessage(chat_id, welcome, "Markdown");
    }
    else
    {
      bot.sendMessage(chat_id, "Неизвестная команда...", "Markdown");
    }
  }
}

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) 
  {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  if( myservo.read() >= 130) servoStatus = true;
  else servoStatus = false;

//TIME
Udp.begin(localPort);
  //Serial.print("Local port: ");
  //Serial.println(Udp.localPort());
  //Serial.println("waiting for sync");
  setSyncProvider(getNtpTime);
  setSyncInterval(300);
}

void loop() 
{
  if (millis() > Bot_lasttime + Bot_mtbs)  
  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) 
    {
      //Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    Bot_lasttime = millis();
  }
  if (millis() > getTime_lasttime + getTime_mtbs)  
  {
    if (timeStatus() != timeNotSet) 
    {
      if (now() != prevDisplay) 
      {
        prevDisplay = now();
        if(hour() == 21 && (minute() > 0 && minute() < 5))
        {
          turnOfServo(false);
        }
      }
     }
     getTime_lasttime = millis();
  }
}

void turnOfServo(bool isOn)
{
  myservo.attach(SERVO_PIN);
  delay(50);
  if(isOn == true)
  {
    for(int i = myservo.read(); i <= 178; i += 1)
    {
      myservo.write(i);
      delay(15);
    } 
    servoStatus = true;
  }
  else if(isOn == false)
  {
    for(int i = myservo.read(); i >= 2; i -= 1)
    {                                
      myservo.write(i);
      delay(15);
    }
    servoStatus = false;
  }
  myservo.detach();
}
//TIME

time_t getNtpTime()
{
  IPAddress ntpServerIP; // NTP server's ip address

  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  //Serial.println("Transmit NTP Request");
  // get a random server from the pool
  WiFi.hostByName(ntpServerName, ntpServerIP);
  //Serial.print(ntpServerName);
  //Serial.print(": ");
  //Serial.println(ntpServerIP);
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  //Serial.println("No NTP Response :-(");
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

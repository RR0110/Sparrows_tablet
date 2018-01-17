#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <Servo.h> 

// Initialize Wifi connection to the router
const char* ssid = "123";
const char* password = "321";

// Initialize Telegram BOT
#define BOTtoken "333"

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

int Bot_mtbs = 500;
long Bot_lasttime;

Servo myservo;
bool servoStatus = true;
bool isKeyboardOn = false;

void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
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

    if (text == "/on" || text == "on") {
      bot.sendMessage(chat_id, "Воробьев пришел!", "");
      myservo.attach(2);
      myservo.write(180);
       delay(400);
       myservo.detach();
      servoStatus = true;
    }
    else if (text == "/off" || text == "off") {
      bot.sendMessage(chat_id, "Воробьев ушел :(", "");
      myservo.attach(2);
      myservo.write(0);
       delay(400);
       myservo.detach();
      servoStatus = false;
    }
    else if (text == "/status" || text == "status") {
      if(servoStatus){
        bot.sendMessage(chat_id, "Воробьев на биофаке!", "");
      } else {
        bot.sendMessage(chat_id, "Воробьева нет. Совсем нет.", "");
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

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  int angle = myservo.read();
  if(angle >= 150) servoStatus = true;
  else servoStatus = false;
}

void loop() {
  if (millis() > Bot_lasttime + Bot_mtbs)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    Bot_lasttime = millis();
  }
}

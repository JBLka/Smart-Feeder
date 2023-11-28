#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_camera.h"
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <ESP32Servo.h>

const char* ssid = "AndroidA";
const char* password = "chromech";

// Initialize Telegram BOT
String BOTtoken = "6979726247:AAEpus1xcMHLw3ysGZqr6yLZXYrjmv9JKMY";  // your Bot Token (Get from Botfather)

// Use @myidbot to find out the chat ID of an individual or a group
// Also note that you need to click "start" on a bot before it can
// message you

bool birdHere = false;

bool sendPhoto = false;
String CHAT_ID = "";
String BIRD = "";

float distanceCm;

WiFiClientSecure clientTCP;
UniversalTelegramBot bot(BOTtoken, clientTCP);

#define FLASH_LED_PIN 4
bool flashState = LOW;

//Checks for new messages every 1 second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

#define PWDN_GPIO_NUM    -1
#define RESET_GPIO_NUM   -1
#define XCLK_GPIO_NUM    21
#define SIOD_GPIO_NUM    26
#define SIOC_GPIO_NUM    27

#define Y9_GPIO_NUM      35
#define Y8_GPIO_NUM      34
#define Y7_GPIO_NUM      39
#define Y6_GPIO_NUM      36
#define Y5_GPIO_NUM      19
#define Y4_GPIO_NUM      18
#define Y3_GPIO_NUM       5
#define Y2_GPIO_NUM       4
#define VSYNC_GPIO_NUM   25
#define HREF_GPIO_NUM    23
#define PCLK_GPIO_NUM    22

#define TRIG_PIN         33
#define ECHO_PIN         32

Servo SERVO_DOWN;
Servo SERVO_LEFT;
Servo SERVO_RIGHT;

void configInitCamera(){
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode = CAMERA_GRAB_LATEST;

  //init with high specs to pre-allocate larger buffers
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;  //0-63 lower number means higher quality
    config.fb_count = 1;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;  //0-63 lower number means higher quality
    config.fb_count = 1;
  }
  
  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    delay(1000);
    ESP.restart();
  }
}

void handleNewMessages(int numNewMessages) {
  Serial.print("Handle New Messages: ");
  Serial.println(numNewMessages);

  for (int i = 0; i < numNewMessages; i++) {
    CHAT_ID = String(bot.messages[i].chat_id);
    
    // Print the received message
    String text = bot.messages[i].text;
    Serial.println(text);
    
    String from_name = bot.messages[i].from_name;
    if (text == "/start") {
      String welcome = "Привет, " + from_name + "! \n";
      welcome += "Вот команды, которые сейчас доступны: \n";
      welcome += "/photo : сделать фото\n";
      welcome += "/vorobey : 'прилетел воробей' \n";
      welcome += "/sinica : 'прилетела синица' \n";
      welcome += "/end : 'птица улетела' \n";
      bot.sendMessage(CHAT_ID, welcome, "");
    }
    if (text == "Не, ну мне лично кажется, что это Воробей" or text == "Не, ну мне лично кажется, что это Соловей") {
      birdHere = true;
      SERVO_LEFT.write(90);
      delay(300);
      SERVO_LEFT.write(0);
    }
    if (text == "Не, ну мне лично кажется, что это Синица") {
      birdHere = true;
      SERVO_RIGHT.write(90);
      delay(300);
      SERVO_RIGHT.write(0);
    }
    if (text == "/chat_id") {
      bot.sendMessage(CHAT_ID, CHAT_ID, "");
    }
    if (text == "/photo") {
      sendPhoto = true;
      Serial.println("New photo request");
    }
    if (text == "/vorobey") {
//      bot.sendMessage("-1002112431587", "Представим, левый бак открылся", "");
      SERVO_LEFT.write(90);
      delay(300);
      SERVO_LEFT.write(0);
      BIRD = "Воробей";
      sendPhoto = true;
      Serial.println("New photo request");
    }
    if (text == "/sinica") {
//      bot.sendMessage("-1002112431587", "Представим, правый бак открылся", "");
      SERVO_RIGHT.write(90);
      delay(300);
      SERVO_RIGHT.write(0);
      BIRD = "Синица";
      sendPhoto = true;
      Serial.println("New photo request");
    }
    if (text == "/end") {
//      bot.sendMessage("-1002112431587", "Представим, дно покрутилось", "");
      SERVO_DOWN.write(90);
      delay(500);
      SERVO_DOWN.write(0);
    }
  }
}

void setup(){
  SERVO_DOWN.attach(13);
  SERVO_LEFT.attach(12);
  SERVO_RIGHT.attach(14);

  SERVO_DOWN.write(0);
  SERVO_LEFT.write(0);
  SERVO_RIGHT.write(0);

  pinMode(TRIG_PIN, OUTPUT); // Sets the trigPin as an Output
  pinMode(ECHO_PIN, INPUT); // Sets the echoPin as an Input
  
  String CHAT_ID = "";
  bool sendPhoto = false;
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); 
  // Init Serial Monitor
  Serial.begin(115200);

  // Set LED Flash as output
  pinMode(FLASH_LED_PIN, OUTPUT);
  digitalWrite(FLASH_LED_PIN, flashState);

  // Config and init the camera
  configInitCamera();

  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  clientTCP.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("ESP32-CAM IP Address: ");
  Serial.println(WiFi.localIP()); 
}

void loop() {
  digitalWrite(TRIG_PIN, LOW);
  delay(2);
  digitalWrite(TRIG_PIN, HIGH);
  delay(10);
  digitalWrite(TRIG_PIN, LOW);
  distanceCm = pulseIn(ECHO_PIN, HIGH) * 0.034/2;
  if (distanceCm < 15 and not birdHere) {
    Serial.println(distanceCm);
    bot.sendMessage("-1002045637456", String(distanceCm), "");
    sendPhotoTelegram("-1002045637456");
  } else if (distanceCm >= 15 and birdHere){
    Serial.println(distanceCm);
    bot.sendMessage("-1002045637456", String(distanceCm), "");
    SERVO_DOWN.write(90);
    delay(500);
    SERVO_DOWN.write(0);
    birdHere = false;
  }
  if (sendPhoto) {
    Serial.println("Preparing photo");
    if (BIRD != ""){
      sendPhotoTelegram("-1002112431587"); 
    } else {
      sendPhotoTelegram(CHAT_ID); 
    }
    sendPhoto = false; 
  }
  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
//  SERVO_DOWN.write(0);
//  delay(1000);
//  SERVO_DOWN.write(90);
//  delay(1000);
}

String sendPhotoTelegram(String CHAT_ID) {
  const char* myDomain = "api.telegram.org";
  String getAll = "";
  String getBody = "";

  //Dispose first picture because of bad quality
  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();
  esp_camera_fb_return(fb); // dispose the buffered image
  
  // Take a new photo
  fb = NULL;  
  fb = esp_camera_fb_get();  
  if(!fb) {
    Serial.println("Camera capture failed");
    delay(1000);
    ESP.restart();
    return "Camera capture failed";
  }  
  
  Serial.println("Connect to " + String(myDomain));


  if (clientTCP.connect(myDomain, 443)) {
    Serial.println("Connection successful");
    
    String head = "--RandomNerdTutorials\r\nContent-Disposition: form-data; name=\"chat_id\"; \r\n\r\n" + CHAT_ID + "\r\n--RandomNerdTutorials\r\nContent-Disposition: form-data; name=\"photo\"; filename=\"esp32-cam.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
    String tail = "\r\n--RandomNerdTutorials--\r\n";

    size_t imageLen = fb->len;
    size_t extraLen = head.length() + tail.length();
    size_t totalLen = imageLen + extraLen;
  
    clientTCP.println("POST /bot"+BOTtoken+"/sendPhoto HTTP/1.1");
    clientTCP.println("Host: " + String(myDomain));
    clientTCP.println("Content-Length: " + String(totalLen));
    clientTCP.println("Content-Type: multipart/form-data; boundary=RandomNerdTutorials");
    clientTCP.println();
    clientTCP.print(head);
  
    uint8_t *fbBuf = fb->buf;
    size_t fbLen = fb->len;
    for (size_t n=0;n<fbLen;n=n+1024) {
      if (n+1024<fbLen) {
        clientTCP.write(fbBuf, 1024);
        fbBuf += 1024;
      }
      else if (fbLen%1024>0) {
        size_t remainder = fbLen%1024;
        clientTCP.write(fbBuf, remainder);
      }
    }  
    
    clientTCP.print(tail);
    
    esp_camera_fb_return(fb);
    
    int waitTime = 10000;   // timeout 10 seconds
    long startTimer = millis();
    boolean state = false;
    
    while ((startTimer + waitTime) > millis()){
      Serial.print(".");
      delay(100);      
      while (clientTCP.available()) {
        char c = clientTCP.read();
        if (state==true) getBody += String(c);        
        if (c == '\n') {
          if (getAll.length()==0) state=true; 
          getAll = "";
        } 
        else if (c != '\r')
          getAll += String(c);
        startTimer = millis();
      }
      if (getBody.length()>0) break;
    }
    clientTCP.stop();
    Serial.println(getBody);
  }
  else {
    getBody="Connected to api.telegram.org failed.";
    Serial.println("Connected to api.telegram.org failed.");
  }
  if (BIRD != ""){
    bot.sendMessage(CHAT_ID, BIRD, "");
    BIRD = "";
  }
  return getBody;
}

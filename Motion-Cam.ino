// 
// I leave anyone that find this useful the ultimate freedom of using, changing, alter, missuse as they please
// I don't promise it will work, neither do I promise that your equipment will work after trying it.
// 

#include "esp_camera.h"
#include <FS.h>
#include "SD_MMC.h"
#include "EEPROM.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

// CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27
#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22
#define motionPin 3
const char* ssid = "YourAPHere"; // Note!! This is where you put your AP
const char* password = "YourPWDHere";// Note!! This is where you put your PWD! 

AsyncWebServer server(80); // If this address doesn't fit you, by all means chose another! But you have to remeber the port to be able to check on the pictures!
String fileNames[520]; // Just so I can keep the file names in an array
int fileCount = 0; // Counter is a counter is a counter. 
// All this you can find everywhere, I was a bit stupid and spent hours on reading when it was readily available on all places.
void configCamera() {
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
  config.frame_size = FRAMESIZE_UXGA;
  config.jpeg_quality = 10;
  config.fb_count = 2;
  esp_err_t err = esp_camera_init(&config);

  sensor_t* s = esp_camera_sensor_get();
  s->set_brightness(s, 0);
  s->set_contrast(s, 0);
  s->set_saturation(s, 0);
  s->set_special_effect(s, 0);
  s->set_whitebal(s, 1);
  s->set_awb_gain(s, 1);
  s->set_wb_mode(s, 0);
  s->set_exposure_ctrl(s, 1);
  s->set_aec2(s, 0);
  s->set_ae_level(s, 0);
  s->set_aec_value(s, 300);
  s->set_gain_ctrl(s, 1);
  s->set_agc_gain(s, 0);
  s->set_gainceiling(s, (gainceiling_t)0);
  s->set_bpc(s, 0);
  s->set_wpc(s, 1);
  s->set_raw_gma(s, 1);
  s->set_lenc(s, 1);
  s->set_hmirror(s, 0);
  s->set_vflip(s, 0);
  s->set_dcw(s, 1);
  s->set_colorbar(s, 0);
}

unsigned int incCounter() {
  unsigned int cnt = 0;
  EEPROM.get(0, cnt);
  EEPROM.put(0, cnt + 1);
  EEPROM.commit();
  fileCount = 0;
  return cnt;
}
//Found this on the Internet I haven't got a clue what it does but it seems important to use?? Why?? More reading to do!
void skipPictures(int n) {
  for(int i=0; i<n; i++) {
    camera_fb_t* fb = esp_camera_fb_get();
    esp_camera_fb_return(fb);
  }
}

void takePicture() {
  camera_fb_t* fb = esp_camera_fb_get();
  unsigned int cnt = incCounter();
  String path = "/img" + String(cnt) + ".jpg";
  File file = SD_MMC.open(path.c_str(), FILE_WRITE);
  if(!file){
    Serial.println("Failed to open file in writing mode");
    Serial.println("Path: " + path);
    Serial.println("Is the file already open?");
    Serial.println("Is the SD card full?");
    Serial.println("Is the filename format correct?");
    return;
  }else{
  file.write(fb->buf, fb->len);
  Serial.println("Filen skriven");  
  }
  file.close();
  esp_camera_fb_return(fb);
  delay(100);
  updateFileList();// Update the list so that the webpage is up to date
  delay(500);
}

void setupWiFi(){
  WiFi.persistent(false); // Not really needed but I forgot to remove it so it just as well stay. (was testing with another webserver library that needed it)
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  WiFi.setSleep(false);
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println("Kopplad mot nätet"); //Connected
  Serial.print("Adress: ");// Your address to connect to the server
  Serial.println(WiFi.localIP());//The numbers!
}
void setupCardReader() {
  if (!SD_MMC.begin()) {
    Serial.println("\nKortfel!");// Something wrong
    return;
  }
  Serial.println("\nKort initialiserat");
  File root = SD_MMC.open("/");
  if (!root) {
    Serial.println("Kunde inte öppna /!");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    String fileName = file.name();
    fileNames[fileCount++] = fileName; // Lägg till filnamnet i arrayen
    file = root.openNextFile();
  }
  if(!SD_MMC.exists("/oldpictures")){
    SD_MMC.mkdir("/oldpictures");
  }
}

void updateFileList(){
  fileCount = 1;
  File root = SD_MMC.open("/");
  if (!root) {
    Serial.println("Kunde inte öppna /!");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    String fileName = file.name();
    fileNames[fileCount++] = fileName; // Lägg till filnamnet i arrayen
    if (fileCount >= 511){
      moveJpgToOldPictures(SD_MMC,"/");
      fileCount = 1;
    }
    file = root.openNextFile();
  }
}
void moveJpgToOldPictures(fs::FS &fs, const char * dirname){
    File root = fs.open(dirname);
    if(!root){
        Serial.println("Failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(!file.isDirectory()){
            String fileName = file.name();
            if(fileName.endsWith(".jpg")){
                String newFileName = "/oldpictures/" + fileName;
                fs.rename(file.name(), newFileName.c_str());
                Serial.println("Moved file: " + fileName + " to " + newFileName);
            }
        }
        file = root.openNextFile();
    }
    resetEEPROM();
}
void resetEEPROM(){
  unsigned int resetValue = 0;
    EEPROM.put(0, resetValue);
    EEPROM.commit(); // Skriv ändringarna permanent till EEPROM.

    Serial.println("Räknaren har nollställts.");
}
void setup() {
  Serial.begin(115200);
  Serial.println("\n");
  pinMode(motionPin, INPUT/*_PULLUP*/);
  digitalWrite(motionPin, LOW);
  setupWiFi();
  setupCardReader();
  EEPROM.begin(9);
  SD_MMC.begin();
  configCamera();
  skipPictures(10);
  takePicture();
  Serial.println("Startar Server");
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    Serial.println("/");
    Serial.println(request->url());
    String html = "<html><head><meta http-equiv='refresh' content='15'>";
      html += "</head><body><h1>Fil Lista:</h1><ul>";
    for (int i = 0; i < fileCount; i++) {
      html += "<li><a href='/";
      html += fileNames[i].substring(fileNames[i].lastIndexOf('/') + 1); // Endast filnamn
      html += "'>";
      html += fileNames[i].substring(fileNames[i].lastIndexOf('/') + 1); // Endast filnamn
      html += "</a></li>";
    }
    html += "</ul></body></html>";
    request->send(200, "text/html", html);
  });
  
  server.on("/*", HTTP_GET, [](AsyncWebServerRequest* request) {
    String filePath = "/" + request->url().substring(request->url().lastIndexOf('/') + 1);
    Serial.print("Försöker öppna fil: ");
    Serial.println(filePath);
    File file = SD_MMC.open(filePath);
    if (file) {
      request->send(SD_MMC, filePath, "image/jpeg"); // Skicka filen
    } else {
      request->send(404, "text/plain", "Filen hittades inte");
    }
    file.close();
  });
  server.begin();
  Serial.print("IP adress: " );
  Serial.println(WiFi.localIP());
}

void loop() {
  int reading = digitalRead(motionPin);
  //Serial.println(reading);
  if (reading == HIGH){
    //Serial.println("Hög");
    takePicture();
    delay(2000);
  }
  /*if (reading == LOW){
    //Serial.println("Låg");
  }*/
  delay(1000);
}

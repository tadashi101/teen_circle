#include <ArduinoJson.h>
#include <SPI.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include "time.h"
#include "freertos/task.h"
#include "esp_task_wdt.h"
#include "math.h"

#define LED_DRIVER_NUM (8)
#define CHANNEL_NUM (16)
#define OE_PIN    (17)

//led driver has 16 channels output for led array
//min : index from 0 to 7
//hour : 8 to 15
uint8_t led_data[LED_DRIVER_NUM*2] = {0};
uint8_t mintable[8] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};
int brightness = 0;

AsyncWebServer server(80);
const char *ap_ssid = ""; //ESP32 softAP SSID
const char *ap_pass = ""; //ESP32 softAP password
const IPAddress ip(192, 168, 4, 1);      //
const IPAddress subnet(255, 255, 255, 0); //

const char *sta_ssid = ""; //ESP32 STA SSID
const char *sta_pass = ""; //ESP32 STA password

// NTP
const char* ntpServer =  "ntp.jst.mfeed.ad.jp";
const long  gmtOffset_sec = 9 * 3600;
const int   daylightOffset_sec = 0;

TaskHandle_t taskHandle1;

enum ACTION_MODE {
  CLOCK,
  DEMO1,
  DEMO2,
  DEMO3,
};

enum ACTION_MODE action_mode = CLOCK;

void onBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
  //Handle body
  StaticJsonBuffer<200> jsonBuffer;
  String url = request->url();
  Serial.println(url);
  
    if (url == "/setting") {
        Serial.printf("setting\r\n");
        JsonObject& root = jsonBuffer.parseObject(data);
    
        // パースが成功したか確認。できなきゃ終了
        if (!root.success()) {
          Serial.println("parseObject() failed");
        }

        struct tm tmset;
        tmset.tm_year = root["year"];
        tmset.tm_mon = root["month"];
        tmset.tm_mday = root["day"];
        tmset.tm_hour = root["hour"];
        tmset.tm_min = root["min"];
        tmset.tm_sec = root["sec"];

        Serial.printf("year: %d\n", tmset.tm_year);
        Serial.printf("mon: %d\n", tmset.tm_mon);
        Serial.printf("day: %d\n", tmset.tm_mday);
        Serial.printf("hour: %d\n", tmset.tm_hour);
        Serial.printf("min: %d\n", tmset.tm_min);
        Serial.printf("sec: %d\n", tmset.tm_sec);

        tmset.tm_year -= 1900;
        tmset.tm_mon  -= 1;

        time_t elap_time = mktime(&tmset);
        struct timeval tv;
        tv.tv_sec = elap_time;
        tv.tv_usec = 0;

        struct timezone tz;
        //setenv("TZ", time_zone, 1);
        //tzset();
        tz.tz_minuteswest = 0;
        tz.tz_dsttime = 0;

        settimeofday(&tv, &tz);
        
        Serial.printf("settime=%ld\r\n", elap_time);
        
    }else if(url == "/mode"){
      Serial.printf("mode\r\n");
      //remove count for null string
      if(strncmp("CLOCK",(char*)data,sizeof("CLOCK")-1)==0){
        action_mode = CLOCK;
        Serial.printf("clock_mode\r\n");
      }else if(strncmp("DEMO1",(char*)data,sizeof("DEMO1")-1)==0){
        action_mode = DEMO1;
        Serial.printf("demo1\r\n");
      }else if(strncmp("DEMO2",(char*)data,sizeof("DEMO2")-1)==0){
        action_mode = DEMO2;
        Serial.printf("demo2\r\n");
      }else if(strncmp("DEMO3",(char*)data,sizeof("DEMO3")-1)==0){
        action_mode = DEMO3;
        Serial.printf("demo3\r\n"); 
      }
      
    }else if(url == "/light"){
      Serial.printf("light\r\n");
      JsonObject& root_1 = jsonBuffer.parseObject(data);
      
         // パースが成功したか確認。できなきゃ終了
        if (!root_1.success()) {
          Serial.println("parseObject() failed");
        }
        brightness = root_1["light"];

        set_brightness(brightness);
        
      Serial.printf("light: %d\n", brightness);
    }
  
  Serial.printf("Bodylen = %d\r\n", len);

  request->send(200, "text/html", "");
}

void IRAM_ATTR clear_led_data(){
    int j=0;
    
        for(j=0;j<LED_DRIVER_NUM*2;j++){
          led_data[j] = 0x00;
        }
}

void IRAM_ATTR fill_led_data(){
    int j=0;
    
        for(j=0;j<LED_DRIVER_NUM*2;j++){
          led_data[j] = 0xff;
        }
}

void set_brightness(int brightness_num){
  ledcWrite(1, 255-brightness_num);
}

void setup() {
  // put your setup code here, to run once:

  pinMode(OE_PIN, OUTPUT);
  digitalWrite(OE_PIN, LOW);

  SPI.begin();
  SPI.setDataMode(SPI_MODE0);
  SPI.setHwCs(true);
  
  Serial.begin(115200);
  delay(100);
  SPI.writeBytes(led_data,sizeof(led_data));
  delay(500);

  startup_pattern();

  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(ap_ssid, ap_pass);           // SSID
  delay(100);                        //
  WiFi.softAPConfig(ip, ip, subnet); //

  IPAddress myIP = WiFi.softAPIP();  // WiFi.softAPIP()

  delay(500);

  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }

  server.serveStatic("/", SPIFFS, "/www").setDefaultFile("default.html");
  server.onRequestBody(onBody);

  server.begin();

  //init pwm for brightness
  ledcAttachPin(12, 1); // assign RGB led pins to channels
  ledcSetup(1, 12000, 8); // 12 kHz PWM, 8-bit resolution

  set_brightness(255);
  

    //1秒毎のtask生成
  xTaskCreatePinnedToCore(one_sec_task, "task1", 4096 * 3, NULL, 1, &taskHandle1, 1);
  
  // connect to WiFi
  Serial.printf("Connecting to %s ", sta_ssid);
  WiFi.begin(sta_ssid, sta_pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" CONNECTED");
  
  // NTP サーバを設定
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

}

void loop() {
  demo_led_pattern(action_mode);

}

void IRAM_ATTR set_min_led_data(int input_min,uint8_t *data){

  int quotient = 0;
  int rest = 0;
  //
  quotient = input_min/8;
  rest = input_min%8;


  data[7-quotient] = mintable[rest];

  
}

void IRAM_ATTR set_hour_led_data(int input_hour,int input_min,uint8_t *data){
  // hour : data[8 to 15]
  unsigned int bitshift = 0;
  int i = 0;
  int quotient = 0,quotient_min = 0;
  int rest = 0;

  //hourからどのLEDを付けるか計算
  if(input_hour > 0){
    
    if(input_hour < 12){
      bitshift = (input_hour)*5 ;

    }else if(input_hour == 12){
      bitshift = 0;
      
    }else{
      bitshift = (input_hour-12)*5 ;
      
    }
    
  }

  //minから12分毎に1個LEDの列を進める
  quotient_min = input_min/12;

  bitshift = bitshift + quotient_min;
  
  quotient = bitshift/8;
  rest = bitshift%8;
  
      data[15-quotient] = 1 << rest;

      //12時までは時針の該当ビットまでをすべて1に
      if(input_hour < 12){
        //該当バイトまで全てLED点灯
        for(i=15;i>15-quotient;i--){
          data[i] = 0xff;
        }

        //時針のバイトにおいて、点灯させてるLED列以下のビットを全て1に
        for(i=0;i<rest;i++){
          data[15-quotient] += pow(2,i);
        }

      //12以降は時針のビット以降をすべて１に
      }else{
        
         //該当バイト以降全てLED点灯
        for(i=8;i<15-quotient;i++){
          data[i] = 0xff;
        }
        
        //時針のバイトにおいて、点灯させてるLED列以降のビットを全て1に
        for(i=7;i>rest;i--){
          data[15-quotient] += pow(2,i);
        }
        
        
      }

}

void one_sec_task(void *arg){
    portTickType xLastWakeTime;
 xLastWakeTime = xTaskGetTickCount();
  const portTickType xDelay = 1000 / portTICK_RATE_MS; // 1000ms

  while (1) {
    vTaskDelayUntil(&xLastWakeTime, xDelay/portTICK_RATE_MS);

      struct tm date_now;
      getLocalTime(&date_now);
    
        if(action_mode == CLOCK){
  
            clear_led_data();
            set_min_led_data(date_now.tm_min, led_data);
            set_hour_led_data(date_now.tm_hour, date_now.tm_min, led_data);
            SPI.writeBytes(led_data,sizeof(led_data));
            
        }
  }
}


void startup_pattern(){
  int i = LED_DRIVER_NUM*2-1;
  int j=0;
  int k=0;
  int l=0;
  uint8_t send_data = 0x01;

  for(l=0;l<(LED_DRIVER_NUM*CHANNEL_NUM);l++){
  
        for(j=0;j<16;j++){
          led_data[j] = 0x00;
        }
        
      led_data[i] =  send_data;
      
      SPI.writeBytes(led_data,sizeof(led_data));
    
      delay(30);
      
      send_data = (send_data << 1);
    
      if(send_data == 0x00){
        send_data = 0x01;
      }
    
      k= k+1;
      if(k ==8){
        i = i -1;
        k=0;
      }
      if(i < 0){
        i = LED_DRIVER_NUM*2-1;
        
          clear_led_data();
        
        SPI.writeBytes(led_data,sizeof(led_data));
    
        delay(50);
      }
  }

}

void demo_led_pattern(ACTION_MODE now_mode){
  
  if(now_mode == CLOCK){
    return;
  }
  
  if(now_mode == DEMO1){
    demo_pattern1();
    
  }else if(now_mode == DEMO2){
    demo_pattern2();
  }else if(now_mode == DEMO3){
    demo_pattern3();
  }
  
}

void demo_pattern1(){
  static int i = LED_DRIVER_NUM*2-1;
  static int j=0;
  static int k=0;
  static uint8_t send_data = 0x01;

    clear_led_data();

    led_data[i] =  send_data;
    
    SPI.writeBytes(led_data,sizeof(led_data));
    delay(20);
    
    send_data = (send_data << 1);

      if(send_data == 0x00){
        send_data = 0x01;
      }

      k= k+1;
      if(k ==8){
        i = i -1;
        k=0;
      }

      if(i < 0){
        i = LED_DRIVER_NUM*2-1;
      }
}

void demo_pattern2(){
  static int i = LED_DRIVER_NUM*2-1;
  static int j=0;
  static int k=0;
  static uint8_t send_data = 0x01;

    led_data[i] =  led_data[i] + send_data;
    
    SPI.writeBytes(led_data,sizeof(led_data));
    delay(20);
    
    send_data = (send_data << 1);

      if(send_data == 0x00){
        send_data = 0x01;
      }

      k= k+1;
      if(k ==8){
        i = i -1;
        k=0;
      }

      if(i < 0){
        i = LED_DRIVER_NUM*2-1;
        clear_led_data();
      }
  
}

void demo_pattern3(){
  static int b = 0;
  static int flg = 0;

  if(flg == 0){
    b = b + 5;
  }else{
    b = b - 5;
  }

  if(b > 255){
    flg = 1;
    b = 255;
  }else if(b < 0){
    flg = 0;
    b=0;
  }

  ledcWrite(1, b);
  
  
  static int i = LED_DRIVER_NUM*2-1;
  static int j=0;
  static int k=0;
  static uint8_t send_data = 0x01;

    if(j==0){
      led_data[i] =  led_data[i] + send_data;
    }else{
      led_data[i] =  led_data[i] - send_data;
    }
    
    SPI.writeBytes(led_data,sizeof(led_data));
    delay(20);


      send_data = (send_data << 1);
    

      if(send_data == 0x00){
        send_data = 0x01;
      }

      k= k+1;
      if(k ==8){
        i = i -1;
        k=0;
      }

      if(i < 0){
        i = LED_DRIVER_NUM*2-1;
        j = ~j;
        if(j==0){
          clear_led_data();
        }else{
          fill_led_data();
        }
        send_data = 0x01;
      }
      
  
}


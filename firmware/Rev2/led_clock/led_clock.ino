#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager

// wifimanager can run in a blocking mode or a non blocking mode
// Be sure to know how to process loops with no delay() if using non blocking
bool wm_nonblocking = true; // change to true to use non blocking

WiFiManager wm; // global wm instance
WiFiManagerParameter custom_field; // global param ( for non blocking w params )

char ap_ssid[20] = "teencircle"; 

#include <math.h>
#include <time.h>

#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include <SPI.h>
#include <SPIFFS.h>
#include <WiFi.h>

#include <freertos/task.h>
#include <esp_task_wdt.h>

#define LED_DRIVER_NUM (8)
#define CHANNEL_NUM (16)
#define OE_PIN (17)

//led driver has 16 channels output for led array
//min : index from 0 to 7
//hour : 8 to 15
static uint8_t led_data[LED_DRIVER_NUM * 2] = { 0 };
const uint8_t mintable[8] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };
static int brightness = 128;

// NTP
const char* ntpServer = "ntp.jst.mfeed.ad.jp";
const long gmtOffset_sec = 9 * 3600;
const int daylightOffset_sec = 0;

TaskHandle_t taskHandle1;

enum ACTION_MODE {
    CLOCK,
    DEMO1,
    DEMO2,
    DEMO3,
};

enum ACTION_MODE action_mode = CLOCK;

void onBody(String url)
{
    //Handle body
#if ARDUINOJSON_VERSION_MAJOR >= 7
    JsonDocument root;
#else
    StaticJsonBuffer<200> jsonBuffer;
#endif
    String data = wm.server->arg("plain");
    uint8_t len = sizeof(data); 
    if (url == "/setting") {
        Serial.printf("setting\r\n");
#if ARDUINOJSON_VERSION_MAJOR >= 7
        DeserializationError error = deserializeJson(root, data);
        // パースが成功したか確認、できなきゃ終る
        if (error) {
#else        
        JsonObject& root = jsonBuffer.parseObject(data);

        // パースが成功したか確認。できなきゃ終了
        if (!root.success()) {
#endif            
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
        tmset.tm_mon -= 1;

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

    } else if (url == "/mode") {
        Serial.printf("mode\r\n");
        //remove count for null string
        if (strncmp("CLOCK", data.c_str(), sizeof("CLOCK") - 1) == 0) {
            action_mode = CLOCK;
            Serial.printf("clock_mode\r\n");
        } else if (strncmp("DEMO1", data.c_str(), sizeof("DEMO1") - 1) == 0) {
            action_mode = DEMO1;
            Serial.printf("demo1\r\n");
        } else if (strncmp("DEMO2", data.c_str(), sizeof("DEMO2") - 1) == 0) {
            action_mode = DEMO2;
            Serial.printf("demo2\r\n");
        } else if (strncmp("DEMO3", data.c_str(), sizeof("DEMO3") - 1) == 0) {
            action_mode = DEMO3;
            Serial.printf("demo3\r\n");
        }

    } else if (url == "/light") {
        Serial.printf("light\r\n");
#if ARDUINOJSON_VERSION_MAJOR >= 7
        JsonDocument root_1;
        DeserializationError error = deserializeJson(root_1, data);
        // パースが成功したか確認、できなきゃ終る
        if (error) {
#else
        JsonObject& root_1 = jsonBuffer.parseObject(data);

        // パースが成功したか確認。できなきゃ終了
        if (!root_1.success()) {
#endif
            Serial.println("parseObject() failed");
        }
        brightness = root_1["light"];

        set_brightness(brightness);
        write_brightness(brightness); 

        Serial.printf("light: %d\n", brightness);
    }

    Serial.printf("Bodylen = %d\r\n", len);

    wm.server->send(200, "text/html", "OK");
}

void bindServerCallback(){
    wm.server->on("/setting", HTTP_POST, [](){onBody("/setting");}); 
    wm.server->on("/mode", HTTP_POST, [](){onBody("/mode");}); 
}

void saveParamCallback(){
    if(wm.server->hasArg("light")) {
        brightness = wm.server->arg("light").toInt(); 
        set_brightness(brightness);
        write_brightness(brightness);
    } 
}

void IRAM_ATTR clear_led_data()
{
    int j = 0;

    for (j = 0; j < LED_DRIVER_NUM * 2; j++) {
        led_data[j] = 0x00;
    }
}

void IRAM_ATTR fill_led_data()
{
    int j = 0;

    for (j = 0; j < LED_DRIVER_NUM * 2; j++) {
        led_data[j] = 0xff;
    }
}

void set_brightness(int brightness_num)
{
    ledcWrite(1, 255 - brightness_num);
}

void write_brightness(int brightness_num)
{
    File fp = SPIFFS.open("/brightness.txt", "w"); 
    if (!fp) {
        Serial.printf("File Open Error\r\n"); 
    }
    else {
        //Serial.printf("Write File: brightness.txt = %d\r\n", brightness_num); 
        fp.print(brightness_num); 
        fp.close(); 
    }
}

uint8_t read_brightness() {
    uint8_t val = 128;
    if (SPIFFS.exists("/brightness.txt")) {
        File fp = SPIFFS.open("/brightness.txt", "r"); 
        if (!fp) {
            Serial.printf("File Open Error\r\n"); 
        }
        else {
            val = fp.readString().toInt(); 
            //Serial.printf("Read File: brightness.txt = %d\r\n", val); 
            fp.close(); 
        }
    }
    return val; 
}

static void setupArduinoOTA(void)
{
    // Port defaults to 3232
    // ArduinoOTA.setPort(3232);

    // Hostname defaults to esp3232-[MAC]
    ArduinoOTA.setHostname("TeenCircle");

    // No authentication by default
    // ArduinoOTA.setPassword("admin");

    // Password can be set with it's md5 value as well
    // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
    // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

    ArduinoOTA
        .onStart([]() {
            String type;
            if (ArduinoOTA.getCommand() == U_FLASH)
                type = "sketch";
            else // U_SPIFFS
                type = "filesystem";

            // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
            Serial.println("Start updating " + type);
        })
        .onEnd([]() {
            Serial.println("\nEnd");
        })
        .onProgress([](unsigned int progress, unsigned int total) {
            Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
        })
        .onError([](ota_error_t error) {
            Serial.printf("Error[%u]: ", error);
            if (error == OTA_AUTH_ERROR)
                Serial.println("Auth Failed");
            else if (error == OTA_BEGIN_ERROR)
                Serial.println("Begin Failed");
            else if (error == OTA_CONNECT_ERROR)
                Serial.println("Connect Failed");
            else if (error == OTA_RECEIVE_ERROR)
                Serial.println("Receive Failed");
            else if (error == OTA_END_ERROR)
                Serial.println("End Failed");
        });

    ArduinoOTA.begin();
}

void setup()
{
    pinMode(OE_PIN, OUTPUT);
    digitalWrite(OE_PIN, LOW);

    SPI.begin();
    SPI.setDataMode(SPI_MODE0);
    SPI.setHwCs(true);

    Serial.begin(115200);
    delay(100);
    SPI.writeBytes(led_data, sizeof(led_data));
    delay(500);

    startup_pattern();

    WiFi.mode(WIFI_AP_STA);

    // WiFiManager Settings
    //wm.setDebugOutput(true, WM_DEBUG_DEV);
    //wm.debugPlatformInfo();
    // wm.resetSettings(); // wipe settings
    // wm.erase(); 

    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS Mount Failed");
        return;
    }
    brightness = read_brightness(); 

    if(wm_nonblocking) wm.setConfigPortalBlocking(false);

    // callbacks
    wm.setWebServerCallback(bindServerCallback); 
    wm.setSaveParamsCallback(saveParamCallback); 

    // set custom html head content , inside <head>
    const char* headhtml = "<style>\
                                .wrap_left_align {\
                                text-align: left;\
                                display: inline-block;\
                                min-width: 75px;\
                                max-width: 150px\
                            }\
                            </style>";
    wm.setCustomHeadElement(headhtml);
    
    // custom html
    char* custom_link = (char *)malloc(1024); 
    sprintf(custom_link, "\
            <div class='wrap_left_align'> \
                <button type=\"button\" onclick=\"history.back()\">Back</button> \
            </div>\
            <hr />\
            <h1></h1>\
            <h3>teencircle</h3>\
            <div>\
                Brightness Setting<br/><input type=\"number\" name=\"light\" id=\"id_light\" size=\"4\" min=\"0\" max=\"255\" value=\"%d\">", brightness);
    new (&custom_field) WiFiManagerParameter(custom_link); // custom html input
    wm.addParameter(&custom_field);
    // custom menu
    const char* menuhtml = "<form action='/setting_new.html' method='get'><button>Manual Time Settings</button></form><br/> \
                            <form action='/demo.html' method='get'><button>Mode Setting</button></form>\n";
    wm.setCustomMenuHTML(menuhtml); 
    //std::vector<const char *> menu = {"wifi","wifinoscan","info","param","custom","close","sep","erase","update","restart","exit"};
    std::vector<const char *> menu = {"wifi","param","custom", "sep", "update", "erase", "restart"};
    wm.setMenu(menu);
    // set dark theme
    wm.setClass("invert");
    wm.setTitle(""); 
    
    // wifi scan settings
    // wm.setRemoveDuplicateAPs(false); // do not remove duplicate ap names (true)
    wm.setMinimumSignalQuality(40);  // set min RSSI (percentage) to show in scans, null = 8%
    // wm.setShowInfoErase(false);      // do not show erase button on info page
    // wm.setScanDispPerc(true);       // show RSSI as percentage not graph icons
    
    //wm.setBreakAfterConfig(true);   // always exit configportal even if wifi save fails

    uint8_t ap_mac[6]; 
    esp_read_mac(ap_mac, ESP_MAC_WIFI_SOFTAP); 
    sprintf(ap_ssid, "teencircle_%02X%02X%02X", ap_mac[3], ap_mac[4], ap_mac[5]); 
    if(!wm.autoConnect(ap_ssid)) {
        Serial.println("Failed to connect");
        // ESP.restart();
    } 
    else {
        //if you get here you have connected to the WiFi    
        Serial.println("connected...yeey :)");
        wm.startConfigPortal(ap_ssid); 
    }

    delay(500);
    wm.server->serveStatic("/", SPIFFS, "/www/"); 

    //init pwm for brightness
    ledcAttachPin(12, 1); // assign RGB led pins to channels
    ledcSetup(1, 12000, 8); // 12 kHz PWM, 8-bit resolution

    set_brightness(brightness);

    //1秒毎のtask生成
    xTaskCreatePinnedToCore(one_sec_task, "task1", 4096 * 3, NULL, 1, &taskHandle1, 1);

    // NTP サーバを設定
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

    setupArduinoOTA();
}

void loop()
{
    wm.process();

    if(!wm.getConfigPortalActive()) {
        wm.startConfigPortal(ap_ssid); 
        wm.server->serveStatic("/", SPIFFS, "/www/"); 
    }; 
        
    demo_led_pattern(action_mode);

    ArduinoOTA.handle();
}

void IRAM_ATTR set_min_led_data(int input_min, uint8_t* data)
{

    int quotient = 0;
    int rest = 0;
    //
    quotient = input_min / 8;
    rest = input_min % 8;

    data[7 - quotient] = mintable[rest];
}

void IRAM_ATTR set_hour_led_data(int input_hour, int input_min, uint8_t* data)
{
    // hour : data[8 to 15]
    unsigned int bitshift = 0;
    int i = 0;
    int quotient = 0, quotient_min = 0;
    int rest = 0;

    //hourからどのLEDを付けるか計算
    if (input_hour > 0) {

        if (input_hour < 12) {
            bitshift = (input_hour)*5;

        } else if (input_hour == 12) {
            bitshift = 0;

        } else {
            bitshift = (input_hour - 12) * 5;
        }
    }

    //minから12分毎に1個LEDの列を進める
    quotient_min = input_min / 12;

    bitshift = bitshift + quotient_min;

    quotient = bitshift / 8;
    rest = bitshift % 8;

    data[15 - quotient] = 1 << rest;

    //12時までは時針の該当ビットまでをすべて1に
    if (input_hour < 12) {
        //該当バイトまで全てLED点灯
        for (i = 15; i > 15 - quotient; i--) {
            data[i] = 0xff;
        }

        //時針のバイトにおいて、点灯させてるLED列以下のビットを全て1に
        for (i = 0; i < rest; i++) {
            data[15 - quotient] += pow(2, i);
        }

        //12以降は時針のビット以降をすべて１に
    } else {

        //該当バイト以降全てLED点灯
        for (i = 8; i < 15 - quotient; i++) {
            data[i] = 0xff;
        }

        //時針のバイトにおいて、点灯させてるLED列以降のビットを全て1に
        for (i = 7; i > rest; i--) {
            data[15 - quotient] += pow(2, i);
        }
    }
}

void one_sec_task(void* arg)
{
    portTickType xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    const portTickType xDelay = 1000 / portTICK_RATE_MS; // 1000ms

    while (1) {
        vTaskDelayUntil(&xLastWakeTime, xDelay / portTICK_RATE_MS);

        struct tm date_now;
        getLocalTime(&date_now);

        if (action_mode == CLOCK) {
            clear_led_data();
            set_min_led_data(date_now.tm_min, led_data);
            set_hour_led_data(date_now.tm_hour, date_now.tm_min, led_data);
            SPI.writeBytes(led_data, sizeof(led_data));
        }
    }
}

void startup_pattern()
{
    int i = LED_DRIVER_NUM * 2 - 1;
    int j = 0;
    int k = 0;
    int l = 0;
    uint8_t send_data = 0x01;

    for (l = 0; l < (LED_DRIVER_NUM * CHANNEL_NUM); l++) {

        for (j = 0; j < 16; j++) {
            led_data[j] = 0x00;
        }

        led_data[i] = send_data;

        SPI.writeBytes(led_data, sizeof(led_data));

        delay(30);

        send_data = (send_data << 1);

        if (send_data == 0x00) {
            send_data = 0x01;
        }

        k = k + 1;
        if (k == 8) {
            i = i - 1;
            k = 0;
        }
        if (i < 0) {
            i = LED_DRIVER_NUM * 2 - 1;

            clear_led_data();

            SPI.writeBytes(led_data, sizeof(led_data));

            delay(50);
        }
    }
}

void demo_led_pattern(ACTION_MODE now_mode)
{
    if (now_mode == CLOCK) {
        set_brightness(brightness); 
        return;
    }

    if (now_mode == DEMO1) {
        demo_pattern1();

    } else if (now_mode == DEMO2) {
        demo_pattern2();
    } else if (now_mode == DEMO3) {
        demo_pattern3();
    }
}

void demo_pattern1()
{
    static int i = LED_DRIVER_NUM * 2 - 1;
    static int k = 0;
    static uint8_t send_data = 0x01;

    clear_led_data();

    led_data[i] = send_data;

    SPI.writeBytes(led_data, sizeof(led_data));
    delay(20);

    send_data = (send_data << 1);

    if (send_data == 0x00) {
        send_data = 0x01;
    }

    k = k + 1;
    if (k == 8) {
        i = i - 1;
        k = 0;
    }

    if (i < 0) {
        i = LED_DRIVER_NUM * 2 - 1;
    }
}

void demo_pattern2()
{
    static int i = LED_DRIVER_NUM * 2 - 1;
    static int k = 0;
    static uint8_t send_data = 0x01;

    led_data[i] = led_data[i] + send_data;

    SPI.writeBytes(led_data, sizeof(led_data));
    delay(20);

    send_data = (send_data << 1);

    if (send_data == 0x00) {
        send_data = 0x01;
    }

    k = k + 1;
    if (k == 8) {
        i = i - 1;
        k = 0;
    }

    if (i < 0) {
        i = LED_DRIVER_NUM * 2 - 1;
        clear_led_data();
    }
}

void demo_pattern3()
{
    static int b = 0;
    static int flg = 0;

    if (flg == 0) {
        b = b + 5;
    } else {
        b = b - 5;
    }

    if (b > 255) {
        flg = 1;
        b = 255;
    } else if (b < 0) {
        flg = 0;
        b = 0;
    }

    ledcWrite(1, b);

    static int i = LED_DRIVER_NUM * 2 - 1;
    static int j = 0;
    static int k = 0;
    static uint8_t send_data = 0x01;

    if (j == 0) {
        led_data[i] = led_data[i] + send_data;
    } else {
        led_data[i] = led_data[i] - send_data;
    }

    SPI.writeBytes(led_data, sizeof(led_data));
    delay(20);

    send_data = (send_data << 1);

    if (send_data == 0x00) {
        send_data = 0x01;
    }

    k = k + 1;
    if (k == 8) {
        i = i - 1;
        k = 0;
    }

    if (i < 0) {
        i = LED_DRIVER_NUM * 2 - 1;
        j = ~j;
        if (j == 0) {
            clear_led_data();
        } else {
            fill_led_data();
        }
        send_data = 0x01;
    }
}

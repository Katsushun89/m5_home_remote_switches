#include <FirebaseESP32.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <M5Atom.h>
#include <WiFi.h>

#include "addons/TokenHelper.h"
#include "config.h"
#include "switches.h"

const uint32_t SWITCH_NUM = sizeof(SWITCH_DEF) / sizeof(String);

Switches switches(SWITCH_DEF, SWITCH_NUM);

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// XQueueSend/Receive
struct DATA_SET_BOOL {
    int32_t switch_number;
    bool power;
};

QueueHandle_t xQueueSetBool;
QueueHandle_t xQueueStream;

String parentPath = "/switches";

struct DATA_STREAM {
    bool switch_updated[SWITCH_NUM];
    bool switch_power[SWITCH_NUM];
};

TaskHandle_t th[4];

// Port 32 for IR Remote Unit
const uint16_t kIrLed = 26;
IRsend irsend(kIrLed);

enum {
    REMOTE_OFF = 0,
    REMOTE_ON,
};

const uint64_t remote_cmd[] = {
    0xE730D12EUL,  // OFF
    0xE730E916UL,  // ON
};

void setupWiFi() {
    WiFi.begin(SSID, PASSWD);

    // Wait some time to connect to wifi
    for (int i = 0; i < 10 && WiFi.status() != WL_CONNECTED; i++) {
        Serial.print(".");
        delay(1000);
    }

    // Check if connected to wifi
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("No Wifi!");
        return;
    }

    Serial.println("Connected to Wifi, Connecting to server.");
}

void setupFirebase(void) {
    config.api_key = API_KEY;
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;
    config.database_url = DATABASE_URL;
    config.token_status_callback =
        tokenStatusCallback;  // see addons/TokenHelper.h

    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);

    // Set the size of HTTP response buffers in the case where we want to work
    // with large data.
    fbdo.setResponseSize(1024);

    // The data under the node being stream (parent path) should keep small
    // Large stream payload leads to the parsing error due to memory allocation.
    if (!Firebase.beginStream(fbdo, parentPath.c_str())) {
        Serial.println("Can't begin stream connection...");
        Serial.println("REASON: " + fbdo.errorReason());
    }
}

void setupQueue(void) {
    xQueueSetBool = xQueueCreate(3, sizeof(DATA_SET_BOOL));
    if (xQueueSetBool == NULL) {
        Serial.println("rtos queue create error setBool");
    }
    xQueueStream = xQueueCreate(3, sizeof(DATA_STREAM));
    if (xQueueStream == NULL) {
        Serial.println("rtos queue create error stream");
    }
}

void setupTask(void) {
    xTaskCreatePinnedToCore(firebaseControl, "FirebaseControl", 4096 * 2, NULL,
                            1, &th[0], 1);
}

void sendQueueSetBool(int32_t switch_number, bool power) {
    DATA_SET_BOOL send_data = {switch_number, power};

    BaseType_t xStatus = xQueueSend(xQueueSetBool, &send_data, 0);

    if (xStatus != pdPASS)  // send error check
    {
        Serial.println("xQeueuSend error setBool");
    }
}

bool receiveQueueSetBool(DATA_SET_BOOL &recv_data) {
    const TickType_t xTicksToWait = 500U;  // [ms]
    BaseType_t xStatus = xQueueReceive(xQueueSetBool, &recv_data, xTicksToWait);
    if (xStatus == pdPASS)  // receive error check
    {
        return true;
    }
    if (uxQueueMessagesWaiting(xQueueSetBool) != 0) {
        Serial.println("xQueueReceive error setBool");
        return false;
    }
    return false;  // not received
}

void sendQueueStream(DATA_STREAM send_data) {
    BaseType_t xStatus = xQueueSend(xQueueStream, &send_data, 0);

    if (xStatus != pdPASS)  // send error check
    {
        Serial.println("xQeueuSend error stream");
    }
}

bool receiveQueueStream(DATA_STREAM &recv_data) {
    UBaseType_t queue_num = uxQueueMessagesWaiting(xQueueStream);
    if (queue_num == 0) {
        return false;
    }
    // Serial.printf("xQueueReceive stream num %d\n", queue_num);
    const TickType_t xTicksToWait = 10U;  // [ms]
    BaseType_t xStatus = xQueueReceive(xQueueStream, &recv_data, xTicksToWait);
    if (xStatus == pdPASS)  // receive error check
    {
        return true;
    }
    return false;  // not received
}

void setupIRRemote(void) { irsend.begin(); }

void setup() {
    M5.begin(true, false, true);

    Serial.begin(115200);

    setupIRRemote();

    setupQueue();
    setupTask();

    M5.dis.drawpix(0, 0x0000f0);
}

void readStreamBool(void) {
    DATA_STREAM send_data;
    for (int i = 0; i < SWITCH_NUM; i++) {
        send_data.switch_updated[i] = false;
    }

    for (int j = 0; j < SWITCH_NUM; j++) {
        if (fbdo.dataPath().indexOf(switches.getFirebasePath(j)) >= 0) {
            bool result_power = false;
            if (fbdo.boolData() == 1) {
                result_power = true;
            }
            send_data.switch_power[j] = result_power;
            send_data.switch_updated[j] = true;
            Serial.printf("%s power:%d\n", switches.getFirebasePath(j).c_str(),
                          result_power);
            break;
        }
    }
    sendQueueStream(send_data);
}

void readStreamJson(void) {
    DATA_STREAM send_data;
    for (int i = 0; i < SWITCH_NUM; i++) {
        send_data.switch_updated[i] = false;
    }
    FirebaseJson &json = fbdo.jsonObject();
    size_t len = json.iteratorBegin();
    String key, value = "";
    int type = 0;
    for (size_t i = 0; i < len; i++) {
        json.iteratorGet(i, type, key, value);
        for (int j = 0; j < SWITCH_NUM; j++) {
            if (switches.getFirebasePath(j).endsWith(key) &&
                value.indexOf("power") >= 0) {
                bool result_power = false;
                if (value.indexOf("true") >= 0) {
                    result_power = true;
                }
                send_data.switch_power[j] = result_power;
                send_data.switch_updated[j] = true;
                Serial.printf("%s power:%d\n",
                              switches.getFirebasePath(j).c_str(),
                              result_power);
                break;
            }
        }
    }
    sendQueueStream(send_data);
}

void syncSetBool(void) {
    if (Firebase.ready()) {
        DATA_SET_BOOL recv_data;
        if (receiveQueueSetBool(recv_data)) {
            Serial.printf("Free internal heap before TLS %u\n",
                          ESP.getFreeHeap());
            bool ret = Firebase.setBool(
                fbdo,
                parentPath + switches.getFirebasePath(recv_data.switch_number) +
                    "/power",
                recv_data.power);
            if (!ret) {
                Serial.printf("setBool error %s\n", fbdo.errorReason().c_str());
            }
        }
    }
}

void syncStream(void) {
    if (Firebase.ready()) {
        if (!Firebase.readStream(fbdo)) {
            Serial.println("Can't read stream data...");
            Serial.println("REASON: " + fbdo.errorReason());
        }

        if (fbdo.streamTimeout()) {
            Serial.println("Stream timeout, resume streaming...");
        }

        if (fbdo.streamAvailable()) {
            if (fbdo.dataType() == "boolean") {
                readStreamBool();
            } else if (fbdo.dataType() == "json") {
                readStreamJson();
            }
        }
    }
}

void firebaseControl(void *pvParameters) {
    setupWiFi();
    setupFirebase();

    while (1) {
        syncSetBool();
        syncStream();
        vTaskDelay(10);
    }
}

void switchON(void) {
    Serial.println("ON");
    irsend.sendNEC(remote_cmd[REMOTE_ON]);
    M5.dis.drawpix(0, 0xf00000);
}

void switchOFF(void) {
    Serial.println("OFF");
    irsend.sendNEC(remote_cmd[REMOTE_OFF]);
    M5.dis.drawpix(0, 0x00f000);
}

/*
void printStatus() {
    switch (sw_state) {
        case SW_ON:
            break;
        case SW_OFF:
            break;
        case SW_NOT_SET:
        default:
            M5.dis.drawpix(0, 0x0000f0);
            break;
    }
}*/

void checkButton() {
    if (M5.Btn.wasPressed()) {
        bool is_switched_on = switches.toggleSwitch();
        if (is_switched_on) {
            switchON();
        } else {
            switchOFF();
        }
        // sync firebase rtdb
        sendQueueSetBool(switches.getCurrentSwitchNumber(), is_switched_on);
    }
}

void updateSwitchStatus(void) {
    if (switches.isSwitchedOnCurrentSwitch()) {
        switchON();
    } else {
        switchOFF();
    }
}

void updateStreamSwitchStatus(void) {
    DATA_STREAM recv_data;
    if (receiveQueueStream(recv_data)) {
        for (int32_t i = 0; i < SWITCH_NUM; i++) {
            if (recv_data.switch_updated[i]) {
                switches.updatePowerStatus(i, recv_data.switch_power[i]);
                updateSwitchStatus();
            }
        }
    }
}

void loop() {
    M5.update();  // ボタン状態更新
    checkButton();
    updateStreamSwitchStatus();
    delay(1);
}

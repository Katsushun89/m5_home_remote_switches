#include <FirebaseESP32.h>
#include <M5Core2.h>
#include <WiFi.h>

#include <LovyanGFX.hpp>

#include "addons/TokenHelper.h"
#include "config.h"
#include "switches.h"
#include "ui_draw.h"

const uint32_t CENTER_ON_TIME = 1200;
const uint32_t CENTER_OFF_TIME = 650;
const uint32_t INVALID_DURATION = 1000 * 1;
bool is_in_transition_center_state = false;
bool is_once_released_after_switch_on = false;
uint32_t last_operation_time = 0;
uint32_t start_time_push_center = 0;
uint32_t keep_time_push_center = 0;
uint32_t invalid_time = 0;

const uint32_t SWITCH_NUM = sizeof(SWITCH_DEF) / sizeof(String);

Switches switches(SWITCH_DEF, SWITCH_NUM);
UIDraw uidraw;

/////////////////////////////////////////////////////////////////
// task and firebase
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
// task and firebase
/////////////////////////////////////////////////////////////////

void setup(void) {
    Serial.begin(115200);

    uidraw.setup();

    setupQueue();
    setupTask();

    sleep(1);  // wait 1sec
    updateDrawingCenter();
    sleep(1);  // wait 1sec
}

void tryDrawCenter(void) {
    ButtonStatus status;
    if (uidraw.popEvent(status)) {
        uidraw.drawCenter(status, getDecisionTime(),
                          switches.getStrCurrentSwitch());
    }
}

void updateDrawingCenter(void) {
    ButtonStatus status;
    status.is_switched_on = switches.isSwitchedOnCurrentSwitch();
    status.is_in_transition = false;
    uidraw.drawCenter(status, getDecisionTime(),
                      switches.getStrCurrentSwitch());
}

uint32_t getDecisionTime(void) {
    if (switches.isSwitchedOnCurrentSwitch()) {
        return CENTER_OFF_TIME;
    } else {
        return CENTER_ON_TIME;
    }
}

void keepTouchCenterButton(void) {
    uint32_t cur_time = millis();

    if (cur_time <= invalid_time) {
        // Disable INVALID_DURATION after the button state is switched.
        // Serial.printf("invalid center after switched %d %d\n", cur_time,
        // invalid_time);
        return;
    }

    // Once the switch is turned on, it will not turn on again until you release
    // it.
    if (is_once_released_after_switch_on) {
        // Serial.printf("invalid until once released\n");
        return;
    }

    if (!is_in_transition_center_state) {
        is_in_transition_center_state = true;
        start_time_push_center = cur_time;
        last_operation_time = cur_time;

        // Serial.printf("start transition %d\n", cur_time);
        return;
    }

    int32_t diff = cur_time - last_operation_time;
    keep_time_push_center += diff;
    last_operation_time = cur_time;

    // switch on
    if (keep_time_push_center >= getDecisionTime()) {
        keep_time_push_center = 0;
        bool is_switched_on = switches.toggleSwitch();

        // sync firebase rtdb
        sendQueueSetBool(switches.getCurrentSwitchNumber(), is_switched_on);

        is_in_transition_center_state = false;
        invalid_time = cur_time + INVALID_DURATION;
        is_once_released_after_switch_on = true;
        uidraw.pushEvent(collectButtonStatus());
        return;
    }

    // Serial.printf("1:%d, %d\n", cur_time, keep_time_push_center);
    uidraw.pushEvent(collectButtonStatus());
    return;
}

ButtonStatus collectButtonStatus(void) {
    ButtonStatus status;
    status.is_switched_on = switches.isSwitchedOnCurrentSwitch();
    status.is_in_transition = is_in_transition_center_state;
    status.keep_push_time = keep_time_push_center;
    return status;
}

void keepReleaseCenterButton(void) {
    if (is_once_released_after_switch_on)
        is_once_released_after_switch_on = false;

    if (is_in_transition_center_state) {
        resetTranstionPrams();
        uidraw.pushEvent(collectButtonStatus());
    }
}

void judgeCenterButton(TouchPoint_t pos, bool is_touch_pressed) {
    const int center_px = uidraw.getCenterPx();
    const int center_py = uidraw.getCenterPy();
    if (is_touch_pressed) {
        if (pos.y >= center_py - uidraw.CENTER_BUTTON_WIDTH / 2 &&
            pos.y < center_py + uidraw.CENTER_BUTTON_WIDTH / 2) {
            if (pos.x >= center_px - uidraw.CENTER_BUTTON_WIDTH / 2 &&
                pos.x < center_px + uidraw.CENTER_BUTTON_WIDTH / 2) {
                keepTouchCenterButton();
                return;
            }
        }
    }
    keepReleaseCenterButton();
    return;
}

void resetTranstionPrams(void) {
    // Serial.println("resetTranstionPrams");
    is_in_transition_center_state = false;
    is_once_released_after_switch_on = false;
    start_time_push_center = 0;
    keep_time_push_center = 0;
    invalid_time = 0;
}

void judgeBottomButtons(TouchPoint_t pos, bool is_touch_pressed) {
    static bool is_button_pressed = false;
    if (!is_touch_pressed) is_button_pressed = false;

    if (!is_button_pressed) {
        if (pos.y > 240) {
            if (pos.x < 120) {  // btnA
                is_button_pressed = true;
                resetTranstionPrams();
            } else if (pos.x > 240) {  // btnC
                is_button_pressed = true;
            } else if (pos.x >= 180 && pos.x <= 210) {  // btnB
                is_button_pressed = true;
            }
        }
    }
}

void judgeLRButton(TouchPoint_t pos, bool is_touch_pressed) {
    static bool is_button_pressed = false;
    if (!is_touch_pressed) is_button_pressed = false;

    if (!is_button_pressed) {
        if (pos.y > uidraw.getCenterPy() - 40 &&
            pos.y < uidraw.getCenterPy() + 40) {
            if (pos.x < 50) {  // L
                is_button_pressed = true;
                Serial.println("push L");
                switches.movedown();
                updateDrawingCenter();
            } else if (pos.x > uidraw.getLcdWidth() - 50) {  // R
                is_button_pressed = true;
                Serial.println("push R");
                switches.moveup();
                updateDrawingCenter();
            }
        }
    }
}

void updateStreamSwitchStatus(void) {
    DATA_STREAM recv_data;
    if (receiveQueueStream(recv_data)) {
        for (int32_t i = 0; i < SWITCH_NUM; i++) {
            if (recv_data.switch_updated[i]) {
                switches.updatePowerStatus(i, recv_data.switch_power[i]);
            }
        }
        updateDrawingCenter();
    }
}

void loop(void) {
    TouchPoint_t pos = M5.Touch.getPressPoint();
    bool is_touch_pressed = false;
    if (M5.Touch.ispressed()) is_touch_pressed = true;

    updateStreamSwitchStatus();
    judgeBottomButtons(pos, is_touch_pressed);
    judgeCenterButton(pos, is_touch_pressed);
    judgeLRButton(pos, is_touch_pressed);
    tryDrawCenter();
    delay(10);
}

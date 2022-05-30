/*#include <WiFi.h>
#include <freertos/FreeRTOS.h>
#include <esp_wifi.h>
#include <esp_wifi_types.h>
#include <esp_system.h>
#include <esp_event.h>
#include <esp_event_loop.h>
#include <nvs_flash.h>
#include <driver/gpio.h>

#define LED_GPIO_PIN 5

uint8_t level = 0;
uint8_t channel = 1;

// Переменные для синхронизации
// Возможно три случая:
// 1)Плата - отправитель, данные идут с компьютера (is_sender не проверяется)
// 2)Плата - отправитель (is_sender == true)
// 3)Плата - не отправитель (is_sender == false)
bool is_sender = true;
// state и state_lv из отрезка [0, 9]
int state = 9;  // Используется для хранения значения сигнала в памяти платы
float state_lv = 0;  // Используется для принятия и отправления сигнала
// Если плата оптравитель, то отправляем хранимое в state значение,
// с помощью state_lv. Иначе state используется для хранения
// принятого в state_lv сигнала

// Переменные и счетчики
#define N_NEIGHBORS_CHOISE 10  // Количество соседей, которые выбираем случайно из n_neighbors
float ssid_name_g[N_NEIGHBORS_CHOISE];
int rssi_g[N_NEIGHBORS_CHOISE];
int i_g = 0;  // Количество считанных соседей, с подходящим названием
float alpha = 0.7;  // Коэффициент расчета протокола LV (коэффициент доверия)
float epsilon = 0.2;  // Переменная для проверки синхронизации
// Если разница между abs(ssid_name_g[i] - state) < epsilon для всех i, достигается синхронизация

static wifi_country_t wifi_country = {.cc = "CN", .schan = 1, .nchan = 13};

static esp_err_t event_handler(void *ctx, system_event_t *event);
static void wifi_sniffer_init(void);
static void wifi_sniffer_packet_handler(void *buff, wifi_promiscuous_pkt_type_t type);

esp_err_t event_handler(void *ctx, system_event_t *event) {
    return ESP_OK;
}

void wifi_sniffer_init(void) {
    nvs_flash_init();
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK(esp_wifi_set_country(&wifi_country));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL));
    ESP_ERROR_CHECK(esp_wifi_start());
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_promiscuous_rx_cb(&wifi_sniffer_packet_handler);
}

// Сканируем сети
void wifi_sniffer_packet_handler(void* buff, wifi_promiscuous_pkt_type_t type) {
    if (type != WIFI_PKT_MGMT)
        return;

    if (i_g >= N_NEIGHBORS_CHOISE)
        return;

    const wifi_promiscuous_pkt_t *ppkt = reinterpret_cast<wifi_promiscuous_pkt_t *>(buff);

    if (ppkt->payload[38] == 49 && ppkt->payload[39] == 49 && ppkt->payload[40] == 49) {
        float ssid_name = ((ppkt->payload[41] - '0') * 10 + (ppkt->payload[42] - '0')) / 10;
        int bssid[6] = {ppkt->payload[10], ppkt->payload[11], ppkt->payload[12],
            ppkt->payload[13], ppkt->payload[14], ppkt->payload[15]};

        int bssid1[6] = {148, 185, 126, 233, 155, 253};
        // if (bssid[0] == bssid1[0] && bssid[1] == bssid1[1] && bssid[2] == bssid1[2]
        //    && bssid[3] == bssid1[3] && bssid[4] == bssid1[4] && bssid[5] == bssid1[5]) return;

        for (int i = 0; i < i_g; i++)
            if (ssid_name_g[i] == ssid_name)
                return;

        ssid_name_g[i_g] = ssid_name;
        rssi_g[i_g] = ppkt->rx_ctrl.rssi;
        i_g++;
    }
}

void setup() {
    Serial.begin(9600);
    delay(10);
    wifi_sniffer_init();
    pinMode(LED_GPIO_PIN, OUTPUT);
}

int counter = 0;

void loop() {
    // Запускаем перехват сообщений
    if (digitalRead(LED_GPIO_PIN) == LOW)
        digitalWrite(LED_GPIO_PIN, HIGH);
    else
        digitalWrite(LED_GPIO_PIN, LOW);

    // Если в Serial (ввод в плату) есть сигнал, то мы считываем его и записывамаем в state,
    // считаем плату отправителем
    // Иначе проверяем, является ли плата отправителем
    if (Serial.available() > 0) {
    state = Serial.parseInt();
    state_lv = state;
    } else if (is_sender) {
        state_lv = state;
    } else {
        state = round(state_lv);
    }

    // Отправление сигнала: запись данных в имя сети
    // 111 в начале названия - идентификатор необходимой сети
    // Используем state_lv с точностью до десятых
    // Последний символ говорит, что строка кончилась
    char first_number = round(state_lv) + '0';
    char second_number = static_cast<int>(10 * state_lv) % 10 + '0';
    const char * ap_ssid = new char[6]
    {'1', '1', '1', first_number, second_number, '\0'};

    // Создаем сеть с новым названием и без пароля
    WiFi.softAP(ap_ssid, NULL);

    // Обновляем данные согласно протоколу LV
    for (int i = 0; i < i_g; i++) {
        state_lv += alpha * (ssid_name_g[i] - state_lv);
    }

    // Проверяем стабилизацию.
    bool is_stabilization = true;
    for (int i = 0; i < i_g; i++)
        if (abs(ssid_name_g[i] - state) > epsilon) {
            is_stabilization = false;
            break;
        }

    // Отправляем данные о синхронизации
    // int ideal[9] = {0, 2, 3, 4, 5, 6, 7, 8, 9};
    // bool find = false;
    // for (int j = 0; j < 9; j++) {
        // for (int i = 0; i < i_g; i++)
            // if (ideal[j] == ssid_name_g[i])
                // find = true;
        // if (find == false)
        //     Serial.print(ideal[j]);
        // find = false;
    // }
    Serial.println();
    if (i_g == 0) {
        Serial.println("Not found networks ");
        Serial.println(counter);
    } else if (is_stabilization) {
        Serial.println("Stabilization");
        Serial.println(state);
    } else {
        Serial.println("Non stabilization");
        Serial.println(state);
        Serial.println(state_lv);
    }

    i_g = 0;
    delay(5000);

    if (counter >= 600) ESP.restart();
    else
        counter++;
}
*/
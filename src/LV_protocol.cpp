#include <WiFi.h>

// Переменные для синхронизации
// Возможно три случая:
// 1)Плата - отправитель, данные идут с компьютера (is_sender не проверяется)
// 2)Плата - отправитель (is_sender == true)
// 3)Плата - не отправитель (is_sender == false)
bool is_sender = false;
// state и state_lv из отрезка [0, 9]
int state = 0;  // Используется для хранения значения сигнала в памяти платы
float state_lv = 0;  // Используется для принятия и отправления сигнала
// Если плата оптравитель, то отправляем хранимое в state значение,
// с помощью state_lv. Иначе state используется для хранения
// принятого в state_lv сигнала

// Переменные и счетчики
int n_neighbors_choise = 5;  // Количество соседей, которые выбираем случайно из n_neighbors
float alpha = 0.7;  // Коэффициент расчета протокола LV (коэффициент доверия)
float grad = 0.2;  // Переменная для проверки синхронизации
// Если разница между abs(ssid_name_g[i] - state) < grad для всех i, достигается синхронизация

void setup() {
  Serial.begin(9600);
}

void loop() {
  // Если в Serial есть сигнал, то мы считываем его и записывамаем в state,
  // считаем плату отправителем.
  // Иначе проверяем, является ли плата отправителем.
  if (Serial.available() > 0) {
    state = Serial.parseInt();
    state_lv = state;
  } else if (is_sender) {
    state_lv = state;
  } else {
    state = round(state_lv);
  }

  // Отправление сигнала: запись данных в имя сети.
  // 111 в начале названия - идентификатор необходимой сети.
  // Используем state_lv с точностью до десятых.
  // Последний символ говорит, что строка кончилась.
  const char * ap_ssid = new char[6]
  {'1', '1', '1', round(state_lv) + '0', static_cast<int>(10 * state_lv) % 10 + '0', '\0'};

  // Создаем сеть с новым названием и без пароля.
  WiFi.softAP(ap_ssid, NULL);  // Start AP mode

  // Сканируем сети
  int networksFound = WiFi.scanNetworks();

  // Смотрим список сетей и заполняем массивы с названием и уровнем сигнала.
  int ssid_name_g[n_neighbors_choise];
  int rssi_g[n_neighbors_choise];
  int i_g = 0;  // Количество считанных соседей, с подходящим названием
  int rssi;  // Мощность сигнала
  int ssid_name;  // Название сети с данными

  // Если сети есть, считываем данные.
  if (networksFound > 0) {
    for (int i_scan = 0; i_scan < networksFound; i_scan++) {
      if (i_g >= n_neighbors_choise) {
        break;
      }

      rssi = WiFi.RSSI(i_scan);
      ssid_name = WiFi.SSID(i_scan).toInt();
      if (ssid_name / 100 == 111) {
        ssid_name_g[i_g] = (ssid_name % 100) / 10;
        rssi_g[i_g] = rssi;
        i_g++;
      }
    }
  }

  // Обновляем данные согласно протоколу LV.
  if (i_g != 0) {
    for (int i = 0; i < i_g; i++) {
      state_lv += alpha * (ssid_name_g[i] - state_lv);
    }
  }

  // Проверяем стабилизацию.
  bool is_stabilization = true;
  for (int i = 0; i < i_g; i++) {
    if (abs(ssid_name_g[i] - state) > grad) {
      is_stabilization = false;
      break;
    }
  }

  // Отправляем данные о синхронизации.
  Serial.println("Iteration");
  if (i_g == 0) {
    Serial.println("Not found networks");
  } else if (is_stabilization) {
    Serial.println("Stabilization");
    Serial.println(state);
  } else {
    Serial.println(state);
    Serial.println(state_lv);
  }
}

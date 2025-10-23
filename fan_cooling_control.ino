/*
 * Управление вентилятором охлаждения двигателя через транзисторный ключ
 * Подключение:
 * - Датчик температуры (DS18B20 или аналоговый NTC) -> A0
 * - Транзистор (база через резистор 1кОм) -> D9 (PWM)
 * - Реле (опционально для мощной нагрузки) -> D10
 */

// Пины подключения
const int TEMP_SENSOR_PIN = A0;    // Аналоговый вход датчика температуры
const int FAN_PWM_PIN = 9;         // PWM выход на транзистор (MOSFETили TIP120)
const int FAN_RELAY_PIN = 10;      // Реле для полного включения (опционально)

// Температурные пороги (в градусах Цельсия)
const float TEMP_FAN_ON = 85.0;    // Температура включения вентилятора
const float TEMP_FAN_MAX = 95.0;   // Температура максимальных оборотов
const float TEMP_HYSTERESIS = 5.0; // Гистерезис для предотвращения дребезга

// Параметры PWM
const int PWM_MIN = 100;           // Минимальная скорость (0-255)
const int PWM_MAX = 255;           // Максимальная скорость (0-255)

// Переменные состояния
bool fanActive = false;
float currentTemp = 0.0;

void setup() {
  Serial.begin(9600);
  
  // Настройка пинов
  pinMode(FAN_PWM_PIN, OUTPUT);
  pinMode(FAN_RELAY_PIN, OUTPUT);
  pinMode(TEMP_SENSOR_PIN, INPUT);
  
  // Начальное состояние - вентилятор выключен
  analogWrite(FAN_PWM_PIN, 0);
  digitalWrite(FAN_RELAY_PIN, LOW);
  
  Serial.println("Система охлаждения запущена");
}

void loop() {
  // Чтение температуры
  currentTemp = readTemperature();
  
  // Управление вентилятором
  controlFan(currentTemp);
  
  // Вывод информации в Serial Monitor
  printStatus();
  
  delay(1000); // Обновление каждую секунду
}

// Функция чтения температуры с аналогового датчика NTC
float readTemperature() {
  int rawValue = analogRead(TEMP_SENSOR_PIN);
  
  // Преобразование для NTC термистора 10кОм с делителем напряжения
  // Формула зависит от конкретного датчика
  float voltage = rawValue * (5.0 / 1023.0);
  
  // Упрощенная линейная калибровка (замените на формулу вашего датчика)
  // Для примера: 0V = 0°C, 5V = 150°C
  float temperature = voltage * 30.0; 
  
  // Для точного измерения используйте формулу Стейнхарта-Харта
  // или калибровочную таблицу вашего датчика
  
  return temperature;
}

// Функция управления вентилятором
void controlFan(float temp) {
  int pwmValue = 0;
  
  if (temp >= TEMP_FAN_MAX) {
    // Максимальная скорость при высокой температуре
    pwmValue = PWM_MAX;
    fanActive = true;
    digitalWrite(FAN_RELAY_PIN, HIGH);
    
  } else if (temp >= TEMP_FAN_ON) {
    // Пропорциональное управление скоростью
    float tempRange = TEMP_FAN_MAX - TEMP_FAN_ON;
    float tempDelta = temp - TEMP_FAN_ON;
    pwmValue = map(tempDelta * 10, 0, tempRange * 10, PWM_MIN, PWM_MAX);
    fanActive = true;
    digitalWrite(FAN_RELAY_PIN, HIGH);
    
  } else if (temp < (TEMP_FAN_ON - TEMP_HYSTERESIS)) {
    // Выключение вентилятора с гистерезисом
    pwmValue = 0;
    fanActive = false;
    digitalWrite(FAN_RELAY_PIN, LOW);
    
  } else if (fanActive) {
    // Поддержание минимальных оборотов в зоне гистерезиса
    pwmValue = PWM_MIN;
  }
  
  // Установка скорости вентилятора
  analogWrite(FAN_PWM_PIN, pwmValue);
}

// Функция вывода информации
void printStatus() {
  Serial.print("Температура: ");
  Serial.print(currentTemp, 1);
  Serial.print("°C | Вентилятор: ");
  
  if (fanActive) {
    int pwmPercent = map(analogRead(FAN_PWM_PIN), 0, 255, 0, 100);
    Serial.print("ВКЛ (");
    Serial.print(pwmPercent);
    Serial.println("%)");
  } else {
    Serial.println("ВЫКЛ");
  }
}

/*
 * СХЕМА ПОДКЛЮЧЕНИЯ ТРАНЗИСТОРНОГО КЛЮЧА:
 * 
 * Arduino D9 (PWM) ----[R=1кОм]---- База NPN транзистора (TIP120/BD139)
 *                                   |
 *                                   Коллектор
 *                                   |
 *                        +12V ---- Вентилятор ---- Коллектор
 *                                   |
 *                                   Эмиттер ---- GND
 *                                   |
 *                              [Диод 1N4007] (катод к +12V)
 * 
 * Для MOSFET (IRLZ44N/IRF540):
 * Arduino D9 (PWM) ----[R=100Ом]---- Затвор (Gate)
 *                        +12V ---- Вентилятор ---- Сток (Drain)
 *                                   Исток (Source) ---- GND
 *                              [Диод 1N4007] параллельно вентилятору
 * 
 * ВАЖНО:
 * - Обязательно использовать обратный диод для защиты от ЭДС самоиндукции
 * - Для мощных вентиляторов (>5А) используйте реле или мощный MOSFET
 * - Общий GND между Arduino и силовой частью обязателен
 */

//Bibliotecas utilizadas
#include <Arduino.h>
#include <WiFi.h>
#include <esp_wpa2.h>
#include <Arduino_MQTT_Client.h>
#include <ThingsBoard.h>

// Configurações do ThingsBoard
#define THINGSBOARD_SERVER "labiot.eletrica.ufpr.br"
#define THINGSBOARD_TOKEN "L4PWSUJSMlL2Z50PoalD"
#define THINGSBOARD_PORT 1883 // define a porta do thingsboard

// Tempo de amostragem de 30 s
#define SAMPLE_TIME_MS 30000  

// Wi-Fi eduroam
const char* ssid = "eduroam";
const char* username = ""; //preencher
const char* password = ""; //preencher

// Pino de saída de dados do sensor PPD42 conectado ao pino D4 do Esp32
int sensor_pin = 4;
volatile uint64_t t_baixo = 0; // tempo acumulado em que o sinal do sensor permanece em nível lógico baixo
volatile uint64_t t_start = 0; // instante em que se inicia um pulso em nível lógico baixo

// Variáveis de cálculo
unsigned long lastTime = 0;
unsigned long ocup_baixo = 0;
float razao = 0;
float concentracao = 0;

// WiFi e MQTT
WiFiClient espClient;
Arduino_MQTT_Client mqttClient(espClient);

// Define o tamanho máximo da mensagem (1 KB é suficiente)
constexpr uint32_t MAX_MESSAGE_SIZE = 1024U;

// Inicializa ThingsBoard no formato moderno
ThingsBoard tbClient(mqttClient, MAX_MESSAGE_SIZE);

// Função de interrupção para medições
void IRAM_ATTR pulso_in() {
  unsigned long agora = micros();
  if (digitalRead(sensor_pin) == LOW) {
    t_start = agora;
  } else if ((agora - t_start) > 100) {
    t_baixo += (agora - t_start);
    t_start = 0;
  }
}

void setup() {
  Serial.begin(115200);

  // Configurações para conectar ao Wi-Fi
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  esp_wifi_sta_wpa2_ent_enable();
  esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)username, strlen(username));
  esp_wifi_sta_wpa2_ent_set_username((uint8_t *)username, strlen(username));
  esp_wifi_sta_wpa2_ent_set_password((uint8_t *)password, strlen(password));
  WiFi.begin(ssid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando ao WiFi...");
  }
  Serial.println("Conectado ao WiFi!");
  Serial.print("Conectando ao WiFi");

  // Configurar MQTT para o PubSubClient
  mqttClient.set_server(THINGSBOARD_SERVER, THINGSBOARD_PORT); //Caso o código não funcione de primeira, comentar essa linha e tentar novamente
  
  // Define o pino do sensor conectado ao Esp32 como entrada de dados, pois a medição sai do medidor e entra no microcontrolador
  pinMode(sensor_pin, INPUT);
  attachInterrupt(digitalPinToInterrupt(sensor_pin), pulso_in, CHANGE); // atribui a função de interrupção ao pino com a entrada de dados do sensor, 
                                                                       //quando o nível lógico do sinal medido muda
}

// Função para reconectar ao Thingsboard caso a conexão inicial dê errado
void reconnectTB() {
  while (!tbClient.connected()) {
    Serial.println("Conectando ao ThingsBoard...");
    
    // CORREÇÃO: Conectar usando o método correto
    if (tbClient.connect(THINGSBOARD_SERVER, THINGSBOARD_TOKEN)) {
      Serial.println("Conectado ao ThingsBoard!");
    } else {
      Serial.print("Falha na conexão! Código: ");
    }
  }
}

void loop() {

  // Caso ainda não tenha conectado ao thingsboard, tenta reconectar
  if (!tbClient.connected()) {
    reconnectTB();
  }

  // Chama de maneira iterativa a função de conexão do cliente MQTT do thingsboard
  tbClient.loop();

  // Realiza as medições dentro do tempo de amostragem
  unsigned long agora = millis();
  if (agora - lastTime >= SAMPLE_TIME_MS) {
    lastTime = agora;

    noInterrupts(); // desabilita as interrupções
    ocup_baixo = t_baixo; // registra o tempo em que a leitura permaneceu em nível lógico baixo
    t_baixo = 0; // zera o tempo em nível lógico baixo para permitir que a operação seja repetida iterativamente
    interrupts(); // habilita a função de interrupção para realizar as medições

    // Aplica a equação de calibração do datasheet para obter a concentração de partículas de poeira em pcs/0.01cf a partir do tempo em que o sensor permaneceu em nível lógico baixo
    razao = ocup_baixo / (SAMPLE_TIME_MS * 10.0);
    concentracao = 1.1 * pow(razao, 3) - 3.8 * pow(razao, 2) + 520 * razao + 0.62;

    // Exibe os resultados no monitor serial para auxiliar na interpretação e depuração do sistema, mais utilizado para debug
    Serial.print("Razão = ");
    Serial.print(razao);
    Serial.print(" | Concentração = ");
    Serial.print(concentracao);
    Serial.println(" pcs/0.01cf");

    // Envia dados de telemetria
    tbClient.sendTelemetryData("concentracao", concentracao);
  }
  
  // Espera 100 ms para repetir a medição 
  delay(100);
}
// Definições obrigatórias do Blynk: 

// ID e nome do template e o token de autenticação (gerados na plataforma Blynk) 

#define BLYNK_TEMPLATE_ID "TMPL2K4g5WGzp" 

#define BLYNK_TEMPLATE_NAME "Intrumentação" 

#define BLYNK_AUTH_TOKEN "eTSoYY5az-Npzi_8CJhGmIG8Tog5r6eI" 

 

// Habilita os prints do Blynk no Monitor Serial (útil para depuração) 

#define BLYNK_PRINT Serial 

 

// Bibliotecas essenciais para conexão Wi-Fi com o ESP32 e para uso do Blynk 

#include <WiFi.h> 

#include <esp_wpa2.h>               // Biblioteca necessária para autenticação WPA2-Enterprise (como o eduroam) 

#include <BlynkSimpleEsp32.h>       // Biblioteca específica para ESP32 se comunicar com o Blynk 

 

// Credenciais para conexão ao Wi-Fi eduroam (WPA2-Enterprise) 

const char* ssid = "INSERIR SSID"; 

const char* username = "INSERIR NOME DE USUÁRIO"; 

const char* password = "INSERIR SENHA"; 

 

// Inicializa o temporizador do Blynk (permite chamadas periódicas de funções) 

BlynkTimer timer; 

// Definição dos pinos 

const int sensor_pin = 2;    // Pino de entrada conectado ao sensor de fluxo 

const int bomba_pin = 4;     // Pino de saída para controle da bomba via PWM 

 
// Configurações do sinal PWM para controlar a bomba 

const int canal_pwm = 0;     // Canal 0 do PWM do ESP32 

const int freq_pwm = 5000;   // Frequência do PWM em Hz (5kHz) 

const int resolucao = 8;     // Resolução de 8 bits (valores de 0 a 255) 

int duty = 255;              // Duty cycle inicial em 100% (255) 

 
// Fator de calibração baseado em testes e datasheet do sensor de fluxo 

const float FATOR_CALIBRACAO = 8.6;  // Pulsos por L/min 

// Variáveis usadas para contabilizar pulsos sem acumular pulsos duplos 

volatile int pulsos = 0;                 // Contador de pulsos 

volatile unsigned long lastDebounceTime = 0;  // Armazena o tempo do último pulso válido 

const unsigned long debounceDelay = 120;      // Tempo mínimo entre pulsos (em microssegundos) 

// Função de interrupção que conta os pulsos do sensor de fluxo de água 

// A cada pulso (queda de borda no pino), verificamos se passou o tempo de debounce e incrementamos o contador 

void IRAM_ATTR pulso_in() { 

  unsigned long agora = micros();  // Tempo atual em microssegundos 

  if ((agora - lastDebounceTime) > debounceDelay) { 

    pulsos++; 

    lastDebounceTime = agora; 

  } 

} 
// Função chamada automaticamente a cada 1.2 segundos para calcular e enviar os dados de vazão ao app do Blynk 

void enviarDadosFluxo() { 

  // Desativa interrupções temporariamente para evitar conflito na leitura da variável pulsos 

  noInterrupts(); 

  int pulsosLidos = pulsos; 

  pulsos = 0;  // Zera o contador para a próxima leitura

   interrupts(); 

 

  // Calcula a frequência de pulsos por segundo (1.2s equivale a fator 0.8333) 

  double pulsos_intervalo = pulsosLidos * 0.8333; 

 

  // Calcula o fluxo em L/min usando o fator de calibração 

  double fluxo = pulsos_intervalo / FATOR_CALIBRACAO; 

 

  // Calcula a frequência teórica (Hz) do sensor com base no fluxo (datasheet: F = 10 * Q) 

  double freq = fluxo * 10; 

 

  // Envia os dados via serial (para depuração) 

  Serial.print(freq, 4); 

  Serial.print(" Hz | Duty: "); 

  Serial.println(duty); 

 

  // Envia os valores para o app do Blynk nos pinos virtuais V0 e V2 

  Blynk.virtualWrite(V0, freq);   // Frequência em Hz 

  Blynk.virtualWrite(V2, fluxo);  // Vazão em L/min 

} 

 

// Função chamada automaticamente sempre que o Slider virtual V1 (no app Blynk) for ajustado 

// Atualiza o duty cycle do PWM com o novo valor vindo do app 

BLYNK_WRITE(V1) { 

  int novo_duty = param.asInt();  // Lê o valor do slider 

  if (novo_duty >= 0 && novo_duty <= 255) { // Confere se está dentro da resolução configurada 

    duty = novo_duty; 

    ledcWrite(canal_pwm, duty);   // Aplica o novo duty PWM à bomba 

    Serial.print("Duty atualizado via Blynk: "); 

    Serial.println(duty); 

  } 

} 

// Função para autenticar e conectar ao Wi-Fi eduroam (WPA2-Enterprise) 

void setupEduroam() { 

  WiFi.disconnect(true);        // Desconecta de redes anteriores 

  WiFi.mode(WIFI_STA);          // Coloca o ESP32 no modo Station (cliente) 

  esp_wifi_sta_wpa2_ent_enable();  // Habilita WPA2-Enterprise 

 

  // Define credenciais de autenticação do eduroam (identity e senha) 

  esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)username, strlen(username)); 

  esp_wifi_sta_wpa2_ent_set_username((uint8_t *)username, strlen(username)); 

  esp_wifi_sta_wpa2_ent_set_password((uint8_t *)password, strlen(password)); 

 

  WiFi.begin(ssid);  // Inicia a conexão com o Wi-Fi 

 

  // Espera até a conexão ser estabelecida 

  Serial.print("Conectando ao eduroam"); 

  while (WiFi.status() != WL_CONNECTED) { 

    Serial.print("."); 

    delay(1000);  // Espera 1 segundo entre tentativas 

  } 

  Serial.println("\nConectado ao eduroam com sucesso!"); 

} 
// Função principal de configuração, chamada uma única vez ao iniciar o programa 

void setup() { 

  Serial.begin(115200);  // Inicia comunicação serial para debug 

 

  // Inicializa o PWM na bomba 

  ledcSetup(canal_pwm, freq_pwm, resolucao); 

  ledcAttachPin(bomba_pin, canal_pwm); 

  ledcWrite(canal_pwm, duty);  // Aplica o duty inicial 

 

  // Configura o sensor de fluxo como entrada com resistor de pull-up interno 

  pinMode(sensor_pin, INPUT_PULLUP); 

  attachInterrupt(digitalPinToInterrupt(sensor_pin), pulso_in, FALLING);  // Configura o pino do sensor para ser lido por interrupção por borda de descida 

 

  // Conecta ao Wi-Fi eduroam usando WPA2-Enterprise 

  setupEduroam(); 

 

  // Conecta ao servidor Blynk com o token definido anteriormente 

  Blynk.config(BLYNK_AUTH_TOKEN); 

 

  // Configura o timer para executar a função de envio de dados a cada 1.2s 

  timer.setInterval(1200L, enviarDadosFluxo); 

  Serial.println("Sistema iniciado com controle de PWM via Blynk."); 

} 

// Loop principal que roda continuamente após o setup 

void loop() { 

  Blynk.run();   // Mantém a conexão com o Blynk ativa e trata eventos 

  timer.run();   // Executa funções agendadas com o timer (como envio dos dados) 

} 
 
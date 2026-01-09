// Configurações do Thingsboard
#define ENCRYPTED false
#define MAX_RPC_CALLBACKS 2
#include "WifiManager.h"
#include "IoT_Manager.h"

#define THINGSBOARD_SERVER "demo.thingsboard.io"
#define THINGSBOARD_TOKEN "rKAd4uaHOWVALdqKoxgk"
#define MEASUREMENT_INTERVAL 30 // segundos

#define WIFI_SSID "INSERIR SSID"
#define WIFI_PASSWORD "" 
#define WIFI_LOGIN_USERNAME "INSERIR NOME DE USUÁRIO"
#define WIFI_LOGIN_PASSWORD "INSERIR SENHA" 

// O sensor Shinyei PPD42 possui um LED infravermelho que emite feixes de luz,
// um fotodiodo que capta a luz emitida e um ventilador que faz o ar circular.
// Quando partículas de poeira, fumaça e aerossóis entram na câmara do sensor,
// a luz emitida se espalha no fotodiodo e, quando o espalhamento ultrapassa um limiar,
// o circuito eletrônico do sensor emite uma saída digital em nível lógico baixo. 
// Por isso, a lógica utilizada no código visa medir o tempo que o sensor fica em nível lógico baixo,
// utilizando-o para determinar a concentração de partículas de poeira no ar.

// Declarações iniciais
int sensor_pin = 4;

volatile uint64_t t_baixo = 0;
volatile uint64_t t_start = 0;

unsigned long sampletime_ms = 30000; //amostragem de 30s, recomendado pelo datasheet ;
unsigned long lastTime = 0;

unsigned long ocup_baixo = 0;
float razao = 0;
float concentracao = 0;

//Função padrão Thingsboard
void DoSomething(const JsonVariantConst &data, JsonDocument &response) {
   Serial.println("Executando código solicitado via método RPC.");

	// ... Executar código desejável
   //const float example_data = data["SomeServerData"];
   //response["bool"] = true;
}

const std::array<RPC_Callback, MAX_RPC_CALLBACKS> CALLBACKS = { 
	RPC_Callback { "DoSomething", &DoSomething } // Nome x em string enviada do ThingsBoard que vai chamar a função y no ESP32
	//RPC_Callback { "AnotherThing", AnotherThing } // Coloque mais métodos RPC se necessário.
};

// Seus atributos para enviar seus respectivos valores. Os atributos do dispositivo no ThingsBoard devem estar com o mesmo nome
std::vector<const char*> DEVICE_ATRIBUTES = {
	"concentracao"
};

IoT_Manager device(THINGSBOARD_SERVER, THINGSBOARD_TOKEN, MEASUREMENT_INTERVAL, &DEVICE_ATRIBUTES, &CALLBACKS);

//Função de interrupção
void IRAM_ATTR pulso_in(){
  unsigned long agora = micros();

  if(digitalRead(sensor_pin) == LOW){
    // Início do pulso
    t_start = agora;
  }

  else if((agora - t_start) > 100){
    // Fim do pulso
    t_baixo += (agora - t_start);
    t_start = 0;
  }
}

void setup() {

 Serial.begin(9600);

 WifiManager::LoginWifi(WIFI_SSID, WIFI_PASSWORD, WIFI_LOGIN_USERNAME, WIFI_LOGIN_PASSWORD);
 device.Initialize();

 // Configura pino como entrada para fazer a leitura, e atribui interrupção a ele
 pinMode(sensor_pin,INPUT);
 attachInterrupt(digitalPinToInterrupt(sensor_pin), pulso_in, CHANGE); // CHANGE detecta quando o pulso muda de nível lógico para poder medir quanto tempo fica em nível lógico BAIXO

}

void loop() {

  unsigned long agora = millis();

  if (agora - lastTime >= sampletime_ms){ // Se já deu o tempo de amostragem
    lastTime = agora;

    // Leitura via interrupção para medir quanto tempo o pulso é "ocupado" pelo nível lógico BAIXO
    noInterrupts();
    ocup_baixo = t_baixo; 
    t_baixo = 0;
    interrupts();

    // Calibração de acordo com o datasheet
    razao = ocup_baixo/(sampletime_ms*10.0); // porcentagem inteira 0=>100
    concentracao = 1.1*pow(razao,3)-3.8*pow(razao,2)+520*razao+0.62; // calibrando pelas especificações do datasheet

    // Escreve no monitor serial
    Serial.print("concentracao = ");
    Serial.print(concentracao);
    Serial.println(" pcs/0.01cf");
    Serial.println("\n");

    // Zera o tempo de ocupação do nível lógico BAIXO para repetir a lógica do loop
    ocup_baixo = 0;
  }

  if (device.NextMeasurement()) {
		Serial.println("Novo ciclo.");
		
		// Código de medição
		std::vector<float> medicao = { concentracao }; // temperatura e umidade respectivamente. Devem ser na mesma ordem que DEVICE_ATRIBUTES
		device.SendData(medicao);
	}
}

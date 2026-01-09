// =====================
// Configurações Fixas
// =====================

// Pino de entrada do sensor de fluxo
const int sensor_pin = 2;

// Pino de controle do PWM da bomba (saída)
const int bomba_pin = 4;

// Número do canal PWM utilizado pelo ESP32 (0 a 15)
const int canal_pwm = 0;

// Frequência do sinal PWM em Hz (adequada à bomba)
const int freq_pwm = 1000;

// Resolução do PWM em bits (8 bits → duty de 0 a 255)
const int resolucao = 8;

// Fator de calibração para converter a frequência lida do sensor para valor real em Hz
const float FATOR_CALIBRACAO = 0.86;

// =====================
// Parâmetros do Sistema (Ajuste Aqui)
// =====================

// Período de amostragem do controlador (em milissegundos)
const int Ts = 30;

// Valor desejado do fluxo (em L/min) 
byte SETPOINT = 3;

// Ganhos do controlador PID (ajustados por tentativa ou método de sintonia)
// Kp: Ganho proporcional
const float Kp = 28;

// Ki: Ganho integral → já ajustado com o tempo de amostragem Ts
const float Ki = 89.34 * Ts/1000 ;

// Kd: Ganho derivativo
const float Kd = 0.525; 

// Valor inicial de duty cycle para garantir que a bomba parta com força suficiente (kickstart)
const int KICKSTART_DUTY = 150; // ~59% com resolução de 8 bits

// Duração do kickstart em milissegundos
const int KICKSTART_DURATION = 500;

// Duty mínimo permitido após o kickstart (protege contra valores muito baixos que não movimentam a bomba)
const int DUTY_MINIMO = 45; // ~18%

// =====================
// Variáveis do Sistema
// =====================

// Armazena o timestamp do último pulso detectado (em microssegundos)
volatile unsigned long ultimoPulso = 0;

// Armazena a frequência calculada a partir dos pulsos (Hz)
volatile float frequenciaAtual = 0;

// Delay de debounce para evitar múltiplas contagens por pulso
const unsigned long debounceDelay = 120;

// Controle de tempo entre atualizações do controlador PID
unsigned long lastTime = 0;

// Momento em que o kickstart termina (millis)
unsigned long kickstart_end = 0;

// Indica se o kickstart ainda está ativo
bool kickstart_active = true;

// Parâmetros do filtro de média móvel para suavizar leitura de frequência
const int m = 8;
float freq_amostras[m]; // vetor circular de amostras
int amostra_idx = 0;    // índice atual no vetor de amostras

// =====================
// Interrupção do Sensor
// =====================

// Função de interrupção chamada a cada pulso do sensor (borda de descida)
void IRAM_ATTR pulso_in() {
  unsigned long agora = micros();
  
  // Verifica se passou tempo suficiente desde o último pulso (debounce)
  if ((agora - ultimoPulso) > debounceDelay) {
    // Calcula a frequência com base no tempo entre pulsos
    frequenciaAtual = 1000000.0 / (agora - ultimoPulso); //10e6 pois "agora" é medido em microssegundos 
    ultimoPulso = agora;
  }
}

// =====================
// Setup Inicial
// =====================
void setup() {
  Serial.begin(115200); // Inicializa comunicação serial para monitoramento
  Serial.setTimeout(50); //limita o tempo de espera de caracteres de entrada da serial
  
  // Configura o PWM da bomba
  ledcSetup(canal_pwm, freq_pwm, resolucao);         // Inicializa canal PWM
  ledcAttachPin(bomba_pin, canal_pwm);               // Associa o canal ao pino físico
  ledcWrite(canal_pwm, KICKSTART_DUTY);              // Aplica kickstart no início

  // Configura pino do sensor e liga a interrupção
  pinMode(sensor_pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(sensor_pin), pulso_in, FALLING);

  // Marca o fim do kickstart
  kickstart_end = millis() + KICKSTART_DURATION;
}

// =====================
// Loop Principal
// =====================
void loop() {
  unsigned long agora = millis();

 // ===============================
  // Leitura do Setpoint via Serial
  // ===============================
  if (Serial.available() > 0) {  //verifica se há dados na serial 
    SETPOINT = Serial.parseInt(); //lê caracteres ASCII da serial e converte em um número inteiro
    SETPOINT = constrain(SETPOINT,1,10);  //limita os valores da variável PWM entre 0 a 255
  }

  // Verifica se é hora de fazer nova iteração do controle
  if (agora - lastTime >= Ts) {
    lastTime = agora;

    // ===============================
    // Leitura segura da frequência (com interrupções desabilitadas temporariamente)
    // ===============================
    noInterrupts();
    float freq = frequenciaAtual;
    unsigned long tempo_sem_pulso = micros() - ultimoPulso;
    interrupts();

    // Se passaram mais de 500ms sem pulsos, considera frequência igual a zero
    if (tempo_sem_pulso > 500000) freq = 0;

    // ===============================
    // Filtro de Média Móvel
    // ===============================
    freq_amostras[amostra_idx] = freq;
    amostra_idx = (amostra_idx + 1) % m;

    float freq_media = 0;
    for (int i = 0; i < m; i++) {
      freq_media += freq_amostras[i];
    }
    freq_media /= m;

    // ===============================
    // Calibração da frequência lida
    // ===============================
    float freq_calibrada = freq_media / FATOR_CALIBRACAO;
    float fluxo = freq_calibrada/10;

    // ===============================
    // Controle PID (apenas após o kickstart)
    // ===============================
    if (!kickstart_active) {
      static float erro_anterior = 0;  // Armazena erro anterior para cálculo da derivada
      static float int_erro = 0;       // Soma dos erros para o termo integral
      
      float erro = SETPOINT - fluxo;

      // Anti-windup: só acumula o erro integral se estiver perto do setpoint
      if (abs(erro) < SETPOINT * 0.6) { // reset do integrador
        int_erro += erro;
      } else {
        int_erro = 0;
      }

      // Calcula termo derivativo com base na diferença entre erros
      float deriv_erro = (erro - erro_anterior) / (Ts / 1000.0);

      // Calcula saída do PID
      float output = Kp * erro + Ki * int_erro - Kd * deriv_erro;

      // Constrange o duty para manter dentro dos limites operacionais
      int duty = constrain(output, DUTY_MINIMO, 255);
      ledcWrite(canal_pwm, duty);

      // Atualiza erro anterior
      erro_anterior = erro;

       // ===============================
      // Envia fluxo para o monitor serial
     // ===============================
    Serial.print(fluxo, 2); // 2 casas decimais
    Serial.print("  ");

    Serial.print(SETPOINT);
    Serial.print("  ");


      // ========== PRINTS DOS TERMOS DO PID ==========
      // Serial.print(erro, 2);

    //   Serial.print(",");
    //   Serial.print(int_erro, 2);

    //   Serial.print(",");
    //   Serial.print(deriv_erro, 2);

    //   Serial.print(",");
    Serial.println(duty);
    }

    // ===============================
    // Verificação do fim do kickstart
    // ===============================
    if (kickstart_active && millis() > kickstart_end) {
      kickstart_active = false;
    }
  }

  // Pequeno delay para reduzir uso de CPU
  delay(1);
}

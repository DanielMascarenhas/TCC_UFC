// Defina o pino de entrada do sensor de fluxo
const int sensorDeFluxo = 2; // O pino digital 2
volatile int ContadorPulso = 0; // Contador de pulsos
float totalLitros = 0.0; // Total de litros contados
unsigned long temporizador = 0; // Tempo da última leitura
const unsigned long intervalo = 10000; // Intervalo de tempo para cálculos (em milissegundos, 10 segundos)
const float pulsosLitro = 450;//7.5; // Pulsos por litro para o sensor YF-S201
float alpha = 1.077;

void setup() {
  // Inicializa o pino do sensor como entrada
  pinMode(sensorDeFluxo, INPUT);
  // Configura a interrupção para contar pulsos
  attachInterrupt(digitalPinToInterrupt(sensorDeFluxo), contarPulsos, FALLING);
  // Inicializa a comunicação serial a 115200 baud rate
  Serial.begin(115200);
}

void loop() {
  unsigned long currentTime = millis();
  
  // Se passaram 10 segundos
  if (currentTime - temporizador >= intervalo ) {
    if (ContadorPulso > 0) { // Somente envia dados se houver fluxo
      // Calcula o total de litros nos últimos 10 segundos
      totalLitros = (ContadorPulso / pulsosLitro) * alpha;
      
      // Converte o valor para string e substitui o ponto por vírgula
      String totalLitrosStr = String(totalLitros);
      totalLitrosStr.replace('.', ',');

      // Envia apenas os números para o monitor serial
      Serial.println(totalLitrosStr);

      // Comentado: Envio de texto adicional
      // Serial.print("Total de Litros nos últimos 10 segundos: ");
      // Serial.println(" L");
    }

    // Reseta o contador de pulsos e o tempo da última leitura
    ContadorPulso = 0;
    temporizador = currentTime;
  }
}

// Função chamada pela interrupção para contar pulsos
void contarPulsos() {
  ContadorPulso++;
}

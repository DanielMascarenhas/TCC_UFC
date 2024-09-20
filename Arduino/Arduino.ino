// Defina o pino de entrada do sensor de fluxo
const int flowSensorPin = 2; // O pino digital 2
volatile int pulseCount = 0; // Contador de pulsos
float totalLiters = 0.0; // Total de litros contados
unsigned long lastTime = 0; // Tempo da última leitura
const unsigned long interval = 10000; // Intervalo de tempo para cálculos (em milissegundos, 10 segundos)
const float pulsesPerLiter = 450;//7.5; // Pulsos por litro para o sensor YF-S201
float alpha = 1.077;

void setup() {
  // Inicializa o pino do sensor como entrada
  pinMode(flowSensorPin, INPUT);
  // Configura a interrupção para contar pulsos
  attachInterrupt(digitalPinToInterrupt(flowSensorPin), countPulse, FALLING);
  // Inicializa a comunicação serial a 115200 baud rate
  Serial.begin(115200);
}

void loop() {
  unsigned long currentTime = millis();
  
  // Se passaram 10 segundos
  if (currentTime - lastTime >= interval) {
    if (pulseCount > 0) { // Somente envia dados se houver fluxo
      // Calcula o total de litros nos últimos 10 segundos
      totalLiters = (pulseCount / pulsesPerLiter) * alpha;
      
      // Converte o valor para string e substitui o ponto por vírgula
      String totalLitersStr = String(totalLiters);
      totalLitersStr.replace('.', ',');

      // Envia apenas os números para o monitor serial
      Serial.println(totalLitersStr);

      // Comentado: Envio de texto adicional
      // Serial.print("Total de Litros nos últimos 10 segundos: ");
      // Serial.println(" L");
    }

    // Reseta o contador de pulsos e o tempo da última leitura
    pulseCount = 0;
    lastTime = currentTime;
  }
}

// Função chamada pela interrupção para contar pulsos
void countPulse() {
  pulseCount++;
}

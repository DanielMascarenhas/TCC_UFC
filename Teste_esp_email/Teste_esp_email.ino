#include <ESP8266WiFi.h>
#include <ESP_Mail_Client.h> // Biblioteca para envio de e-mail

const char* ssid = "brisa-1761350";
const char* password = "xhwd107q";

// Configuração do SMTP
const char* emailSender = "seuemail@gmail.com";
const char* emailSenderPassword = "suasenha";
const char* emailRecipient = "destinatario@gmail.com";
const char* smtpServer = "smtp.gmail.com";
const int smtpPort = 465;

SMTPSession smtp;
ESP_Mail_Session mailSession;
SMTP_Message message;

unsigned long lastDataReceivedTime = 0;
unsigned long lastFlowDetectedTime = 0;
unsigned long lastEmailSentTime = 0;
const unsigned long dataTimeout = 10000; // Tempo sem dados antes de considerar como sem fluxo (10 segundos)
const unsigned long continuousFlowDuration = 3600000; // 1 hora em milissegundos

void setup() {
  Serial.begin(115200);

  // Conectar ao Wi-Fi
  WiFi.begin(ssid, password);

  Serial.println("Conectando ao WiFi...");
  unsigned long startAttemptTime = millis();

  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - startAttemptTime >= 30000) { // Timeout após 30 segundos
      Serial.println("Falha ao conectar ao WiFi.");
      return;
    }
    delay(1000);
    Serial.println("Conectando...");
  }
  Serial.println("WiFi conectado!");

  // Configurar a sessão SMTP
  mailSession.server.host_name = smtpServer;
  mailSession.server.port = smtpPort;
  mailSession.login.email = emailSender;
  mailSession.login.password = emailSenderPassword;
  mailSession.login.user_domain = "";

  smtp.debug(1);
}

void loop() {
  if (Serial.available() > 0) {
    String valor = Serial.readStringUntil('\n');  // Lê o valor da serial
    valor.trim();  // Remove espaços em branco extras

    float flowValue = valor.toFloat();
    unsigned long currentTime = millis();

    // Atualiza o tempo da última recepção de dados
    lastDataReceivedTime = currentTime;

    if (flowValue > 0) { // Se há fluxo
      lastFlowDetectedTime = currentTime; // Marca o último momento em que o fluxo foi detectado

      // Verifica se passou 1 hora desde o último e-mail e o fluxo contínuo ainda está presente
      if (currentTime - lastFlowDetectedTime >= continuousFlowDuration) {
        if (currentTime - lastEmailSentTime >= 3600000) { // Se passou 1 hora desde o último e-mail
          sendEmail();
          lastEmailSentTime = currentTime;
        }
      }
    } else {
      // Se não há fluxo e passou o tempo sem dados, reseta o último momento de detecção de fluxo
      if (currentTime - lastDataReceivedTime >= dataTimeout) {
        lastFlowDetectedTime = currentTime;
      }
    }
  } else {
    // Se não há dados disponíveis, verifica o tempo desde a última recepção
    unsigned long currentTime = millis();
    if (currentTime - lastDataReceivedTime >= dataTimeout) {
      // Se passou o tempo sem dados, reseta o último momento de detecção de fluxo
      lastFlowDetectedTime = currentTime;
    }
  }
}

void sendEmail() {
  // Configurar a mensagem de e-mail
  message.sender.name = "Nome";
  message.sender.email = emailSender;
  message.subject = "Alerta de Fluxo Contínuo";
  message.addRecipient("Destinatário", emailRecipient);
  message.text.content = "Fluxo contínuo detectado! Verifique imediatamente.";

  // Conectar ao servidor SMTP e enviar o e-mail
  if (!smtp.connect(&mailSession)) {
    Serial.println("Erro ao conectar ao servidor SMTP.");
    return;
  }

  if (!MailClient.sendMail(&smtp, &message)) {
    Serial.println("Erro ao enviar e-mail.");
  }

  smtp.closeSession();
}

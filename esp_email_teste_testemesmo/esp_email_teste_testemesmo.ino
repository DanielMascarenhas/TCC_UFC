#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ESP_Mail_Client.h> // Biblioteca para envio de e-mail

const char* ssid = "brisa-1761350";
const char* password = "xhwd107q";

const char* serverName = "https://script.google.com/macros/s/AKfycbzUxnpDp_jvjggB1rU2zm7-Pvo9sRHgR5CEHdh0Q_BAJfYI1Guqicnp0XTzczArUQ3G/exec";

// Configuração do SMTP
const char* emailSender = "dan.mmascar@gmail.com";
const char* emailSenderPassword = "sdvdlnpmriavesrk";
const char* emailRecipient = "dan.mmascar@gmail.com"; // Mesma conta para enviar e receber
const char* smtpServer = "smtp.gmail.com";
const int smtpPort = 465;

SMTPSession smtp;
ESP_Mail_Session mailSession;
SMTP_Message message;

unsigned long lastFlowDetectedTime = 0;
unsigned long lastEmailSentTime = 0;
unsigned long lastDataReceivedTime = 0;
const unsigned long oneMinuteDuration = 60000; // 1 minuto em milissegundos
const unsigned long fiveMinutesDuration = 300000; // 5 minutos em milissegundos
bool flowStoppedForOneMinute = false;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando ao WiFi...");
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
  unsigned long currentTime = millis();

  if (WiFi.status() == WL_CONNECTED) {
    if (Serial.available() > 0) {
      String valor = Serial.readStringUntil('\n');  // Lê o valor da serial
      valor.trim();  // Remove espaços em branco extras

      WiFiClientSecure client;
      client.setInsecure();  // Ignora a verificação do certificado SSL

      HTTPClient http;

      String url = serverName;
      url += "?valor=" + valor;  // Adiciona o valor à URL
    
      http.begin(client, url);  // Inicia a conexão HTTPS

      int httpResponseCode = http.GET();  // Envia a requisição GET

      if (httpResponseCode > 0) {
        Serial.println("Dado enviado com sucesso!");  // Confirma que o dado foi enviado
      } else {
        Serial.println("Erro ao enviar o dado.");  // Indica que houve um erro no envio
      }

      http.end();  // Encerra a conexão

      // Atualiza o tempo do último fluxo detectado
      lastFlowDetectedTime = currentTime;
      flowStoppedForOneMinute = false;  // Indica que o fluxo está ativo
    }
  } else {
    Serial.println("WiFi desconectado");
  }

  // Verifica se o fluxo parou por 1 minuto
  if ((currentTime - lastFlowDetectedTime >= oneMinuteDuration) && !flowStoppedForOneMinute) {
    flowStoppedForOneMinute = true;  // Marca que o fluxo foi interrompido por pelo menos 1 minuto
  }

  // Verifica se se passaram 5 minutos desde o último e-mail
  if (currentTime - lastEmailSentTime >= fiveMinutesDuration) {
    // Se o fluxo foi contínuo nos últimos 5 minutos (ou seja, não houve interrupção de 1 minuto)
    if (!flowStoppedForOneMinute) {
      sendEmail();
      lastEmailSentTime = currentTime;
      
      // Aqui atualizamos lastFlowDetectedTime para garantir que a condição seja verificada novamente nos próximos 5 minutos
      lastFlowDetectedTime = currentTime;
    }

    // Reseta a variável para monitorar o próximo período de 5 minutos
    flowStoppedForOneMinute = false;
  }
}

void sendEmail() {
  // Configurar a mensagem de e-mail
  message.sender.name = "Nome";
  message.sender.email = emailSender;
  message.subject = "Alerta de Fluxo Contínuo nos Últimos 5 Minutos";
  message.addRecipient("Destinatário", emailRecipient);
  message.text.content = "O fluxo de dados foi contínuo nos últimos 5 minutos, sem interrupção de 1 minuto. Verifique o sistema.";

  // Conectar ao servidor SMTP e enviar o e-mail
  if (!smtp.connect(&mailSession)) {
    Serial.println("Erro ao conectar ao servidor SMTP.");
    return;
  }

  if (!MailClient.sendMail(&smtp, &message)) {
    Serial.println("Erro ao enviar o e-mail.");
    return;
  } else {
    Serial.println("E-mail enviado com sucesso.");
  }

  smtp.closeSession();
}

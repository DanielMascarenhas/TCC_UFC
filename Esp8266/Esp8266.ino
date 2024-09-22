#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ESP_Mail_Client.h> // Biblioteca para envio de e-mail

const char* ssid = "Wi-fi";
const char* password = "password";

const char* serverName = "https://script.google.com/macros/s/insertSheetCode/exec";

// Configuração do SMTP
const char* emailSender = "email@gmail.com";
const char* emailSenderPassword = "emailPassword";
const char* emailRecipient = "email@gmail.com"; // Mesma conta para enviar e receber
const char* smtpServer = "smtp.gmail.com";
const int smtpPort = 465;

SMTPSession smtp;
ESP_Mail_Session mailSession;
SMTP_Message message;

unsigned long ultimaDeteccaoFluxo = 0;
unsigned long ultimoEmailEnviado = 0;
const unsigned long umMinutoEmMilisegundos = 60000; // 1 minuto em milissegundos
const unsigned long umDiaEmMilisegundos = 86400000; // 24 horas em milissegundos
const unsigned long horaEmMilisegundos = 3600000; // 1 hora em milissegundos
bool fluxoParado = false;
bool emailSent = false; // Flag para verificar se o e-mail foi enviado

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
      ultimaDeteccaoFluxo = currentTime;
      fluxoParado = false;  // Indica que o fluxo está ativo
    }
  } else {
    Serial.println("WiFi desconectado");
  }

  // Verifica se o fluxo parou por 1 minuto
  if ((currentTime - ultimaDeteccaoFluxo >= umMinutoEmMilisegundos) && !fluxoParado) {
    fluxoParado = true;  // Marca que o fluxo foi interrompido por pelo menos 1 minuto
  }

  // Verifica se se passaram 24 horas desde o último e-mail
  if (currentTime - ultimoEmailEnviado >= umDiaEmMilisegundos) {
    // Se o fluxo foi contínuo nas últimas 24 horas (ou seja, não houve interrupção de 1 minuto)
    if (!fluxoParado) {
      sendEmail();
      ultimoEmailEnviado = currentTime;
      emailSent = true;
      
      // Atualiza o tempo do último fluxo detectado para garantir que a condição seja verificada novamente nas próximas 24 horas
      ultimaDeteccaoFluxo = currentTime;
    }
  } 
  // Se um e-mail foi enviado e 1 hora se passou desde o último e-mail
  else if (emailSent && currentTime - ultimoEmailEnviado >= horaEmMilisegundos) {
    if (!fluxoParado) {
      sendEmail();
      ultimoEmailEnviado = currentTime;
    }
  }
}

void sendEmail() {
  // Configurar a mensagem de e-mail
  message.sender.name = "Sensor de Fluxo";
  message.sender.email = emailSender;
  message.subject = "Fluxo Anormal Detectado";
  message.addRecipient("Destinatário", emailRecipient);
  message.text.content = "Nas últimas 24 horas, foi detectado um fluxo contínuo de água sem interrupção de 1 minuto. Existe um alerta de possível vazamento. Verifique o sistema.";

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

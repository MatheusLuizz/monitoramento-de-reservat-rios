#include <ESP8266WiFiMulti.h>
#include <ArduinoMqttClient.h>
#include <NewPing.h>
#include <LiquidCrystal.h>

// Conectando à rede sem fio - início
#ifndef STASSID
#define STASSID "" // Inserir o nome da rede WiFi 
#define STAPSK ""  // Inserir a senha da rede WiFi
#endif

const char* ssid = STASSID;
const char* senha = STAPSK;

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);
ESP8266WiFiMulti WiFiMulti;
// fim da conexão na rede sem fio

// Declarações do MQTT
const char broker[] = "192.168.1.3";  // IP servidor, obtido a partir do comando "ipconfig /all" no terminal
int        port     = 1883; // Porta padrão do MQTT
const char topic[]  = "";   // Inserir o nome do tópico ao qual o MQTT enviará os dados

// Definindo tempo para envio das mensagens ao servidor MQTT
const long interval = 8000;
unsigned long previousMillis = 0;
int count = 0;

// Inicializar Pinos (Defina da maneira que desejar!!!)

#define reservatorioAlto	4
#define reservatorioMedio	5
#define reservatorioBaixo	16

//LCD

LiquidCrystal lcd(0, 2, 15, 13, 12, 14);

//Sensor Ultrassonico
#define trigger      1
#define echo         3
#define maxDist 450

//Definição de Variáveis

float volumeReservatorio;
float distancia;
float duracao;

NewPing sonar(trigger, echo, maxDist);


void setup() {
  Serial.begin(9600);
    // Iniciando a conexão WiFi
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(ssid, senha);

  Serial.println();
  Serial.println();
  Serial.println("Aguardando.");

  while (WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.println("WiFi Conectado com sucesso");
  Serial.print("Endereco IP: ");
  Serial.println(WiFi.localIP());

    // Iniciando a conexão com o MQTT
  Serial.print("Tentando conectar com o MQTT broker: ");
  Serial.println(broker);

  if (!mqttClient.connect(broker, port)) {
    Serial.print("Falha ao conectar ao MQTT! Erro: ");
    Serial.println(mqttClient.connectError());

    while(1);
  }
  Serial.println("MQTT broker conectado com sucesso!");
  Serial.println();

  pinMode(reservatorioAlto, OUTPUT);
  pinMode(reservatorioMedio, OUTPUT);
  pinMode(reservatorioBaixo, OUTPUT);
  pinMode(trigger, OUTPUT);
  pinMode(echo, INPUT_PULLUP);

}

void loop() {

  long distancia = sonar.ping_cm(); // Recebe a distância obtida pelo sensor ultrassônico em cm
  Serial.print("Distancia: ");
  Serial.println(distancia);

  mqttClient.poll();
  unsigned long currentMillis = millis();

  if(currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    Serial.print("Enviando mensagem ao topic: ");
    Serial.print(topic);
    Serial.print(": ");
    Serial.print(volumeReservatorio);       // volume da caixa, deve ser calculado com as informações de distância do sensor ultrassônico
    Serial.println("%");

  // Enviando mensagem ao servidor 
  mqttClient.beginMessage(topic);
  mqttClient.print(String(volumeReservatorio));
  mqttClient.endMessage();
  }
  
  if(distancia >= 50){
    digitalWrite(reservatorioBaixo, LOW);
    digitalWrite(reservatorioMedio, LOW);
    digitalWrite(reservatorioAlto, HIGH);
  }
  if(distancia < 50 && distancia >= 30){
    digitalWrite(reservatorioBaixo, LOW);
    digitalWrite(reservatorioMedio, HIGH);
    digitalWrite(reservatorioAlto, LOW);
  }
  if(distancia < 30){
    digitalWrite(reservatorioBaixo, HIGH);
    digitalWrite(reservatorioMedio, LOW);
    digitalWrite(reservatorioAlto, LOW);
  }

  // Configuração do painel LCD
  
  lcd.clear();

  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Volume: " + (String) volumeReservatorio + "%");

  delay(2000);
}

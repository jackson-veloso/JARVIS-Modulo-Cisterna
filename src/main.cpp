#include <Arduino.h>

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "ArduinoJson.h"
#include "painlessMesh.h"
#include "DHT.h"

#define MESH_PREFIX "whateverYouLike"
#define MESH_PASSWORD "somethingSneaky"
#define MESH_PORT 5555
#define ID 2

// Configurar a rede WIFI
#define STATION_SSID "Grogu"
#define STATION_PASSWORD "123@Mudar"
#define STATION_CHANNEL 3

// #######################################
//  Configuração Sensores (GPIO)
#define pin_SensorCheio 33
#define pin_SensorMeio 32
#define pin_SensorVazio 27
#define pin_Rele 19
#define pin_Button 21
#define pin_DHT11 4

DHT dht(pin_DHT11, DHT11);

bool statusSensorCheio = LOW, statusSensorMeio = LOW, statusSensorVazio = LOW;
bool ultimoStatusSensorCheio = LOW, ultimoStatusSensorMeio = LOW, ultimoStatusSensorVazio = LOW;
bool ultimoStatusButton = LOW;
bool statusRele = HIGH;

unsigned long tempoUltimoDebounceSensorCheio = 0, tempoUltimoDebounceSensorMeio = 0, tempoUltimoDebounceSensorVazio = 0;
unsigned long tempoUltimoDebounceButton = 0;
byte tempoDebounce = 40;

void readSensorCheio();
void readSensorMeio();
void readSensorVazio();
void readButton();
void setModuleRele(bool status);

void readSensorCheio()
{
  bool leitura = digitalRead(pin_SensorCheio);

  if (leitura != ultimoStatusSensorCheio)
  {
    tempoUltimoDebounceSensorCheio = millis();
  }

  if ((millis() - tempoUltimoDebounceSensorCheio) > tempoDebounce)
  {
    if (leitura != statusSensorCheio)
    {
      statusSensorCheio = leitura;
    }
  }

  ultimoStatusSensorCheio = leitura;
}

void readSensorMeio()
{
  bool leitura = digitalRead(pin_SensorMeio);

  if (leitura != ultimoStatusSensorMeio)
  {
    tempoUltimoDebounceSensorMeio = millis();
  }

  if ((millis() - tempoUltimoDebounceSensorMeio) > tempoDebounce)
  {
    if (leitura != statusSensorMeio)
    {
      statusSensorMeio = leitura;
    }
  }

  ultimoStatusSensorMeio = leitura;
}

void readSensorVazio()
{
  bool leitura = digitalRead(pin_SensorVazio);

  if (leitura != ultimoStatusSensorVazio)
  {
    tempoUltimoDebounceSensorVazio = millis();
  }

  if ((millis() - tempoUltimoDebounceSensorVazio) > tempoDebounce)
  {
    if (leitura != statusSensorVazio)
    {
      statusSensorVazio = leitura;

      // desliga o Rele se o sensor Vazio for ativado
      if (statusSensorVazio)
      {
        setModuleRele(true); // desliga modulo rele
      }
    }
  }

  ultimoStatusSensorVazio = leitura;
}

void readButton()
{
  bool leitura = digitalRead(pin_Button);

  if (leitura != ultimoStatusButton)
  {
    tempoUltimoDebounceButton = millis();
  }

  if ((millis() - tempoUltimoDebounceButton) > 2000) // botão acionado após 02 segundos
  {
    if (leitura != LOW)
    {
      Serial.println(leitura);
      setModuleRele(!statusRele);
      tempoUltimoDebounceButton = millis();
    }
  }

  ultimoStatusButton = leitura;
}

void setModuleRele(bool status)
{
  // false/LOW - liga Rele
  // true/HIGH - desliga rele
  if (!status)
  {
    // se sensor vazio acionado (TRUE) rele não pode ligar
    if (!statusSensorVazio)
    {
      statusRele = status;
    }
  }
  else
  {
    statusRele = status;
  }
}
// #######################################

//$$$$$$$$$$$$$$$$$$$$$
// WATCHDOG
hw_timer_t *timer = NULL; // faz o controle do temporizador (interrupção por tempo)

// função que o temporizador irá chamar, para reiniciar o ESP32
void IRAM_ATTR resetModule()
{
  ets_printf("(watchdog) reiniciar\n"); // imprime no log
  esp_restart();                        // reinicia o chip
  // esp_restart_noos(); //não funcionou foi substituída pela linha de cima.
}
//$$$$$$$$$$$$$$$$$$$$$

//================================
// Configuracao REDE MESH
byte contCheckConnection = 0;
byte contCheckConnectionRoot = 0;

Scheduler userScheduler;
painlessMesh mesh;

// Prototype
void receivedCallback(uint32_t from, String &msg);
void getMessageHello(JsonDocument myObject);
void sendMessageHello();
void checkConnection();
void sendSensorReandingNivelCaixaDagua();
void sendStatusRele();
void sendSensorDHT11();
void setStatusSensors(JsonDocument doc);

Task taskSendMessageHello(TASK_SECOND * 11, TASK_FOREVER, &sendMessageHello);
Task taskCheckConnection(TASK_SECOND * 20, TASK_FOREVER, &checkConnection);
Task taskSendSensorReandingNivelCaixaDagua(TASK_SECOND * 5, TASK_FOREVER, &sendSensorReandingNivelCaixaDagua);
Task taskSendStatusRele(TASK_SECOND * 5, TASK_FOREVER, &sendStatusRele);
Task taskSendSensorDHT11(TASK_SECOND * 5, TASK_FOREVER, &sendSensorDHT11);

void sendSensorReandingNivelCaixaDagua()
{
  JsonDocument doc;

  doc["code"] = 200;
  doc["idSensor"] = 1002;
  doc["description"] = "nivel de agua";
  doc["cheio"] = statusSensorCheio;
  doc["Meio"] = statusSensorMeio;
  doc["vazio"] = statusSensorVazio;

  String json;
  serializeJson(doc, json);
  mesh.sendBroadcast(json);
  Serial.print("Enviando leitura de sensores = ");
  Serial.println(json);
}

void sendStatusRele()
{
  JsonDocument doc;

  doc["code"] = 200;
  doc["idSensor"] = 1102;
  doc["description"] = "chave rele";
  doc["status"] = statusRele;

  String json;
  serializeJson(doc, json);
  mesh.sendBroadcast(json);
  Serial.print("Enviando leitura de sensores = ");
  Serial.println(json);
}

void sendSensorDHT11()
{
  JsonDocument doc;

  doc["code"] = 200;
  doc["idSensor"] = 1402;
  doc["description"] = "sensor DHT11";

  float humi = dht.readHumidity();
  float temp = dht.readTemperature();

  doc["temp"] = roundf(temp);
  doc["humi"] = roundf(humi);

  String json;
  serializeJson(doc, json);
  mesh.sendBroadcast(json);
  Serial.print("Enviando leitura de sensores = ");
  Serial.println(json);
}

void sendMessageHello()
{
  JsonDocument hello;
  hello["code"] = 100;
  hello["id"] = ID;
  hello["description"] = "Modulo Cisterna";

  String json;
  serializeJson(hello, json);
  mesh.sendBroadcast(json);
  Serial.print("Send Message Hello = ");
  Serial.println(json);
}

void getMessageHello(JsonDocument doc)
{
  byte id = doc["id"];

  Serial.print("ID recebido = ");
  Serial.println(id);
}

void setStatusSensors(JsonDocument doc)
{
  byte idModule = doc["idModule"];

  if (idModule == ID)
  {
    int idSensor = doc["idSensor"];

    if (idSensor == 1102)
    {
      bool status = doc["status"];

      setModuleRele(status);

      Serial.print("chave Rede = ");
      Serial.println(statusRele);
    }
  }
}

void checkConnection()
{
  // Se a Lista de Node igual a 01 reinicia a rede mesh para tentar se reconectar
  contCheckConnection++;
  Serial.print("NodeList = ");
  Serial.println(mesh.getNodeList(true).size());
  Serial.print("total de verificações de conexão ativa: ");
  Serial.println(contCheckConnection);

  if (mesh.getNodeList(true).size() == 1)
  {
    if (contCheckConnection > 7)
    {
      Serial.println(" ###### ==== ##### REINICIAR ESP32 ******&&&&&******&&&&");
      esp_restart(); // reinicia o chip
    }
  }
  else
  {
    contCheckConnection = 0;
  }
}

void receivedCallback(uint32_t from, String &msg)
{
  Serial.printf("startHere: Received from %u msg= %s\n", from, msg.c_str());
  Serial.println();

  JsonDocument doc;
  deserializeJson(doc, msg.c_str());

  int code = doc["code"];

  switch (code)
  {
  case 100:
    getMessageHello(doc);
    break;

  case 200:
    // msg de leitura de sensores destina ao Node Root
    break;
  case 300:
    setStatusSensors(doc);
    break;

  default:
    break;
  }
}

void newConnectionCallback(uint32_t nodeId)
{
  Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
  Serial.println();
}

void changedConnectionCallback()
{
  Serial.printf("Changed connections\n");
  Serial.println();
}

void nodeTimeAdjustedCallback(int32_t offset)
{
  Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset);
  Serial.println();
}
//================================

void setup()
{
  Serial.begin(115200);

  // Configuracao dos Sensores de Nivel de Agua
  pinMode(pin_SensorCheio, INPUT);
  pinMode(pin_SensorMeio, INPUT);
  pinMode(pin_SensorVazio, INPUT);
  pinMode(pin_Button, INPUT);
  pinMode(pin_Rele, OUTPUT);

  dht.begin();

  mesh.setDebugMsgTypes(ERROR | STARTUP | CONNECTION);

  // Configurar o canal do AP, deverá ser o mesmo para toda a rede mesh e roteador wifi
  mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT, WIFI_AP_STA, STATION_CHANNEL);

  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  userScheduler.addTask(taskSendMessageHello);
  userScheduler.addTask(taskCheckConnection);
  userScheduler.addTask(taskSendSensorReandingNivelCaixaDagua);
  userScheduler.addTask(taskSendStatusRele);
  userScheduler.addTask(taskSendSensorDHT11);

  taskSendMessageHello.enable();
  taskCheckConnection.enable();
  taskSendSensorReandingNivelCaixaDagua.enable();
  taskSendStatusRele.enable();
  taskSendSensorDHT11.enable();

  // This node and all other nodes should ideally know the mesh contains a root, so call this on all nodes
  mesh.setContainsRoot(true);

  // WATCHDOG
  // hw_timer_t * timerBegin(uint8_t num, uint16_t divider, bool countUp)
  /*
     num: é a ordem do temporizador. Podemos ter quatro temporizadores, então a ordem pode ser [0,1,2,3].
    divider: É um prescaler (reduz a frequencia por fator). Para fazer um agendador de um segundo,
    usaremos o divider como 80 (clock principal do ESP32 é 80MHz). Cada instante será T = 1/(80) = 1us
    countUp: True o contador será progressivo
  */
  timer = timerBegin(0, 80, true); // timerID 0, div 80
  // timer, callback, interrupção de borda
  timerAttachInterrupt(timer, &resetModule, true);
  // timer, tempo (us), repetição
  timerAlarmWrite(timer, 10000000, true); // reinicia após 10s
  timerAlarmEnable(timer);                // habilita a interrupção
}

void loop()
{
  digitalWrite(pin_Rele, statusRele);

  mesh.update();

  // Sensores de nivel de agua
  readSensorCheio();
  readSensorMeio();
  readSensorVazio();
  readButton();
  // WATCHDOGs
  timerWrite(timer, 0); // reseta o temporizador (alimenta o watchdog)

}

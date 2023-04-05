#include <DHT.h>
#include<WiFi.h>
#include <PubSubClient.h>

#define SAIDA 2

#define DHTPIN 5 
#define DHTTYPE DHT11

char ssid[] = "nome_rede";
char pass[] = "senha_rede";

const char* server = "mqtt.thingspeak.com";
char mqttUserName[] = "********";
char mqttPass[] = "********";
long ChannelID = 1320249;
char writeAPIKey[] = "********";
char readAPIKey[] = "********";

WiFiClient esp32client;
PubSubClient mqttClient( esp32client );

float Umidade;
float Temperatura;

boolean EstadoSaida;
unsigned long lastConnectionTime = 0;
const unsigned long postingInterval = 15000;

void calcula_umidade();
void calcula_temperatura();
int mqttSubscriptionCallback(char* topic, byte* payload, unsigned int lenght);
void mqConnect();
int mqttSubscribe();
int mqttUnSubscribe();
void mqttPublish();
int connectWifi();
void getID(char clientID[], int idLength);

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  Serial.println("Inicio");
  connectWifi();
  mqttClient.setServer( server, 1883 );
  mqttClient.setCallback( mqttSubscriptionCallback );

  pinMode(DHTPIN,INPUT);
  pinMode(SAIDA,OUTPUT);
  digitalWrite(SAIDA,LOW);

  dht.begin();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED){
    connectWifi();
    }
  if (!mqttClient.connected()){
    mqConnect();
    if(mqttSubscribe()==1){
        Serial.println("Subscrito");
        }
    }

    mqttClient.loop();
        if (millis() - lastConnectionTime> postingInterval){
            mqttpublish();
          }
}

void mqttpublish(){
    calcula_umidade();
    calcula_temperatura();

    String data = String("field1=" + String(Umidade, DEC) + "&field2=" + String(Temperatura,DEC));
    int length = data.length();
    char msgBuffer[length];
    data.toCharArray(msgBuffer,length+1);
    Serial.println(msgBuffer);

    String topicString = "channels/" + String( ChannelID )+ "/publish/" + String(writeAPIKey);
    length=topicString.length();
    char topicBuffer[length];
    topicString.toCharArray(topicBuffer,length+1);

    mqttClient.publish( topicBuffer, msgBuffer );

    lastConnectionTime = millis();
  }

void mqConnect(){
    char clientID[ 9 ];

    while (!mqttClient.connected() ){
      getID(clientID,8);
      Serial.print("Testando conexão MQTT");
        if(mqttClient.connect( clientID, mqttUserName, mqttPass)){
          Serial.println("Conectado com o Cliente ID: " + String(clientID) + "Usuário" + String(mqttUserName)+ "Pwd" + String(mqttPass));
          }else{
            Serial.print("Falha");
            Serial.print(mqttClient.state());
            delay(5000);
          }
      }
  }

void getID(char clientID[], int idLength){
  static const char alphanum[]= "0123456789"
  "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
  "abcdefghijklmnopqrstuvwxyz";

  for (int i = 0; i<idLength; i++) {
    clientID[i]= alphanum[random(51)];
    }
    clientID[idLength] = '\0';
  }

int mqttSubscribe(){
  String myTopic;
  
  myTopic= "channels/" + String(ChannelID) + "/subscribe/fields/field3/" + String(readAPIKey);
  Serial.println( "Subscrito de " +myTopic );
  Serial.println( "Estado= " + String( mqttClient.state() ) );
  char charBuf[myTopic.length()+1];
  myTopic.toCharArray(charBuf, myTopic.length()+1);

  return mqttClient.subscribe(charBuf);
  
}
  
int mqttUnSubscribe(){
  String myTopic;
  myTopic = "channels/"+String(ChannelID)+"/subscribe/fields/field4/"+String(readAPIKey);

  char charBuf[myTopic.length()+1];
  myTopic.toCharArray(charBuf, myTopic.length()+1);

  return mqttClient.subscribe(charBuf);
}


int connectWifi(){
  
  while(WiFi.status() != WL_CONNECTED){
    WiFi.begin(ssid,pass);
    delay(2500);
    Serial.println("Conectando ao WIFI");
    }
    Serial.println("Conectado");
  }

void calcula_umidade(){
  delay(2000);
  Umidade = dht.readHumidity();
}

void calcula_temperatura(){
  delay(2000);
  Temperatura = dht.readTemperature();
  if(isnan(Temperatura)){
    Serial.println("Falha na leitura do sensor.");
  }
}

int mqttSubscriptionCallback(char* topic, byte* payload, unsigned int mesLength){
  String msg;
    for(int i=0; i <mesLength; i++){
      char c = (char)payload[i];
      msg += c;
    }

    if(msg.equals("1")){
      digitalWrite(SAIDA,HIGH);
      EstadoSaida = 1;
}
    if(msg.equals("0")){
      digitalWrite(SAIDA,LOW);
      EstadoSaida = 0;
}      
}  
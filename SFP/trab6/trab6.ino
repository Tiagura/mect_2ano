#include <ESP8266WiFi.h>
#include <PubSubClient.h>

WiFiClient TCP_Client; // Objecto do tipo TCP
PubSubClient client(TCP_Client); // Objecto do tipo cliente MQTT
int gpio0, gpio2=0, gpio4, a0; // Variáveis para os pinos do ESP8266

void setup() {
    Serial.begin(115200);

    // WiFi network
    Serial.println(); Serial.print("Connecting");
    WiFi.mode(WIFI_STA);
    WiFi.begin("idk", "tiagomarques");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500); Serial.print(".");
    }
    Serial.println(); Serial.print("Connected, IP address: "); Serial.println(WiFi.localIP());

    // MQTT
    client.setServer("192.168.43.118", 1883); // O Servidor MQTT, Broker, está no docker container
    client.setCallback(callback);
    client.connect("esp8266"); // O ESP regista-se no Broker, com o nome “esp8266"
    client.subscribe("led_control"); // O ESP subscreve o tópico “led_control"

    // GPIO
    pinMode(0,INPUT);
    pinMode(2,OUTPUT);
    pinMode(4,INPUT);
    pinMode(A0, INPUT);
}

void loop() {
    if (!client.connected()) {
        client.connect("esp8266"); // O ESP Regista-se no Broker com o nome esp8266
    }

    client.loop();
    //read values 
    gpio0 = digitalRead(0);
    gpio2 = digitalRead(2);
    gpio4 = digitalRead(4);
    a0 = analogRead(A0);

    if(!(client.publish("flash_state", String(gpio0).c_str()))) {
        Serial.println("Publish failed");
    }

    if(!(client.publish("led_state", String(gpio2).c_str()))) {
        Serial.println("Publish led_state failed");
    }

    if(!(client.publish("press_state", String(gpio4).c_str()))){
        Serial.println("Publish press_state failed");
    }

    if(!(client.publish("foto_state", String(a0).c_str()))){
        Serial.println("Publish foto_state failed");
    }
    delay(1000);
} 

void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message arrived ["); Serial.print(topic); Serial.print("] ");
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();

    //if topic is "led_control" then turn on/off LED
    if (strcmp(topic, "led_control") == 0) {
        if ((char)payload[0] == '1') {
            digitalWrite(2, LOW);
        } else {
            digitalWrite(2, HIGH);
        }
    }
}

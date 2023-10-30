#include <ESP8266WiFi.h>

const char* ssid = "idk"; // Nome da rede Wireless
const char* password = "tiagomarques"; // Password da rede Wireless
const char* host = "192.168.43.118"; // Endereço do PC do Apache
String url, url1; // Irá de conter a msg HTTP enviada para o Apache
String s; // Irá conter o estado do botão nº4 (0 ou 1)
WiFiClient client; // WiFiClient, permite enviar ou receber dados por TCP/IP
const int httpPort = 80;

int gpio0, gpio2=0, gpio4, a0;

void setup() {
    Serial.begin(115200);
    pinMode(4,INPUT); // Config o pino 4 como input , o sensor/botão está ligado no pino 4

    // Envia para o Serial Monitor (só para Debub)
    Serial.println(); 
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.mode(WIFI_STA); // O ESP irá como Client Wireless
    WiFi.begin(ssid, password); // ESP tenta ligar ao Router
    while (WiFi.status() != WL_CONNECTED) { // status da ligação Wifi com o router= 0 - desligado 4- ligado ...
        delay(500); Serial.print("."); 
    }
    // O ESP envia por porta série, para o PC local esta informaçao:
    Serial.println("O ESP está ligado por WiFi ao Router, o Router deu-lhe o IP "); 
    Serial.println(WiFi.localIP());

    pinMode(0,INPUT);
    pinMode(2,OUTPUT);
    pinMode(4,INPUT);
    pinMode(A0, INPUT);
} 

void loop() {
    // ------------------- TENTA LIGAR POR TCP/IP ao APACHE
    // Se o status da ligação TCP/IP entre o ESP e o computador remoto for igual a 0 (client.status= 0 - desligado)
    // Neste caso, o ESP tenta estabelecer ligacao TCP/IP com o Apache
    if(client.status()==0){
        Serial.print("connecting to "); Serial.println(host); // envia para o serial monitor
        if (!client.connect(host, httpPort)) { // ------------------- Tenta LIGAÇAO TCP
            Serial.println("connection failed");
            client.flush();
            delay(1000);
        }
    }

    //read values 
    gpio0 = digitalRead(0);
    gpio2 = digitalRead(2);
    gpio4 = digitalRead(4);
    a0 = analogRead(A0);

    //print values
    Serial.println("gpio0: "+ String(gpio0));
    Serial.println("gpio2: "+ String(gpio2));
    Serial.println("gpio4: "+ String(gpio4));
    Serial.println("a0: "+ String(a0));

    // ----------------- SE LIGACAO TCP ESTABELECIDA ...
    // Se o status da ligação for 4, significa que a ligação TCP/IP está estabelecida entre o ESP e o Apache
    if(client.status()==4){

        String url1 = "/CtrPLC.php";
        String postData = "saidaY0="+ String(0) +"&saidaY1="+ String(1) +"&saidaY2="+ String(2) + "&leituraX0=" + String(gpio0) + "&leituraX1=" + String(gpio2) + "&leituraX2=" + String(gpio4) + "&leituraX3=" + String(a0);
        String url = String("POST ") + url1 + " HTTP/1.1\r\n" +
              "Host: " + host + "\r\n" +
              "Content-Type: application/x-www-form-urlencoded\r\n" +
              "Content-Length: " + String(postData.length()) + "\r\n" +
              "Connection: keep-alive\r\n\r\n" +
              postData;

        client.print(url); // Send the HTTP request to the server// Envia pedido HTTP, da linha anterior "url", para o Apache
        // Espera pela resposta do Apache
        delay(100);
    }

    // ------------------- SE CHEGARAM DADOS por TCP/IP ....
    if (client.available() > 0) { // Se o ESP tiver recebido bytes
        // Le todos os carateres enviados pelo Apache até receber o caracter 'ç' ou exceder um tempo máximo.
        String line = client.readStringUntil('ç');
        Serial.print(line); // envia para o Serial Monitor (para Debug)
        client.flush();
    }

    Serial.println();
} 
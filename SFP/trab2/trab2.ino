#include <ESP8266WiFi.h>

const char* html = R"(
<!DOCTYPE html>
<html>
<head>
    <title>GUI</title>
    <meta charset="utf-8">
    <style>
        body {
            font-family: Arial, Helvetica, sans-serif;
            font-size: 14px;
        }
        button {
            padding: 12px 20px;
            margin: -16px 4px; /* Adjust the margin for spacing between buttons */
            box-sizing: border-box;
            text-align: center;
            font-size: 16px; /* Larger font size */
            font-weight: bold; /* Bolder text */
            border-radius: 10px; /* Rounded corners */
        }
        /* Style the ON button */
        .on-button {
            background-color: green;
            color: white;
        }
        /* Style the OFF button */
        .off-button {
            background-color: red;
            color: white;
        }
        div {
            padding: 10px;
        }
        /* Style the "Blue Led" text */
        .label-text {
            font-size: 20px; /* Slightly smaller font size */
            font-weight: bold; /* Bolder text */
        }
        /* Style the "STATUS" and "CONTROLS" text */
        .title-text {
            font-size: 24px; /* Larger font size for STATUS */
            font-weight: bold; /* Bolder text */
        }
        /* Style additional text */
        .additional-text {
            font-size: 20px; /* Larger font size for additional text */
            font-weight: bold; /* Bolder text */
        }
        /* Add CSS styles for better appearance */
        .column {
            flex: 1;
            padding: 10px;
            text-align: center;
            display: flex;
            flex-direction: column;
            align-items: center;
        }
        .credits {
            font-size: 20px; /* Larger font size for additional text */
            font-weight: bold; /* Bolder text */
            flex: 1;
            padding: 10px;
            text-align: center;
            display: flex;
            flex-direction: column;
            align-items: center;
        }
    </style>
    <script>
        function refreshButtonStates() {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
            var response = JSON.parse(this.responseText);
            document.getElementById('gpio2Status').innerHTML = response.gpio2Status;
            document.getElementById('gpio2Status').style.color = response.gpio2Color;
            document.getElementById('gpio0Status').innerHTML = response.gpio0Status;
            document.getElementById('gpio0Status').style.color = response.gpio0Color;
            document.getElementById('gpio4Status').innerHTML = response.gpio4Status;
            document.getElementById('gpio4Status').style.color = response.gpio4Color;
            document.getElementById('a0Value').innerHTML = response.analog0Value;
            }
        };
        xhttp.open("GET", "/getButtonStates", true);
        xhttp.send();
        }
        
        // Periodically refresh button states every 500ms
        setInterval(refreshButtonStates, 500);
    </script>
</head>
<body>
    <div style="display: flex;" align="center">
        <!-- Left Column -->
        <div class="column">
            <a href="https://www.ua.pt" target="_blank">
                <img src="http://bestaveiro.web.ua.pt/images/university/ua_logo.png" alt="uaLogo" width="350" height="150">
            </a>
        </div>

        <!-- Middle Column -->
        <div class="column"> <!-- Added class "middle-column" -->
            <p class="title-text">CONTROLS</p>
            <p class="label-text">BLUE LED</p> <!-- Adjusted font size -->
            <div>
                <button class="on-button" onclick="window.location.href = '/GPIO2=0'" >ON</button>
                <button class="off-button" onclick="window.location.href = '/GPIO2=1'" >OFF</button>
            </div>
        </div>

        <!-- Right Column -->
        <div class="column">
            <p class="title-text">STATUS</p>
            <p class="additional-text">GPIO0: <span id="gpio0Status" style="color: red"> OFF </span> </p>
            <p class="additional-text">GPIO2: <span id="gpio2Status" style="color: red"> OFF </span> </p>
            <p class="additional-text">GPIO4: <span id="gpio4Status" style="color: red"> OFF </span> </p>
            <p class="additional-text">A0: <span id="a0Value" > 0 </span> </p>
        </div>
    </div>
    <div class="credits"> <b>Developed by: Tiago Marques, 98459</b> </div>
</body>
</html>
)";

WiFiServer server(80); 
int gpio0, gpio2, gpio4, a0;

void setup(){
    Serial.begin(115200);
    WiFi.begin("idk", "tiagomarques");

    // espera que o Router aceite a ligação WiFi
    while(WiFi.status() != WL_CONNECTED ) {
    delay(500); Serial.println("Wainting for connection...");
    }
    Serial.println("ESP connecto to router , by Wifi");

    // ativa o servidor TCP/IP
    server.begin();
    Serial.println("TCP server active");
    // IP address
    Serial.print("Server IP:");
    Serial.println(WiFi.localIP());

    pinMode(0,INPUT);
    pinMode(2,OUTPUT);
    pinMode(4,INPUT);
    pinMode(A0, INPUT);
}

void loop(){
    // Espera que o Browser peça ligação TCP/IP
    WiFiClient client = server.available();
    if (client) { // Se chegou uma ligação TCPIP

        // Espera que o Browser envie dados , ex. GET index.html
        Serial.println("New client");
        while(!client.available()){ delay(1); }
        Serial.println("Message sent from client:");

        // Lê a mensagem recebida pelo ESP, enviada pelo Browser
        String ss = client.readStringUntil('\r');
        Serial.println(ss);
        client.flush();

        // Read
        gpio0 = digitalRead(0);
        gpio4 = digitalRead(4);
        a0 = analogRead(A0);

        if(ss.indexOf("/getButtonStates") != -1){
            // Generate JSON
            String json = R"({"gpio2Status":")" +
                            String(gpio2 == 0 ? "ON" : "OFF") + R"(","gpio2Color":")" +
                            String(gpio2 == 0 ? "green" : "red") + R"(","gpio0Status":")" +
                            String(gpio0 == 0 ? "ON" : "OFF") + R"(","gpio0Color":")" +
                            String(gpio0 == 0 ? "green" : "red") + R"(","gpio4Status":")" +
                            String(gpio4 == 0 ? "ON" : "OFF") + R"(","gpio4Color":")" +
                            String(gpio4 == 0 ? "green" : "red") + R"(","analog0Value":")" +
                            String(a0) + R"("})";
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: application/json");
            client.println("Connection: close");
            client.println();
            client.print(json);
        }else{
            // Processa mesg recebida pelo ESP
            if (ss.indexOf("GPIO2=1") >0 ){gpio2 = 1;}
            if (ss.indexOf("GPIO2=0") >0 ){gpio2 = 0;}
            digitalWrite(2,gpio2);

            // Envia pág HTML para o Browser
            client.print(html);

            //Generate JavaScript based on the current GPIO status
            String script = R"(
            <script>
                document.getElementById('gpio2Status').innerHTML = ')" +
                            String(gpio2 == 0 ? "ON" : "OFF") + R"(';
                document.getElementById('gpio2Status').style.color = ')" +
                            String(gpio2 == 0 ? "green" : "red") + R"(';
                document.getElementById('gpio0Status').innerHTML = ')" +
                            String(gpio0 == 0 ? "ON" : "OFF") + R"(';
                document.getElementById('gpio0Status').style.color = ')" +
                            String(gpio0 == 0 ? "green" : "red") + R"(';
                document.getElementById('gpio4Status').innerHTML = ')" +
                            String(gpio4 == 0 ? "ON" : "OFF") + R"(';
                document.getElementById('gpio4Status').style.color = ')" +
                            String(gpio4 == 0 ? "green" : "red") + R"(';
                document.getElementById('a0Value').innerHTML = ')" +
                            String(a0) + R"(';
            </script>
            )";
            client.print(script.c_str()); // Send the JavaScript
        }
    }
}
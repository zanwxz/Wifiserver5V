#include <WiFi.h>

// Replace with your network credentials
const char* ssid = "@SPVC_Student";
const char* password = "spvcwifi";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String output26State = "off";
String output27State = "off";

// Assign output variables to GPIO pins
const int output26 = 26;
const int output27 = 27;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void setup() {
  Serial.begin(115200);
  // Initialize the output variables as outputs
  pinMode(output26, OUTPUT);
  pinMode(output27, OUTPUT);
  // Set outputs to LOW
  digitalWrite(output26, HIGH);
  digitalWrite(output27, HIGH);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop() {
  WiFiClient client = server.available();  // Listen for incoming clients

  if (client) {  // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");                                             // print a message out in the serial port
    String currentLine = "";                                                   // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
      if (client.available()) {  // if there's bytes to read from the client,
        char c = client.read();  // read a byte, then
        Serial.write(c);         // print it out the serial monitor
        header += c;
        if (c == '\n') {  // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // turns the GPIOs on and off
            if (header.indexOf("GET /26/on") >= 0) {
              Serial.println("GPIO 26 on");
              output26State = "on";
              digitalWrite(output26, LOW);
            } else if (header.indexOf("GET /26/off") >= 0) {
              Serial.println("GPIO 26 off");
              output26State = "off";
              digitalWrite(output26, HIGH);
            } else if (header.indexOf("GET /27/on") >= 0) {
              Serial.println("GPIO 27 on");
              output27State = "on";
              digitalWrite(output27, LOW);
            } else if (header.indexOf("GET /27/off") >= 0) {
              Serial.println("GPIO 27 off");
              output27State = "off";
              digitalWrite(output27, HIGH);
            }

            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta charset=\"UTF-8\" name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            client.println("<style>");
            client.println("html { font-family: Helvetica; text-align: center; margin: 0; padding: 0; }");
            client.println("body { margin: 0; padding: 0; }");
            client.println(".container { max-width: 600px; margin: auto; padding: 20px; }");
            client.println("h1 { font-size: 24px; }");
            client.println(".switch { position: relative; display: inline-block; width: 60px; height: 34px; margin: 10px; }");
            client.println(".switch input { opacity: 0; width: 0; height: 0; }");
            client.println(".slider { position: absolute; cursor: pointer; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; transition: .4s; border-radius: 34px; }");
            client.println(".slider:before { position: absolute; content: \"\"; height: 26px; width: 26px; left: 4px; bottom: 4px; background-color: white; transition: .4s; border-radius: 50%; }");
            client.println("input:checked + .slider { background-color: #4CAF50; }");
            client.println("input:checked + .slider:before { transform: translateX(26px); }");
            client.println("@media (max-width: 600px) { h1 { font-size: 18px; } }");
            client.println("</style>");
            client.println("</head>");
            client.println("<body>");
            client.println("<div class=\"container\">");
            client.println("<h1>ESP32 Web Server ระบบควบคุมไฟ</h1>");
            client.println("<h3>GPIO 26</h3>");
            client.println("<label class=\"switch\">");
            client.println(String("<input type=\"checkbox\" onchange=\"location.href='/26/' + (this.checked ? 'on' : 'off')\" ") + (output26State == "on" ? "checked" : "") + ">");
            client.println("<span class=\"slider\"></span>");
            client.println("</label>");
            client.println("<h3>GPIO 27</h3>");
            client.println("<label class=\"switch\">");
            client.println(String("<input type=\"checkbox\" onchange=\"location.href='/27/' + (this.checked ? 'on' : 'off')\" ") + (output27State == "on" ? "checked" : "") + ">");
            client.println("<span class=\"slider\"></span>");
            client.println("</label>");
            client.println("</div>");
            client.println("</body></html>");


            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else {  // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
#include <ESP8266WiFi.h>         // Library to use Wi-Fi on ESP8266
#include <ESP8266HTTPClient.h>   // Library to make HTTP requests
#include <DHT.h>                 // Library for DHT sensor (temperature and humidity)

// WiFi credentials
const char* ssid = "ASUS_C0_2G";         // Replace with your Wi-Fi network name
const char* password = "َQweasd774411"; // Replace with your Wi-Fi password

// Server endpoint where the ESP8266 will send data
const String serverUrl = "https://https://iotserver.djangionic.com/api/api/update_sensor_data";

// DHT sensor configuration
#define DHTPIN D4        // DHT22 data pin connected to GPIO D4 on ESP8266
#define DHTTYPE DHT22    // We are using the DHT22 sensor (or change to DHT11 if using that)
DHT dht(DHTPIN, DHTTYPE);

// Door sensor (Reed switch) configuration
#define DOOR_SENSOR_PIN D5 // Door sensor connected to GPIO D5 on ESP8266

// Timing
unsigned long lastSent = 0;           // Variable to store the last time data was sent
unsigned long interval = 60000;       // Interval between sending data (60 seconds)

WiFiClient wifiClient;  // Create a WiFiClient object

void setup() {
  Serial.begin(115200); // Start the serial communication at 115200 baud rate
  delay(10);            // Short delay for stability
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password); // Start connecting to the specified Wi-Fi
  while (WiFi.status() != WL_CONNECTED) { // Wait until Wi-Fi is connected
    delay(500);            // Wait for 500ms before retrying
    Serial.print(".");     // Print a dot to show the connection process
  }
  Serial.println("Connected to Wi-Fi"); // Print a message when connected

  // Initialize the DHT sensor
  dht.begin(); 
  // Set the door sensor pin mode
  pinMode(DOOR_SENSOR_PIN, INPUT); 
}

void loop() {
  // Get the current time in milliseconds since the ESP started
  unsigned long now = millis();
  
  // Check if it's time to send data based on the interval
  if (now - lastSent >= interval) {
    lastSent = now; // Update the lastSent time to the current time
    
    // Read the temperature from the DHT sensor
    float temperature = dht.readTemperature();
    
    // Read the door sensor status (HIGH if open, LOW if closed)
    int doorStatus = digitalRead(DOOR_SENSOR_PIN);

    // Check if the temperature reading is valid
    if (isnan(temperature)) {
      Serial.println("Failed to read from DHT sensor!"); // Print an error if reading failed
      return; // Exit the loop and try again in the next iteration
    }

    // Send the door status and temperature data to the server
    sendToServer(doorStatus, temperature);
  }
}

// Function to send data to the server
void sendToServer(int doorStatus, float temperature) {
  if (WiFi.status() == WL_CONNECTED) { // Check if the ESP is connected to Wi-Fi
    HTTPClient http; // Create an HTTP client object
    http.begin(wifiClient, serverUrl); // Updated API: Pass WiFiClient and server URL to begin()

    http.addHeader("Content-Type", "application/json"); // Set the content type to JSON
    
    // Create the JSON payload with door status and temperature
    String payload = "{\"door_status\": " + String(doorStatus) + ", \"temperature\": " + String(temperature) + "}";
    
    // Send the data to the server with a POST request
    int httpResponseCode = http.POST(payload);
    
    if (httpResponseCode > 0) { // Check if the request was successful
      String response = http.getString(); // Get the response from the server
      Serial.println("Server Response: " + response); // Print the server's response
    } else {
      Serial.println("Error on sending POST: " + String(httpResponseCode)); // Print an error if the request failed
    }
    
    http.end(); // End the HTTP connection
  } else {
    Serial.println("Error in Wi-Fi connection"); // Print an error if Wi-Fi is not connected
  }
}

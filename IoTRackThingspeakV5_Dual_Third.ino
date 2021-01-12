#include <HX711.h>
#include <ESP8266WiFi.h>;
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#define updateInterval 15000 //Count Updated every X(xxxxxx) millisec

LiquidCrystal_I2C lcd(0x27, 16, 2);
//LiquidCrystal_I2C lcd2(0x3F, 16, 2);

String apiKey = "0VLQM2VSUVHTZYOA"; // Enter your Write API key from ThingSpeak
const char *ssid = "Password is Password"; // replace with your wifi ssid and wpa2 key
const char *pass = "iwonttell";
const char* server = "api.thingspeak.com";

WiFiClient client;

HX711 scale(D7, D8);//(DAT, SCK)

int rbutton = D4; // this button will be used to reset the scale to 0.
float weight;

float productWeight1 = 50; //Product weight in grams

float calibration_factor = productWeight1 * 4.24; //889*17.78 = 889 Calibration Factor

long unsigned int lastUpdate;

void setup()
{
  Serial.begin(115200);
  pinMode(rbutton, INPUT_PULLUP);
  scale.set_scale(calibration_factor); //Set to calibrated value from sketch

  //scale.tare(); //Reset the scale to 0 //uncomment to not tare to 0
  //long zero_factor = scale.get_units(), 5; //Get a baseline reading

  long zero_factor = 141520; //Get a baseline reading (manually found & entered)
  scale.set_offset(zero_factor); //Zero out the scale using a previously known zero_factor (permanet setup)
  Serial.print("Zero factor: "); //This can be used to remove the need to tare the scale. Useful in permanent scale projects.
  Serial.println(zero_factor);

  Wire.begin(D2, D1);//
  lcd.begin();
  lcd.setCursor(3, 0);
  lcd.print("Smart IOT");
  lcd.setCursor(5, 1);
  lcd.print("Rack");
  delay(2000);
  lcd.clear();

  lcd.print("Connecting Wifi");
  lcd.setCursor(0, 1);
  lcd.print("& Thingspeak");

  WiFi.begin(ssid, pass);
  {
    delay(500);
    Serial.print(".");
  }
  delay(1000);
  Serial.println("");
  lcd.clear();
  lcd.print(WiFi.localIP());
  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
  delay(2000);
}

void loop(){

  scale.set_scale(calibration_factor); //Adjust to this calibration factor
  weight = scale.get_units(), 5;

  int maxWeight1 = 2000;
  int stock;   //
  stock = map(weight, 0, maxWeight1, 0, (maxWeight1 / productWeight1)); //Eg. 1900/50 = 40

  lcd.setCursor(0, 0);
  lcd.print("R3 Stock: ");
  lcd.print(stock);
  lcd.print(" NOS");
  delay(500);
  lcd.clear();

  Serial.print("Weight: ");
  Serial.print(weight);
  Serial.println(" grams");
  Serial.print("Stock: "); Serial.print(stock); Serial.println(" NOS");
  Serial.println();
  if ( digitalRead(rbutton) == LOW)
  {
    scale.set_scale();
    scale.tare(); //Reset the scale to 0
    Serial.println("Tare Complete!");
  }
  if ((millis() - lastUpdate) > updateInterval)  //Update to Memory with intervals
  {
    lastUpdate = millis();   //update lastUpdate Interval
    lcd.setCursor(0, 0);
    lcd.print("R3 Stock: ");
    lcd.print(stock);
    lcd.print(" NOS");
    lcd.clear();
    if (client.connect(server, 80)) // "184.106.153.149" or api.thingspeak.com
    {
      String postStr = apiKey;
      postStr += "&field3=";    //Eg. URL Format https://api.thingspeak.com/update.json?api_key=XXXXXXXXXX&field1=58&field2=23
      postStr += String(stock);
      postStr += "\r\n\r\n";

      client.print("POST /update HTTP/1.1\n");
      client.print("Host: api.thingspeak.com\n");
      client.print("Connection: close\n");
      client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
      client.print("Content-Type: application/x-www-form-urlencoded\n");
      client.print("Content-Length: ");
      client.print(postStr.length());
      client.print("\n\n");
      client.print(postStr);
      Serial.println("Data Sent to Cloud");
    }
    client.stop();
  }
}

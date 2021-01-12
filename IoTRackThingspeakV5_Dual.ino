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

HX711 scale(D5, D6);//(DAT, SCK)
HX711 scale2(D7, D8);//(DAT, SCK)

int rbutton = D4; // this button will be used to reset the scale to 0.
float weight;

float productWeight1 = 50; //Product weight in grams
float productWeight2 = 35;//Product weight in grams

float calibration_factor = productWeight1 * 17.78; //889*17.78 = 889 Calibration Factor
float weight2;
float calibration_factor2 = productWeight2 * 17.86; //893*17.86 = 893 Calibration Factor

long unsigned int lastUpdate;

void setup()
{
  Serial.begin(115200);
  pinMode(rbutton, INPUT_PULLUP);
  scale.set_scale(calibration_factor); //Set to calibrated value from sketch
  scale2.set_scale(calibration_factor2); //Set to calibrated value from sketch

  //scale.tare(); //Reset the scale to 0 //uncomment to not tare to 0
  //long zero_factor = scale.get_units(), 5; //Get a baseline reading

  long zero_factor = 183573; //Get a baseline reading (manually found & entered)
  scale.set_offset(zero_factor); //Zero out the scale using a previously known zero_factor (permanet setup)
  Serial.print("Zero factor: "); //This can be used to remove the need to tare the scale. Useful in permanent scale projects.
  Serial.println(zero_factor);

  long zero_factor2 = 69296; //Get a baseline reading (manually found & entered)
  scale2.set_offset(zero_factor2); //Zero out the scale using a previously known zero_factor (permanet setup)
  Serial.print("Zero factor: "); //This can be used to remove the need to tare the scale. Useful in permanent scale projects.
  Serial.println(zero_factor2);

  Wire.begin(D2, D1);//
  lcd.begin();
  lcd.setCursor(3, 0);
  lcd.print("Smart IOT");
  lcd.setCursor(5, 1);
  lcd.print("Rack");
  delay(3000);
  lcd.clear();

  lcd.print("Connecting Wifi");
  lcd.setCursor(0, 1);
  lcd.print("& Thingspeak");

  WiFi.begin(ssid, pass);
  {
    delay(500);
    Serial.print(".");
    lcd.clear();
  }
  delay(1000);
  Serial.println("");
  lcd.clear();
  lcd.print(WiFi.localIP());
  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
  delay(2000);
}

void loop()

{

  scale.set_scale(calibration_factor); //Adjust to this calibration factor
  weight = scale.get_units(), 5;

  scale2.set_scale(calibration_factor2); //Adjust to this calibration factor
  weight2 = scale2.get_units(), 5;

  int maxWeight1 = 1900;
  int maxWeight2 = 1900;
  int stock;   //
  stock = map(weight, 0, maxWeight1, 0, (maxWeight1 / productWeight1)); //Eg. 1900/50 = 40

  int stock2;   //
  stock2 = map(weight2, 0, maxWeight2, 0, (maxWeight2 / productWeight2)); //Eg. 1900/35 = 54

  /*
    lcd.setCursor(0, 0);
    lcd.print("Measured Weight");
    lcd.setCursor(0, 1);
    lcd.print(weight);
    lcd.print(" grams  ");
    delay(1500);
    lcd.clear();
  */
  lcd.setCursor(0, 0);
  lcd.print("R1 Stock: ");
  lcd.print(stock);
  lcd.print(" NOS");
  
  lcd.setCursor(0, 1);
  lcd.print("R2 Stock: ");
  lcd.print(stock2);
  lcd.print(" NOS");
  delay(1500);
  lcd.clear();

  Serial.print("Weight: ");
  Serial.print(weight);
  Serial.println(" grams");
  Serial.print("Stock: "); Serial.print(stock); Serial.println(" NOS");
  Serial.println();

  Serial.print("Weight2: ");
  Serial.print(weight2);
  Serial.println(" grams");
  Serial.print("Stock2: "); Serial.print(stock2); Serial.println(" NOS");
  Serial.println();

  if ( digitalRead(rbutton) == LOW)
  {
    scale.set_scale();
    scale.tare(); //Reset the scale to 0
    scale2.set_scale();
    scale2.tare(); //Reset the scale to 0
    Serial.println("Tare Complete!");
  }
  if ((millis() - lastUpdate) > updateInterval)  //Update to Memory with intervals
  {
    lastUpdate = millis();   //update lastUpdate Interval

    if (client.connect(server, 80)) // "184.106.153.149" or api.thingspeak.com
    {
      String postStr = apiKey;
      postStr += "&field1=";    //Eg. URL Format https://api.thingspeak.com/update.json?api_key=XXXXXXXXXX&field1=58&field2=23
      postStr += String(stock);
      postStr += "&field2=";
      postStr += String(stock2);
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

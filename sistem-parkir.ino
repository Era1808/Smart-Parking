#include <WiFi.h>
#include <Arduino.h>
#include <FirebaseESP32.h>
#include <RTClib.h>

#define FIREBASE_HOST "https://smart-parking-22b47-default-rtdb.firebaseio.com/"
#define FIREBASE_AUTH "MYp7bqPYfjG5zZmCAEvr4xieMBumuIB8NDcfN2YQ"
#define WIFI_SSID "PUTRI"
#define WIFI_PASSWORD "Kosputrififialfiah01##"

RTC_DS3231 rtc;

struct Sensor {

  int trigPin;
  int echoPin;
  int duration;
};

Sensor sensors[5] = {
  {5, 4, 0},
  {19, 18, 0},
  {32, 34, 0},
  {26, 25, 0},
  {13, 12, 0}
};

long duration;
float distanceCm;
float distanceInch;
String parkingStatus;

FirebaseData firebaseData;
FirebaseJson dataObject;

void setup() {
  Serial.begin(115200);

  for (int i = 0; i < 5; i++) {
    pinMode(sensors[i].trigPin, OUTPUT);
    pinMode(sensors[i].echoPin, INPUT);
  }

  while (! rtc.begin()) {
    Serial.println("RTC module is NOT found");
    Serial.flush();
    delay(100);
  }

  // automatically sets the RTC to the date & time on PC this sketch was compiled
  rtc.adjust(DateTime(__DATE__, __TIME__));
  //  rtc.adjust(DateTime(2025, 4, 09, 16, 39, 00));

  initWiFi();
  initFirebase();
}

void loop() {

  DateTime now = rtc.now();

  for (int i = 0; i < 5; i++) {
    digitalWrite(sensors[i].trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(sensors[i].trigPin, LOW);
    duration = pulseIn(sensors[i].echoPin, HIGH);
    distanceCm = duration * 0.034 / 2;
    parkingStatus = calculateParkingStatus(distanceCm);
    dataObject.set("dataRealtime/" + String(i + 1), parkingStatus);
    dataObject.set("dataHistory/" + String(i + 1) +  "/" + getCurrentDateTime(now,":") , parkingStatus);
    dataObject.set("dataRecent/" + parkingStatus + "/" + String(i + 1) , getCurrentDateTime(now, "-"));
  }
  Firebase.updateNode(firebaseData, "data" , dataObject);

  Serial.println();
}

void initWiFi() {
  Serial.println("Menghubungkan Wi-Fi");
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED)
  {
  }

  Serial.println("Wifi Terhubung");
}

void initFirebase() {
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
  Firebase.setReadTimeout(firebaseData, 1000 * 60);
  Firebase.setwriteSizeLimit(firebaseData, "tiny");
}

String calculateParkingStatus(float distanceCm) {
  Serial.println(distanceCm);
  return (distanceCm < 50 && distanceCm >= 1) ? "Terisi" : "Kosong";
}

String formatTwoDigits(int num) {

  return (num < 10) ? "0" + String(num) : String(num);
}

String getCurrentDateTime(DateTime now, String format) {
  if (format == "-") {
    return String(now.year()) + "-" +
           formatTwoDigits( now.month()) + "-" +
           formatTwoDigits(now.day()) + "-" +
           formatTwoDigits(now.hour()) + "-" +
           formatTwoDigits(now.minute());
  }
  return String(now.year()) + "-" +
         formatTwoDigits( now.month()) + "-" +
         formatTwoDigits(now.day()) + " " +
         formatTwoDigits(now.hour()) + ":" +
         formatTwoDigits(now.minute());
}

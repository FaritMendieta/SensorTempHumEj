
/*
  =====================================================
  PROYECTO: MONITOREO AMBIENTAL CON ADAFRUIT IO
  =====================================================

  Este programa permite:
  1. Medir la temperatura con un sensor DHT11.
  2. Medir la humedad con el sensor DHT11.
  3. Medir la luminosidad con una fotoresistencia LDR.
  4. Activar un LED RGB segun la temperatura.
  5. Enviar las mediciones a Adafruit IO.

  NIVELES DE TEMPERATURA:
  Nivel 1: Menos de 20 C -> Azul.
  Nivel 2: De 20 a menos de 28 C -> Verde.
  Nivel 3: De 28 a menos de 35 C -> Amarillo.
  Nivel 4: 35 C o mas -> Rojo.

  CONEXIONES:
  DHT11 DATA -> GPIO 14
  LDR -> GPIO 34 mediante divisor de voltaje
  LED rojo -> GPIO 25 con resistencia
  LED verde -> GPIO 26 con resistencia
  LED azul -> GPIO 27 con resistencia

  Este codigo utiliza un LED RGB de catodo comun.
*/

// =====================================================
// 1. INCLUIR LAS LIBRERIAS
// =====================================================

// Permite conectar el ESP32 a una red WiFi.
#include <WiFi.h>

// Permite enviar datos a Adafruit IO mediante MQTT.
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>

// Permite leer temperatura y humedad del DHT11.
#include <DHT.h>


// =====================================================
// 2. CONFIGURAR WIFI Y ADAFRUIT IO
// =====================================================

// Escribe el nombre de tu red WiFi.
#define WIFI_SSID "TU_WIFI"

// Escribe la contrasena de tu red WiFi.
#define WIFI_PASSWORD "TU_PASSWORD"

// Escribe tu usuario de Adafruit IO.
#define AIO_USERNAME "TU_USUARIO"

// Escribe tu clave de Adafruit IO.
#define AIO_KEY "TU_CLAVE"


// =====================================================
// 3. CONFIGURAR LA CONEXION CON ADAFRUIT IO
// =====================================================

// Crear el cliente que utiliza la conexion WiFi.
WiFiClient cliente;

// Configurar la conexion MQTT con Adafruit IO.
Adafruit_MQTT_Client mqtt(
  &cliente,
  "io.adafruit.com",
  1883,
  AIO_USERNAME,
  AIO_KEY
);


// =====================================================
// 4. CREAR LOS FEEDS
// =====================================================

// Feed para almacenar la temperatura.
Adafruit_MQTT_Publish tempFeed(
  &mqtt, AIO_USERNAME "/feeds/temperatura"
);

// Feed para almacenar la humedad.
Adafruit_MQTT_Publish humFeed(
  &mqtt, AIO_USERNAME "/feeds/humedad"
);

// Feed para almacenar la luminosidad.
Adafruit_MQTT_Publish luzFeed(
  &mqtt, AIO_USERNAME "/feeds/luminosidad"
);

// Feed para almacenar el nivel de alerta.
Adafruit_MQTT_Publish alertaFeed(
  &mqtt, AIO_USERNAME "/feeds/nivel-alerta"
);


// =====================================================
// 5. CONFIGURAR LOS SENSORES Y EL LED RGB
// =====================================================

// Pin de datos del sensor DHT11.
#define DHTPIN 14

// Indicar que el sensor es un DHT11.
#define DHTTYPE DHT11

// Crear el objeto del sensor.
DHT dht(DHTPIN, DHTTYPE);

// Pin analogico donde se conecta la LDR.
#define LDRPIN 34

// Pines de los tres colores del LED RGB.
#define ROJO 25
#define VERDE 26
#define AZUL 27


// =====================================================
// 6. FUNCION SETUP
// =====================================================

void setup() {

  // Iniciar el monitor serie a 115200 baudios.
  Serial.begin(115200);

  // Configurar los pines del LED como salidas.
  pinMode(ROJO, OUTPUT);
  pinMode(VERDE, OUTPUT);
  pinMode(AZUL, OUTPUT);

  // Configurar la resolucion analogica del ESP32.
  // Las lecturas estaran entre 0 y 4095.
  analogReadResolution(12);

  // Iniciar el sensor DHT11.
  dht.begin();

  // Apagar los colores del LED al iniciar.
  digitalWrite(ROJO, LOW);
  digitalWrite(VERDE, LOW);
  digitalWrite(AZUL, LOW);

  // Iniciar la conexion WiFi.
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  // Esperar hasta conectarse a la red.
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Conectando a WiFi...");
  }

  Serial.println("WiFi conectado.");
  Serial.println("Sistema de monitoreo iniciado.");
}


// =====================================================
// 7. FUNCION LOOP
// =====================================================

void loop() {

  // ---------------------------------------------------
  // A. CONECTAR CON ADAFRUIT IO
  // ---------------------------------------------------

  // Verificar si MQTT esta conectado.
  if (!mqtt.connected()) {

    // Intentar conectarse al servidor.
    int resultado = mqtt.connect();

    // Si falla, desconectar y volver a intentar
    // despues de cinco segundos.
    if (resultado != 0) {

      Serial.println("Error de conexion con Adafruit IO.");

      mqtt.disconnect();

      delay(5000);

      return;
    }

    Serial.println("Adafruit IO conectado.");
  }


  // ---------------------------------------------------
  // B. LEER TEMPERATURA Y HUMEDAD
  // ---------------------------------------------------

  // Leer la temperatura en grados Celsius.
  float temperatura = dht.readTemperature();

  // Leer la humedad relativa en porcentaje.
  float humedad = dht.readHumidity();

  // Comprobar si las lecturas del DHT11 son validas.
  if (isnan(temperatura) || isnan(humedad)) {

    Serial.println("Error al leer el sensor DHT11.");

    // Esperar antes de volver a intentar.
    delay(2000);

    return;
  }


  // ---------------------------------------------------
  // C. LEER LA LUMINOSIDAD
  // ---------------------------------------------------

  // Leer el valor analogico de la fotoresistencia.
  // El resultado puede variar entre 0 y 4095.
  int luminosidad = analogRead(LDRPIN);


  // ---------------------------------------------------
  // D. DETERMINAR EL NIVEL DE ALERTA
  // ---------------------------------------------------

  // Variable para guardar el nivel de temperatura.
  int nivel;

  // Apagar todos los colores antes de seleccionar
  // el color correspondiente a la temperatura.
  digitalWrite(ROJO, LOW);
  digitalWrite(VERDE, LOW);
  digitalWrite(AZUL, LOW);


  // NIVEL 1: temperatura menor de 20 grados.
  if (temperatura < 20) {

    nivel = 1;

    // Encender el color azul.
    digitalWrite(AZUL, HIGH);

    Serial.println("NIVEL 1: TEMPERATURA BAJA");
  }


  // NIVEL 2: temperatura de 20 a menos de 28 grados.
  else if (temperatura < 28) {

    nivel = 2;

    // Encender el color verde.
    digitalWrite(VERDE, HIGH);

    Serial.println("NIVEL 2: TEMPERATURA NORMAL");
  }


  // NIVEL 3: temperatura de 28 a menos de 35 grados.
  else if (temperatura < 35) {

    nivel = 3;

    // Encender rojo y verde al mismo tiempo.
    // Estos dos colores producen amarillo.
    digitalWrite(ROJO, HIGH);
    digitalWrite(VERDE, HIGH);

    Serial.println("NIVEL 3: PRECAUCION");
  }


  // NIVEL 4: temperatura igual o superior a 35 grados.
  else {

    nivel = 4;

    // Encender el color rojo.
    digitalWrite(ROJO, HIGH);

    Serial.println("NIVEL 4: ALERTA ALTA");
  }


  // ---------------------------------------------------
  // E. MOSTRAR LOS RESULTADOS
  // ---------------------------------------------------

  Serial.println();
  Serial.println("======= MEDICIONES =======");

  // Mostrar la temperatura.
  Serial.print("Temperatura: ");
  Serial.print(temperatura);
  Serial.println(" grados Celsius");

  // Mostrar la humedad.
  Serial.print("Humedad: ");
  Serial.print(humedad);
  Serial.println(" %");

  // Mostrar la lectura de la LDR.
  Serial.print("Luminosidad (ADC): ");
  Serial.println(luminosidad);

  // Mostrar el nivel de alerta actual.
  Serial.print("Nivel de alerta: ");
  Serial.println(nivel);

  Serial.println("==========================");


  // ---------------------------------------------------
  // F. ENVIAR DATOS A ADAFRUIT IO
  // ---------------------------------------------------

  // Enviar la temperatura al feed temperatura.
  if (!tempFeed.publish(temperatura)) {
    Serial.println("Error al publicar temperatura.");
  }

  // Enviar la humedad al feed humedad.
  if (!humFeed.publish(humedad)) {
    Serial.println("Error al publicar humedad.");
  }

  // Enviar la luminosidad al feed luminosidad.
  if (!luzFeed.publish(luminosidad)) {
    Serial.println("Error al publicar luminosidad.");
  }

  // Enviar el nivel de alerta como numero entero.
  if (!alertaFeed.publish((int32_t)nivel)) {
    Serial.println("Error al publicar nivel de alerta.");
  }


  // ---------------------------------------------------
  // G. ESPERAR PARA LA SIGUIENTE MEDICION
  // ---------------------------------------------------

  // Esperar cinco segundos antes de repetir el proceso.
  delay(5000);
}


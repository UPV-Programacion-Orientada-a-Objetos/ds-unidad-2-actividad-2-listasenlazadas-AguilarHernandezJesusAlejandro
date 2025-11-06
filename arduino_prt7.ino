const char* tramas[] = {
  "L,H",
  "L,O",
  "L,L",
  "M,2",
  "L,A",
  "L,Space",
  "L,W",
  "M,-2",
  "L,O",
  "L,R",
  "L,L",
  "L,D"
};

const int numTramas = sizeof(tramas) / sizeof(tramas[0]);

int tramaActual = 0;

bool transmisionCompleta = false;

void setup() {
  Serial.begin(9600);
  
  while (!Serial) {
    ;
  }
  
  delay(2000);
  
  Serial.println("=== TRANSMISOR PRT-7 INICIADO ===");
  Serial.println("Comenzando transmisión de tramas...");
  Serial.println("");
  
  delay(1000);
}

void loop() {
  if (!transmisionCompleta) {
    if (tramaActual < numTramas) {
      Serial.println(tramas[tramaActual]);
      
      // Serial.print("DEBUG: Enviada trama ");
      // Serial.println(tramaActual + 1);
      
      tramaActual++;
      
      delay(1000);
      
    } else {
      transmisionCompleta = true;
      
      Serial.println("");
      Serial.println("=== TRANSMISIÓN COMPLETA ===");
      Serial.print("Total de tramas enviadas: ");
      Serial.println(numTramas);
    }
  }
}

/*
void loop() {
  if (Serial.available() > 0) {
    String comando = Serial.readStringUntil('\n');
    comando.trim();
    
    if (comando == "SEND") {
      if (tramaActual < numTramas) {
        Serial.println(tramas[tramaActual]);
        tramaActual++;
      } else {
        Serial.println("FIN");
      }
    } else if (comando == "RESET") {
      tramaActual = 0;
      Serial.println("OK");
    } else if (comando == "STATUS") {
      Serial.print("Tramas enviadas: ");
      Serial.print(tramaActual);
      Serial.print("/");
      Serial.println(numTramas);
    }
  }
} 
*/
#include <SPI.h>
#include <SD.h>
#include "EasyNextionLibrary.h"

// Definizioni pin per ESP32
// Pin SPI per SD card
#define SD_CS   5    // Pin CS per SD card su ESP32
#define SD_MISO 19   // Pin MISO (Master In Slave Out)
#define SD_MOSI 23   // Pin MOSI (Master Out Slave In)
#define SD_SCK  18   // Pin CLK (Clock)

// Pin UART per Nextion
#define UART_RX 16   // Pin RX per Nextion
#define UART_TX 17   // Pin TX per Nextion

// Inizializzazione della comunicazione seriale per Nextion su Serial2
EasyNex myNex(Serial2);

int folderCount = 0;
char folderNames[4][13];  // Array per memorizzare i nomi delle cartelle (max 4)
char folderName[13];      // Variabile per memorizzare il nome della cartella selezionata

// Variabili per memorizzare i dati
float MaxAlt, MaxSpeed;
int Altm, Speedm, MaxVario, Variom, MaxCanTemp, MaxGainAlt, TotGainAlt, FieldP, FieldT, FieldPressure;
int Alt, Speed, Vario;
float VarioFloat, MVarioFloat, Course, maxCanopyTemp, MaxGainedAltitude, TotGainedAltitude, FieldTemperature;
char Last[20], StartFlight[20], EndFlight[20], Date[20], maxTimeInThermal[20], TotThermalDuration[20];

// Variabile per interrompere la lettura del file
volatile bool interruptTrigger1 = false;
// Timestamp dell'ultimo controllo
unsigned long lastInterruptCheck = 0;

void setup() {
  // Inizializza la comunicazione seriale per Nextion
  Serial2.begin(9600, SERIAL_8N1, UART_RX, UART_TX);
  delay(500);
  myNex.begin(9600);
  
  // Configura SPI per la SD card
  SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  
  // Inizializza la SD card
  if (!SD.begin(SD_CS)) {
    myNex.writeStr("page0.t14.txt", "SDCard fail!");
    return;
  } else {
    myNex.writeNum("page0.t14.pco", 1024);
    myNex.writeStr("page0.t14.txt", "SDCard Ready");
  }

  // Leggi le cartelle
  listFolders();
  
  // Invia i nomi delle cartelle al Nextion
  myNex.writeStr("b50.txt", folderNames[0]);  // Cartella fittizia per evitare di saltare la prima lettura
  delay(500);
  
  // Invia i nomi delle cartelle ai rispettivi pulsanti
  myNex.writeStr("b3.txt", folderNames[0]);
  if (folderCount > 1) myNex.writeStr("b4.txt", folderNames[1]);
  if (folderCount > 2) myNex.writeStr("b5.txt", folderNames[2]);
  if (folderCount > 3) myNex.writeStr("b6.txt", folderNames[3]);
}

void loop() {
  // Ascolta gli eventi dal Nextion
  myNex.NextionListen();
  
  // Controlla se ci sono stati eventi di timeout
  unsigned long currentMillis = millis();
  if (currentMillis - lastInterruptCheck > 100) {  // Controlla ogni 100 ms
    lastInterruptCheck = currentMillis;
    
    // Processa eventi di interrupt nel loop principale
    if (interruptTrigger1) {
      // Resetta la variabile di interruzione
      interruptTrigger1 = false;
    }
  }
  
  delay(50);  // Breve pausa per le attività in background
}

void listFolders() {
  folderCount = 0;
  
  // Apri la directory principale
  File root = SD.open("/");
  if (!root || !root.isDirectory()) {
    return;
  }

  // Leggi tutte le cartelle
  while (folderCount < 4) {
    File entry = root.openNextFile();
    if (!entry) {
      break;  // Non ci sono più file
    }
    
    if (entry.isDirectory()) {
      // Ottieni il nome della cartella
      String name = entry.name();
      
      // Se il nome inizia con '/', rimuovilo
      if (name.startsWith("/")) {
        name = name.substring(1);
      }
      
      // Verifica se la cartella ha un nome di 8 cifre
      bool isValidFolder = (name.length() == 8);
      if (isValidFolder) {
        for (int i = 0; i < 8; i++) {
          if (!isDigit(name.charAt(i))) {
            isValidFolder = false;
            break;
          }
        }
      }
      
      // Se la cartella è valida, aggiungila all'elenco
      if (isValidFolder) {
        strcpy(folderNames[folderCount], name.c_str());
        folderNames[folderCount][8] = '\0';  // Assicura la terminazione a 8 caratteri
        folderCount++;
      }
    }
    
    entry.close();
  }
  
  root.close();
  
  // Ordina l'elenco delle cartelle
  for (int i = 0; i < folderCount - 1; i++) {
    for (int j = i + 1; j < folderCount; j++) {
      if (strcmp(folderNames[i], folderNames[j]) > 0) {
        char temp[13];
        strcpy(temp, folderNames[i]);
        strcpy(folderNames[i], folderNames[j]);
        strcpy(folderNames[j], temp);
      }
    }
  }
}

void trigger1() {
  // Assicuriamoci che l'interruzione sia disattivata
  interruptTrigger1 = false;
  DatiVoloFilePath();
}

void trigger2() {
  MaxDatiFilePath();
  // Invia i valori al Nextion con formattazione e ritardi adeguati
  myNex.writeStr("t29.txt", Date);
  delay(10);
  myNex.writeStr("t21.txt", StartFlight);
  delay(10);
  myNex.writeStr("t22.txt", EndFlight);
  delay(10);
  myNex.writeStr("t20.txt", Last);
  delay(10);
  myNex.writeNum("x20.val", (int)Course);
  delay(10);
  myNex.writeNum("n3.val", (int)MaxSpeed);
  delay(10);
  myNex.writeNum("x21.val", (int)(MVarioFloat * 10));
  delay(10);
  myNex.writeNum("z1.val", (int)MaxAlt);
  delay(10);
  myNex.writeNum("n2.val", (int)MaxAlt);
  delay(10);
  myNex.writeNum("z0.val", (int)(20 + MaxSpeed));
  delay(10);
  myNex.writeNum("j0.val", (int)(MVarioFloat * 10));
  delay(10);
}

void trigger8() {
  StatDatiFilePath();
  myNex.writeStr("t340.txt", maxTimeInThermal);
  myNex.writeStr("t360.txt", TotThermalDuration);
  TotGainAlt = TotGainedAltitude * 100;
  myNex.writeNum("x330.val", TotGainAlt);
  MaxGainAlt = MaxGainedAltitude * 100;
  myNex.writeNum("x320.val", MaxGainAlt);
  FieldT = FieldTemperature * 100;
  myNex.writeNum("x350.val", FieldT);
  MaxCanTemp = maxCanopyTemp * 100;
  myNex.writeNum("x300.val", MaxCanTemp);
  myNex.writeNum("x310.val", Course);
  myNex.writeNum("n370.val", FieldPressure);
}

void selectFolder(int folderIndex) {
  if (folderIndex >= 0 && folderIndex < folderCount) {
    strcpy(folderName, folderNames[folderIndex]);
    char GGMM[5], HHmm[5]; // Estrai GGMM e HHmm dal nome della cartella
    strncpy(GGMM, folderName, 4);
    GGMM[4] = '\0';
    strncpy(HHmm, folderName + 4, 4);
    HHmm[4] = '\0';
    char formattedGGMM[6]; // Riformatta GGMM come GG:MM
    snprintf(formattedGGMM, sizeof(formattedGGMM), "%.2s:%.2s", GGMM, GGMM + 2);  
    char formattedHHmm[6]; // Riformatta HHmm come HH:mm
    snprintf(formattedHHmm, sizeof(formattedHHmm), "%.2s:%.2s", HHmm, HHmm + 2);
    myNex.writeStr("t11.txt", formattedGGMM);  // Invia i valori formattati al Nextion
    myNex.writeStr("t12.txt", formattedHHmm);
  }
}

void trigger4() {
  selectFolder(0);
}

void trigger5() {
  selectFolder(1);
}

void trigger6() {
  selectFolder(2);
}

void trigger7() {
  selectFolder(3);
}

void getFilePath(const char* folderName, const char* fileName, char* filePath) {
  snprintf(filePath, 26, "/%s/%s", folderName, fileName);  // Aggiunto "/" iniziale per compatibilità ESP32
}

void MaxDatiFilePath() {
  char MaxDatiFilePath[26];
  getFilePath(folderName, "MaxDati.txt", MaxDatiFilePath);
  readMaxDatiFile(MaxDatiFilePath);
}

void DatiVoloFilePath() {
  char datiVoloFilePath[26];
  getFilePath(folderName, "DatiVolo.txt", datiVoloFilePath);
  readDatiVoloFile(datiVoloFilePath);
}

void StatDatiFilePath() {
  char StatdatiFilePath[26];
  getFilePath(folderName, "StatDati.txt", StatdatiFilePath);
  readStatDatiFile(StatdatiFilePath);
}

void readDatiVoloFile(const char* filePath) {
  File dataFile = SD.open(filePath);
  if (dataFile) {
    char line[50];  // Buffer per la riga
    int index = 0;
    
    // Leggi il file carattere per carattere
    unsigned long lastCheckTime = millis();
    
    while (dataFile.available()) {
      // Controlla periodicamente se è stata richiesta un'interruzione
      unsigned long currentTime = millis();
      if (currentTime - lastCheckTime > 50) {  // Controlla ogni 50ms
        lastCheckTime = currentTime;
        
        // Verifica la richiesta di interruzione
        if (interruptTrigger1) {
          break;  // Esci dal ciclo
        }
        
        // Verifica anche gli eventi Nextion durante la lettura
        myNex.NextionListen();
      }
      
      // Leggi un carattere
      char c = dataFile.read();
      
      // Se è un carattere di fine riga o abbiamo raggiunto la fine del buffer
      if (c == '\n' || index >= sizeof(line) - 1) {
        line[index] = '\0';  // Termina la stringa
        
        // Analizza i dati dalla riga solo se non è vuota
        if (index > 0) {
          Alt = atoi(getColumnValue(line, 3));
          Speed = atoi(getColumnValue(line, 4));
          VarioFloat = atof(getColumnValue(line, 5));
          Vario = VarioFloat * 100;
          
          // Invia i dati al Nextion
          myNex.writeNum("n0.val", Alt);
          myNex.writeNum("n1.val", Speed);
          myNex.writeNum("x10.val", Vario);
          
          // Breve ritardo per consentire al Nextion di elaborare i dati
          delay(5);
        }
        
        index = 0;  // Resetta l'indice per la prossima riga
      } else {
        line[index++] = c;  // Aggiungi il carattere alla riga
      }
    }
    
    dataFile.close();
    
    // Assicuriamoci che l'interruzione venga resettata
    interruptTrigger1 = false;
  }
}

void readMaxDatiFile(const char* filePath) {
  File dataFile = SD.open(filePath);
  if (dataFile) {
    char line[100];  // Buffer per contenere la riga del file
    int index = 0;
    
    // Leggi la prima riga del file
    while (dataFile.available() && index < sizeof(line) - 1) {
      char c = dataFile.read();
      if (c == '\n' || !dataFile.available()) {
        line[index] = '\0';  // Termina la stringa
        break;
      }
      line[index++] = c;
    }
    
    // Analizza i valori dalla riga
    MaxAlt = atof(getColumnValue(line, 0));
    MaxSpeed = atof(getColumnValue(line, 1));
    MVarioFloat = atof(getColumnValue(line, 2));
    Course = atof(getColumnValue(line, 3));
    strncpy(Last, getColumnValue(line, 4), sizeof(Last) - 1);
    Last[sizeof(Last) - 1] = '\0';
    strncpy(StartFlight, getColumnValue(line, 5), sizeof(StartFlight) - 1);
    StartFlight[sizeof(StartFlight) - 1] = '\0';
    strncpy(EndFlight, getColumnValue(line, 6), sizeof(EndFlight) - 1);
    EndFlight[sizeof(EndFlight) - 1] = '\0';
    
    // Riformatta la data da DD/M/YY a DD:MM:YY
    const char* dateStr = getColumnValue(line, 7);
    
    // Copia la data in un buffer locale
    char rawDate[20];
    memset(rawDate, 0, sizeof(rawDate)); // Inizializza il buffer a zero
    strncpy(rawDate, dateStr, sizeof(rawDate) - 1);

    // Estrai manualmente giorno, mese e anno usando stringhe
    char dateCopy[20];
    strcpy(dateCopy, rawDate); // Copia per strtok che modifica la stringa
    
    char* token = strtok(dateCopy, "/");
    int day = token ? atoi(token) : 0;
    token = strtok(NULL, "/");
    int month = token ? atoi(token) : 0;
    token = strtok(NULL, "/");
    int year = token ? atoi(token) : 0;

    // Correggi l'anno se necessario
    if (year < 5) {
        year = 25;
    }

    // Formatta come DD:MM:YY con zeri iniziali
    snprintf(Date, sizeof(Date), "%02d:%02d:%02d", day, month, year);
    
    dataFile.close();
  }
}

void readStatDatiFile(const char* filePath) {
  File dataFile = SD.open(filePath);
  if (dataFile) {
    char line[100];  // Buffer aumentato per contenere righe più lunghe
    int index = 0;
    
    // Leggi la prima riga del file
    while (dataFile.available() && index < sizeof(line) - 1) {
      char c = dataFile.read();
      if (c == '\n' || !dataFile.available()) {
        line[index] = '\0';  // Termina la stringa
        break;
      }
      line[index++] = c;
    }
    
    // Ora che il file StatDati ha lo stesso formato di MaxDati (con spazi dopo le virgole),
    // possiamo usare getColumnValue come per gli altri file
    strncpy(maxTimeInThermal, getColumnValue(line, 0), sizeof(maxTimeInThermal) - 1);
    maxTimeInThermal[sizeof(maxTimeInThermal) - 1] = '\0';
    
    strncpy(TotThermalDuration, getColumnValue(line, 1), sizeof(TotThermalDuration) - 1);
    TotThermalDuration[sizeof(TotThermalDuration) - 1] = '\0';
    
    maxCanopyTemp = atof(getColumnValue(line, 2));
    MaxGainedAltitude = atof(getColumnValue(line, 3));
    TotGainedAltitude = atof(getColumnValue(line, 4));
    FieldPressure = atoi(getColumnValue(line, 5));
    FieldTemperature = atof(getColumnValue(line, 6));
    
    dataFile.close();
  }
}

void interruptTrigger1Now() {
  // Imposta la variabile volatile in modo atomico
  noInterrupts();  // Disabilita temporaneamente le interruzioni
  interruptTrigger1 = true;  // Imposta il flag di interruzione
  interrupts();    // Riattiva le interruzioni
}

void trigger9() {
  interruptTrigger1Now();  // Interrompi Trigger1
}

// Funzione per confrontare due nomi di cartelle (per l'ordinamento)
int compareFolderNames(const void* a, const void* b) {
  return strcmp((const char*)a, (const char*)b);
}

const char* getColumnValue(const char* data, int column) {
  static char buffer[20];  // Buffer per memorizzare il valore della colonna
  int startIndex = 0;
  int endIndex = 0;
  int currentColumn = 0;
  
  while (data[endIndex] != '\0') {
    if (data[endIndex] == ',' || data[endIndex + 1] == '\0') {
      if (currentColumn == column) {
        // Rimuovi gli spazi iniziali
        while (data[startIndex] == ' ') {
          startIndex++;
        }
        // Rimuovi gli spazi finali
        int length = endIndex - startIndex;
        while (length > 0 && data[startIndex + length - 1] == ' ') {
          length--;
        }
        // Gestisci l'ultima colonna (senza virgola finale)
        if (data[endIndex + 1] == '\0') {
          length++;  // Includi l'ultimo carattere
        }
        // Copia il valore della colonna
        strncpy(buffer, data + startIndex, length);
        buffer[length] = '\0';  // Termina la stringa con null
        return buffer;
      }
      startIndex = endIndex + 1;
      currentColumn++;
    }
    endIndex++;
  }
  
  return "";  // Restituisci una stringa vuota se la colonna non viene trovata
}
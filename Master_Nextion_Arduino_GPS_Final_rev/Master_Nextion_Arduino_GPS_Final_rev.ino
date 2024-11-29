
/* voidtriggerx() functions Nextion Evento : `printh 23 02 54 XX` , 'XX` = id di triggerXX() in HEX.
  printh 23 02 54 01 chiama trigger1() ... printh 23 02 54 0A chiama trigger10() ...
Logger su void trigger1 grafica altitudine e velocita. Su schermata iniziale data, ora di inizio volo e durata volo, 
su void trigger2 valori massimi altezza , velocita e satellit.
colonna da leggere 0=data, 1=time,2=long, 3=lat, 4=alt,5=speed,6=sat
programa nextion di riferimento : 
 */

#include "EasyNextionLibrary.h"  // Include EasyNextionLibrary
#include <SPI.h>
#include <SD.h>

EasyNex myNex(Serial) ; 
File dataFile;
const int chipSelect = 4; // Pin di selezione della scheda SD !!!!!!!!!okkio e il pin di reset
String refLineData, refLineTime, lastLineTime, Durata; //LData, da inserire se correggiamo il valore della data
int refHH, refMIN, refSS;
int lastGG, lastMM, lastYYYY, lastHH, lastMIN, lastSS;
int StartCount = 50; // riga di riferimento per iniziare a legger

void setup() {
  myNex.begin(9600);
  Serial.begin(9600);
  if (!SD.begin(chipSelect)) {
  Serial.println("Errore nell'apertura della scheda SD!");
    return;
  }
    dataFile = SD.open("dativolo.txt");
  if (!dataFile) {
    Serial.println("Errore nell'apertura del file!");
    return;
  }
  myNex.NextionListen();

  calculateDateTime();
  calculateDuration(refHH, refMIN, refSS, lastHH, lastMIN, lastSS);// Calcolo della differenza di tempo in secondi e poi hh:mm:ss
 
 myNex.writeStr("page0.t11.txt",refLineData); // l'indicazione della pagina rende il valore globale. quindi cambiando pagina viene memorizzato
 myNex.writeNum("page0.t11.pco", 2016);
 myNex.writeStr("page0.t12.txt",refLineTime); 
 myNex.writeNum("page0.t12.pco", 2016);
 myNex.writeStr("page0.t13.txt",Durata);
 myNex.writeNum("page0.t13.pco", 2016);
 myNex.writeStr("page0.t14.txt", " "); 
 myNex.writeNum("page0.p2.pic", 3); 
 }

void loop() {
  myNex.NextionListen(); 
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void trigger1(){ //logger
int AltGraph, speedGraph ;
int Alt = 0, Speed = 0, Sat=0 ;
 myNex.writeNum("page1.p4.pic", 3); 
 dataFile = SD.open("dativolo.txt");
  if (dataFile) {
     while (dataFile.available()) {
    String line = dataFile.readStringUntil('\n'); // Legge una riga del file
    String Time = getColumnValue(line, 1);
    Alt = getColumnValue(line, 4).toInt(); // Ottiene e converte il valore della quarta colonna
    Speed = getColumnValue(line, 5).toInt(); // Ottiene e converte il valore della quinta colonna
    
    AltGraph = map(Alt, 0, 350, 0, 255); // Supponendo che il valore originale sia tra 0 e 350
    speedGraph = map(Speed, 0, 200, 0, 255); // Supponendo che il valore originale sia tra 0 e 250

       myNex.writeStr("page1.t16.txt",Time); 
       myNex.writeNum("NaltGraph.val", AltGraph); 
       myNex.writeNum("n0.val", Alt);
       myNex.writeNum("NspeedGraph.val", speedGraph); 
       myNex.writeNum("n1.val", Speed);
     }
  dataFile.close(); // Chiude il file
  }

  //delay(1000); 
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void trigger2(){ // Max data con gauges solo agiornamento max value/////////////////////////////////////////////////
 Serial.print("Max data ");
 int Altm, Speedm,Satm;
 int AltMax = 0, SatMax = 0, SpeedMax = 0;
  myNex.writeStr("page2.t21.txt",refLineTime); 
   myNex.writeStr("page2.t22.txt",lastLineTime); 
 dataFile = SD.open("dativolo.txt");
 if (dataFile) {
   while (dataFile.available()) {
    String line = dataFile.readStringUntil('\n'); // Legge una riga del file
    int valore4 = getColumnValue(line, 4).toInt(); // Ottiene e converte il valore della quarta colonna
    int valore5 = getColumnValue(line, 5).toInt(); // Ottiene e converte il valore della quinta colonna
    int valore6 = getColumnValue(line, 6).toInt(); // Ottiene e converte il valore della sesta colonna
       
    if (valore4 > AltMax) {
      AltMax = valore4;
      if (valore4<51){
      Altm = AltMax + 310;
       } else {
         Altm = map(AltMax, 51, 300, 0, 230); 
       } 
      myNex.writeNum("z1.val", Altm);
      myNex.writeNum("n2.val", AltMax);
      }
    if (valore5 > SpeedMax) {
      SpeedMax= valore5;
       if (valore5<51){
       Speedm = valore5 + 310;
       } else {
         Speedm = map(valore5, 51, 300, 0, 230); 
          }      
     myNex.writeNum("z0.val", Speedm );
     myNex.writeNum("n3.val", SpeedMax);
     }
    if (valore6 > SatMax) {
      SatMax = valore6;
      Satm = map(valore6, 0, 15, 0, 180);
    myNex.writeNum("z2.val", Satm);
    myNex.writeNum("n4.val", valore6);
     }
      
  }
   myNex.writeStr("page2.t15.txt","  "); 
   myNex.writeNum("p3.pic", 5); 
   myNex.writeNum("n2.val", AltMax);
   myNex.writeNum("n2.pco", 1024);
   myNex.writeNum("n3.val", SpeedMax);
   myNex.writeNum("n3.pco", 1024);
   myNex.writeNum("n4.val", SatMax);
   myNex.writeNum("n4.pco", 1024);
  
  dataFile.close(); // Chiude il file
  }    
  
} 

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void calculateDateTime() { /////////////////////////////da verificare lasttime
 dataFile = SD.open("dativolo.txt");
  if (dataFile) {
 String line;
  int lineCount = 0;
  while (dataFile.available()) {
   
    line = dataFile.readStringUntil('\n'); // Legge una riga del file
    lineCount++;
    if (lineCount == StartCount) { //  riga di riferimento perche all'inizio potrebbe non registare il gps
      refLineData = getColumnValue(line, 0);// Estrazione valori dalla riga di riferimento
      refLineTime = getColumnValue(line, 1);
      sscanf(refLineTime.c_str(), "%d:%d:%d", &refHH, &refMIN, &refSS);
      
    }
    lastLineTime = getColumnValue(line, 1);
  }
  sscanf(lastLineTime.c_str(), "%d:%d:%d", &lastHH, &lastMIN, &lastSS);// Estrazione valori dall'ultima riga
  dataFile.close(); // Chiude il file
  }
  return refLineData,refLineTime,lastLineTime;
}

//////////////////////////////////////////////////////////////////////////////////  
String getColumnValue(String line, int column) {
  int commaIndex = 0;
  int startIndex = 0;
  for (int i = 0; i < line.length(); i++) {
    if (line.charAt(i) == ',') {
      if (commaIndex == column) {
        return line.substring(startIndex, i);
      }
      commaIndex++;
      startIndex = i + 1;
    }
  }
  return line.substring(startIndex);
}
////////////////////////////////////////////////////////////////
String calculateDuration(int refHH,int refMIN,int refSS,int lastHH,int lastMIN,int lastSS) {
  // Converte l'orario in secondi
  int time1 = refHH * 3600 + refMIN * 60 + refSS;
  int time2 = lastHH * 3600 + lastMIN * 60 + lastSS;
  int secondi = time2 - time1;
  int hh = secondi / 3600;
  int mm = (secondi % 3600) / 60;
  int ss = secondi % 60;
  Durata = String(hh) + ":" + String(mm) + ":" + String(ss) ;
  return Durata;
}


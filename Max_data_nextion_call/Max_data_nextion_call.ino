

/* This is the most important method of the library. 
 * And this is because, it gives you the ability to use the predefined functions and run your code from there. 
 * These predefined functions are named trigger1(), trigger2(), trigger3()... up to trigger50(). 
 * You can use them as a simple void out of the loop, in which you will have written a block of code to run every time it is called.
 * You can call those trigger() functions and run the code they contain anytime by simply writing in a Nextion Event the command:
 * `printh 23 02 54 XX` , where `XX` the id for the triggerXX() in HEX.
 * Example: printh 23 02 54 01 to call trigger1() ... printh 23 02 54 0A to call trigger10() and so on...
 */

/*
  Declare the void by simply writing:
  void trigger1(){
  [ put your code here !!!!]
  }
*/

#include "EasyNextionLibrary.h"  // Include EasyNextionLibrary
#include <SPI.h>
#include <SD.h>

const int chipSelect = 4; // Pin di selezione della scheda SD
File MaxFile;
const int columnToRead = 1; // Colonna da leggere (0-based index) 0=data, 1=time,2=long, 3=lat, 4=alt,5=speed,6=sat
char* token;
  int column ;

float prova ;

EasyNex myNex(Serial)  ; // Create an object of EasyNex class with the name < myNex >
                       // Set as parameter the Serial you are going to use

void setup(){
  Serial.begin(9600);
  myNex.begin(9600); // Begin the object with a baud rate of 9600
                     // If no parameter was given in the begin(), the default baud rate of 9600 will be used
  if (!SD.begin(chipSelect)) {
    Serial.println("Errore nell'inizializzazione della scheda SD!");
    return;
  }
  Serial.println("Scheda SD inizializzata.");

  if (!MaxFile) {
    Serial.println("Errore nell'apertura del file!");
    return;
  }
   
   else {
    Serial.println("Errore nell'apertura del file!");
  } 
}

void loop(){
  myNex.NextionListen(); // WARNING: This function must be called repeatedly to response touch events
                         // from Nextion touch panel. Actually, you should place it in your loop function.
}

void trigger1(){
  File MaxFile = SD.open("DATIMAX.txt");
  if (MaxFile) {
    while (MaxFile.available()) {
        String line = MaxFile.readStringUntil('\n'); // Legge una riga del file
         token = strtok((char*)line.c_str(), ","); // Separa i valori della riga
         int column = 0;
           while (token != NULL) {
           if (column == columnToRead) {
           Serial.println(token); // Stampa il valore della colonna specificata
   myNex.writeNum("b0.bco", 2016); // Set button b0 background color to GREEN (color code: 2016)
   myNex.writeStr("b0.txt", "altitude"); // Set button b0 text to "ON"// To call this void send from Nextion's component's Event:  printh 23 02 54 01
   myNex.writeStr("t0.txt", token); 
           
           break;
           }
         token = strtok(NULL, ",");
        column++;
           }
        }
     MaxFile.close(); // Chiude il file dopo aver letto tutte le righe
     
      } 
   
 
}


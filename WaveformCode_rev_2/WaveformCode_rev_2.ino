#include <SPI.h>
#include <SD.h>

#include "EasyNextionLibrary.h" // Include EasyNextionLibrary

EasyNex myNex(Serial);   // Create an object of EasyNex class with the name < myNex >                      
float altGraph;   // a variable to store the reading
                 // for simplicity reasons, we do not use float and we are going to take the measure in millivolts

File MaxFile;
const int columnToRead = 4; // Colonna da leggere (0-based index) 0=data, 1=time,2=long, 3=lat, 4=alt,5=speed,6=sat
const int chipSelect = 4; // Pin di selezione della scheda SD
char* token;
int column ;
int Alt;
void setup() {
  myNex.begin(9600); // Begin the object with a baud rate of 9600
 Serial.begin(9600);
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

void loop() {
  
File MaxFile = SD.open("DatiVolo.txt");
  if (MaxFile) {
    while (MaxFile.available()) {
        String line = MaxFile.readStringUntil('\n'); // Legge una riga del file
         token = strtok((char*)line.c_str(), ","); // Separa i valori della riga
         int column = 0;
           while (token != NULL) {
           if (column == columnToRead) {
           int Alt = atoi(token);
           altGraph = map(Alt, 0, 350 , 0, 255); 
           myNex.writeNum("NaltGraph.val", Alt); 
           myNex.writeNum("n1.val", Alt);
           break; 
           }
           token = strtok(NULL, ",");
           column++;
          delay (25);
           }
        }
     MaxFile.close(); // Chiude il file dopo aver letto tutte le righe
     } 
    
    }        


/* The rest work is on Nextion with the code on a timers user event

   sys0=NaltGraph.val*255/5000  // use sys0 to make the calculations
   add 2,0,sys0                  // add the value to the waveform with id=2 at first channel (0) 
   n0.val=NaltGraph.val      // write NaltGraph.val to n0.val
 //
 // Waveform can take values from 0-250
 // we map the value from arduino 0-5000 :
 // the math type for map the range is:
 // return = (value - low1) * (high2 - low2) / (high1 - low1) + low2
 // as both ranges start from zero low1 and low2 = 0 
 // the type becomes
 // return = value*hight2/hight1
 // return=value*255/5000
 //
 
 
 // And some graphic effects
if(n0.val>3300)
{
  n0.bco=RED
}else
{
  n0.bco=YELLOW
}
*/


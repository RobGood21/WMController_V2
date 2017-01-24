/*
   WM-Controller
   DCC controller voor accessoires

   Waardes:
   1-bit duurt 2x 58 microsec.(pos en neg helft) 0-bit duurt minstens 90microsec. Beste kiezen we nu het dubbele van de 1-bit dus 116 micro
   Als geen data dan een stroom van 1-bits sturen. Minimaal 12 1-bits na laatst gezonden packett (boodschap, message whatever). DCC term is packett
*/


// Variabelen
//**********************
unsigned long KNOPTIMER;
boolean KNOPSTATUS = false; //geeft de vorige stand van de programmeerknop
boolean STOP = true;
int BITFALSE = 120; // duur van de helft van een pauze bit enof nul bit
unsigned long BITTIMER; // houdt verlopen tijd bij gedurende een bit
int DCCFASE = 0; // welke fase  in DC verzending
boolean BITPART = true; //welke helft van het bit is momenteel aktief, pos of neg.
boolean BYTEA1[8]; // byte adres 1, volgorde 0 1 2 3 4 5 6 7 
boolean BYTED1[8]; //byte data 1
boolean BYTEE[8]; // byte error

const int AP = 12; //aantal DCC packets, kun je uitbreiden als de boel op aantal vastloopt
typedef struct DCCPACKET{ //packets definieren in void MAKEPACKETS?
  int LOOPS; //aantal keren dat moet worden doorlopen
  int ADRES; // humaan dcc adres
  boolean STATE; //accessoire aan of uit, links of rechts
}
DCCPACKETS;
DCCPACKETS DCCPACKET[AP]; //aantal aangemaakte packets

int POINTERREAD = 0; //pointer read onthoud welke packet laatst is gelezen
int POINTERWRITE = 0; // pointer write welk packet het laatse is ingeschreven
int NEWPACKET; //Current packet, packet wat wordt verzonden
boolean PACKETACTIVE = false; //Geeft aan dat een packet actief is dus wordt verzonden

//Functies, voids
//**************************
void MAKEPACKETS() { //gebruikt is ontwikkeling om voorbeeld packets te maken.
  int i = 0;
  while (i < AP) {
    DCCPACKET[i].LOOPS = random(4);
    DCCPACKET[i].ADRES = random(2560);
    DCCPACKET[i].STATE = true;
    ++ i;
  }

}

void PRINT() { // alleen bij ontwikkeling.
  int i = 0;

  while ( i < AP) {
    delay(1000);
    Serial.println();
    Serial.println(i);
    Serial.println(DCCPACKET[i].LOOPS);
    Serial.println(DCCPACKET[i].ADRES);
    Serial.println(DCCPACKET[i].STATE);
    ++ i ;
  }
}


void SETOUTPUTS(boolean HL) {
  digitalWrite(3, HL);
  digitalWrite(4, !HL);
}

void NOODSTOP() {
  STOP = true;
  digitalWrite(3, LOW);
  digitalWrite(4, LOW);
  digitalWrite(12, LOW);
  digitalWrite(13, HIGH);
} //einde void Noodstop



void START() { // deze functie start de controller, eventueel oproepen uit setup
  STOP = false;
  digitalWrite(12, HIGH);
  digitalWrite(13, LOW);
  SETOUTPUTS(true);


} //einde void START

void KNOP() { // acties na indrukken knop
  if (digitalRead(6) != KNOPSTATUS) { //alleen iets doen als de knopstand is veranderd
    if (KNOPSTATUS == true) {
      KNOPSTATUS = false;
    } else {
      KNOPSTATUS = true;
      if (STOP == true) {
        START();
      } else {
        NOODSTOP();
      }
    }
  }
} //einde void knop

// vanaf hier straks in een aparte library doen.
void DCCLOOP() {
  /*
       Achter een volgend
       Status 0 : Haal volgend packet op, vertaal naar bytes
       Als geldig packet >> naar status 2
        anders doorgaan met nulbits sturen
       Status 1 : Zend preample,
       do while aantal te zenden BYTES > 0
       status 2 : nulbit laad BYTE
       Status 3 : eindig nulbit Zend BYTE
   *    *
  */
  switch (DCCFASE) {
      int i;
    case 0: //Pauze bits verzenden en Teverzenden Packet zoeken.
      if (millis() - BITTIMER > BITFALSE) { // even millis van gemaakt tijdens ontwikkeling, moet micros zijn als mede hieronder.
        BITPART = !BITPART;
        SETOUTPUTS(BITPART);
        BITTIMER = millis();
      }

      i = 0;
      while (i < AP) {
        if (PACKETACTIVE == false and DCCPACKET[POINTERREAD].LOOPS > 0) { //dus een nieuw te zenden packet gevonden
          NEWPACKET = POINTERREAD;
          PACKETACTIVE = true; //Packet actief is waar
          CONSTRUCTBYTES();
          DCCFASE = 1; // volgende doorloop naar fase 1 (preample verzenden)
        } else { //Loops van deze = 0
          ++ POINTERREAD; //volgende testen
        }
        i ++ ;
      }
      //als geen te zenden packet wordt gevonden, gebeurt er dus niks nada, en wordt het pauze bit doorgezet.
      break;

    case 1: //1e fase in packet verzenden, bytes aanmaken


      break;

    case 2:
      break;

    case 3:
      break;

  }


} //einde DCCLOOP()

void CONSTRUCTBYTES() {
  // cp current packet, zojuist gevonden message. tellen vanaf 0 --01234567
  /*
    3 bytes voor een basic accessory decoder packet format
    byte 1 eerst  1 0 daarna 6 adres bytes bit 0 is de LSB van het adres
    byte 2 eerst een 1 daarna 3 adres bytes geinverteerd (one complement) bit 6 het MSB van het adres.
    Daarna bit 3 een aan/uit ? daarna bit 1 en 2 geven aan welke van de vier uitgangen per adres bit o welke van de twee (afbuigend of rechtdoor)
    Ervan uitgaand dat bit 0 van databyte niet in het adres hoort is dus bit 1 het LSB bit 2 de 2 bit 0 in adres byte de 4 bit 1 in adres byte de 8 enz.
    Dus kunnen de bits eenvoudig worden berekend met de aftrektruck beginnend van uit waarde 1024
    max adres = 2047
  */

  int REST = DCCPACKET[NEWPACKET].ADRES;
  int BITWAARDE = 2048;

  if (REST - BITWAARDE >  0) {
    BYTED1[6] = false; //adres hoger dan 2048, dit bit wordt geinverteerd (one complement)
    REST = REST - BITWAARDE; //adres verminderen met waarde van dit JA bit
  } else {
    BYTED1[6] = true;
  }
  BITWAARDE = BITWAARDE / 2;
 
  BYTEA1[7] = true;
  BYTEA1[6] = false;
  BYTED1[7] = true;
  PRINTBYTES();
}// einde void constructbytes

void PRINTBYTES() {
	int i = 7;
	while (i >= 0) {
		Serial.print(BYTEA1[i]);
		i--;
	}
	Serial.println();
	i = 7;
	while (i >= 0) {
		Serial.print(BYTED1[i]);
		i --;
		}
Serial.println();
	i = 7;
	while (i >= 0 ) {
		Serial.print(BYTEE[i]);
		i --;
	}
Serial.println();
}
void setup() {
  pinMode(3, OUTPUT); //Hbrug
  pinMode(4, OUTPUT);//Hbrug
  pinMode(12, OUTPUT); //groene led
  pinMode(13, OUTPUT); // rode led
  pinMode(5, INPUT); //kortsluiting voeler
  pinMode(6, INPUT); //programmeerknop
  Serial.begin(9600);
  //**alleen tijdens ontwikkeling
  MAKEPACKETS();
  // START();
}

void loop() {
  ///Alleen tijdens ontwikkeling
  //PRINT(); //de aangemaakte packets
  //*******************
  DCCLOOP();
  /*
     if (STOP == false) { //in noodstop, alleen knop uitlezen
    DCCLOOP();
    }
       if (digitalRead(5) == LOW and STOP == false) NOODSTOP();
       if (millis() - KNOPTIMER > 100) {
        KNOPTIMER = millis();
        KNOP();
    }
  */

} //einde void loop




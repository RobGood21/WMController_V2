/*
   WM-Controller
   DCC controller voor accessoires

   Waardes:
   1-bit duurt 2x 58 microsec.(pos en neg helft) 0-bit duurt minstens 90microsec. Beste kiezen we nu het dubbele van de 1-bit dus 116 micro
   Als geen data dan een stroom van 1-bits sturen. Minimaal 12 1-bits na laatst gezonden packett (boodschap, message whatever). DCC term is packett
*/


// Variabelen
//**********************

//TIJDELIJKE VARIABELEN alleen in ontwerpfase
int WELKEKNOP = 0; //telt de knoppen dit NIET in library ?? 
boolean TKNOPSTATUS1 = false;
boolean TKNOPSTATUS2 = false;
boolean TKNOPSTATUS3 = false;
boolean TKNOPSTATUS4 = false;
boolean TKNOPSTATE1 = false; //is er laatst een aan of uit gestuurd, voor 1 knops bediening
boolean TKNOPSTATE2 = false;
boolean TKNOPSTATE3 = false;
boolean TKNOPSTATE4 = false;



//BLIJVENDE variabelen
unsigned long KNOPTIMER;
boolean KNOPSTATUS = false; //geeft de vorige stand van de programmeerknop
boolean STOP = true;



unsigned long BITTIMER; // houdt verlopen tijd bij gedurende een bit
int DCCFASE = 0; // welke fase  in DC verzending
int NEXTDCCFASE = 0; //geeft aan naar welk dccfase wordt gegaan na afloop van een nul bit.
int VANDCCFASE = 0; //uit welke fase wordt het nulbit aangevraagd.
boolean BITPART = true; //welke helft van het bit is momenteel aktief, pos of neg.
boolean BYTEA1[8]; // byte adres 1, volgorde 0 1 2 3 4 5 6 7 
boolean BYTED1[8]; //byte data 1
boolean BYTEE[8]; // byte error

const int AP = 12; //aantal DCC packets, kun je uitbreiden als de boel op aantal vastloopt
typedef struct DCCPACKET{ //packets definieren in void MAKEPACKETS?
  int LOOPS; //aantal keren dat moet worden doorlopen
  int LOOPTIME; //Periode tussen twee keer dit packett verzenden in millisec
  unsigned long LOOPTIMER; //tijd waarop laatste keer dit packett is verzonden 
  int ADRES; // humaan dcc adres
  boolean STATE; //accessoire aan of uit, links of rechts
}
DCCPACKETS;
DCCPACKETS DCCPACKET[AP]; //aantal aangemaakte packets

int POINTERREAD = 0; //pointer read onthoud welke packet laatst is gelezen
int POINTERWRITE = 0; // pointer write welk packet het laatse is ingeschreven
int POINTERBYTE; //POINTER wijst naar welk BYTE er wordt verzonden
int POINTERBIT; //POinter wijst naar welk bit van een byte wordt verzonden

//Functies, voids
//**************************
void MAKEPACKET(int loops, int looptime, int adres, boolean state) { 
//Maakt packet, DCC command. 
//aleerst een vrij packet zoeken, beginnen bij laatse POINTERWRITE
//loops is aantal doorlopen , Looptimer= tijd tussen twee uitvoeringen in millisec standaard ff 250ms, adres is humaan decimaal dcc adres, state = aan of uit bit 3 van databyte. 
	int TELLER = 0;
	boolean FOUNDFREE = false;
	do
	{
	++ POINTERWRITE;
	if (POINTERWRITE > AP) POINTERWRITE = 0;

	if (DCCPACKET[POINTERWRITE].LOOPS == 0) //Door POINTERWRITE aangewezen packet is vrij.
	{
		FOUNDFREE = true;
		DCCPACKET[POINTERWRITE].LOOPS=loops;
		DCCPACKET[POINTERWRITE].LOOPTIME = looptime;
		DCCPACKET[POINTERWRITE].LOOPTIMER = millis() + looptime; 
		DCCPACKET[POINTERWRITE].ADRES = adres;
		DCCPACKET[POINTERWRITE].STATE = state;

		//Serial.println(DCCPACKET[POINTERWRITE].ADRES = adres);
		//Serial.println(POINTERWRITE);

		TELLER = AP + 2; // om uit de do while te springen
	} // niet vrij gebeurt niks, volgende POINTERWRITE testen.

	++TELLER;

	} while (TELLER < AP); //als er meer dan aantal packets vol zijn, foutmelding geven. 
	if (FOUNDFREE == false) Serial.println("Er gaat iets mis in MAKEPACKET, geen vrij packet gevonden verhoog AP");
}




void SETOUTPUTS(boolean hl) {
  digitalWrite(3, hl);
  digitalWrite(4, !hl);
}

void NOODSTOP() {
  STOP = true;
  digitalWrite(3, LOW);
  digitalWrite(4, LOW);
  digitalWrite(12, LOW);
  digitalWrite(13, HIGH);
} //einde void Noodstop

//****onderstaande niet in library
void TKNOP() { //deze functie NIET IN DE LIBRARY
//tijdelijk maakt een accessory opdracht. 
	//******** knop 1
		if (digitalRead(8)!=TKNOPSTATUS1) { //status knop dus veranderd
			if (TKNOPSTATUS1==false){
				TKNOPSTATUS1 = true; //knop dus ingedrukt
				//nu iets doen, dcc boodschap aanmaken bv. 
				TKNOPSTATE1 = !TKNOPSTATE1;
				MAKEPACKET(1,250,1, TKNOPSTATE1);
				}else {
				TKNOPSTATUS1 = false; //knop dus losgelaten
			}
		}
  //*************KNOP2
		if (digitalRead(9) != TKNOPSTATUS2) { //status knop dus veranderd
			if (TKNOPSTATUS2 == false) {
				TKNOPSTATUS2 = true; //knop dus ingedrukt
									 //nu iets doen, dcc boodschap aanmaken bv. 
				TKNOPSTATE2 = !TKNOPSTATE2;
				MAKEPACKET(4,250, 4, TKNOPSTATE2);
			}
			else {
				TKNOPSTATUS2 = false; //knop dus losgelaten
			}
		}
///************KNOP3
		if (digitalRead(10) != TKNOPSTATUS3) { //status knop dus veranderd
			if (TKNOPSTATUS3 == false) {
				TKNOPSTATUS3 = true; //knop dus ingedrukt
									 //nu iets doen, dcc boodschap aanmaken bv. 
				TKNOPSTATE3 = !TKNOPSTATE3;
				MAKEPACKET(3,250, 6, TKNOPSTATE3);
			}
			else {
				TKNOPSTATUS3 = false; //knop dus losgelaten
			}
		}
  ///***********KNOP4
		if (digitalRead(11) != TKNOPSTATUS4) { //status knop dus veranderd
			if (TKNOPSTATUS4 == false) {
				TKNOPSTATUS4 = true; //knop dus ingedrukt
									 //nu iets doen, dcc boodschap aanmaken bv. 
				TKNOPSTATE4 = !TKNOPSTATE4;
				MAKEPACKET(4,250, 10, TKNOPSTATE4);
			}
			else {
				TKNOPSTATUS4 = false; //knop dus losgelaten
			}
		}
}

void START() { // deze functie start de controller, eventueel oproepen uit setup
  STOP = false;
  digitalWrite(12, HIGH);
  digitalWrite(13, LOW);
  SETOUTPUTS(true);
  BITPART = true;


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


void DCCLOOP() { //verzend de commands, packets.
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
   int i;

  switch (DCCFASE) {  
 	  	  
    case 0: //Te verzenden packet zoeken, als geen een nulbit verzenden	  

		POINTERREAD ++; //volgende pointer, ook doen als dccpacket verzonden is...
		if (POINTERREAD > AP-1) POINTERREAD = 0; //per doorloop 1 van de DCCpacket plekken checken alleen in case 0, dus geen packet in behandeling
	     if (DCCPACKET[POINTERREAD].LOOPS > 0) { //dus een nieuw te zenden packet gevonden
			 if (millis() - DCCPACKET[POINTERREAD].LOOPTIMER > DCCPACKET[POINTERREAD].LOOPTIME)
			 { //is er voldoende tijd verlopen.
				 CONSTRUCTBYTES();
				 //  Serial.println("..........");
				  // Serial.println(POINTERREAD);
				   //Serial.println(DCCPACKET[POINTERREAD].LOOPS);
				   //Serial.println("...........");
				 DCCPACKET[POINTERREAD].LOOPS--; //aantal te doorlopen verkleinen
				 DCCFASE = 1; // volgende doorloop naar fase 1 (preample verzenden)  
				 DCCPACKET[POINTERREAD].LOOPTIMER = millis(); //reset timer voor deze packet-plek
			 }
			 else { //tijd nog niet verlopen dus volgende pointer
				 NEWBIT(0);
				 DCCFASE = 100;
			 }
		  }
		  else { //geen te verzenden packett gevonden, dus nulbit verzenden, DAARNA weer opnieuw na dccfase 0
			  NEWBIT(0);
			  DCCFASE = 100; //volgende doorloop voor nulbit, BITPART komt false retour
			}
		
      break;

    case 1: //1e fase in packet verzenden, bytes aanmaken
		//als we hier zijn aangekomen is er een te verzenden boodschap. dus NEWPACKET =true;
		//verzenden van het gevonden Packet waar POINTERREAD naar wijst. Beginnen  met een preample van 15 true bit.

		SENDTRUE(15); //is blocking, anders niet snel genoeg. Timing is hier kritisch
		NEWBIT(2); //hier wordt aangegeven naar welke dccfase als het nulbit klaar is
		DCCFASE = 100; //volgende doorloop naar fase 2, verzend een 0 bit 
		
      break;

    case 2:
		Serial.println("preamble en 1 nul bit verzonden, nu eerste byte...");
		
		//**************einde packetverzendng
		DCCFASE = 0; 
		// POINTERREAD++;
		//****************
      break;
	case 3:
		NOODSTOP();
		break;



    case 100: 
		
		/*verzend een nulbit, deze is NIET blocking dus wordt telkens doorlopen tot bit klaar is. Daarna naar volgende DCCfase
		we stellen dat bij start van het nulbit outputs true staan, bij het verlaten van het nulbit staan ze laag
		bittimer moet gelijk  gesteld zijn aan micros, setoutputs(true)
		*/

		// Serial.print("hier");

		if(millis()-BITTIMER > 120) //  if (micros()-BITTIMER > 120) //tijd verlopen werkend micros, test millis
		{
			if (BITPART==true)
			{
			SETOUTPUTS(false);
			BITPART = false;
			BITTIMER = millis(); // micros();
			}
			else {
				//tijdverlopen, nu teruggeven aan aan DCCFASE
				DCCFASE = NEXTDCCFASE; 
			}

		}
      break;

  }


} //einde DCCLOOP()
void NEWBIT(int nextdccfase) { //maakt alle standaard instellingen voor een new bit
	SETOUTPUTS(true);
	BITPART = true;
	NEXTDCCFASE = nextdccfase;
	BITTIMER = millis(); //micros();
}
void SENDTRUE(int count) { // aantal waar bits verzenden, voorlopig deze void blocking maken. 
	int i = count;
	while (i > 0)
	{
		SETOUTPUTS(true);
		delay(58); //delayMicroseconds(58); //millis in test, micros in werking
		SETOUTPUTS(false);
		delay(58); //delayMicroseconds(58);
		i--;
		
	}
	BITPART = false;
} //merkop outputs worden dus in de false stand achtergelaten...


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

  int REST = DCCPACKET[POINTERREAD].ADRES;
  int BITWAARDE = 2048;
  int i = 0; //teller
    i = 6;
  while (i > 3) {
  if (REST - BITWAARDE >= 0) {
	  //Serial.print("HIER!");
	  BYTED1[i] = 0;
	  REST = REST - BITWAARDE;
  }
  else {
	  BYTED1[i] = 1;
  }
 BITWAARDE = BITWAARDE / 2;
  i--;
  }
  //samenstellen adresbits (0-1-2-3-4-5) van BYTEA1 (niet geinverteerd)
  i = 5;
  while (i >= 0) {
	  if (REST - BITWAARDE >= 0) {
		  BYTEA1[i] = true;
		  REST = REST - BITWAARDE;
	  }
	  else
		  BYTEA1[i] = false;
	  {
  	  }
	  BITWAARDE = BITWAARDE / 2;
	  i--;
  }
  //samenstellen bit 0-3 van BYTED1, databyte. 
  //bit 0 is een beethe onduidelijk wat wordt bedoeld, keuze van de twee helften van een wissel ?
  //bit 1-2 geven aan welke van de 4 binnen 1 decoder... ? Ook ingewikkeld maar gewoon bit 2,4,8 lijkt mij ...
  //we zien wel hoe in de praktijk een commercieel decoder hierop reageert
  i = 2;
  while (i > 0) {
	  if (REST - BITWAARDE >= 0) {
		  BYTED1[i] = true;
		  REST = REST - BITWAARDE;
	  }	  else
	  {		  BYTED1[i] = false;
	  }
	 
	  BITWAARDE = BITWAARDE / 2; // BITWAARDE ee int dus bij waarde 1 stoppen
	  i--;
	  //blijft over BIT 0 van BYTED1 is de rest van de aftrekking....toch?
	  if (REST == 1) {
		  BYTED1[0] = true;
	  }
	  else {
		  BYTED1[0] = false;
	  }
	  {

	  }
  }

  Serial.println();
  Serial.print("Restwaarde adres:");
  Serial.println(REST);

  //invullen vaste bits voor aanduiding wat voor packet, message.
  BYTEA1[7] = true;
  BYTEA1[6] = false;
  BYTED1[7] = true;
  BYTED1[4] = DCCPACKET[POINTERREAD].STATE; //aan of uit.
  i = 0;
  while (i < 8) {
	  BYTEE[i] = BYTEA1[i] ^ BYTED1[i];
	  i++;
  }

  PRINTBYTES();


}// einde void constructbytes

void PRINTBYTES() {
	//Serial.print(" adres: ");
	//Serial.println(DCCPACKET[POINTERREAD].ADRES);
	//Serial.println("----------");

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
  //**tijdelijk bedienknoppen
  
  //tijdelijk als input knoppen
  pinMode(8, INPUT);
  pinMode(9, INPUT);
  pinMode(10, INPUT);
  pinMode(11, INPUT);

  
  Serial.begin(9600);
  while (!Serial)  {
	  ;
  }
  //**alleen tijdens ontwikkeling
 // MAKEPACKET(); //na 28jan niet meer nodig
  // START();
}

void loop() {
  ///Alleen tijdens ontwikkeling
  //PRINT(); //de aangemaakte packets
  //*******************
  // DCCLOOP();

     if (STOP == false) { //in noodstop, alleen knop uitlezen
    DCCLOOP();
    }


       if (digitalRead(5) == LOW && STOP == false) NOODSTOP();

       if (millis() - KNOPTIMER > 100) {
        KNOPTIMER = millis();
        KNOP();
		TKNOP(); //deze tijdelijk komt straks NIET in de library, zet tijdelijk even een opdracht in de rij. 
    }

//*********hiertussen komt stratks NIET in de library maar in het aansturend deel


//*************************************************
} //einde void loop




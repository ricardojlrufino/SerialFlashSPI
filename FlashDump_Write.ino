/*
  Flash Winbond chips using Serial

  Using library: https://github.com/Marzogh/SPIFlash

  TIP: You can use a program like "Cutecom" to send files

  NOTE: In the tests I did using higher baud in the serial but the maximum
  that worked out was 115200 - Note that this is very slow, +/- 5 min for 4MB

  @author Ricardo JL Rufino
  @date 21/04/2017
*/

#include<SPIFlash.h>

#define PAGESIZE 256

uint8_t page_buffer[PAGESIZE];
uint8_t data_first[PAGESIZE];
uint16_t page = 0;

long data_length = 0;

// Ajust for file size (multiple of 256)
// long max_pages = 32768; // see in Datasheet (SIZE 8MB)
// long max_pages = 15360; // see in Datasheet (SIZE 4MB)
long max_pages = 512; // UBOOT (128KiB)

SPIFlash flash;

#define FSERIAL Serial

void setup() {
  delay(10);
  FSERIAL.begin(115200);     // Initialize USB port
  while (!FSERIAL);          // Wait until connection is established
  memset(page_buffer, 0, sizeof(page_buffer));

  pinMode(13, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  digitalWrite(8, HIGH);
  digitalWrite(9, HIGH);

  FSERIAL.print(F("Initializing Flash memory"));
  for (int i = 0; i < 10; ++i) {
    FSERIAL.print(F("."));
  }

  flash.begin();
  FSERIAL.println(F(".... OK"));
  Serial.println(F("--------------------------------"));
  Serial.println(F("    Winbond Flash               "));
  Serial.println(F(" SPIFlash  library WRITE v1.0.0                                                 "));
  Serial.println(F("--------------------------------"));
  printFlashInfo();

  printMenu();
}

bool dumpFinish = false;
bool writeMode = false;

void loop() {
  // put your main code here, to run repeatedly:

  if (writeMode && !dumpFinish) {

    if (readPage()) {

      if (page == 0) copyData(page_buffer, data_first, PAGESIZE);

      flash.writeByteArray(page, 0, page_buffer, PAGESIZE, /*error=*/false);
      FSERIAL.print(page);
      FSERIAL.print("/");
      FSERIAL.println(max_pages);

      page++;

      if (page == max_pages) {
        dumpFinish = true;
        FSERIAL.println("100% [Finish]");
        FSERIAL.print("Last Page Size: "); FSERIAL.println(sizeof(page_buffer));
        digitalWrite(13, HIGH);
        while (FSERIAL.available() > 0) FSERIAL.read();  // clear buffer
      }


    }

  } else if(writeMode && dumpFinish) {

    FSERIAL.println("Press any Key:");
    while (!FSERIAL.available()) {
      delay(100);
    }

    if (FSERIAL.available()) {
      while (FSERIAL.available()) FSERIAL.read();  // clear buffer

      FSERIAL.println("==========\nFirst: \n ==========");
      _printPageBytes(data_first, 1);
      FSERIAL.println("==========\nLast \n ==========");
      _printPageBytes(page_buffer, 1);

      delay(100);
      FSERIAL.println(" ===== DONE !!! === ");

      printMenu();
    }

  }
}


boolean readPage() {

  while (FSERIAL.available() > 0) {
    if (data_length < sizeof(page_buffer)) {
      page_buffer[data_length++] = FSERIAL.read();
    } else {
      data_length = 0;
      return true;
    }
  }

  if (data_length == sizeof(page_buffer)) {
    data_length = 0;
    return true;
  }

  return false;
}


void copyData(uint8_t* src, uint8_t* dst, int len) {
  for (int i = 0; i < len; i++) {
    *dst++ = *src++;
  }
}

void _printPageBytes(uint8_t *page_buffer, uint8_t outputType) {
  char buffer[10];
  for (int a = 0; a < 16; ++a) {
    for (int b = 0; b < 16; ++b) {
      if (outputType == 1) {
        sprintf(buffer, "%02x ", page_buffer[a * 16 + b]);
        FSERIAL.print(buffer);
      }
      else if (outputType == 2) {
        uint8_t x = page_buffer[a * 16 + b];
        if (x < 10) FSERIAL.print("0");
        if (x < 100) FSERIAL.print("0");
        FSERIAL.print(x);
        FSERIAL.print(',');
      }
    }
    FSERIAL.println();
  }
}

void printPage(uint16_t page_number) {
  memset(page_buffer, 0, sizeof(page_buffer));
  flash.readByteArray(page_number, 0, &page_buffer[0], PAGESIZE);
  _printPageBytes(page_buffer, 1);
}


void printMenu() {
  FSERIAL.println(F("================"));
  FSERIAL.println(F("Select Options :"));
  FSERIAL.println(F("================"));
  FSERIAL.println(F("[E] Erase Clip"));
  FSERIAL.println(F("[W] Write File"));
  FSERIAL.println(F("[D] Dump x pages to output"));
  FSERIAL.println(F("[S] Set <size/num_pages> of dump/write"));
  
  while (!FSERIAL.available()) {}

  
  char opt = Serial.read();

  FSERIAL.print(F(">>> Selected: "));
  FSERIAL.println(opt);

  if (opt == 'E' || opt == 'e') eraseChip();
  if (opt == 'W' || opt == 'w') writeChip();
  if (opt == 'D' || opt == 'd') dumpChip();
  if (opt == 'S' || opt == 's') setPageSize();
  
}

void eraseChip() {
  FSERIAL.println(F("This function will erase the entire flash memory."));
  FSERIAL.println(F("Do you wish to continue? (Y/N)"));
  char c;
  waitSerial();
  c = (char)Serial.read();
  if (c == 'Y' || c == 'y') {
    FSERIAL.print(F("erasing ...."));
    if (flash.eraseChip())
      FSERIAL.println(F("[ok]"));
    else
      FSERIAL.println(F("[error]"));
  }

  printMenu();
}

void writeChip() {
  FSERIAL.println(F("[Waiting File...]"));
  writeMode = true;  // goto loop !!
}

void dumpChip() {
  FSERIAL.println("\n\n\n\n");s
  FSERIAL.println(F("[Drump File...]"));
  for (uint16_t i = 0; i < max_pages; ++i){
    printPage(i);
  }

  printMenu();
}

void setPageSize() {
  FSERIAL.println(F("    - Use: s 512 - to set '512' pages of '256' bytes"));
  FSERIAL.println(F("Current = "));FSERIAL.println(max_pages);
  FSERIAL.println(F("Set num pages/size of file (multiple of 256):"));
  waitSerial();
  max_pages = FSERIAL.parseInt();
  FSERIAL.print("Set = ");FSERIAL.println(max_pages);
  FSERIAL.flush();
  printMenu();
}

void printFlashInfo() {
  char printBuffer[128];
  uint8_t b1, b2, b3;
  uint32_t JEDEC = flash.getJEDECID();
  //uint16_t ManID = flash.getManID();
  b1 = (JEDEC >> 16);
  b2 = (JEDEC >> 8);
  b3 = (JEDEC >> 0);
  sprintf(printBuffer, "Manufacturer ID: %02xh\nMemory Type: %02xh\nCapacity: %02xh", b1, b2, b3);
  FSERIAL.println(printBuffer);
  memset(printBuffer, 0, sizeof(printBuffer));
  sprintf(printBuffer, "JEDEC ID: %04lxh", JEDEC);
  FSERIAL.println(printBuffer);
}


void waitSerial() {
  while (!FSERIAL.available()) {delay(10);}
}


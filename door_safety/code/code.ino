/*
 *    Turgut Ozal Universitesi
 *    Robotik ve Bilgisayar Destekli Uretim Kulubu
 *
 *    Kapi Guvenlik Sistemi
 *
 *    Sezai Furkan Pur & Mustafa Ozcan
 */

#include <SPI.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define	uchar	unsigned char
#define	uint	unsigned int


#define MAX_LEN 16

const int chipSelectPin = 10;
const int NRSTPD = 5;

const int delayDisp = 400;

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);


#define PCD_IDLE              0x00
#define PCD_AUTHENT           0x0E
#define PCD_RECEIVE           0x08
#define PCD_TRANSMIT          0x04
#define PCD_TRANSCEIVE        0x0C
#define PCD_RESETPHASE        0x0F
#define PCD_CALCCRC           0x03

#define PICC_REQIDL           0x26
#define PICC_REQALL           0x52
#define PICC_ANTICOLL         0x93
#define PICC_SElECTTAG        0x93
#define PICC_AUTHENT1A        0x60
#define PICC_AUTHENT1B        0x61
#define PICC_READ             0x30
#define PICC_WRITE            0xA0
#define PICC_DECREMENT        0xC0
#define PICC_INCREMENT        0xC1
#define PICC_RESTORE          0xC2
#define PICC_TRANSFER         0xB0
#define PICC_HALT             0x50


#define MI_OK                 0
#define MI_NOTAGERR           1
#define MI_ERR                2


//Page 0:Command and Status
#define     Reserved00            0x00    
#define     CommandReg            0x01    
#define     CommIEnReg            0x02    
#define     DivlEnReg             0x03    
#define     CommIrqReg            0x04    
#define     DivIrqReg             0x05
#define     ErrorReg              0x06    
#define     Status1Reg            0x07    
#define     Status2Reg            0x08    
#define     FIFODataReg           0x09
#define     FIFOLevelReg          0x0A
#define     WaterLevelReg         0x0B
#define     ControlReg            0x0C
#define     BitFramingReg         0x0D
#define     CollReg               0x0E
#define     Reserved01            0x0F
//Page 1:Command     
#define     Reserved10            0x10
#define     ModeReg               0x11
#define     TxModeReg             0x12
#define     RxModeReg             0x13
#define     TxControlReg          0x14
#define     TxAutoReg             0x15
#define     TxSelReg              0x16
#define     RxSelReg              0x17
#define     RxThresholdReg        0x18
#define     DemodReg              0x19
#define     Reserved11            0x1A
#define     Reserved12            0x1B
#define     MifareReg             0x1C
#define     Reserved13            0x1D
#define     Reserved14            0x1E
#define     SerialSpeedReg        0x1F
//Page 2:CFG    
#define     Reserved20            0x20  
#define     CRCResultRegM         0x21
#define     CRCResultRegL         0x22
#define     Reserved21            0x23
#define     ModWidthReg           0x24
#define     Reserved22            0x25
#define     RFCfgReg              0x26
#define     GsNReg                0x27
#define     CWGsPReg	          0x28
#define     ModGsPReg             0x29
#define     TModeReg              0x2A
#define     TPrescalerReg         0x2B
#define     TReloadRegH           0x2C
#define     TReloadRegL           0x2D
#define     TCounterValueRegH     0x2E
#define     TCounterValueRegL     0x2F
//Page 3:TestRegister     
#define     Reserved30            0x30
#define     TestSel1Reg           0x31
#define     TestSel2Reg           0x32
#define     TestPinEnReg          0x33
#define     TestPinValueReg       0x34
#define     TestBusReg            0x35
#define     AutoTestReg           0x36
#define     VersionReg            0x37
#define     AnalogTestReg         0x38
#define     TestDAC1Reg           0x39  
#define     TestDAC2Reg           0x3A   
#define     TestADCReg            0x3B   
#define     Reserved31            0x3C   
#define     Reserved32            0x3D   
#define     Reserved33            0x3E   
#define     Reserved34			  0x3F


uchar serNum[5];

uchar  writeData[16]={
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 100};
uchar  moneyConsume = 18 ;
uchar  moneyAdd = 10 ;

uchar sectorKeyA[16][16] = {
  {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF  }
  ,
  {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF  }
  ,
  {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF  }
  ,
};
uchar sectorNewKeyA[16][16] = {
  {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF  }
  ,
  {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xff,0x07,0x80,0x69, 0x19,0x84,0x07,0x15,0x76,0x14  }
  ,
  {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xff,0x07,0x80,0x69, 0x19,0x33,0x07,0x15,0x34,0x14  }
  ,
};

void setup() {                
  Serial.begin(9600);
  
  pinMode(2,OUTPUT);
  pinMode(3,OUTPUT);
  pinMode(4,OUTPUT);
  
  lcd.begin(16,2);

  for(int i = 0; i< 3; i++)
  {
    lcd.backlight();
    delay(250);
    lcd.noBacklight();
    delay(250);
  }

  lcd.backlight(); 

 SPI.begin();

  pinMode(chipSelectPin,OUTPUT); 
  digitalWrite(chipSelectPin, LOW);
  pinMode(NRSTPD,OUTPUT);
  digitalWrite(NRSTPD, HIGH);

  MFRC522_Init();  
}

void loop()
{
  uchar i,tmp;
  uchar status;
  uchar str[MAX_LEN];
  uchar RC_size;
  uchar blockAddr;	
  String mynum = "";

	
  status = MFRC522_Request(PICC_REQIDL, str);	
  if (status == MI_OK)
  {
    Serial.println("Card detected");
    Serial.print(str[0],BIN);
    Serial.print(" , ");
    Serial.print(str[1],BIN);
    Serial.println(" ");
    
    lcd.setCursor(0,0);
    lcd.print("Giris yapildi !");
    lcd.setCursor(0,1);
    lcd.print("TOU ROBOTIK");
      delay(delayDisp);
      lcd.clear();
      
      digitalWrite(2,HIGH);
      digitalWrite(3,LOW);
      digitalWrite(4,HIGH);
      delay(30);
      digitalWrite(2,LOW);
  }
  else
  {
    digitalWrite(4,LOW);
    digitalWrite(3,HIGH);
    lcd.clear();
    digitalWrite(2,LOW);
  }

 

  MFRC522_Halt();       

}


void Write_MFRC522(uchar addr, uchar val)
{
  digitalWrite(chipSelectPin, LOW);


  SPI.transfer((addr<<1)&0x7E);	
  SPI.transfer(val);

  digitalWrite(chipSelectPin, HIGH);
}

uchar Read_MFRC522(uchar addr)
{
  uchar val;

  digitalWrite(chipSelectPin, LOW);


  SPI.transfer(((addr<<1)&0x7E) | 0x80);	
  val =SPI.transfer(0x00);

  digitalWrite(chipSelectPin, HIGH);

  return val;	
}

void SetBitMask(uchar reg, uchar mask)  
{
  uchar tmp;
  tmp = Read_MFRC522(reg);
  Write_MFRC522(reg, tmp | mask);
}


void ClearBitMask(uchar reg, uchar mask)  
{
  uchar tmp;
  tmp = Read_MFRC522(reg);
  Write_MFRC522(reg, tmp & (~mask));
} 


void AntennaOn(void)
{
  uchar temp;

  temp = Read_MFRC522(TxControlReg);
  if (!(temp & 0x03))
  {
    SetBitMask(TxControlReg, 0x03);
  }
}

void AntennaOff(void)
{
  ClearBitMask(TxControlReg, 0x03);
}



void MFRC522_Reset(void)
{
  Write_MFRC522(CommandReg, PCD_RESETPHASE);
}



void MFRC522_Init(void)
{
  digitalWrite(NRSTPD,HIGH);

  MFRC522_Reset();

  Write_MFRC522(TModeReg, 0x8D);
  Write_MFRC522(TPrescalerReg, 0x3E);
  Write_MFRC522(TReloadRegL, 30);           
  Write_MFRC522(TReloadRegH, 0);

  Write_MFRC522(TxAutoReg, 0x40);
  Write_MFRC522(ModeReg, 0x3D);

  AntennaOn();
}

uchar MFRC522_Request(uchar reqMode, uchar *TagType)
{
  uchar status;  
  uint backBits;			
  
  Write_MFRC522(BitFramingReg, 0x07);

  TagType[0] = reqMode;
  status = MFRC522_ToCard(PCD_TRANSCEIVE, TagType, 1, TagType, &backBits);

  if ((status != MI_OK) || (backBits != 0x10))
  {    
    status = MI_ERR;
  }

  return status;
}



uchar MFRC522_ToCard(uchar command, uchar *sendData, uchar sendLen, uchar *backData, uint *backLen)
{
  uchar status = MI_ERR;
  uchar irqEn = 0x00;
  uchar waitIRq = 0x00;
  uchar lastBits;
  uchar n;
  uint i;

  switch (command)
  {
  case PCD_AUTHENT:		
    {
      irqEn = 0x12;
      waitIRq = 0x10;
      break;
    }
  case PCD_TRANSCEIVE:	
    {
      irqEn = 0x77;
      waitIRq = 0x30;
      break;
    }
  default:
    break;
  }

  Write_MFRC522(CommIEnReg, irqEn|0x80);
  ClearBitMask(CommIrqReg, 0x80);
  SetBitMask(FIFOLevelReg, 0x80);

  Write_MFRC522(CommandReg, PCD_IDLE);


  for (i=0; i<sendLen; i++)
  {   
    Write_MFRC522(FIFODataReg, sendData[i]);    
  }


  Write_MFRC522(CommandReg, command);
  if (command == PCD_TRANSCEIVE)
  {    
    SetBitMask(BitFramingReg, 0x80);
  }   

  i = 2000;
  do 
  {
    n = Read_MFRC522(CommIrqReg);
    i--;
  }
  while ((i!=0) && !(n&0x01) && !(n&waitIRq));

  ClearBitMask(BitFramingReg, 0x80);

    if (i != 0)
  {    
    if(!(Read_MFRC522(ErrorReg) & 0x1B))
    {
      status = MI_OK;
      if (n & irqEn & 0x01)
      {   
        status = MI_NOTAGERR;  
      }

      if (command == PCD_TRANSCEIVE)
      {
        n = Read_MFRC522(FIFOLevelReg);
        lastBits = Read_MFRC522(ControlReg) & 0x07;
        if (lastBits)
        {   
          *backLen = (n-1)*8 + lastBits;   
        }
        else
        {   
          *backLen = n*8;   
        }

        if (n == 0)
        {   
          n = 1;    
        }
        if (n > MAX_LEN)
        {   
          n = MAX_LEN;   
        }


        for (i=0; i<n; i++)
        {   
          backData[i] = Read_MFRC522(FIFODataReg);    
        }
      }
    }
    else
    {   
      status = MI_ERR;  
    }

  }

  return status;
}

uchar MFRC522_Anticoll(uchar *serNum)
{
  uchar status;
  uchar i;
  uchar serNumCheck=0;
  uint unLen;


  Write_MFRC522(BitFramingReg, 0x00);

  serNum[0] = PICC_ANTICOLL;
  serNum[1] = 0x20;
  status = MFRC522_ToCard(PCD_TRANSCEIVE, serNum, 2, serNum, &unLen);

  if (status == MI_OK)
  {

    for (i=0; i<4; i++)
    {   
      serNumCheck ^= serNum[i];
    }
    if (serNumCheck != serNum[i])
    {   
      status = MI_ERR;    
    }
  }


  return status;
} 


void CalulateCRC(uchar *pIndata, uchar len, uchar *pOutData)
{
  uchar i, n;

  ClearBitMask(DivIrqReg, 0x04);
  SetBitMask(FIFOLevelReg, 0x80);

  for (i=0; i<len; i++)
  {   
    Write_MFRC522(FIFODataReg, *(pIndata+i));   
  }
  Write_MFRC522(CommandReg, PCD_CALCCRC);

  i = 0xFF;
  do 
  {
    n = Read_MFRC522(DivIrqReg);
    i--;
  }
  while ((i!=0) && !(n&0x04));


  pOutData[0] = Read_MFRC522(CRCResultRegL);
  pOutData[1] = Read_MFRC522(CRCResultRegM);
}

uchar MFRC522_SelectTag(uchar *serNum)
{
  uchar i;
  uchar status;
  uchar size;
  uint recvBits;
  uchar buffer[9]; 

  buffer[0] = PICC_SElECTTAG;
  buffer[1] = 0x70;
  for (i=0; i<5; i++)
  {
    buffer[i+2] = *(serNum+i);
  }
  CalulateCRC(buffer, 7, &buffer[7]);		
  status = MFRC522_ToCard(PCD_TRANSCEIVE, buffer, 9, buffer, &recvBits);

  if ((status == MI_OK) && (recvBits == 0x18))
  {   
    size = buffer[0]; 
  }
  else
  {   
    size = 0;    
  }

  return size;
}


uchar MFRC522_Auth(uchar authMode, uchar BlockAddr, uchar *Sectorkey, uchar *serNum)
{
  uchar status;
  uint recvBits;
  uchar i;
  uchar buff[12]; 


  buff[0] = authMode;
  buff[1] = BlockAddr;
  for (i=0; i<6; i++)
  {    
    buff[i+2] = *(Sectorkey+i);   
  }
  for (i=0; i<4; i++)
  {    
    buff[i+8] = *(serNum+i);   
  }
  status = MFRC522_ToCard(PCD_AUTHENT, buff, 12, buff, &recvBits);

  if ((status != MI_OK) || (!(Read_MFRC522(Status2Reg) & 0x08)))
  {   
    status = MI_ERR;   
  }

  return status;
}

uchar MFRC522_Read(uchar blockAddr, uchar *recvData)
{
  uchar status;
  uint unLen;

  recvData[0] = PICC_READ;
  recvData[1] = blockAddr;
  CalulateCRC(recvData,2, &recvData[2]);
  status = MFRC522_ToCard(PCD_TRANSCEIVE, recvData, 4, recvData, &unLen);

  if ((status != MI_OK) || (unLen != 0x90))
  {
    status = MI_ERR;
  }

  return status;
}

uchar MFRC522_Write(uchar blockAddr, uchar *writeData)
{
  uchar status;
  uint recvBits;
  uchar i;
  uchar buff[18]; 

  buff[0] = PICC_WRITE;
  buff[1] = blockAddr;
  CalulateCRC(buff, 2, &buff[2]);
  status = MFRC522_ToCard(PCD_TRANSCEIVE, buff, 4, buff, &recvBits);

  if ((status != MI_OK) || (recvBits != 4) || ((buff[0] & 0x0F) != 0x0A))
  {   
    status = MI_ERR;   
  }

  if (status == MI_OK)
  {
    for (i=0; i<16; i++)
    {    
      buff[i] = *(writeData+i);   
    }
    CalulateCRC(buff, 16, &buff[16]);
    status = MFRC522_ToCard(PCD_TRANSCEIVE, buff, 18, buff, &recvBits);

    if ((status != MI_OK) || (recvBits != 4) || ((buff[0] & 0x0F) != 0x0A))
    {   
      status = MI_ERR;   
    }
  }

  return status;
}



void MFRC522_Halt(void)
{
  uchar status;
  uint unLen;
  uchar buff[4]; 

  buff[0] = PICC_HALT;
  buff[1] = 0;
  CalulateCRC(buff, 2, &buff[2]);

  status = MFRC522_ToCard(PCD_TRANSCEIVE, buff, 4, buff,&unLen);
}


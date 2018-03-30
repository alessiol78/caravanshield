/*********************************************************************
This is a library for our Monochrome OLEDs based on SSD1306 drivers

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/category/63_98

These displays use SPI to communicate, 4 or 5 pins are required to  
interface

Adafruit invests time and resources providing this open source code, 
please support Adafruit and open-source hardware by purchasing 
products from Adafruit!

Written by Limor Fried/Ladyada  for Adafruit Industries.  
BSD license, check license.txt for more information
All text above, and the splash screen below must be included in any redistribution

02/18/2013  Charles-Henri Hallard (http://hallard.me)
            Modified for compiling and use on Raspberry ArduiPi Board
            LCD size and connection are now passed as arguments on
            the command line (no more #define on compilation needed)
            ArduiPi project documentation http://hallard.me/arduipi
07/01/2013  Charles-Henri Hallard 
            Reduced code size removed the Adafruit Logo (sorry guys)
            Buffer for OLED is now dynamic to LCD size
            Added support of Seeed OLED 64x64 Display
            
07/26/2013  Charles-Henri Hallard 
            modified name for generic library using different OLED type

02/24/2015  Charles-Henri Hallard 
            added support for 1.3" I2C OLED with SH1106 driver
            
*********************************************************************/

#include "ArduiPi_OLED_lib.h"
#include "Adafruit_GFX.h"
#include "ArduiPi_OLED.h"
extern "C" {
#include "../bcm2835/bcm2835.h"
#include "glcdfont.c"
}

//#define I2C_SPOT

bool ArduiPi_OLED::isOk()
{
    return initialized;
}
inline bool ArduiPi_OLED::isSPI(void) {
    return (dc != -1 ? true : false);
}
inline bool ArduiPi_OLED::isI2C(void) {
    return (dc == -1 ? true : false);
}
// Low level I2C and SPI Write function
inline void ArduiPi_OLED::fastSPIwrite(uint8_t d) {
    bcm2835_spi_transfer(d);
}
inline void ArduiPi_OLED::fastI2Cwrite(uint8_t d) {
    bcm2835_i2c_write((char*)&d,1);
}
inline void ArduiPi_OLED::fastSPIwrite(char* tbuf, uint32_t len) {
    bcm2835_spi_writenb(tbuf, len);
}
inline void ArduiPi_OLED::fastI2Cwrite(char* tbuf, uint32_t len) {
    bcm2835_i2c_write(tbuf, len);
}

int16_t ArduiPi_OLED::getOledHeight() {
    return oled_height;
}
int16_t ArduiPi_OLED::getOledWidth() {
    return oled_width;
}

// Display instantiation
ArduiPi_OLED::ArduiPi_OLED()
{
    // Init all var, and clean
    // Command I/O
    rst = 0 ;
    dc  = 0 ;

    // Lcd size
    oled_width  = 0;
    oled_height = 0;

    // Empty pointer to OLED buffer
    poledbuff = NULL;

    initialized = false;
}

ArduiPi_OLED::~ArduiPi_OLED()
{

}

// When not initialized program using this library may
// know protocol for correct init call, he could just know
// oled number in driver list
bool ArduiPi_OLED::oled_is_spi_proto(uint8_t OLED_TYPE)
{
    switch (OLED_TYPE)
    {
    case OLED_ADAFRUIT_SPI_128x32:
    case OLED_ADAFRUIT_SPI_128x64:
        return true;
        break;
    }
    
    // default
    return false;

}

// initializer for OLED Type
bool ArduiPi_OLED::select_oled(uint8_t OLED_TYPE)
{
    // Default type
    oled_width  = 128;
    oled_height = 64;
    _i2c_addr = 0x00;
    oled_type = OLED_TYPE;

    // default OLED are using internal boost VCC converter
    vcc_type = SSD_Internal_Vcc;

    // Oled supported display
    // Setup size and I2C address
    switch (OLED_TYPE)
    {
    case OLED_ADAFRUIT_SPI_128x32:
        oled_height = 32;
        break;

    case OLED_ADAFRUIT_SPI_128x64:
        ;
        break;

    case OLED_ADAFRUIT_I2C_128x32:
        oled_height = 32;
        _i2c_addr = ADAFRUIT_I2C_ADDRESS;
        break;

    case OLED_ADAFRUIT_I2C_128x64:
        _i2c_addr = ADAFRUIT_I2C_ADDRESS;
        break;

    case OLED_SEEED_I2C_128x64:
        _i2c_addr = SEEED_I2C_ADDRESS ;
        vcc_type = SSD_External_Vcc;
        break;

    case OLED_SEEED_I2C_96x96:
        oled_width  = 96;
        oled_height = 96;
        _i2c_addr = SEEED_I2C_ADDRESS ;
        break;

    case OLED_SH1106_I2C_128x64:
        _i2c_addr = SH1106_I2C_ADDRESS;
        break;

        // houston, we have a problem
    default:
        return false;
        break;
    }

    // Buffer size differ from OLED type, 1 pixel is one bit
    // execpt for 96x96 seed, 1 pixel is 1 nible
    oled_buff_size = oled_width * oled_height ;

    if ( OLED_TYPE == OLED_SEEED_I2C_96x96 )
        oled_buff_size = oled_buff_size / 2 ;
    else
        oled_buff_size = oled_buff_size / 8;

    // De-Allocate memory for OLED buffer if any
    if (poledbuff)
        free(poledbuff);
    
    // Allocate memory for OLED buffer
    poledbuff = (uint8_t *) malloc ( oled_buff_size );

    if (!poledbuff)
        return false;
    
    memset(poledbuff,0,oled_buff_size);
    return true;

}

// initializer for SPI - we indicate the pins used and OLED type
//
bool ArduiPi_OLED::init(int8_t DC, int8_t RST, uint8_t OLED_TYPE)
{
    rst = RST;  // Reset Pin
    dc = DC;    // Data / command Pin
    initialized = false;

    // Select OLED parameters
    if (!select_oled(OLED_TYPE))
        return false;

    // Init & Configure Raspberry PI SPI
    bcm2835_spi_begin();
    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
    bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);

    // 16 MHz SPI bus, but Worked at 62 MHz also
    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_16);

    // Set the pin that will control DC as output
    bcm2835_gpio_fsel(dc, BCM2835_GPIO_FSEL_OUTP);

    // Setup reset pin direction as output
    if(rst>=0) bcm2835_gpio_fsel(rst, BCM2835_GPIO_FSEL_OUTP);

    complete_init();

    initialized = true;
    return true;
}

// initializer for I2C - we only indicate the reset pin and OLED type !
bool ArduiPi_OLED::init(int8_t RST, uint8_t OLED_TYPE)
{
    dc = -1; // DC and chip Select do not exist in I2C
    rst = RST;
    initialized = false;

    // Select OLED parameters
    if (!select_oled(OLED_TYPE))
        return false;

    // Setup reset pin direction as output
    if(rst>=0) bcm2835_gpio_fsel(rst, BCM2835_GPIO_FSEL_OUTP);

    complete_init();

    initialized = true;
    return true;
}

void ArduiPi_OLED::complete_init()
{
    uint8_t multiplex;
    uint8_t chargepump;
    uint8_t compins;
    uint8_t contrast;
    uint8_t precharge;

//    constructor(oled_width, oled_height, oled_type == OLED_SEEED_I2C_96x96);

    if(rst>=0) {
        // Setup reset pin direction (used by both SPI and I2C)
        bcm2835_gpio_fsel(rst, BCM2835_GPIO_FSEL_OUTP);
        bcm2835_gpio_write(rst, HIGH);

        // VDD (3.3V) goes high at start, lets just chill for a ms
        usleep(1000);

        // bring reset low
        bcm2835_gpio_write(rst, LOW);

        // wait 10ms
        usleep(10000);

        // bring out of reset
        bcm2835_gpio_write(rst, HIGH);
    }

    // depends on OLED type configuration
    if (oled_height == 32)
    {
        multiplex = 0x1F;
        compins   = 0x02;
        contrast  = 0x8F;
    }
    else
    {
        if (oled_type == OLED_SEEED_I2C_96x96 )
        {
            multiplex = 0x5F;
            compins   = 0x12;
            contrast  = 0x53;
        }
        // So 128x64
        else
        {
            multiplex = 0x3F;
            compins   = 0x12;

            if (oled_type == OLED_SH1106_I2C_128x64)
                contrast = 0x80;
            else
                contrast = (vcc_type==SSD_External_Vcc?0x9F:0xCF);
        }

    }

    if (vcc_type == SSD_External_Vcc)
    {
        chargepump = 0x10;
        precharge  = 0x22;
    }
    else
    {
        chargepump = 0x14;
        precharge  = 0xF1;
    }

    if(!begin()) return;

    printf("ssd1306 complete_init..\n");

    if (oled_type == OLED_SEEED_I2C_96x96 )
        sendCommand(SSD1327_Set_Command_Lock, 0x12); // Unlock OLED driver IC MCU interface from entering command. i.e: Accept commands

    sendCommand(SSD_Display_Off);
    sendCommand(SSD_Set_Muliplex_Ratio, multiplex);

    if (oled_type == OLED_SEEED_I2C_96x96 )
    {
        sendCommand(SSD1327_Set_Display_Clock_Div, 0x01);
        sendCommand(SSD1327_Set_Display_Start_Line    , 0   );
        sendCommand(SSD1327_Set_Display_Offset, 96  );
        sendCommand(SSD_Set_Segment_Remap     , 0x46);

        sendCommand(0xAB); // set vdd internal
        sendCommand(0x01); //

        sendCommand(0xB1); // Set Phase Length
        sendCommand(0X51); //

        sendCommand(0xB9); //

        sendCommand(0xBC); // set pre_charge voltage/VCOMH
        sendCommand(0x08); // (0x08);

        sendCommand(0xBE); // set VCOMH
        sendCommand(0X07); // (0x07);

        sendCommand(0xB6); // Set second pre-charge period
        sendCommand(0x01); //

        sendCommand(0xD5); // enable second precharge and enternal vsl
        sendCommand(0X62); // (0x62);

        // Set Normal Display Mode
        sendCommand(SSD1327_Normal_Display);

        // Row Address
        // Start 0 End 95
        sendCommand(SSD1327_Set_Row_Address, 0, 95);

        // Column Address
        // Start from 8th Column of driver IC. This is 0th Column for OLED
        // End at  (8 + 47)th column. Each Column has 2 pixels(segments)
        sendCommand(SSD1327_Set_Column_Address, 8, 0x37 );

        // Map to horizontal mode
        sendCommand(0xA0); // remap to
        sendCommand(0x46); // Vertical mode
    }
    else if (oled_type == OLED_SH1106_I2C_128x64)
    {
        sendCommand(SSD1306_Set_Lower_Column_Start_Address|0x02); /*set lower column address*/
        sendCommand(SSD1306_Set_Higher_Column_Start_Address);     /*set higher column address*/
        sendCommand(SSD1306_Set_Start_Line);                      /*set display start line*/
        sendCommand(SH1106_Set_Page_Address);    /*set page address*/
        sendCommand(SSD_Set_Segment_Remap|0x01); /*set segment remap*/
        sendCommand(SSD1306_Normal_Display);     /*normal / reverse*/
        sendCommand(0xad);    /*set charge pump enable*/
        sendCommand(0x8b);    /*external VCC   */
        sendCommand(0x30);    /*0X30---0X33  set VPP   9V liangdu!!!!*/
        sendCommand(SSD1306_Set_Com_Output_Scan_Direction_Remap);    /*Com scan direction*/
        sendCommand(SSD1306_Set_Display_Offset);    /*set display offset*/
        sendCommand(0x00);   /*   0x20  */
        sendCommand(SSD1306_Set_Display_Clock_Div);    /*set osc division*/
        sendCommand(0x80);
        sendCommand(SSD1306_Set_Precharge_Period);    /*set pre-charge period*/
        sendCommand(0x1f);    /*0x22*/
        sendCommand(SSD1306_Set_Com_Pins);    /*set COM pins*/
        sendCommand(0x12);
        sendCommand(SSD1306_Set_Vcomh_Deselect_Level);    /*set vcomh*/
        sendCommand(0x40);
    }
    else
    {
        sendCommand(SSD1306_Charge_Pump_Setting, chargepump);
        sendCommand(SSD1306_Set_Memory_Mode, 0x00);              // 0x20 0x0 act like ks0108
        sendCommand(SSD1306_Set_Display_Clock_Div, 0x80);      // 0xD5 + the suggested ratio 0x80
        sendCommand(SSD1306_Set_Display_Offset, 0x00);        // no offset
        sendCommand(SSD1306_Set_Start_Line | 0x0);            // line #0
        // use this two commands to flip display
        sendCommand(SSD_Set_Segment_Remap | 0x1);
        sendCommand(SSD1306_Set_Com_Output_Scan_Direction_Remap);

        sendCommand(SSD1306_Set_Com_Pins, compins);
        sendCommand(SSD1306_Set_Precharge_Period, precharge);
        sendCommand(SSD1306_Set_Vcomh_Deselect_Level, 0x40); // 0x40 -> unknown value in datasheet
        sendCommand(SSD1306_Entire_Display_Resume);
        sendCommand(SSD1306_Normal_Display);         // 0xA6

        // Reset to default value in case of
        // no reset pin available on OLED,
        sendCommand( SSD_Set_Column_Address, 0, 127 );
        sendCommand( SSD_Set_Page_Address, 0,   7 );
    }

    sendCommand(SSD_Set_ContrastLevel, contrast);

    stopscroll();

    // Empty uninitialized buffer
    memset(poledbuff,0,oled_buff_size);

    // turn on oled panel
    printf("ssd1306 turn on panel...\n");
    sendCommand(SSD_Display_On);

    // wait 100ms
    usleep(100000);

#ifdef I2C_SPOT
    release();
#endif
}

void ArduiPi_OLED::close( void )
{
    // De-Allocate memory for OLED buffer if any
    if (poledbuff)
        free(poledbuff);
    
    poledbuff = NULL;

    // Release Raspberry SPI
    if ( isSPI() )
        bcm2835_spi_end();
/*
    // Release Raspberry I2C
    if ( isI2C() )
        bcm2835_i2c_end();
*/
    // Release Raspberry I/O control
    bcm2835_close();
}


bool ArduiPi_OLED::begin( void )
{
    if( isI2C() ) {
        printf("i2c begin addr: 0x%02x.\n",_i2c_addr);

        // Init & Configure Raspberry PI I2C
        bcm2835_i2c_begin();
        bcm2835_i2c_setSlaveAddress(_i2c_addr);

        // Set clock to 400 KHz
        // does not seem to work, will check this later
        bcm2835_i2c_set_baudrate(400000);

        usleep(50000);

    }
    return true;
}

void ArduiPi_OLED::release( void )
{
    // Release Raspberry I2C
    if ( isI2C() ) {
        printf("i2c release.\n");
        bcm2835_i2c_end();
    }
}

void ArduiPi_OLED::setSeedTextXY(uint8_t Row, uint8_t Column)
{
#ifdef I2C_SPOT
    if(!begin()) return;
#else
    if(!initialized) return;
#endif
    //Column Address
    sendCommand(0x15);             /* Set Column Address */
    sendCommand(0x08+(Column*4));  /* Start Column: Start from 8 */
    sendCommand(0x37);             /* End Column */
    // Row Address
    sendCommand(0x75);             /* Set Row Address */
    sendCommand(0x00+(Row*8));     /* Start Row*/
    sendCommand(0x07+(Row*8));     /* End Row*/
#ifdef I2C_SPOT
    release();
#endif
}

void ArduiPi_OLED::putSeedChar(char C, uint8_t grayH, uint8_t grayL)
{
    if((uint8_t)C < 32 || (uint8_t)C > 127) //Ignore non-printable ASCII characters. This can be modified for multilingual font.
    {
        C=' '; //Space
    }
#ifdef I2C_SPOT
    if(!begin()) return;
#else
    if(!initialized) return;
#endif
    for(int i=0;i<8;i=i+2)
    {
        for(int j=0;j<8;j++)
        {
            // Character is constructed two pixel at a time using vertical mode from the default 8x8 font
            uint8_t c=0x00;
            uint8_t bit1=( seedfont[C-32][i]   >> j) & 0x01;
            uint8_t bit2=( seedfont[C-32][i+1] >> j) & 0x01;
            // Each bit is changed to a nibble
            c|=(bit1)?grayH:0x00;
            c|=(bit2)?grayL:0x00;
            sendData(c);
        }
    }
#ifdef I2C_SPOT
    release();
#endif
}

void ArduiPi_OLED::putSeedString(const char *String, uint8_t grayH, uint8_t grayL)
{
    int i=0;
    while(String[i])
    {
        putSeedChar( String[i], grayH, grayL );
        i++;
    }
}

// http://asf.atmel.com/bugzilla/show_bug.cgi?id=3236
void ArduiPi_OLED::setRotation(uint8_t type)
{
    switch (type) {
    case 1:
        sendCommand(0xA0);
        sendCommand(0xC0);
        break;
    case 2:
        sendCommand(0xA0);
        sendCommand(0xC8);
        break;
    case 3:
        sendCommand(0xA1);
        sendCommand(0xC0);
        break;
    case 4:
        sendCommand(0xA1);
        sendCommand(0xC8);
        break;
    default:
        break;
    }
}

void ArduiPi_OLED::setBrightness(uint8_t Brightness)
{
#ifdef I2C_SPOT
    if(!begin()) return;
#else
    if(!initialized) return;
#endif
    sendCommand(SSD_Set_ContrastLevel);
    sendCommand(Brightness);
#ifdef I2C_SPOT
    release();
#endif
}


void ArduiPi_OLED::invertDisplay(uint8_t i) 
{
#ifdef I2C_SPOT
    if(!begin()) return;
#else
    if(!initialized) return;
#endif
    if (i)
        sendCommand(SSD_Inverse_Display);
    else
        sendCommand(oled_type==OLED_SEEED_I2C_96x96 ? SSD1327_Normal_Display : SSD1306_Normal_Display);
#ifdef I2C_SPOT
    release();
#endif
}

void ArduiPi_OLED::sendCommand(uint8_t c) 
{ 
    // Is SPI
    if (isSPI())
    {
        // Setup D/C line to low to switch to command mode
        bcm2835_gpio_write(dc, LOW);

        // Write Data on SPI
        fastSPIwrite(c);
    }
    // so I2C
    else
    {
        char buff[2] ;

        // Clear D/C to switch to command mode
        buff[0] = SSD_Command_Mode ;
        buff[1] = c;

        // Write Data on I2C
        fastI2Cwrite(buff, sizeof(buff))  ;
    }
}

void ArduiPi_OLED::sendCommand(uint8_t c0, uint8_t c1) 
{ 
    char buff[3] ;
    buff[1] = c0;
    buff[2] = c1;

    // Is SPI
    if (isSPI())
    {
        // Setup D/C line to low to switch to command mode
        bcm2835_gpio_write(dc, LOW);

        // Write Data
        fastSPIwrite(&buff[1], 2);
    }
    // I2C
    else
    {
        // Clear D/C to switch to command mode
        buff[0] = SSD_Command_Mode ;

        // Write Data on I2C
        fastI2Cwrite(buff, 3) ;
    }
}

void ArduiPi_OLED::sendCommand(uint8_t c0, uint8_t c1, uint8_t c2) 
{ 
    char buff[4] ;
    
    buff[1] = c0;
    buff[2] = c1;
    buff[3] = c2;

    // Is SPI
    if (isSPI())
    {
        // Setup D/C line to low to switch to command mode
        bcm2835_gpio_write(dc, LOW);

        // Write Data
        fastSPIwrite(&buff[1], 3);
    }
    // I2C
    else
    {
        // Clear D/C to switch to command mode
        buff[0] = SSD_Command_Mode;

        // Write Data on I2C
        fastI2Cwrite(buff, sizeof(buff))  ;
    }
}


// startscrollright
// Activate a right handed scroll for rows start through stop
// Hint, the display is 16 rows tall. To scroll the whole display, run:
// display.scrollright(0x00, 0x0F) 
void ArduiPi_OLED::startscrollright(uint8_t start, uint8_t stop)
{
#ifdef I2C_SPOT
    if(!begin()) return;
#else
    if(!initialized) return;
#endif
    sendCommand(SSD_Right_Horizontal_Scroll);
    sendCommand(0X00);
    sendCommand(start);
    sendCommand(0X00);
    sendCommand(stop);
    sendCommand(0X01);
    sendCommand(0XFF);
    sendCommand(SSD_Activate_Scroll);
#ifdef I2C_SPOT
    release();
#endif
}

// startscrollleft
// Activate a right handed scroll for rows start through stop
// Hint, the display is 16 rows tall. To scroll the whole display, run:
// display.scrollright(0x00, 0x0F) 
void ArduiPi_OLED::startscrollleft(uint8_t start, uint8_t stop)
{
#ifdef I2C_SPOT
    if(!begin()) return;
#else
    if(!initialized) return;
#endif
    sendCommand(SSD_Left_Horizontal_Scroll);
    sendCommand(0X00);
    sendCommand(start);
    sendCommand(0X00);
    sendCommand(stop);
    sendCommand(0X01);
    sendCommand(0XFF);
    sendCommand(SSD_Activate_Scroll);
#ifdef I2C_SPOT
    release();
#endif
}

// startscrolldiagright
// Activate a diagonal scroll for rows start through stop
// Hint, the display is 16 rows tall. To scroll the whole display, run:
// display.scrollright(0x00, 0x0F) 
void ArduiPi_OLED::startscrolldiagright(uint8_t start, uint8_t stop)
{
#ifdef I2C_SPOT
    if(!begin()) return;
#else
    if(!initialized) return;
#endif
    sendCommand(SSD1306_SET_VERTICAL_SCROLL_AREA);
    sendCommand(0X00);
    sendCommand(oled_height);
    sendCommand(SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL);
    sendCommand(0X00);
    sendCommand(start);
    sendCommand(0X00);
    sendCommand(stop);
    sendCommand(0X01);
    sendCommand(SSD_Activate_Scroll);
#ifdef I2C_SPOT
    release();
#endif
}

// startscrolldiagleft
// Activate a diagonal scroll for rows start through stop
// Hint, the display is 16 rows tall. To scroll the whole display, run:
// display.scrollright(0x00, 0x0F) 
void ArduiPi_OLED::startscrolldiagleft(uint8_t start, uint8_t stop)
{
#ifdef I2C_SPOT
    if(!begin()) return;
#else
    if(!initialized) return;
#endif
    sendCommand(SSD1306_SET_VERTICAL_SCROLL_AREA);
    sendCommand(0X00);
    sendCommand(oled_height);
    sendCommand(SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL);
    sendCommand(0X00);
    sendCommand(start);
    sendCommand(0X00);
    sendCommand(stop);
    sendCommand(0X01);
    sendCommand(SSD_Activate_Scroll);
#ifdef I2C_SPOT
    release();
#endif
}


void ArduiPi_OLED::setHorizontalScrollProperties(bool direction,uint8_t startRow, uint8_t endRow,uint8_t startColumn, uint8_t endColumn, uint8_t scrollSpeed)
{
#ifdef I2C_SPOT
    if(!begin()) return;
#else
    if(!initialized) return;
#endif
    if(Scroll_Right == direction)
    {
        //Scroll Right
        sendCommand(SSD_Left_Horizontal_Scroll);
    }
    else
    {
        //Scroll Left
        sendCommand(SSD_Right_Horizontal_Scroll);
    }
    sendCommand(0x00);       //Dummmy byte
    sendCommand(startRow);
    sendCommand(scrollSpeed);
    sendCommand(endRow);
    sendCommand(startColumn+8);
    sendCommand(endColumn+8);
    sendCommand(0x00);      //Dummmy byte
#ifdef I2C_SPOT
    release();
#endif
}

void ArduiPi_OLED::stopscroll(void)
{
#ifdef I2C_SPOT
    if(!begin()) return;
#else
    if(!initialized) return;
#endif
    sendCommand(SSD_Deactivate_Scroll);
#ifdef I2C_SPOT
    release();
#endif
}

void ArduiPi_OLED::sendData(uint8_t c) 
{
    // SPI
    if ( isSPI())
    {
        // SPI
        // Setup D/C line to high to switch to data mode
        bcm2835_gpio_write(dc, HIGH);

        // write value
        fastSPIwrite(c);
    }
    // I2C
    else
    {
        char buff[2] ;

        // Setup D/C to switch to data mode
        buff[0] = SSD_Data_Mode;
        buff[1] = c;

        // Write on i2c
        fastI2Cwrite( buff, sizeof(buff)) ;
    }
}

void ArduiPi_OLED::display(Adafruit_GFX *frame_buff)
{
#ifdef I2C_SPOT
    if(!begin()) return;
#else
    if(!initialized) return;
#endif
    if (oled_type == OLED_SEEED_I2C_96x96 )
    {
        sendCommand(SSD1327_Set_Row_Address   , 0x00, 0x5F);
        sendCommand(SSD1327_Set_Column_Address, 0x08, 0x37);
    }
    else
    {
        sendCommand(SSD1306_Set_Lower_Column_Start_Address  | 0x0); // low col = 0
        sendCommand(SSD1306_Set_Higher_Column_Start_Address | 0x0); // hi col = 0
        sendCommand(SSD1306_Set_Start_Line  | 0x0); // line #0
    }

    uint16_t i=0 ;

    // copy frame buffer to OLED data buffer and set pointer
    memcpy(poledbuff,frame_buff->frameBuffer(),oled_buff_size);
    uint8_t * p = poledbuff;

    // SPI
    if ( isSPI())
    {
        // Setup D/C line to high to switch to data mode
        bcm2835_gpio_write(dc, HIGH);

        // Send all data to OLED
        for ( i=0; i<oled_buff_size; i++)
        {
            fastSPIwrite(*p++);
        }

        // I wonder why we have to do this (check datasheet)
        if (oled_height == 32)
        {
            for (uint16_t i=0; i<oled_buff_size; i++)
            {
                fastSPIwrite(0);
            }
        }

    }
    // I2C
    else
    {
        char buff[17] ;
        uint8_t x ;

        // Setup D/C to switch to data mode
        buff[0] = SSD_Data_Mode;

        if (oled_type == OLED_SH1106_I2C_128x64)
        {
            for (uint8_t k=0; k<8; k++)
            {
                sendCommand(0xB0+k);//set page addressSSD_Data_Mode;
                sendCommand(0x02) ;//set lower column address
                sendCommand(0x10) ;//set higher column address

                for( i=0; i<8; i++)
                {
                    for (x=1; x<=16; x++)
                        buff[x] = *p++;

                    fastI2Cwrite(buff, 17);
                }
            }
        }
        else
        {
            // loop trough all OLED buffer and
            // send a bunch of 16 data byte in one xmission
            for ( i=0; i<oled_buff_size; i+=16 )
            {
                for (x=1; x<=16; x++)
                    buff[x] = *p++;

                fastI2Cwrite(buff, 17);
            }
        }
    }
#ifdef I2C_SPOT
    usleep(50000);
    release();
#endif
}


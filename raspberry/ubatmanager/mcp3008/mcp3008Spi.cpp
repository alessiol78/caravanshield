#include "mcp3008Spi.h"
using namespace std;

/**********************************************************
 * spiOpen() :function is called by the constructor.
 * It is responsible for opening the spidev device
 * "devspi" and then setting up the spidev interface.
 * private member variables are used to configure spidev.
 * They must be set appropriately by constructor before calling
 * this function.
 * *********************************************************/
int mcp3008Spi::spiOpen(std::string devspi)
{
#ifdef __arm__
    int statusVal = -1;
    this->spifd = open(devspi.c_str(), O_RDWR);
    if(this->spifd < 0){
        std::cout << "could not open SPI device" << std::endl;
        return 1;
    }

    statusVal = ioctl (this->spifd, SPI_IOC_WR_MODE, &(this->mode));
    if(statusVal < 0){
        std::cout << "Could not set SPIMode (WR)...ioctl fail" << std::endl;
        return 2;
    }

    statusVal = ioctl (this->spifd, SPI_IOC_RD_MODE, &(this->mode));
    if(statusVal < 0) {
        std::cout << "Could not set SPIMode (RD)...ioctl fail" << std::endl;
        return 3;
    }

    statusVal = ioctl (this->spifd, SPI_IOC_WR_BITS_PER_WORD, &(this->bitsPerWord));
    if(statusVal < 0) {
        std::cout << "Could not set SPI bitsPerWord (WR)...ioctl fail" << std::endl;
        return 4;
    }

    statusVal = ioctl (this->spifd, SPI_IOC_RD_BITS_PER_WORD, &(this->bitsPerWord));
    if(statusVal < 0) {
        std::cout << "Could not set SPI bitsPerWord(RD)...ioctl fail" << std::endl;
        return 5;
    }

    statusVal = ioctl (this->spifd, SPI_IOC_WR_MAX_SPEED_HZ, &(this->speed));
    if(statusVal < 0) {
        std::cout << "Could not set SPI speed (WR)...ioctl fail" << std::endl;
        return 6;
    }

    statusVal = ioctl (this->spifd, SPI_IOC_RD_MAX_SPEED_HZ, &(this->speed));
    if(statusVal < 0) {
        std::cout << "Could not set SPI speed (RD)...ioctl fail" << std::endl;
        return 7;
    }
#endif
    return 0;
}

/***********************************************************
 * spiClose(): Responsible for closing the spidev interface.
 * Called in destructor
 * *********************************************************/

int mcp3008Spi::spiClose()
{
    int statusVal = -1;
#ifdef __arm__
    statusVal = close(this->spifd);
    if(statusVal < 0) {
        std::cout << "Could not close SPI device" << std::endl;
        return 1;
    }
#endif
    return 0;
}

/********************************************************************
 * This function writes data "data" of length "length" to the spidev
 * device. Data shifted in from the spidev device is saved back into
 * "data".
 * ******************************************************************/
int mcp3008Spi::spiWriteRead( unsigned char *data, int length)
{

    struct spi_ioc_transfer spi[length];
    int i = 0;
    int retVal = -1;
    bzero(spi, sizeof spi); // ioctl struct must be zeroed

    // one spi transfer for each byte

    for (i = 0 ; i < length ; i++){

        spi[i].tx_buf        = (unsigned long)(data + i); // transmit from "data"
        spi[i].rx_buf        = (unsigned long)(data + i) ; // receive into "data"
        spi[i].len           = sizeof(*(data + i)) ;
        spi[i].delay_usecs   = 0 ;
        spi[i].speed_hz      = this->speed ;
        spi[i].bits_per_word = this->bitsPerWord ;
        spi[i].cs_change = 0;
    }

    retVal = ioctl (this->spifd, SPI_IOC_MESSAGE(length), &spi) ;

    if(retVal < 0){
        std::cout << "Problem transmitting spi data..ioctl" << std::endl;
        return 1;
    }

    return 0;
}

int mcp3008Spi::readChannel(uint8_t channel)
{
    uint8_t data[3];

    data[0] = 1;  //  first byte transmitted -> start bit
    data[1] = 0b10000000 |( ((channel & 7) << 4)); // second byte transmitted -> (SGL/DIF = 1, D2=D1=D0=0)
    data[2] = 0; // third byte transmitted....don't care

#ifdef __arm__
    if(spiWriteRead(data, sizeof(data)))
    {
        std::cout << "Problem write/read spi data..." << std::endl;
        return 0;
    }
#else
    data[1] = rand() & 0x03;
    data[2] = rand();
#endif
    int a2dVal = 0;
    a2dVal = (data[1]<< 8) & 0b1100000000; //merge data[1] & data[2] to get result
    a2dVal |= (data[2] & 0xff);

    //cout << "The Result is: " << a2dVal << endl;

    return a2dVal;
}
/*************************************************
 * Default constructor. Set member variables to
 * default values and then call spiOpen()
 * ***********************************************/

mcp3008Spi::mcp3008Spi()
{
    this->mode = SPI_MODE_0 ;
    this->bitsPerWord = 8;
    this->speed = 1000000;
    this->spifd = -1;

#ifdef __arm__
    this->spiOpen(std::string("/dev/spidev0.0"));
#endif
}

/*************************************************
 * overloaded constructor. let user set member variables to
 * and then call spiOpen()
 * ***********************************************/
mcp3008Spi::mcp3008Spi(std::string devspi, unsigned char spiMode, unsigned int spiSpeed, unsigned char spibitsPerWord)
{
    this->mode = spiMode ;
    this->bitsPerWord = spibitsPerWord;
    this->speed = spiSpeed;
    this->spifd = -1;

#ifdef __arm__
    this->spiOpen(devspi);
#endif
}

/**********************************************
 * Destructor: calls spiClose()
 * ********************************************/
mcp3008Spi::~mcp3008Spi()
{
    this->spiClose();
}

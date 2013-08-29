
#include "BeagleBone_gpio.h"

#define MAXTIMINGS 100
#define DHT11 11
#define DHT22 22
#define AM2302 22

#define DEBUG 1

int readDHT(int type, int pin);

int main(int argc, char **argv)
{

  if (argc != 3) {
	printf("usage: %s [11|22|2302] GPIOpin#\n", argv[0]);
	printf("example: %s 2302 4 - Read from an AM2302 connected to GPIO #4\n", argv[0]);
	return 2;
  }
  int type = 0;
  if (strcmp(argv[1], "11") == 0) type = DHT11;
  if (strcmp(argv[1], "22") == 0) type = DHT22;
  if (strcmp(argv[1], "2302") == 0) type = AM2302;
  if (type == 0) {
	printf("Select 11, 22, 2302 as type!\n");
	return 3;
  }

  int dhtpin = atoi(argv[2]);

  if (dhtpin <= 0) {
	printf("Please select a valid GPIO pin #\n");
	return 3;
  }


  printf("Using pin #%d\n", dhtpin);
  readDHT(type, dhtpin);
  return 0;

} // main


int bits[250], data[100];
int bitidx = 0;

int readDHT(int type, int pin) {
  int counter = 0;
  //int laststate = HIGH;

  struct gpioID inPin;
  // Set GPIO pin to output
  pinMode(&inPin,pin,"out");
  //bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_OUTP);

  int laststate = 1;
  int j=0;







  digitalWrite(inPin,1);
  //bcm2835_gpio_write(pin, HIGH);
  usleep(500000);  // 500 ms
  digitalWrite(inPin,0);
  //bcm2835_gpio_write(pin, LOW);
  usleep(20000);

  //Set pin to input
  pinMode(&inPin,pin,"in");
  //bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_INPT);

  data[0] = data[1] = data[2] = data[3] = data[4] = 0;

  // wait for pin to drop?
  //while (bcm2835_gpio_lev(pin) == 1) {
  while (digitalRead(inPin) == 1) {
    usleep(1);
  }

  // read data!
  int i;
  for (i=0; i< MAXTIMINGS; i++) {
    counter = 0;
    //while ( bcm2835_gpio_lev(pin) == laststate) {
    while ( digitalRead(inPin) == laststate) {
	counter++;
	//nanosleep(1);		// overclocking might change this?
        if (counter == 1000)
	  break;
    }
    laststate = digitalRead(inPin); //bcm2835_gpio_lev(pin);
    if (counter == 1000) break;
    bits[bitidx++] = counter;

    if ((i>3) && (i%2 == 0)) {
      // shove each bit into the storage bytes
      data[j/8] <<= 1;
      if (counter > 200)
        data[j/8] |= 1;
      j++;
    }
  }


#ifdef DEBUG
  int k;
  for (k=3; k<bitidx; k+=2) {
    printf("bit %d: %d\n", k-3, bits[k]);
    printf("bit %d: %d (%d)\n", k-2, bits[k+1], bits[k+1] > 200);
  }
#endif

  printf("Data (%d): 0x%x 0x%x 0x%x 0x%x 0x%x\n", j, data[0], data[1], data[2], data[3], data[4]);

  if ((j >= 39) &&
      (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) ) {
     // yay!
     if (type == DHT11)
	printf("Temp = %d *C, Hum = %d \%\n", data[2], data[0]);
     if (type == DHT22) {
	float f, h;
	h = data[0] * 256 + data[1];
	h /= 10;

	f = (data[2] & 0x7F)* 256 + data[3];
        f /= 10.0;
        if (data[2] & 0x80)  f *= -1;
	printf("Temp =  %.1f *C, Hum = %.1f \%\n", f, h);
    }
    return 1;
  }

  return 0;
}

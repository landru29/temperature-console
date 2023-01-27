#ifndef __TEMPERATURE__H__
#define __TEMPERATURE__H__


#define TEMPERATURE_PIN 33   // on pin D33 (a pullup 4.7K resistor is necessary)


class Temperature {
  public:
    Temperature(byte* address);
    float value();

    static int count;
    static Temperature** sensor;
    static void searchSensors();
    static void sort(Temperature** sensor, int count);

  private:
    void start();
    byte typeS;
    byte addr[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    int compareAddr(Temperature *t);
};

#endif
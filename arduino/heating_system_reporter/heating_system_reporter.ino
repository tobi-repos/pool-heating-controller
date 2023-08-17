#include <RF24.h>

static const int HEATING_SYS_IN = 2;
static const int HEATING_SYS_OUT = 4;
static const int RF_CE = 9;
static const int RF_CSN = 10;

static RF24 radio(RF_CE, RF_CSN);

static const uint8_t radioId[5] = "POOL";
static uint8_t radioPayload[1] = {0};

static uint32_t timeIndex = 0;
static uint8_t heatingSystemState = 0;

void setup()
{
	pinMode(HEATING_SYS_IN, INPUT_PULLUP);
	pinMode(HEATING_SYS_OUT, OUTPUT);

	radio.begin();
	radio.setChannel(1);
	radio.setPALevel(RF24_PA_MAX);
	radio.setAddressWidth(4);
	radio.setPayloadSize(sizeof(radioPayload));
	radio.openWritingPipe(radioId);
	radio.stopListening();
	radio.powerDown();
}

void loop()
{
	uint32_t t = millis() >> 13;
	uint8_t s = digitalRead(HEATING_SYS_IN);
	digitalWrite(HEATING_SYS_OUT, s);

	s = !s;

	if (t != timeIndex || s != heatingSystemState)
	{
		*radioPayload = s;

		radio.powerUp();
		radio.write(radioPayload, sizeof(radioPayload));
		radio.powerDown();

		timeIndex = t;
		heatingSystemState = s;
	}
}

#include <RF24.h>

#define WITH_RADIO 0

static const int POOL_PUMP_IN = 2;
static const int SONOFF_IN = 3;
static const int SONOFF_OUT = 4;
static const int POOL_PUMP_OUT = 5;
static const int HEATING_SYS_OUT = 6;
static const int HEATING_SYS_ERR_OUT = 7;
static const int HEATING_VALVE_OUT = 8;

#if WITH_RADIO
static const int RF_CE = 9;
static const int RF_CSN = 10;

static RF24 radio(RF_CE, RF_CSN);

static const uint8_t radioId[5] = "POOL";
static uint8_t radioPayload[1] = {0};
static uint32_t lastRadioMessage = 0;
#endif

static bool poolPumpOk = false;
static bool sonoffOk = false;
static bool heatingSysOk = false;
static bool conditionsFulfilled = false;
static uint32_t conditionsFulfilledTime = 0;

void setup()
{
	pinMode(POOL_PUMP_IN, INPUT_PULLUP);
	pinMode(SONOFF_IN, INPUT_PULLUP);
	pinMode(SONOFF_OUT, OUTPUT);
	pinMode(POOL_PUMP_OUT, OUTPUT);
	pinMode(HEATING_SYS_OUT, OUTPUT);
	pinMode(HEATING_SYS_ERR_OUT, OUTPUT);
	pinMode(HEATING_VALVE_OUT, OUTPUT);

#if WITH_RADIO
	radio.begin();
	radio.setChannel(1);
	radio.setPALevel(RF24_PA_LOW);
	radio.setAddressWidth(4);
	radio.setPayloadSize(sizeof(radioPayload));
	radio.openReadingPipe(0, radioId);
	radio.startListening();
#else
    heatingSysOk = true;
#endif
}

void loop()
{
	uint32_t t = millis();

	if (!digitalRead(POOL_PUMP_IN))
	{
		digitalWrite(POOL_PUMP_OUT, HIGH);
		poolPumpOk = true;
	}
	else
	{
		digitalWrite(POOL_PUMP_OUT, LOW);
		poolPumpOk = false;
	}
	
	if (digitalRead(SONOFF_IN))
	{
		digitalWrite(SONOFF_OUT, HIGH);
		sonoffOk = true;
	}
	else
	{
		digitalWrite(SONOFF_OUT, LOW);
		sonoffOk = false;
	}
	
#if WITH_RADIO
	if (radio.available())
	{
		radio.read(radioPayload, sizeof(radioPayload));
		lastRadioMessage = t;

		digitalWrite(HEATING_SYS_ERR_OUT, LOW);

		if (*radioPayload)
		{
			digitalWrite(HEATING_SYS_OUT, HIGH);
			heatingSysOk = true;
		}
		else
		{
			digitalWrite(HEATING_SYS_OUT, LOW);
			heatingSysOk = false;
		}
	}
	else if(t - lastRadioMessage > 30000)
	{
		digitalWrite(HEATING_SYS_OUT, LOW);
		digitalWrite(HEATING_SYS_ERR_OUT, HIGH);
		heatingSysOk = false;
	}
#else
    digitalWrite(HEATING_SYS_ERR_OUT, LOW);
    digitalWrite(HEATING_SYS_OUT, HIGH);
#endif

	if (conditionsFulfilled)
	{
		if (poolPumpOk && sonoffOk && heatingSysOk)
		{
			if (t - conditionsFulfilledTime > 2000)
				digitalWrite(HEATING_VALVE_OUT, HIGH);
			else
				digitalWrite(HEATING_VALVE_OUT, LOW);
		}
		else
		{
			digitalWrite(HEATING_VALVE_OUT, LOW);
			conditionsFulfilled = false;
		}
	}
	else if (poolPumpOk && sonoffOk && heatingSysOk)
	{
		conditionsFulfilled = true;
		conditionsFulfilledTime = t;
	}
}

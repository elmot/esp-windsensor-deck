#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_FAST_COMPILE

#include "catch.hpp"
#include <cmath>
#include "nmea-wind-parser.hpp"

using Catch::Matchers::WithinAbs;

struct wind_state state;

int xTaskGetTickCount() { return 100; }

void testWind(const char *nmeaString, bool parseable, enum anem_state state, int angle, float speed);

void testCheckSumFail(const char *nmeaString);

void testAlarm(const char *nmeaString, bool parseable, bool expectedAlarm);

void resetData(bool expectedAlarm = false) {
    state.anemState = ANEMOMETER_CONN_FAIL;
    state.angleAlarm = expectedAlarm;
    state.timestamp = -100;
    state.windAngle = -1;
    state.windSpdMps = -1;
}

#define STATE_UNCHANGED (state.anemState == ANEMOMETER_CONN_FAIL && state.windAngle == -1 && state.windSpdMps == -1&& state.timestamp < 0)


TEST_CASE("Incorrect statements") {
    resetData();
    SECTION("True wind not supported") {
        REQUIRE_FALSE(parseNmea("$WIMWV,45.0,T,31.5,M,A*20"));
    }SECTION("Wrong sentence MWW") {
        REQUIRE_FALSE(parseNmea("$WIMWW,45.0,R,31.5,N,A*24"));
    }
    REQUIRE(STATE_UNCHANGED);
}

TEST_CASE("Broken checksum") {
    resetData();
    REQUIRE_FALSE(parseNmea("$WIMWV,45.0,R,31.5,M,A*2"));
    REQUIRE_FALSE(parseNmea("$WIMWV,45.0,R,31.5,M,A*"));
    REQUIRE_FALSE(parseNmea("$WIMWV,45.0,R,31.5,M,A"));
    REQUIRE_FALSE(parseNmea("$WIMWV,45.0,R,31.5,M,"));
    REQUIRE_FALSE(parseNmea("$WIMWV,45.0,R,31.5,M"));
    REQUIRE_FALSE(parseNmea("$WIMWV,45.0,R,31.5,"));
    REQUIRE_FALSE(parseNmea("$WIMWV,45.0,R,31.5"));
    REQUIRE_FALSE(parseNmea("$WIMWV,45.0,R,31."));
    REQUIRE_FALSE(parseNmea("$WIMWV,45.0,R,3"));
    REQUIRE_FALSE(parseNmea("$WIMWV,45.0,R,"));
    REQUIRE_FALSE(parseNmea("$WIMWV,45.0,R"));
    REQUIRE_FALSE(parseNmea("$WIMWV,45.0,"));
    REQUIRE_FALSE(parseNmea("$WIMWV,45.0"));
    REQUIRE_FALSE(parseNmea("$WIMWV,45."));
    REQUIRE_FALSE(parseNmea("$WIMWV,45"));
    REQUIRE_FALSE(parseNmea("$WIMWV,4"));
    REQUIRE_FALSE(parseNmea("$WIMWV,"));
    REQUIRE_FALSE(parseNmea("$WIMWV"));
    REQUIRE_FALSE(parseNmea("$WIMW"));
    REQUIRE_FALSE(parseNmea(nullptr));
    REQUIRE_FALSE(parseNmea("$WIMWV,54.0,R,31.5,M,A*28"));
    REQUIRE_FALSE(parseNmea("$PEWWT,NONE*66"));
    REQUIRE_FALSE(parseNmea("$PEWWT,NONE*6"));
    REQUIRE_FALSE(parseNmea("$PEWWT,NONE*"));
    REQUIRE_FALSE(parseNmea("$PEWWT,NONE"));
    REQUIRE_FALSE(parseNmea(""));

    REQUIRE(STATE_UNCHANGED);
}

TEST_CASE("Valid statements - [Wind measurement failed]") {
    resetData();
    REQUIRE(parseNmea("$WIMWV,,R,19.1,M,V*20"));
    REQUIRE(state.timestamp > 0);
    REQUIRE(state.anemState == ANEMOMETER_DATA_FAIL);
    REQUIRE(state.windAngle == -1);
    REQUIRE(state.windSpdMps == -1);
}

TEST_CASE("Valid statements", "[Wind statement]") {
    resetData();
    SECTION("Normal data") {
        SECTION("Metric data") {
            REQUIRE(parseNmea("$WIMWV,45.0,R,31.5,M,A*26"));
            REQUIRE(state.windAngle == 45);
            REQUIRE_THAT(state.windSpdMps, WithinAbs(31.5, 0.01));
        }

        SECTION("km/h data") {
            REQUIRE(parseNmea("$WIMWV,45.0,R,31.5,K,A*20"));
            REQUIRE(state.windAngle == 45);
            REQUIRE_THAT(state.windSpdMps, WithinAbs(8.75, 0.01));
        }SECTION("Nautical data") {
            REQUIRE(parseNmea("$WIMWV,45.0,R,31.5,N,A*25"));
            REQUIRE(state.windAngle == 45);
            REQUIRE_THAT(state.windSpdMps, WithinAbs(16.2, 0.01));
        }SECTION("Normal data #2") {
            REQUIRE(parseNmea("$WIMWV,47.0,R,19.1,M,A*2A"));
            REQUIRE(state.windAngle == 47);
            REQUIRE_THAT(state.windSpdMps, WithinAbs(19.1, 0.01));
        }
        REQUIRE(state.anemState == ANEMOMETER_OK);
        REQUIRE(state.timestamp > 0);
    }
}

TEST_CASE("Alarm statement") {
    resetData();
    SECTION("Too close to wind") {
        REQUIRE(parseNmea("$PEWWT,TOO_CLOSE_TO_WIND*3F"));
        REQUIRE(state.angleAlarm);
    }
    SECTION("Dead run") {
        REQUIRE(parseNmea("$PEWWT,TOO_CLOSE_TO_WIND*3F"));
        REQUIRE(state.angleAlarm);
    }
    SECTION("No alarm") {
        REQUIRE(parseNmea("$PEWWT,NONE*67"));
        REQUIRE_FALSE(state.angleAlarm);
    }
    REQUIRE(state.timestamp > 0);
}

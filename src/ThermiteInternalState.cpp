#include <AsyncJson.h>
#include <DallasTemperature.h>
#include <OneWire.h>

#include "ThermiteInternalState.h"

ThermiteInternalState::ThermiteInternalState(
  const ThermiteUserSettingsManager& userSettingsManager,
  DallasTemperature& thermometerManager,
  NTPClient& ntpClient,
  Timezone& timezone
) : _userSettingsManager(userSettingsManager),
    _thermometerManager(thermometerManager),
    _ntpClient(ntpClient),
    _timezone(timezone),
    _heater(false),
    _temp(DEVICE_DISCONNECTED_C),
    _tempLastRequestedAt(0ul),
    _tempTarget(DEVICE_DISCONNECTED_C),
    _tempHysteresis(1.0f) {}

void ThermiteInternalState::_updateHeater() {
  if (_tempTarget == DEVICE_DISCONNECTED_C) {
    /*
     * Wait until we have a valid target temperature; this should be called after
     * `_updateTargetTemperature()`.
     */
    return;
  }
  if (_temp <= _tempTarget - _tempHysteresis) {
    _heater = true;
  } else if (_temp >= _tempTarget + _tempHysteresis) {
    _heater = false;
  }
}

void ThermiteInternalState::_updateTargetTemperature() {
  time_t tUtc = _ntpClient.getEpochTime();
  time_t tLocal = _timezone.toLocal(tUtc);
  _tempTarget = _userSettingsManager.getTargetTemperature(tLocal);
}

void ThermiteInternalState::_updateTemperature(unsigned long updateAt) {
  if (updateAt < _tempLastRequestedAt) {
    /*
     * Underflow condition: `millis()` wraps around to zero every 50 days or so.  To
     * make sure the board can continue to function, we manually set `_tempLastRequestedAt`
     * to `now`; this will cause the next request interval to happen later than usual.
     */
    _tempLastRequestedAt = updateAt;
  } else if (_tempLastRequestedAt == 0ul
    || (updateAt - _tempLastRequestedAt > TEMP_REQUEST_INTERVAL)) {
    /*
     * We've either never requested a temperature (`lastRequestedAt == 0ul`), or the
     * request interval has elapsed.  Either way, request a new temperature reading.
     */
    _thermometerManager.requestTemperatures();
    _tempLastRequestedAt = updateAt;
  } else if (updateAt - _tempLastRequestedAt > TEMP_REQUEST_DELAY) {
    /*
     * To read the DS18B20, you have to wait at least `TEMP_REQUEST_DELAY` ms after
     * the previous temperature reading request.
     */
    float tempNew = _thermometerManager.getTempC(_thermometer);
    if (tempNew != DEVICE_DISCONNECTED_C) {
      _temp = tempNew;
    }
  }
}

bool ThermiteInternalState::init() {
  if (!_thermometerManager.getAddress(_thermometer, 0)) {
    return false;  
  }
  if (OneWire::crc8(_thermometer, 7) != _thermometer[7]) {
    return false;
  }

  _thermometerManager.setResolution(_thermometer, TEMP_RESOLUTION);
  _thermometerManager.setWaitForConversion(false);
  return true;
}

bool ThermiteInternalState::toJSON(const JsonObject& root) const {
  if (!root["dateTime"].set(_dateTimeIso)) {
    return false;
  }
  if (!root["heater"].set(_heater)) {
    return false;
  }
  if (!root["temp"].set(_temp)) {
    return false;
  }
  if (!root["tempTarget"].set(_tempTarget)) {
    return false;
  }
  return true;
}

void ThermiteInternalState::update(unsigned long updateAt) {
  _ntpClient.update();
  _updateTemperature(updateAt);
  _updateTargetTemperature();
  _updateHeater();
}

void ThermiteInternalState::updateDateTimeIso() {
  time_t tUtc = _ntpClient.getEpochTime();
  TimeChangeRule *tcr;
  time_t tLocal = _timezone.toLocal(tUtc, &tcr);

  snprintf(
    _dateTimeIso,
    DATE_TIME_ISO_LEN,
    "%04d-%02d-%02dT%02d:%02d:%02d%c%02d:%02d",
    year(tLocal),
    month(tLocal),
    day(tLocal),
    hour(tLocal),
    minute(tLocal),
    second(tLocal),
    tcr->offset >= 0 ? '+' : '-',
    abs(tcr->offset) / 60,
    abs(tcr->offset) % 60
  );
}
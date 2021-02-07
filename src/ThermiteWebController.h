#ifndef _THERMITE_WEB_CONTROLLER_H__
#define _THERMITE_WEB_CONTROLLER_H__

#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>

#include "JsonIO.h"
#include "ThermiteInternalState.h"
#include "ThermiteUserSettingsManager.h"

#define HTTP_OK 200
#define HTTP_BAD_REQUEST 400
#define HTTP_NOT_FOUND 404

/**
 * HTTP error message, containing a status code and a human-readable message.
 * 
 * We use this to help send HTTP error responses with JSON payloads, which helps
 * us handle error conditions in the frontend.
 */
struct HttpError : public JsonWrite {
  uint16_t _code;
  char _message[32];

  HttpError(uint16_t code, const char* message);
  uint16_t getCode() const;
  bool toJSON(const JsonObject& root) const;
};

class ThermiteWebController {
private:
  ThermiteUserSettingsManager& _userSettingsManager;
  ThermiteInternalState& _internalState;

  void _send(AsyncWebServerRequest* request, const JsonWrite& jsonWrite, size_t maxSize) const;
  void _sendError(AsyncWebServerRequest* request, const HttpError& error) const;
public:
  ThermiteWebController(
    ThermiteUserSettingsManager& scheduleManager,
    ThermiteInternalState& internalState
  );

  void getInternalState(AsyncWebServerRequest* request);
  void getUserSettings(AsyncWebServerRequest* request);

  void putUserSettings(AsyncWebServerRequest* request, const JsonVariant& json);

  void notFound(AsyncWebServerRequest* request);

  void initRoutes(AsyncWebServer& server);
};

#endif
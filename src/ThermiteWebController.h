#ifndef _THERMITE_WEB_CONTROLLER_H__
#define _THERMITE_WEB_CONTROLLER_H__

#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>

#include "Jsonable.h"
#include "ThermiteInternalState.h"
#include "ThermiteUserSettingsManager.h"

#define HTTP_OK 200
#define HTTP_BAD_REQUEST 400
#define HTTP_NOT_FOUND 404

/**
 * HTML entry point to web UI, returned by `getHome`.
 * 
 * It links to externally hosted `thermite` CSS / JS resources, which inject the `thermite` web
 * application into the `#app` div.
 */
const char index_html[] PROGMEM =
  "<!DOCTYPE HTML>"
  "<html><head>"
    "<meta charset=\"utf-8\">"
    "<link href=\"http://192.168.0.13:8000/main.css\" rel=\"stylesheet\">"
  "</head><body>"
    "<div id=\"app\">"
    "<script src=\"http://192.168.0.13:8000/main.js\"></script>"
  "</body></html>";

/**
 * HTTP error message, containing a status code and a human-readable message.
 * 
 * We use this to help send HTTP error responses with JSON payloads, which helps
 * us handle error conditions in the frontend.
 */
struct HttpError : Jsonable {
  uint16_t _code;
  const char* _message;

  HttpError(uint16_t code, const char* message);
  uint16_t getCode() const;
  bool toJSON(const JsonObject& root) const;
};

class ThermiteWebController {
private:
  ThermiteUserSettingsManager& _userSettingsManager;
  ThermiteInternalState& _internalState;

  void _send(AsyncWebServerRequest* request, const Jsonable& jsonable) const;
  void _sendError(AsyncWebServerRequest* request, const HttpError& error) const;
public:
  ThermiteWebController(
    ThermiteUserSettingsManager& scheduleManager,
    ThermiteInternalState& internalState
  );

  void getHome(AsyncWebServerRequest* request);
  void getInternalState(AsyncWebServerRequest* request);
  void getUserSettings(AsyncWebServerRequest* request);

  void notFound(AsyncWebServerRequest* request);

  void initRoutes(AsyncWebServer& server);
};

#endif
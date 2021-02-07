#include <AsyncJson.h>
#include <functional>

#include "Constants.h"
#include "ThermiteWebController.h"

HttpError::HttpError(uint16_t code, const char* message)
: _code(code) {
  strncpy(_message, message, 31);
  _message[31] = '\0';
}

bool HttpError::toJSON(const JsonObject& root) const {
  if (!root["code"].set(_code)) {
    return false;
  }
  if (!root["message"].set(_message)) {
    return false;
  }
  return true;
}

ThermiteWebController::ThermiteWebController(
  ThermiteUserSettingsManager& userSettingsManager,
  ThermiteInternalState& internalState
) : _userSettingsManager(userSettingsManager),
    _internalState(internalState) {}

void ThermiteWebController::_send(AsyncWebServerRequest* request, const JsonWrite& jsonWrite, size_t maxSize) const {
  AsyncJsonResponse* response = new AsyncJsonResponse(false, maxSize);
  const JsonObject& root = response->getRoot();
  jsonWrite.toJSON(root);
  response->setLength();
  request->send(response);
}

void ThermiteWebController::_sendError(AsyncWebServerRequest* request, const HttpError& httpError) const {
  AsyncJsonResponse* response = new AsyncJsonResponse(false, CAPACITY_HTTP_ERROR);
  response->setCode(httpError._code);
  const JsonObject& root = response->getRoot();
  httpError.toJSON(root);
  response->setLength();
  request->send(response);
}

void ThermiteWebController::getInternalState(AsyncWebServerRequest* request) {
  _internalState.updateDateTimeIso();
  _send(request, _internalState, CAPACITY_INTERNAL_STATE);
}

void ThermiteWebController::getUserSettings(AsyncWebServerRequest* request) {
  _send(request, _userSettingsManager, CAPACITY_USER_SETTINGS_MANAGER);
}

void ThermiteWebController::putUserSettings(AsyncWebServerRequest* request, const JsonVariant& json) {
  const JsonObject& root = json.as<JsonObject>();
  if (!_userSettingsManager.updateFromJSONSafe(root)) {
    HttpError error = { HTTP_BAD_REQUEST, "Invalid user settings" };
    _sendError(request, error);
  } else {
    _send(request, _userSettingsManager, CAPACITY_USER_SETTINGS_MANAGER);
  }
}

void ThermiteWebController::notFound(AsyncWebServerRequest* request) {
  if (request->method() == HTTP_OPTIONS) {
    request->send(204);
  } else {
    HttpError error = { HTTP_NOT_FOUND, "Not Found" };
    _sendError(request, error); 
  }
}

void ThermiteWebController::initRoutes(AsyncWebServer& server) {
  server.on(
    "/internalState",
    HTTP_GET,
    std::bind(&ThermiteWebController::getInternalState, this, std::placeholders::_1)
  );

  server.on(
    "/userSettings",
    HTTP_GET,
    std::bind(&ThermiteWebController::getUserSettings, this, std::placeholders::_1)
  );

  AsyncCallbackJsonWebHandler* handlerPutUserSettings = new AsyncCallbackJsonWebHandler(
    "/userSettings",
    std::bind(
      &ThermiteWebController::putUserSettings,
      this,
      std::placeholders::_1,
      std::placeholders::_2
    ),
    CAPACITY_USER_SETTINGS_MANAGER
  );
  handlerPutUserSettings->setMethod(HTTP_PUT);
  server.addHandler(handlerPutUserSettings);

  server.onNotFound(
    std::bind(&ThermiteWebController::notFound, this, std::placeholders::_1)
  );
}
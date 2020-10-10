#include <AsyncJson.h>
#include <functional>

#include "ThermiteWebController.h"

HttpError::HttpError(uint16_t code, const char* message)
: _code(code),
  _message(message) {}

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

void ThermiteWebController::_send(AsyncWebServerRequest* request, const Jsonable& jsonable) const {
  AsyncJsonResponse* response = new AsyncJsonResponse(false, 3072u);
  const JsonObject& root = response->getRoot();
  jsonable.toJSON(root);
  response->setLength();
  request->send(response);
}

void ThermiteWebController::_sendError(AsyncWebServerRequest* request, const HttpError& httpError) const {
  AsyncJsonResponse* response = new AsyncJsonResponse();
  response->setCode(httpError._code);
  const JsonObject& root = response->getRoot();
  httpError.toJSON(root);
  response->setLength();
  request->send(response);
}

void ThermiteWebController::getInternalState(AsyncWebServerRequest* request) {
  _internalState.updateDateTimeIso();
  _send(request, _internalState);
}

void ThermiteWebController::getUserSettings(AsyncWebServerRequest* request) {
  _send(request, _userSettingsManager);
}

void ThermiteWebController::putUserSettings(AsyncWebServerRequest* request, const JsonVariant& json) {
  const JsonObject& root = json.as<JsonObject>();
  if (!_userSettingsManager.updateFromJSONSafe(root)) {
    HttpError error = { HTTP_BAD_REQUEST, "Invalid user settings" };
    _sendError(request, error);
  } else {
    _send(request, _userSettingsManager);
  }
}

void ThermiteWebController::notFound(AsyncWebServerRequest* request) {
  HttpError error = { HTTP_NOT_FOUND, "Not Found" };
  _sendError(request, error);
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
    )
  );
  handlerPutUserSettings->setMethod(HTTP_PUT);
  server.addHandler(handlerPutUserSettings);

  server.onNotFound(
    std::bind(&ThermiteWebController::notFound, this, std::placeholders::_1)
  );
}
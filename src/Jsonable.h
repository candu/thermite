#ifndef _JSONABLE_H__
#define _JSONABLE_H__

#include <ArduinoJson.h>

struct Jsonable {
  virtual bool toJSON(const JsonObject& root) const = 0;
  virtual bool validateJSON(const JsonObject& root) const = 0;
  virtual void updateFromJSON(const JsonObject& root) = 0;

  bool updateFromJSONSafe(const JsonObject& root) {
    if (!validateJSON(root)) {
      return false;
    }
    updateFromJSON(root);
    return true;
  }
};

#endif
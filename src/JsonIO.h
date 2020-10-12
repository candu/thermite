#ifndef _JSON_IO_H__
#define _JSON_IO_H__

#include <ArduinoJson.h>

struct JsonRead {
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

struct JsonWrite {
  virtual bool toJSON(const JsonObject& root) const = 0;
};

#endif
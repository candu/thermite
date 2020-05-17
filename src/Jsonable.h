#ifndef _JSONABLE_H__
#define _JSONABLE_H__

#include <ArduinoJson.h>

struct Jsonable {
  virtual bool toJSON(const JsonObject& root) const = 0;
};

#endif
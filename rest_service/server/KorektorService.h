// Copyright 2013 by Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.

#ifndef KOREKTOR_SERVICE_H
#define KOREKTOR_SERVICE_H

#include "StdAfx.h"

#include "JsonBuilder.h"
#include "RestService.h"

namespace ngramchecker {

struct SpellcheckerDescription {
  string id, file;

  SpellcheckerDescription(string id, string file) : id(id), file(file) {}
};

class KorektorService : public RestService {
 public:
  void init(const vector<SpellcheckerDescription>& spellchecker_descriptions);

  virtual bool handle(RestRequest& req) override;

 private:
  bool handle_strip(RestRequest& req);


  const char* get_data(RestRequest& req, JsonBuilder& error);
  const char* get_model(RestRequest& req, JsonBuilder& error);
  unsigned get_suggestions(RestRequest& req, JsonBuilder& error);
  static bool get_line(const char*& data, StringPiece& line);

  class Spellchecker {
   public:
    virtual void suggestions(const string& str, unsigned num, vector<pair<string, vector<string>>>& suggestions) = 0;
  };
  class SpellcheckerProvider {
   public:
    virtual Spellchecker* new_provider() const = 0;
  };
  class KorektorProvider;
  class StripDiacriticsProvider;

  unordered_map<string, unique_ptr<SpellcheckerProvider>> spellcheckers;
  string default_spellchecker;
  JsonBuilder json_spellcheckers;
};

} // namespace ngramchecker

#endif

// Copyright 2013 by Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.

#include <unicode/normlzr.h>

#include "StdAfx.h"
#include "Spellchecker.hpp"
#include "Tokenizer.hpp"

#include "KorektorService.h"
#include "ResponseGenerator.h"

namespace ngramchecker {

// Custom ResponseGenerator using JsonBuilder as data storage.
// Pointer to input data is used and additional logic for generate
// is implemented
class KorektorResponseGenerator : public ResponseGenerator {
 public:
  KorektorResponseGenerator(const char* data);

  virtual bool next() = 0;

  virtual StringPiece current() override;
  virtual void consume(size_t length) override;
  virtual bool generate() override;

 protected:
  const char* data;
  JsonBuilder result;
};

KorektorResponseGenerator::KorektorResponseGenerator(const char* data) : data(data) {
  result.object().key("result").array();
}

StringPiece KorektorResponseGenerator::current() {
  return result.current();
}

void KorektorResponseGenerator::consume(size_t length) {
  result.discard_prefix(length);
}

bool KorektorResponseGenerator::generate() {
  if (!data) return false;

  if (!next()) {
    result.close_all();
    data = nullptr;
  }
  return true;
}

// Init the MorphoDiTa service -- load the models
bool KorektorService::init(const vector<SpellcheckerDescription>& spellchecker_descriptions) {
  // Load spellcheckers
  spellcheckers.clear();
  for (auto& spellchecker_description : spellchecker_descriptions)
    spellcheckers.emplace("/" + spellchecker_description.id, unique_ptr<Configuration>(new Configuration(string(), spellchecker_description.file)));

  return true;
}

// Handle a request using the specified URL/handler map
bool KorektorService::handle(RestRequest& req) {
  if (req.url == "/strip_diacritics") return handle_strip(req);

  auto spellchecker_it = spellcheckers.find(req.url);
  if (spellchecker_it == spellcheckers.end()) return req.respond_not_found();

  JsonBuilder error;
  auto data = get_data(req, error); if (!data) return req.respond_json(error);

  class generator : public KorektorResponseGenerator {
   public:
    generator(const char* data, Configuration* configuration) : KorektorResponseGenerator(data), spellchecker(configuration) {}

    bool next() {
      StringPiece line;
      if (!get_line(data, line)) return false;

      spellchecker.GetSuggestions(string(line.str, line.len), 5, suggestions);
      for (auto&& suggestion : suggestions) {
        result.array().value(suggestion.first);
        for (auto&& word : suggestion.second)
          result.value(word);
        result.close();
      }

      return true;
    }

   private:
    Spellchecker spellchecker;
    vector<pair<string, vector<string>>> suggestions;
  };
  return req.respond_json(new generator(data, spellchecker_it->second.get()));
}

bool KorektorService::handle_strip(RestRequest& req) {
  JsonBuilder error, output;
  auto data = get_data(req, error); if (!data) return req.respond_json(error);

  class generator : public KorektorResponseGenerator {
   public:
    generator(const char* data) : KorektorResponseGenerator(data) {}

    bool next() {
      StringPiece line;
      if (!get_line(data, line))
        return false;

      u16string text = MyUtils::utf8_to_utf16(string(line.str, line.len));
      vector<vector<TokenP>> sentences = tokenizer.Tokenize(text);
      unsigned text_index = 0;
      for (auto&& sentence : sentences)
        for (auto&& token : sentence) {
          // Strip diacritics from token
          word.setTo((const UChar*) token->str_u16.c_str(), token->str_u16.size());
          UErrorCode status = U_ZERO_ERROR;
          icu::Normalizer::normalize(word, UNORM_NFD, 0, word_decomposed, status);

          word_no_marks.remove();
          bool changed = false;
          for (unsigned i = 0; i < word_decomposed.length(); i = word_decomposed.moveIndex32(i, 1)) {
            UChar32 chr = word_decomposed.char32At(i);
            if (u_charType(chr) == U_NON_SPACING_MARK ) changed = true;
            else word_no_marks.append(chr);
          }
          if (!changed) continue;

          status = U_ZERO_ERROR;
          icu::Normalizer::normalize(word_no_marks, UNORM_NFC, 0, word_stripped, status);

          if (text_index < token->first) result.array().value(MyUtils::utf16_to_utf8(text.substr(text_index, token->first - text_index))).close();
          word_stripped_utf8.clear();
          result.array().value(token->str_utf8).value(word_stripped.toUTF8String(word_stripped_utf8)).close();

          text_index = token->first + token->length;
        }
      if (text_index < text.size()) result.array().value(MyUtils::utf16_to_utf8(text.substr(text_index))).close();

      return true;
    }
   private:
    Tokenizer tokenizer;
    UnicodeString word, word_decomposed, word_no_marks, word_stripped;
    string word_stripped_utf8;
  };
  return req.respond_json(new generator(data));
}

const char* KorektorService::get_data(RestRequest& req, JsonBuilder& error) {
  auto data_it = req.params.find("data");
  if (data_it == req.params.end()) return error.clear().object().key("error").value("Required argument 'data' is missing."), nullptr;

  return data_it->second.c_str();
}

bool KorektorService::get_line(const char*& data, StringPiece& line) {
  line.str = data;
  while (*data && *data != '\r' && *data != '\n') data++;
  line.len = data - line.str;

  // If end of data was reached
  if (!*data) return line.len;

  // Otherwise, include the line ending
  if (*data == '\r') {
    data++;
    if (*data == '\n') data++;
  } else if (*data == '\n') {
    data++;
    if (*data == '\r') data++;
  }
  line.len = data - line.str;
  return true;
}

} // namespace ngramchecker
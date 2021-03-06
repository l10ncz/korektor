// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

#include <cstdlib>

#include "parse.h"

namespace ufal {
namespace korektor {

int Parse::Int(const string& str, const char* value_name) {
  char* end;

  errno = 0;
  long result = strtol(str.c_str(), &end, 10);
  if (*end || errno == ERANGE || result != int(result))
    runtime_failure("Cannot parse int value " << value_name << ": '" << str << "'!");

  return int(result);
}

double Parse::Double(const string& str, const char* value_name) {
  char* end;

  errno = 0;
  double result = strtod(str.c_str(), &end);
  if (*end || errno == ERANGE)
    runtime_failure("Cannot parse double value " << value_name << ": '" << str << "'!");

  return result;
}

} // namespace korektor
} // namespace ufal

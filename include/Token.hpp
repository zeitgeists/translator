#pragma once
#include <string>

enum TokenId {
  tok_eof = -1,

  // commands
  tok_def = -2,
  tok_extern = -3,

  // primary
  tok_identifier = -4,
  tok_number = -5,

  tok_operator_1 = -6,
  tok_operator_2 = -7,
};
namespace Token {

}
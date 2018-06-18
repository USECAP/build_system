#include <string>

static std::string SHARED_LIB_PATTERN =
  "^([^-]*-)*[mg]cc(-\\d+(\\.\\d+){0,2})?\\b.*\\s-(s|rdynamic|shared)\\b.*$|"
  "^([^-]*-)*clang(-\\d+(\\.\\d+){0,2})?\\b.*\\s(-shared)\\b.*$";
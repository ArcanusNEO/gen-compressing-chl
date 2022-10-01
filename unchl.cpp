#include <bits/stdc++.h>
using namespace std;

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__) \
  || defined(__WINDOWS_)
const char opt_head = '/';
#else
const char opt_head = '-';
#endif

// -o -t -vvv -s
string   input_path = "output", output_file = "-";
unsigned thread_number = max(thread::hardware_concurrency(), 1U);

enum log_level_t { LOG_SILENT, LOG_ERROR, LOG_WARNING, LOG_INFO, LOG_DEBUG };
unsigned log_level = LOG_SILENT;

enum cmd_opt_t {
  CMD_OPTION_NULL,
  CMD_OPTION_INPUT_PATH,
  CMD_OPTION_OUTPUT_PATH,
  CMD_OPTION_THREAD_NUMBER,
  CMD_OPTION_LOG_LEVEL,
  CMD_OPTION_LOG_LEVEL_SILENT
};

namespace fs = std::filesystem;

void parse_opt(char* opt_token) {
  if (opt_token == nullptr) return;
  static enum cmd_opt_t opt_type = CMD_OPTION_NULL;
  if (opt_token[0] == opt_head) switch (opt_token[1]) {
      case 'o':
        opt_type = CMD_OPTION_OUTPUT_PATH;
        opt_token += 2;
        break;
      case 't':
        opt_type = CMD_OPTION_THREAD_NUMBER;
        opt_token += 2;
        break;
      case 'v':
        opt_type  = CMD_OPTION_LOG_LEVEL;
        log_level = 1;
        for (opt_token += 2; *opt_token == 'v'; ++opt_token)
          log_level = min(log_level + 1, (unsigned) LOG_DEBUG);
        break;
      case 's':
        opt_type = CMD_OPTION_LOG_LEVEL_SILENT;
        opt_token += 2;
        log_level = 0;
        break;
      default: break;
    }
  if (opt_type == CMD_OPTION_NULL) opt_type = CMD_OPTION_INPUT_PATH;
  switch (opt_type) {
    case CMD_OPTION_INPUT_PATH:
      input_path = opt_token;
      opt_type   = CMD_OPTION_NULL;
      break;
    case CMD_OPTION_OUTPUT_PATH:
      output_file = opt_token;
      opt_type    = CMD_OPTION_NULL;
      break;
    case CMD_OPTION_THREAD_NUMBER: {
      auto i        = atoi(opt_token);
      thread_number = i > 0 ? i : thread_number;
      opt_type      = CMD_OPTION_NULL;
      break;
    }
    case CMD_OPTION_LOG_LEVEL: opt_type = CMD_OPTION_NULL; break;
    case CMD_OPTION_LOG_LEVEL_SILENT: opt_type = CMD_OPTION_NULL; break;
    default: break;
  }
}

signed main(int argc, char* argv[]) {
  for (int i = 1; i < argc; ++i) parse_opt(argv[i]);
  thread_number = max(thread_number, 1U);
  if (log_level >= LOG_INFO)
    cerr << "[info] begin unCHL..." << endl
         << "[info] input_path " << fs::path(input_path) << endl
         << "[info] output_file " << fs::path(output_file) << endl
         << "[info] thread_number " << thread_number << endl
         << "[info] log_level " << log_level << endl;
}
#include <bits/stdc++.h>
using namespace std;

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__) \
  || defined(__WINDOWS_)
const char opt_head = '/';
#else
const char opt_head = '-';
#endif

// -o -t -vvvv -s
string   meta_file = "-", output_file = "-";
unsigned thread_number = max(thread::hardware_concurrency(), 1U);

enum log_level_t { LOG_SILENT, LOG_ERROR, LOG_WARNING, LOG_INFO, LOG_DEBUG };
unsigned log_level = LOG_SILENT;

enum cmd_opt_t {
  CMD_OPTION_NULL,
  CMD_OPTION_OUTPUT_FILE,
  CMD_OPTION_META_FILE,
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
        opt_type = CMD_OPTION_OUTPUT_FILE;
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
  if (opt_type == CMD_OPTION_NULL) opt_type = CMD_OPTION_META_FILE;
  switch (opt_type) {
    case CMD_OPTION_META_FILE:
      meta_file = opt_token;
      opt_type  = CMD_OPTION_NULL;
      break;
    case CMD_OPTION_OUTPUT_FILE:
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

uint8_t    base_ai_map[256];
const char base_ia_map[4] = {'A', 'C', 'G', 'T'};

#ifndef MX_READ_LIST_SZ
#  define MX_READ_LIST_SZ 0x10000000
#endif

uint64_t read_v[MX_READ_LIST_SZ][8];
uint32_t read_counter;

void init() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  base_ai_map['a'] = base_ai_map['A'] = 0x00;
  base_ai_map['t'] = base_ai_map['T'] = 0x03;
  base_ai_map['c'] = base_ai_map['C'] = 0x01;
  base_ai_map['g'] = base_ai_map['G'] = 0x02;
}

signed main(int argc, char* argv[]) {
  init();
  for (int i = 1; i < argc; ++i) parse_opt(argv[i]);
  thread_number = max(thread_number, 1U);
  if (log_level >= LOG_INFO)
    cerr << "[info] begin unCHL..." << endl
         << "[info] meta_file " << fs::path(meta_file) << endl
         << "[info] output_file " << fs::path(output_file) << endl
         << "[info] thread_number " << thread_number << endl
         << "[info] log_level " << log_level << endl;
  ifstream   ifs;
  streambuf* cin_buf = nullptr;
  if (meta_file != "-") {
    ifs.open(meta_file);
    cin_buf = cin.rdbuf(ifs.rdbuf());
  }
  list<fs::path> idp_file_ls;
  fs::path       ido_file;
  uint32_t       exp_read_len;
  string         tag;
  while (getline(cin, tag)) {
    if (tag.find("ido") != string::npos) {
      string f;
      getline(cin, f);
      ido_file = f;
    } else if (tag.find("idp") != string::npos) {
      string f;
      getline(cin, f);
      idp_file_ls.push_back(f);
    } else if (tag.find("read") != string::npos) cin >> exp_read_len;
  }
  if (cin_buf) cin.rdbuf(cin_buf);
}
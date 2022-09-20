#include <bits/stdc++.h>
#include <omp.h>
using namespace std;

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__) \
  || defined(__WINDOWS_)
const char opt_head = '/';
#else
const char opt_head = '-';
#endif

// -o -p -t -vvv
string   input_path = "-", output_path = "out", prime_path = "prime";
unsigned thread_number = thread::hardware_concurrency();
unsigned log_level     = 0;

enum log_level_t { LOG_SILENT, LOG_ERROR, LOG_WARNING, LOG_INFO, LOG_DEBUG };

enum cmd_opt_t {
  CMD_OPTION_NULL,
  CMD_OPTION_INPUT_PATH,
  CMD_OPTION_OUTPUT_PATH,
  CMD_OPTION_PRIME_PATH,
  CMD_OPTION_THREAD_NUMBER,
  CMD_OPTION_LOG_LEVEL
};

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
      case 'p':
        opt_type = CMD_OPTION_PRIME_PATH;
        opt_token += 2;
        break;
      case 'v':
        opt_type  = CMD_OPTION_LOG_LEVEL;
        log_level = 1;
        for (opt_token += 2; *opt_token == 'v'; ++opt_token) ++log_level;
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
      output_path = opt_token;
      opt_type    = CMD_OPTION_NULL;
      break;
    case CMD_OPTION_PRIME_PATH:
      prime_path = opt_token;
      opt_type   = CMD_OPTION_NULL;
      break;
    case CMD_OPTION_THREAD_NUMBER: {
      auto i        = atoi(opt_token);
      thread_number = i > 0 ? i : thread_number;
      opt_type      = CMD_OPTION_NULL;
      break;
    }
    case CMD_OPTION_LOG_LEVEL: opt_type = CMD_OPTION_NULL; break;
    default: break;
  }
}

namespace fs = std::filesystem;

uint8_t    base_ai_map[256];
const char base_ia_map[4] = {'A', 'C', 'G', 'T'};

#ifndef MAX_READ_SZ
#  define MAX_READ_SZ 0xf000000
#endif

uint64_t read_v[MAX_READ_SZ][4];
size_t   read_counter;

size_t hash_table_sz;

void get_hash_sz() {
}

unsigned read_len;

void read_sequence() {
  string base_seq_str;
  while (cin >> base_seq_str) {
    auto&    base_seq = read_v[read_counter++];
    unsigned i        = 0;
    for (const auto& ch : base_seq_str) {
      auto j = base_ai_map[ch];
      base_seq[i >> 5] |= j << ((i & 0x1f) << 1);
      ++i;
    }
    read_len = i;
  }
}

void init() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  base_ai_map['a'] = base_ai_map['A'] = 0x00;
  base_ai_map['t'] = base_ai_map['T'] = 0x03;
  base_ai_map['c'] = base_ai_map['C'] = 0x01;
  base_ai_map['g'] = base_ai_map['G'] = 0x02;
}

signed main(int argc, char* argv[]) {
  for (int i = 1; i < argc; ++i) parse_opt(argv[i]);
  thread_number = max(thread_number, 1U);
  if (log_level >= LOG_INFO)
    cerr << input_path << endl
         << output_path << endl
         << prime_path << endl
         << thread_number << endl
         << log_level << endl;
  fs::remove(output_path);
  fs::create_directory(output_path);
  ifstream   ifs;
  streambuf* cin_buf = nullptr;
  if (input_path != "-") {
    ifs.open(input_path);
    cin_buf = cin.rdbuf(ifs.rdbuf());
  }
  read_sequence();
  if (cin_buf) cin.rdbuf(cin_buf);
}
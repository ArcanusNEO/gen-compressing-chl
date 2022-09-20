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
string   input_file = "-", output_path = "out", prime_path = "prime";
unsigned thread_number = thread::hardware_concurrency();

enum log_level_t { LOG_SILENT, LOG_ERROR, LOG_WARNING, LOG_INFO, LOG_DEBUG };
unsigned log_level = LOG_SILENT;

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
      input_file = opt_token;
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

#ifndef MX_READ_LIST_SZ
#  define MX_READ_LIST_SZ 0x0f000000
#endif

uint64_t read_v[MX_READ_LIST_SZ][4];
size_t   read_counter;

struct chl_key_t {
  uint32_t key_id  = 0;
  uint16_t key_pos = 0;
};

using list_chl_key_t = list<chl_key_t>;

#ifndef MX_HASH_TABLE_SZ
#  define MX_HASH_TABLE_SZ 0x0f000000
#endif
list<list_chl_key_t> hash_table[MX_HASH_TABLE_SZ];

size_t   hash_table_sz;
unsigned read_len;

void get_hash_sz() {
  unsigned min_hash_tab_sz = 1.5 * read_len;
  fs::path p_file(prime_path);
  switch (min_hash_tab_sz) {
    case 2 ... 15485863: p_file /= "2_15485863.txt"; break;
    case 15485864 ... 32452843: p_file /= "15485867_32452843.txt"; break;
    case 32452844 ... 49979687: p_file /= "32452867_49979687.txt"; break;
    case 49979688 ... 67867967: p_file /= "49979693_67867967.txt"; break;
    case 67867968 ... 86028121: p_file /= "67867979_86028121.txt"; break;
    case 86028122 ... 104395301: p_file /= "86028157_104395301.txt"; break;
    case 104395302 ... 122949823: p_file /= "104395303_122949823.txt"; break;
    case 122949824 ... 141650939: p_file /= "122949829_141650939.txt"; break;
    case 141650963 ... 160481183: p_file /= "141650963_160481183.txt"; break;
    case 160481184 ... 179424673: p_file /= "160481219_179424673.txt"; break;
    case 179424674 ... 198491317: p_file /= "179424691_198491317.txt"; break;
    case 198491318 ... 217645177: p_file /= "198491329_217645177.txt"; break;
    case 217645178 ... 236887691: p_file /= "217645199_236887691.txt"; break;
    case 236887692 ... 256203161: p_file /= "236887699_256203161.txt"; break;
    case 256203162 ... 275604541: p_file /= "256203221_275604541.txt"; break;
    case 275604542 ... 295075147: p_file /= "275604547_295075147.txt"; break;
    case 295075148 ... 314606869: p_file /= "295075153_314606869.txt"; break;
    default:
      hash_table_sz = 314606869;
      if (log_level >= LOG_WARNING)
        cerr << "[warning] max prime number (" << hash_table_sz << ") reached"
             << endl;
      break;
  }
  if (log_level >= LOG_INFO)
    cerr << "[info] select prime number file " << p_file << endl;
  ifstream   ifs(p_file);
  streambuf* cin_buf = cin.rdbuf(ifs.rdbuf());
  string     prime_str;
  while (getline(cin, prime_str, ',')) {
    hash_table_sz = stoul(prime_str);
    if (hash_table_sz >= min_hash_tab_sz) break;
  }
  cin.rdbuf(cin_buf);
  if (log_level >= LOG_INFO)
    cerr << "[info] select prime number " << hash_table_sz
         << " for hash table size" << endl;
}

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
  if (log_level >= LOG_INFO) cerr << "[info] reads count " << read_len << endl;
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
    cerr << "[info] begin CHL..." << endl
         << "[info] input_file " << input_file << endl
         << "[info] output_path " << output_path << endl
         << "[info] prime_path " << prime_path << endl
         << "[info] thread_number " << thread_number << endl
         << "[info] log_level " << log_level << endl;
  fs::remove(output_path);
  fs::create_directory(output_path);
  ifstream   ifs;
  streambuf* cin_buf = nullptr;
  if (input_file != "-") {
    ifs.open(input_file);
    cin_buf = cin.rdbuf(ifs.rdbuf());
  }
  read_sequence();
  if (cin_buf) cin.rdbuf(cin_buf);
  get_hash_sz();
}
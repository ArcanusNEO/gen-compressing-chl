#include <bits/stdc++.h>
#include <omp.h>
using namespace std;

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__) \
  || defined(__WINDOWS_)
const char opt_head = '/';
#else
const char opt_head = '-';
#endif

// -o -p -t -vvv -s
string   input_file = "-", output_path = "out", prime_path = "prime";
unsigned thread_number = max(thread::hardware_concurrency(), 1U);

enum log_level_t { LOG_SILENT, LOG_ERROR, LOG_WARNING, LOG_INFO, LOG_DEBUG };
unsigned log_level = LOG_SILENT;

enum cmd_opt_t {
  CMD_OPTION_NULL,
  CMD_OPTION_INPUT_PATH,
  CMD_OPTION_OUTPUT_PATH,
  CMD_OPTION_PRIME_PATH,
  CMD_OPTION_THREAD_NUMBER,
  CMD_OPTION_LOG_LEVEL,
  CMD_OPTION_LOG_LEVEL_SILENT
};

namespace fs = std::filesystem;

uint8_t    base_ai_map[256];
const char base_ia_map[4] = {'A', 'C', 'G', 'T'};

#ifndef MX_READ_LIST_SZ
#  define MX_READ_LIST_SZ 0x10000000
#endif

#ifndef READ_LENGTH
#  define READ_LENGTH 150
#  define BITSET_SZ   (READ_LENGTH * 2)
#endif

using read_t = bitset<BITSET_SZ>;
read_t   read_v[MX_READ_LIST_SZ];
uint32_t read_counter;

struct chl_key_t {
  uint32_t id;
  uint16_t pos;
};
using list_chl_key_t = list<chl_key_t>;

#ifndef MX_HASH_TABLE_SZ
#  define MX_HASH_TABLE_SZ 0x20000000
#endif

list<list_chl_key_t> hash_table[MX_HASH_TABLE_SZ];

uint32_t hash_table_sz;

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
    case CMD_OPTION_LOG_LEVEL_SILENT: opt_type = CMD_OPTION_NULL; break;
    default: break;
  }
}

void init_hash() {
  uint32_t min_hash_tab_sz = 1.5 * read_counter;
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
      return;
  }
  if (log_level >= LOG_INFO)
    cerr << "[info] select prime number file " << fs::path(p_file) << endl;
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
    uint32_t i        = 0;
    for (const auto& ch : base_seq_str) {
      auto j               = base_ai_map[ch];
      base_seq[i << 1]     = j & 0x02;
      base_seq[i << 1 | 1] = j & 0x01;
      ++i;
    }
  }
  if (log_level >= LOG_INFO)
    cerr << "[info] reads count " << read_counter << endl;
}

void init() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  base_ai_map['a'] = base_ai_map['A'] = 0x00;
  base_ai_map['t'] = base_ai_map['T'] = 0x03;
  base_ai_map['c'] = base_ai_map['C'] = 0x01;
  base_ai_map['g'] = base_ai_map['G'] = 0x02;
}

inline void cycle_shift_right(read_t& r) {
  bool b[2] = {r[1], r[0]};
  r >>= 2;
  r.set(BITSET_SZ - 1, b[1]);
  r.set(BITSET_SZ - 2, b[0]);
}

inline void swap(read_t::reference x, read_t::reference y) noexcept {
  bool t = x;
  x      = y;
  y      = t;
}

inline void reverse_read(read_t& r) {
  for (size_t i = 0; i < (BITSET_SZ >> 1); i += 2) {
    swap(r[i], r[BITSET_SZ - 2 - i]);
    swap(r[i + 1], r[BITSET_SZ - i]);
  }
}

uint32_t get_hash(const read_t& r, uint32_t modulo) {
  int      res = BITSET_SZ;
  uint64_t rem = 0;
  while (res > 0) {
    int rsz;
    if (res >= 32) {
      rsz = 32;
      res -= 32;
    } else {
      rsz = res;
      res = 0;
    }
    read_t t = r >> res;
    t &= 0xffffffff;
    uint32_t x = t.to_ulong();
    rem        = (rem << rsz) | x;
    rem %= modulo;
  }
  return rem;
}

template <typename T, typename U>
typename std::enable_if_t<
  std::is_integral_v<T> && std::is_integral_v<U> && sizeof(T) >= sizeof(U), T>
fast_pow(T __base, unsigned long long __exponent, U __modulo) {
  T ans = 1;
  __base %= __modulo;
  while (__exponent) {
    if (__exponent & 1) {
      ans *= __base;
      ans %= __modulo;
    }
    __base *= __base;
    __base %= __modulo;
    __exponent >>= 1;
  }
  return ans;
}

bool hash_collision(const chl_key_t& chl_ref, const chl_key_t& chl_new) {
  return true;
}

// return 1 for fail
// return 0 for success
int rolling_hash_try_insert(uint32_t id, uint32_t hval, int state) {
  const static uint32_t p   = fast_pow(4ULL, READ_LENGTH, hash_table_sz);
  const static uint32_t p_r = fast_pow(4ULL, hash_table_sz - 2, hash_table_sz);
  const read_t&         r   = read_v[id];
  for (uint32_t i = 0; i < READ_LENGTH; ++i) {
    uint32_t cur = (uint32_t) r[i + 1] << 1 | (uint32_t) r[i];
    hval         = (hval + (uint64_t) cur * p) % hash_table_sz;
    hval         = ((uint64_t) hval * p_r) % hash_table_sz;
    if (!hash_table[hval].empty()) {
      chl_key_t new_key = {.id  = id,
                           .pos = state * READ_LENGTH + (i + 1) % READ_LENGTH};
      for (auto& ls : hash_table[hval]) {
        const auto& chl_ref = ls.front();
        if (!hash_collision(chl_ref, new_key)) {
          ls.push_back(new_key);
          return 0;
        }
      }
    }
  }
  return 1;
}

void chl() {
  for (uint32_t id = 0; id < read_counter; ++id) {
    read_t   read_seq = read_v[id];
    uint32_t hval     = get_hash(read_seq, hash_table_sz);
    // 0 源序列循环状态
    if (!rolling_hash_try_insert(id, hval, 0)) continue;

    read_seq.flip();
    uint32_t fhval = get_hash(read_seq, hash_table_sz);
    // 1 补序列循环状态
    if (!rolling_hash_try_insert(id, fhval, 1)) continue;
    read_seq.flip();

    reverse_read(read_seq);
    uint32_t rhval = get_hash(read_seq, hash_table_sz);
    // 2 逆序列循环状态
    if (!rolling_hash_try_insert(id, rhval, 2)) continue;

    read_seq.flip();
    uint32_t rfhval = get_hash(read_seq, hash_table_sz);
    // 3 逆补序列循环状态
    if (!rolling_hash_try_insert(id, rfhval, 3)) continue;

    list_chl_key_t ls;
    ls.push_back((chl_key_t){.id = id, .pos = 0});
    hash_table[hval].push_back(ls);
  }
}

signed main(int argc, char* argv[]) {
  for (int i = 1; i < argc; ++i) parse_opt(argv[i]);
  thread_number = max(thread_number, 1U);
  if (log_level >= LOG_INFO)
    cerr << "[info] begin CHL..." << endl
         << "[info] input_file " << fs::path(input_file) << endl
         << "[info] output_path " << fs::path(output_path) << endl
         << "[info] prime_path " << fs::path(prime_path) << endl
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
  init_hash();
  chl();
}
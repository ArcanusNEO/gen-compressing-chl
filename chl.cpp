#include <bits/stdc++.h>
#include <omp.h>
using namespace std;

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__) \
  || defined(__WINDOWS_)
const char opt_head = '/';
#else
const char opt_head = '-';
#endif

// -o -t -vvvv -s
string   input_file = "-", output_path = "output";
unsigned thread_number = max(thread::hardware_concurrency(), 1U);

enum log_level_t { LOG_SILENT, LOG_ERROR, LOG_WARNING, LOG_INFO, LOG_DEBUG };
unsigned log_level = LOG_SILENT;

enum cmd_opt_t {
  CMD_OPTION_NULL,
  CMD_OPTION_INPUT_FILE,
  CMD_OPTION_OUTPUT_PATH,
  CMD_OPTION_THREAD_NUMBER,
  CMD_OPTION_LOG_LEVEL,
  CMD_OPTION_LOG_LEVEL_SILENT
};

namespace fs = filesystem;

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
uint32_t exp_read_counter;

struct chl_key_t {
  uint32_t id;
  uint16_t pos;
};
using list_chl_key_t = list<chl_key_t>;

#ifndef HASH_TABLE_SZ
#  define HASH_TABLE_SZ 314606869
#endif

list<list_chl_key_t> hash_table[HASH_TABLE_SZ];

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
  if (opt_type == CMD_OPTION_NULL) opt_type = CMD_OPTION_INPUT_FILE;
  switch (opt_type) {
    case CMD_OPTION_INPUT_FILE:
      input_file = opt_token;
      opt_type   = CMD_OPTION_NULL;
      break;
    case CMD_OPTION_OUTPUT_PATH:
      output_path = opt_token;
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
  bool t;
  t = x, x = y, y = t;
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

// return false for noncollision
// return true for collision
bool hash_collision(const chl_key_t& chl_ref, const chl_key_t& chl_new) {
  const auto& read_ref = read_v[chl_ref.id];
  const auto& read_new = read_v[chl_new.id];
  if (chl_new.pos < 2 * READ_LENGTH) {
    for (uint32_t i = 0, j = chl_new.pos % READ_LENGTH; i < READ_LENGTH;
         ++i, j            = (j + 1) % READ_LENGTH)
      if (read_ref[i << 1 | 1] != read_new[j << 1 | 1]
          || read_ref[i << 1] != read_new[j << 1])
        return true;
  } else {
    for (uint32_t i = 0, j = chl_new.pos % READ_LENGTH; i < READ_LENGTH;
         ++i, j            = (READ_LENGTH - 1 + j) % READ_LENGTH)
      if (read_ref[i << 1 | 1] != read_new[j << 1 | 1]
          || read_ref[i << 1] != read_new[j << 1])
        return true;
  }
  return false;
}

// return 1 for fail
// return 0 for success
int rolling_hash_try_insert(uint32_t id, uint32_t hval, int state) {
  const static uint32_t p   = fast_pow(4ULL, READ_LENGTH, HASH_TABLE_SZ);
  const static uint32_t p_r = fast_pow(4ULL, HASH_TABLE_SZ - 2, HASH_TABLE_SZ);
  const read_t&         r   = read_v[id];
  for (uint32_t i = 0; i < READ_LENGTH; ++i) {
    uint32_t cur = (uint32_t) r[i << 1 | 1] << 1 | (uint32_t) r[i << 1];
    hval         = (hval + (uint64_t) cur * p) % HASH_TABLE_SZ;
    hval         = ((uint64_t) hval * p_r) % HASH_TABLE_SZ;
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
    uint32_t hval     = get_hash(read_seq, HASH_TABLE_SZ);
    // 0 源序列循环状态
    if (rolling_hash_try_insert(id, hval, 0) == 0) continue;

    read_seq.flip();
    uint32_t fhval = get_hash(read_seq, HASH_TABLE_SZ);
    // 1 补序列循环状态
    if (rolling_hash_try_insert(id, fhval, 1) == 0) continue;
    read_seq.flip();

    reverse_read(read_seq);
    uint32_t rhval = get_hash(read_seq, HASH_TABLE_SZ);
    // 2 逆序列循环状态
    if (rolling_hash_try_insert(id, rhval, 2) == 0) continue;

    read_seq.flip();
    uint32_t rfhval = get_hash(read_seq, HASH_TABLE_SZ);
    // 3 逆补序列循环状态
    if (rolling_hash_try_insert(id, rfhval, 3) == 0) continue;

    list_chl_key_t ls;
    ls.push_back((chl_key_t){.id = id, .pos = 0});
    hash_table[hval].push_back(ls);
  }
}

const struct bendl_t {
} bendl;

ofstream& operator<<(ofstream& ofs, const bendl_t& be) {
  const uint16_t bendl_ch = 0xffff;
  ofs.write((char*) &bendl_ch, 2 * sizeof(char));
  return ofs;
}

ofstream& operator<<(ofstream& ofs, const read_t& r) {
  string             str = r.to_string();
  int                rem = str.size() % 8;
  static const char* v[] = {"",     "0000000", "000000", "00000",
                            "0000", "000",     "00",     "0"};
  str += v[rem];
  for (size_t i = 0; i < str.size(); i += 8) {
    char j = ((str[i] - '0') << 7) | ((str[i + 1] - '0') << 6)
             | ((str[i + 2] - '0') << 5) | ((str[i + 3] - '0') << 4)
             | ((str[i + 4] - '0') << 3) | ((str[i + 5] - '0') << 2)
             | ((str[i + 6] - '0') << 1) | ((str[i + 7] - '0'));
    ofs.write(&j, sizeof(char));
  }
  return ofs;
}

ofstream& operator<<(ofstream& ofs, const chl_key_t& st) {
  static uint32_t last_id;
  if (st.pos == 0) ofs.write((const char*) &st.id, 4 * sizeof(char));
  else {
    uint32_t diff_id = st.id - last_id;
    ofs.write((const char*) &st.pos, 2 * sizeof(char));
    ofs.write((const char*) &diff_id, 4 * sizeof(char));
  }
  last_id = st.id;
  return ofs;
}

struct chl_id_t {
  uint32_t id;
};

ofstream& operator<<(ofstream& ofs, const chl_id_t& cid) {
  static uint32_t last_id = 0xffffffff;
  uint32_t        diff_id = last_id == 0xffffffff ? cid.id : cid.id - last_id;
  ofs.write((const char*) &diff_id, 4 * sizeof(char));
  last_id = cid.id;
  return ofs;
}

void dump_bin() {
  auto ido_file = fs::path(output_path) / "ido.bin";
  auto idp_file = fs::path(output_path) / "idp.bin";
  if (fs::exists(ido_file) && log_level >= LOG_WARNING)
    cerr << "[warning] ido file " << ido_file
         << " already exists, overwriting..." << endl;
  if (fs::exists(idp_file) && log_level >= LOG_WARNING)
    cerr << "[warning] idp file " << idp_file
         << " already exists, overwriting..." << endl;
  ofstream ido(ido_file, ios::binary | ios::out | ios::trunc);
  ofstream idp(idp_file, ios::binary | ios::out | ios::trunc);
  for (uint32_t i = 0; i < HASH_TABLE_SZ; ++i)
    for (const auto& ls : hash_table[i]) {
      const auto& f_st = ls.front();
      const auto& r    = read_v[f_st.id];
      if (ls.size() == 1) ido << r << (chl_id_t){f_st.id};
      else {
        idp << r;
        for (const auto& st : ls) idp << st;
        idp << bendl;
      }
    }
}

void dump_meta() {
  auto ido_file  = fs::path(output_path) / "ido.bin";
  auto idp_file  = fs::path(output_path) / "idp.bin";
  auto meta_file = fs::path(output_path) / "chl.meta";
  if (fs::exists(meta_file) && log_level >= LOG_WARNING)
    cerr << "[warning] meta file " << meta_file
         << " already exists, overwriting..." << endl;
  ofstream meta(meta_file, ios::out | ios::trunc);
  auto     cout_buf = cout.rdbuf(meta.rdbuf());
#define endl '\n'
  cout << "ido-file" << endl;
  cout << ido_file.string() << endl;
  cout << "idp-file" << endl;
  cout << idp_file.string() << endl;
  cout << "read-length" << endl;
  cout << READ_LENGTH << endl;
#undef endl
  cout.rdbuf(cout_buf);
}

signed main(int argc, char* argv[]) {
  init();
  for (int i = 1; i < argc; ++i) parse_opt(argv[i]);
  thread_number = max(thread_number, 1U);
  if (log_level >= LOG_INFO)
    cerr << "[info] begin CHL..." << endl
         << "[info] input_file " << fs::path(input_file) << endl
         << "[info] output_path " << fs::path(output_path) << endl
         << "[info] hash table size " << HASH_TABLE_SZ << endl
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
  uint32_t cnt;
  string   read_path;
  cin >> exp_read_counter;
  while (getline(cin, read_path)) {
    cin >> cnt;
    ifstream   read_ifs;
    streambuf* cin_buf_bak = nullptr;
    if (read_path != "-") {
      read_ifs.open(read_path);
      cin_buf_bak = cin.rdbuf(read_ifs.rdbuf());
    }
    auto orig_counter = read_counter;
    read_sequence();
    if (orig_counter + cnt != read_counter && log_level >= LOG_WARNING)
      cerr << "[warning] reads (" << read_path << ") count mismatch ("
           << "expected lines " << cnt << " practical lines "
           << read_counter - orig_counter << ")" << endl;
    if (cin_buf_bak) cin.rdbuf(cin_buf_bak);
  }
  if (cin_buf) cin.rdbuf(cin_buf);
  if (exp_read_counter != read_counter && log_level >= LOG_WARNING)
    cerr << "[warning] reads total count mismatch ("
         << "expected total lines " << exp_read_counter
         << " practical total lines " << read_counter << ")" << endl;
  chl();
  dump_bin();
  dump_meta();
}
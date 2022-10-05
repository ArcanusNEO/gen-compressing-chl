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
string     base4_is_map[256];

#ifndef MX_READ_LIST_SZ
#  define MX_READ_LIST_SZ 0x10000000
#endif

uint64_t read_v[MX_READ_LIST_SZ][8];
uint32_t read_counter;
uint32_t read_length;
uint32_t read_bin_length;

void init() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  base_ai_map['a'] = base_ai_map['A'] = 0x00;
  base_ai_map['t'] = base_ai_map['T'] = 0x03;
  base_ai_map['c'] = base_ai_map['C'] = 0x01;
  base_ai_map['g'] = base_ai_map['G'] = 0x02;
  for (uint32_t i = 0; i < 4; ++i)
    for (uint32_t j = 0; j < 4; ++j)
      for (uint32_t k = 0; k < 4; ++k)
        for (uint32_t l = 0; l < 4; ++l) {
          uint32_t index = i << 6 | j << 4 | k << 2 | l;
          base4_is_map[index].resize(4);
          base4_is_map[index][0] = base_ia_map[i];
          base4_is_map[index][1] = base_ia_map[j];
          base4_is_map[index][2] = base_ia_map[k];
          base4_is_map[index][3] = base_ia_map[l];
        }
}

void iter_assign(uint64_t i[8], uint64_t j[8]) noexcept {
  i[0] = j[0];
  i[1] = j[1];
  i[2] = j[2];
  i[3] = j[3];
  i[4] = j[4];
  i[5] = j[5];
  i[6] = j[6];
  i[7] = j[7];
}

void iter_swap(uint64_t i[8], uint64_t j[8]) noexcept {
  swap(i[0], j[0]);
  swap(i[1], j[1]);
  swap(i[2], j[2]);
  swap(i[3], j[3]);
  swap(i[4], j[4]);
  swap(i[5], j[5]);
  swap(i[6], j[6]);
  swap(i[7], j[7]);
}

string translate(uint64_t raw_read[8]) {
  string ret;
  ret.reserve(read_length);
  for (uint8_t* i = (uint8_t*) raw_read;
       i + 1 < (uint8_t*) raw_read + read_bin_length; ++i)
    ret += base4_is_map[*i];
  uint8_t* prem_base4 = (uint8_t*) raw_read + read_bin_length - 1;
  uint8_t  rem_base4  = *prem_base4;
  int      rem        = read_length % 4;
  for (int i = 0; i < rem; ++i)
    ret.push_back(base_ia_map[rem_base4 >> (6 - i * 2)]);
  return ret;
}

void transform_read(uint64_t new_read[8], uint16_t pos, uint64_t read_ref[8]) {
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
    } else if (tag.find("read") != string::npos) cin >> read_length;
  }
  if (cin_buf) cin.rdbuf(cin_buf);
  ifs.close();
  ifs.open(ido_file);
  read_bin_length = (read_length + 3) / 4;  // Byte
  uint64_t buf[8];
  while (ifs.read((char*) buf, read_bin_length)) {
    uint32_t id;
    ifs.read((char*) &id, 4);
    read_counter = max(read_counter, id);
    iter_assign(read_v[id], buf);
  }
  for (const auto& idp_file : idp_file_ls) {
    ifs.open(idp_file);
    while (ifs.read((char*) buf, read_bin_length)) {
      uint32_t id;
      ifs.read((char*) &id, 4);
      read_counter = max(read_counter, id);
      iter_assign(read_v[id], buf);
      auto     read_ref = read_v[id];
      uint16_t pos;
      while (ifs.read((char*) &pos, 2) && ~pos) {
        ifs.read((char*) &id, 4);
        read_counter = max(read_counter, id);
        transform_read(buf, pos, read_ref);
        iter_assign(read_v[id], buf);
      }
    }
  }
  ofstream   ofs;
  streambuf* cout_buf = nullptr;
  if (output_file != "-") {
    ofs.open(output_file);
    cout_buf = cout.rdbuf(ofs.rdbuf());
  }
#define endl '\n'
  for (uint32_t i = 0; i < read_counter; ++i)
    cout << translate(read_v[i]) << endl;
#undef endl
  if (cout_buf) cout.rdbuf(cout_buf);
}
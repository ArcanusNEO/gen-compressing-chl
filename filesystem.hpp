#ifndef _GLIBCXX_CHRONO_H
#define _GLIBCXX_CHRONO_H 0
#include <string>
namespace filesystem {
using std::string;
class path {
public:
  string p_;
  path(const string& path_str = "") : p_(path_str) { }
};
path operator/(const path& a, const path& b) {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__) \
  || defined(__WINDOWS_)
  return path(a.p_ + "\\" + b.p_);
#else
  return path(a.p_ + "/" + b.p_);
#endif
}
void remove_all(const path& p) {
  system(("rm -rf -- " + p.p_).c_str());
}
void create_directory(const path& p) {
  system(("mkdir -p -- " + p.p_).c_str());
}
int exists(const path& p) {
  return system(("[ ! -f " + p.p_ + " ]").c_str());
}
};      // namespace filesystem
#endif  // _GLIBCXX_CHRONO_H
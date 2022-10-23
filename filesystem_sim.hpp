#ifndef _FILESYSTEM_HPP_
#define _FILESYSTEM_HPP_
#include <iostream>
#include <string>
namespace filesystem_sim {
class path {
public:
  std::string p_;
  path(const std::string& path_str) : p_(path_str) { }
  path(const char* path_c_str = nullptr) : p_(std::string(path_c_str)) { }
  std::string string() const { return p_; }
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
std::ostream& operator<<(std::ostream& os, const path& p) {
  return (os << '"' << p.p_ << '"');
}
};      // namespace filesystem_sim
#endif  // _FILESYSTEM_HPP_
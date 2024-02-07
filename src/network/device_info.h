#ifndef __network__device_info__
#define __network__device_info__

#include <string>

class DeviceInfo {
public:
  DeviceInfo() : m_name{""}, m_value{""}, m_registered{false} {}

  ~DeviceInfo() = default;

  bool register_device(const std::string &name, const std::string &value) {
    if (m_registered) {
      return false;
    }
    m_name = name;
    m_value = value;
    m_registered = true;
    return true;
  }

  void set_value(const std::string &value) { m_value = value; }

  std::string get_name() { return m_name; }

  std::string get_value() { return m_value; }

  bool is_registered() { return m_registered; }

private:
  std::string m_name;
  std::string m_value;
  bool m_registered;
};

#endif

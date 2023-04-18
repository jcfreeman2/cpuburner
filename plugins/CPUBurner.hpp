/**
 * @file CPUBurner.hpp
 *
 * Dummy DAQModule for multithread testing, does nothing but waste CPU time
 *
 * This is part of the DUNE DAQ Software Suite, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef CPUBURNER_PLUGINS_CPUBURNER_HPP_
#define CPUBURNER_PLUGINS_CPUBURNER_HPP_

#include "appfwk/DAQModule.hpp"
#include "utilities/WorkerThread.hpp"

#include <atomic>
#include <limits>
#include <string>

namespace dunedaq::cpuburner {

class CPUBurner : public dunedaq::appfwk::DAQModule
{
public:
  explicit CPUBurner(const std::string& name);

  void init(const data_t&) override;
  void init(const dunedaq::dal::DaqModule*) override;

  CPUBurner(const CPUBurner&) = delete;
  CPUBurner& operator=(const CPUBurner&) = delete;
  CPUBurner(CPUBurner&&) = delete;
  CPUBurner& operator=(CPUBurner&&) = delete;

  ~CPUBurner() = default;

private:
  // Commands CPUBurner can receive

  void do_conf(const data_t&);
  void do_start(const nlohmann::json& obj);
  void do_stop(const nlohmann::json& obj);
  // Threading
  dunedaq::utilities::WorkerThread thread_;
  void do_work(std::atomic<bool>&);

  uint32_t m_burnTime;
  uint32_t m_sleepTime;
  uint32_t m_memSize;
  uint32_t m_reserveCores;
};

} // namespace dunedaq::cpuburner

#endif // CPUBURNER_PLUGINS_CPUBURNER_HPP_

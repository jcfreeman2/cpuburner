/**
 * @file CPUBurner.cpp
 *
 * Implementations of CPUBurner's functions
 *
 * This is part of the DUNE DAQ Software Suite, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "logging/Logging.hpp"

#include "cpuburner/cpuburner/Nljs.hpp"
#include "cpuburner/cpuburnerinfo/InfoNljs.hpp"

#include "dunedaqdal/DaqModule.hpp"
#include "dunedaqdal/ProcessingResource.hpp"
#include "dunedaqdal/ProcessingResourceClaim.hpp"

#include "cpuburner/CpuBurnerModule.hpp"

#include "coremanager/CoreManager.hpp"

#include "CPUBurner.hpp"

#include <string>
#include <chrono>

/**
 * @brief Name used by TRACE TLOG calls from this source file
 */
#define TRACE_NAME "CPUBurner" // NOLINT
#define TLVL_ENTER_EXIT_METHODS 10
#define TLVL_LIST_GENERATION 15

namespace dunedaq::cpuburner {

CPUBurner::CPUBurner(const std::string& name)
  : dunedaq::appfwk::DAQModule(name)
  , thread_(std::bind(&CPUBurner::do_work, this, std::placeholders::_1))
{
  register_command("start", &CPUBurner::do_start);
  register_command("stop", &CPUBurner::do_stop);
}

void
CPUBurner::init(const data_t& /* structured args */)
{}

void
CPUBurner::init(const dunedaq::dal::DaqModule* modconf) {
  auto conf = modconf->cast<dunedaq::dal::CpuBurnerModule>();
  if (conf == nullptr) {
    throw appfwk::OksCastIssue(ERS_HERE, get_name(), "dunedaq::dal::CpuBurnerModule");
  }
  m_burnTime = conf->get_burn_time_us();
  m_sleepTime = conf->get_sleep_time_us();
  m_memSize = conf->get_mem_size();
  auto claim = conf->get_used_cpu();
  if (claim.size() > 0) {
    m_reserveCores = claim[0]->get_count();
    if (m_reserveCores != 0) {
      auto node = claim[0]->get_processor()->get_numa_id();
      std::cout << "module " << get_name() << ": node=" << node << std::endl;
      coremanager::CoreManager::get()->allocate(get_name(), node);
    }
  }
}

void
CPUBurner::do_start(const nlohmann::json& /*args*/)
{
  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Entering do_start() method";
  thread_.start_working_thread(get_name());
  TLOG() << get_name() << " successfully started";
  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Exiting do_start() method";
}

void
CPUBurner::do_stop(const nlohmann::json& /*args*/)
{
  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Entering do_stop() method";
  thread_.stop_working_thread();
  TLOG() << get_name() << " successfully stopped";
  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Exiting do_stop() method";
}


void
CPUBurner::do_work(std::atomic<bool>& running) {
  coremanager::CoreManager::get()->setAffinity(get_name());
  coremanager::CoreManager::get()->dump();
  TLOG() << get_name() << ": " << coremanager::CoreManager::get()->affinityString();

  auto interval = std::chrono::duration(std::chrono::microseconds(m_burnTime));
  auto mem=std::vector<float>(m_memSize,1.0);
  while (running) {
    auto start = std::chrono::steady_clock::now();
    // Waste some CPU and memory accesses
    do {
      float pointless_sum=0;
      for (uint32_t i=0; i<m_memSize; ++i) {
        pointless_sum+=mem[i];
      }
    } while ((std::chrono::steady_clock::now()-start < interval));
    // Sleep a bit so we don't use ALL the CPU unless we really want to
    std::this_thread::sleep_for(std::chrono::microseconds(m_sleepTime));
  }
}

} // namespace dunedaq::cpuburner

DEFINE_DUNE_DAQ_MODULE(dunedaq::cpuburner::CPUBurner)

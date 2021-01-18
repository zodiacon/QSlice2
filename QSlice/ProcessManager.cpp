#include "pch.h"
#include "ProcessManager.h"
#include <algorithm>
#include <PdhMsg.h>

#pragma comment(lib, "pdh")

ProcessManager::ProcessManager() {
	_processorCount = ::GetActiveProcessorCount(ALL_PROCESSOR_GROUPS);

	::PdhOpenQuery(nullptr, 0, _hQuery.addressof());

	::PdhAddEnglishCounter(_hQuery.get(), L"\\Process(*)\\ID Process", 0, _hPidCounter.addressof());
	::PdhAddEnglishCounter(_hQuery.get(), L"\\Process(*)\\% Processor Time", 0, _hCpuCounter.addressof());
	::PdhAddEnglishCounter(_hQuery.get(), L"\\Process(*)\\% Privileged Time", 0, _hKernelCpuCounter.addressof());

	::PdhCollectQueryData(_hQuery.get());
	::PdhCollectQueryData(_hQuery.get());

}

const std::vector<ProcessInfo>& ProcessManager::Update() {
	DWORD size = 0;
	DWORD count;
	::PdhGetFormattedCounterArray(_hPidCounter.get(), PDH_FMT_LONG, &size, &count, nullptr);
	auto pidBuffer = std::make_unique<BYTE[]>(size);
	auto pidData = (PPDH_FMT_COUNTERVALUE_ITEM)pidBuffer.get();
	::PdhGetFormattedCounterArray(_hPidCounter.get(), PDH_FMT_LONG, &size, &count, pidData);

	size = 0;
	DWORD count2;
	::PdhGetFormattedCounterArray(_hCpuCounter.get(), PDH_FMT_DOUBLE | PDH_FMT_NOCAP100, &size, &count2, nullptr);
	auto cpuBuffer = std::make_unique<BYTE[]>(size);
	auto cpuData = (PPDH_FMT_COUNTERVALUE_ITEM)cpuBuffer.get();
	::PdhGetFormattedCounterArray(_hCpuCounter.get(), PDH_FMT_DOUBLE | PDH_FMT_NOCAP100, &size, &count2, cpuData);

	size = 0;
	DWORD count3;
	::PdhGetFormattedCounterArray(_hKernelCpuCounter.get(), PDH_FMT_DOUBLE | PDH_FMT_NOCAP100, &size, &count3, nullptr);
	auto kernelCpuBuffer = std::make_unique<BYTE[]>(size);
	auto kernelCpuData = (PPDH_FMT_COUNTERVALUE_ITEM)kernelCpuBuffer.get();
	::PdhGetFormattedCounterArray(_hKernelCpuCounter.get(), PDH_FMT_DOUBLE | PDH_FMT_NOCAP100, &size, &count3, kernelCpuData);

	ATLASSERT(count == count2);

	std::vector<ProcessInfo> processes;
	processes.reserve(count);
	for (DWORD i = 0; i < count; i++) {
		auto& p = pidData[i];
		if (p.FmtValue.longValue == 0)
			continue;
		auto& p2 = cpuData[i];
		auto& p3 = kernelCpuData[i];

		ProcessInfo pi;
		ATLASSERT(::_wcsicmp(p.szName, p2.szName) == 0);
		pi.Name = p.szName;
		pi.Pid = p.FmtValue.longValue;
		pi.CPU = p2.FmtValue.doubleValue / _processorCount;
		pi.KernelCPU = p3.FmtValue.doubleValue / _processorCount;
		processes.push_back(std::move(pi));
	}
	std::sort(processes.begin(), processes.end(), [](const auto& p1, const auto& p2) {
		return p1.CPU > p2.CPU;
		});

	::PdhCollectQueryData(_hQuery.get());
	_processes = std::move(processes);

	return _processes;
}

const std::vector<ProcessInfo>& ProcessManager::GetProcesses() const {
	return _processes;
}

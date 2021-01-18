#pragma once

#include <wil\resource.h>

struct ProcessInfo {
	CString Name;
	DWORD Pid;
	double CPU;
	double KernelCPU;
};

using unique_pdh_query = wil::unique_any_handle_null_only<decltype(&::PdhCloseQuery), ::PdhCloseQuery>;
using unique_pdh_counter = wil::unique_any_handle_null_only<decltype(&::PdhRemoveCounter), ::PdhRemoveCounter>;

class ProcessManager {
public:
	ProcessManager();
	const std::vector<ProcessInfo>& Update();
	const std::vector<ProcessInfo>& GetProcesses() const;

private:
	unique_pdh_counter _hPidCounter, _hCpuCounter, _hKernelCpuCounter;
	unique_pdh_query _hQuery;
	std::vector<ProcessInfo> _processes;
	DWORD _processorCount;
};


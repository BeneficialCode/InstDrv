#include "stdafx.h"
#include "SysManager.h"

bool SysManager::Install() const {
	wil::unique_schandle hSCManager;
	hSCManager.reset(OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS));
	if (NULL == hSCManager.get())
		return false;

	wil::unique_schandle hService;
	hService.reset(CreateService(hSCManager.get(), _serviceName, _displayName,
		SERVICE_ALL_ACCESS,
		_isFileSys ? SERVICE_FILE_SYSTEM_DRIVER : SERVICE_KERNEL_DRIVER,
		SERVICE_DEMAND_START, // driver is not loaded automatically
		SERVICE_ERROR_NORMAL, // The error is logged to the event log service
		_sysPath, nullptr, nullptr, nullptr, nullptr, nullptr));
	if (NULL == hService.get())
		return false;

	return true;
}

SC_HANDLE SysManager::GetServiceHandle() const {
	wil::unique_schandle hSCManager;
	hSCManager.reset(OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS));
	if (NULL == hSCManager.get())
		return NULL;

	SC_HANDLE hService;
	hService = OpenService(hSCManager.get(), _serviceName, SERVICE_ALL_ACCESS);
	if (NULL == hService)
		return NULL;
	return hService;
}

bool SysManager::Run() const {
	wil::unique_schandle hService;
	hService.reset(GetServiceHandle());
	if (NULL == hService.get())
		return false;

	if (!StartService(hService.get(), NULL, nullptr))
		return false;
	return true;
}

bool SysManager::Stop() const {
	wil::unique_schandle hService;
	hService.reset(GetServiceHandle());
	if (NULL == hService.get())
		return false;

	SERVICE_STATUS status;
	if (!ControlService(hService.get(), SERVICE_CONTROL_STOP, &status))
		return false;
	return true;
}

bool SysManager::Remove() const {
	wil::unique_schandle hService;
	hService.reset(GetServiceHandle());
	if (NULL == hService.get())
		return false;

	if (!DeleteService(hService.get()))
		return false;
	return true;
}
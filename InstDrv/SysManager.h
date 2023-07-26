#pragma once

class SysManager {
public:
	SysManager(CString sysPath, CString serviceName, CString displayName,bool isFileSys) :_sysPath(sysPath),
		_serviceName(serviceName), _displayName(displayName),_isFileSys(isFileSys){
	}
	bool Install() const;
	bool Run() const;
	bool Stop() const;
	bool Remove() const;
	SC_HANDLE GetServiceHandle() const;
private:
	CString _sysPath;
	CString _serviceName;
	CString _displayName;
	bool _isFileSys;
};
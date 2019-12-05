#include "../utils/platform.hpp"

#ifdef PLATFORM_WIN
#include "../io/iocontext_impl.hpp"
#include <windows.h>
#include <dbt.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <cctype>
#include <algorithm>

#include <Cfgmgr32.h>
#include <stdio.h>
#include <tchar.h>
#include <strsafe.h>
#include <Setupapi.h>

#include "detection.hpp"
#include "../utils/logger.hpp"

#include <thread>
#include <atomic>
#include <stdexcept>

#define VID_TAG "VID_"
#define PID_TAG "PID_"
#define COM_PATH_PREFIX "\\\\.\\"

#define APP_UNBLOCK_MSG (WM_USER + 1)
#define APP_DEVICE_EVENT_MSG (WM_USER + 2)

typedef std::basic_string<TCHAR> tstring;

using namespace JetBeep;
using namespace std;

#define LIBRARY_NAME "setupapi.dll"
#define DllImport __declspec(dllimport)

#define MAX_THREAD_WINDOW_NAME 64

// https://docs.microsoft.com/en-us/windows-hardware/drivers/install/guid-devinterface-comport
const GUID GUID_DEVINTERFACE_USB_DEVICE = {0x86E0D1E0L, 0x8089, 0x11D0, 0x9C, 0xE4, 0x08, 0x00, 0x3E, 0x30, 0x1F, 0x73};

typedef BOOL(WINAPI* _SetupDiEnumDeviceInfo)(HDEVINFO DeviceInfoSet, DWORD MemberIndex, PSP_DEVINFO_DATA DeviceInfoData);
typedef HDEVINFO(WINAPI* _SetupDiGetClassDevs)(const GUID* ClassGuid, PCTSTR Enumerator, HWND m_hwndParent, DWORD Flags);
typedef BOOL(WINAPI* _SetupDiDestroyDeviceInfoList)(HDEVINFO DeviceInfoSet);
typedef BOOL(WINAPI* _SetupDiGetDeviceInstanceId)(HDEVINFO DeviceInfoSet,
                                                  PSP_DEVINFO_DATA DeviceInfoData,
                                                  PTSTR DeviceInstanceId,
                                                  DWORD DeviceInstanceIdSize,
                                                  PDWORD RequiredSize);
typedef BOOL(WINAPI* _SetupDiGetDeviceRegistryProperty)(HDEVINFO DeviceInfoSet,
                                                        PSP_DEVINFO_DATA DeviceInfoData,
                                                        DWORD Property,
                                                        PDWORD PropertyRegDataType,
                                                        PBYTE PropertyBuffer,
                                                        DWORD PropertyBufferSize,
                                                        PDWORD RequiredSize);
typedef HKEY(WINAPI* _SetupDiOpenDevRegKey)(
  HDEVINFO DeviceInfoSet, PSP_DEVINFO_DATA DeviceInfoData, DWORD Scope, DWORD HwProfile, DWORD KeyType, REGSAM samDesired);

static void charsToUpper(char* buf);
static void normalizeSlashes(char* buf);
static string UTF8Encode(const string& str);

class DeviceDetection::Impl {
public:
  Impl(DeviceDetectionCallback* callback, IOContext context);

  void startMonitoring();
  void stopMonitoring();
  void detectConnected();
  virtual ~Impl();

private:
  IOContext m_context;
  DeviceDetectionCallback* m_callback = nullptr;
  Logger m_log;
  std::atomic<bool> isMonActive;
  std::thread m_thread;

  HWND m_hwnd;

  void loadFunctions();
  void monitorLoop();

  static DeviceDetectionEvent lastDetectedAction;
  static DeviceCandidate lastDetectedCandidate;

  static VidPid extractVidPid(HDEVINFO hDevInfo, SP_DEVINFO_DATA* pspDevInfoData, TCHAR* buf, DWORD buffSize);
  static void checkDetectedDeviceEvent(PDEV_BROADCAST_DEVICEINTERFACE, WPARAM);
  static LRESULT CALLBACK monitorDetectCallback(HWND, UINT, WPARAM, LPARAM);
  static string findPortName(HDEVINFO, SP_DEVINFO_DATA*);

  // win dll lib instance
  static HINSTANCE m_hInstLib;

  // winapi functions
  static _SetupDiEnumDeviceInfo DllSetupDiEnumDeviceInfo;
  static _SetupDiGetClassDevs DllSetupDiGetClassDevs;
  static _SetupDiDestroyDeviceInfoList DllSetupDiDestroyDeviceInfoList;
  static _SetupDiGetDeviceInstanceId DllSetupDiGetDeviceInstanceId;
  static _SetupDiGetDeviceRegistryProperty DllSetupDiGetDeviceRegistryProperty;
  static _SetupDiOpenDevRegKey DllSetupDiOpenDevRegKey;
};

// static initialization
DeviceDetectionEvent DeviceDetection::Impl::lastDetectedAction = DeviceDetectionEvent::added;
DeviceCandidate DeviceDetection::Impl::lastDetectedCandidate = {0, 0, ""};

HINSTANCE DeviceDetection::Impl::m_hInstLib = nullptr;
_SetupDiEnumDeviceInfo DeviceDetection::Impl::DllSetupDiEnumDeviceInfo = nullptr;
_SetupDiGetClassDevs DeviceDetection::Impl::DllSetupDiGetClassDevs = nullptr;
_SetupDiDestroyDeviceInfoList DeviceDetection::Impl::DllSetupDiDestroyDeviceInfoList = nullptr;
_SetupDiGetDeviceInstanceId DeviceDetection::Impl::DllSetupDiGetDeviceInstanceId = nullptr;
_SetupDiGetDeviceRegistryProperty DeviceDetection::Impl::DllSetupDiGetDeviceRegistryProperty = nullptr;
_SetupDiOpenDevRegKey DeviceDetection::Impl::DllSetupDiOpenDevRegKey = nullptr;

// DeviceDetection implementation

DeviceDetection::Impl::Impl(DeviceDetectionCallback* callback, IOContext context)
  : m_callback(callback), m_log("detection"), m_context(context) {
  isMonActive.store(false);
  if (!m_hInstLib) {
    loadFunctions();
  }
}

DeviceDetection::Impl::~Impl() {
  try {
    stopMonitoring();
  } catch (exception& e) {
    m_log.e() << e.what() << Logger::endl;
  }
}

void DeviceDetection::Impl::stopMonitoring() {
  isMonActive.store(false);

  // post message to window thread, to unblock GetMessage and exit
  if (m_hwnd && !PostMessageA(m_hwnd, APP_UNBLOCK_MSG, 0, 0)) {
    throw runtime_error("Unable to PostMessageA");
  }
  if (m_thread.joinable()) {
    m_thread.join();
  }

  m_log.d() << "Window mon stopped" << Logger::endl;
}

void DeviceDetection::Impl::startMonitoring() {
  if (isMonActive.load()) {
    return;
  }
  isMonActive.store(true);

  m_thread = thread(&DeviceDetection::Impl::monitorLoop, this);
}

void DeviceDetection::Impl::monitorLoop() {
  char className[MAX_THREAD_WINDOW_NAME];
  _snprintf_s(className, MAX_THREAD_WINDOW_NAME, "ListnerThreadUsbDetection_%d", GetCurrentThreadId());

  WNDCLASSA wincl = {0};
  wincl.hInstance = GetModuleHandle(0);
  wincl.lpszClassName = className;
  wincl.lpfnWndProc = monitorDetectCallback;

  if (!RegisterClassA(&wincl)) {
    DWORD le = GetLastError();
    m_log.d() << "RegisterClassA() failed, error " << le << Logger::endl;
    throw runtime_error("RegisterClassA() failed");
  }

  m_hwnd = CreateWindowExA(WS_EX_TOPMOST, className, className, 0, 0, 0, 0, 0, NULL, 0, 0, 0);
  if (!m_hwnd) {
    DWORD le = GetLastError();
    m_log.d() << "CreateWindowExA() failed, error " << le << Logger::endl;
    throw runtime_error("CreateWindowExA() failed");
  }

  DEV_BROADCAST_DEVICEINTERFACE_A notifyFilter = {0};
  notifyFilter.dbcc_size = sizeof(notifyFilter);
  notifyFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
  notifyFilter.dbcc_classguid = GUID_DEVINTERFACE_USB_DEVICE;

  HDEVNOTIFY hDevNotify = RegisterDeviceNotificationA(m_hwnd, &notifyFilter, DEVICE_NOTIFY_WINDOW_HANDLE);
  if (!hDevNotify) {
    DWORD le = GetLastError();
    m_log.d() << "RegisterDeviceNotificationA() failed, error " << le << Logger::endl;
    throw runtime_error("RegisterDeviceNotificationA() failed");
  }

  MSG msg;
  while (isMonActive.load()) {
    BOOL bRet = GetMessage(&msg, m_hwnd, 0, 0);
    if ((bRet == 0) || (bRet == -1)) {
      break;
    }

    TranslateMessage(&msg);
    DispatchMessage(&msg);

    if (msg.message != APP_UNBLOCK_MSG && lastDetectedCandidate.pid != 0) {
      auto action = DeviceDetection::Impl::lastDetectedAction;
      auto candidat = DeviceDetection::Impl::lastDetectedCandidate;
      m_context.m_impl->ioService.post([&, action, candidat] {
        auto callback = *m_callback;
        if (callback) {
          callback(action, candidat);
        }
      });
    }
  }
}

void DeviceDetection::Impl::detectConnected() {
  DWORD dwFlag = (DIGCF_ALLCLASSES | DIGCF_PRESENT);
  HDEVINFO hDevInfo = DllSetupDiGetClassDevs(NULL, "USB", NULL, dwFlag);

  if (INVALID_HANDLE_VALUE == hDevInfo) {
    throw runtime_error("hDevInfo INVALID_HANDLE_VALUE");
  }

  SP_DEVINFO_DATA* pspDevInfoData = (SP_DEVINFO_DATA*)HeapAlloc(GetProcessHeap(), 0, sizeof(SP_DEVINFO_DATA));

  if (!pspDevInfoData) {
    throw runtime_error("pspDevInfoData HeapAlloc fail");
  }

  pspDevInfoData->cbSize = sizeof(SP_DEVINFO_DATA);

  for (int i = 0; DllSetupDiEnumDeviceInfo(hDevInfo, i, pspDevInfoData); i++) {
    DWORD nSize = 0;
    TCHAR buf[MAX_PATH];

    if (!DllSetupDiGetDeviceInstanceId(hDevInfo, pspDevInfoData, buf, sizeof(buf), &nSize)) {
      break;
    }
    normalizeSlashes(buf);

    DWORD DataT;
    DllSetupDiGetDeviceRegistryProperty(hDevInfo, pspDevInfoData, SPDRP_LOCATION_INFORMATION, &DataT, (PBYTE)buf, MAX_PATH, &nSize);
    DllSetupDiGetDeviceRegistryProperty(hDevInfo, pspDevInfoData, SPDRP_HARDWAREID, &DataT, (PBYTE)(buf + nSize - 1),
                                        MAX_PATH - nSize, &nSize);

    auto vidPid = extractVidPid(hDevInfo, pspDevInfoData, buf, MAX_PATH);

    if (!DeviceDetection::isValidVidPid(vidPid)) {
      continue;
    }

    DeviceCandidate candidate;
    candidate.vid = vidPid.vid;
    candidate.pid = vidPid.pid;

    string portName = findPortName(hDevInfo, pspDevInfoData);
    if (portName.length()) {
      candidate.path = COM_PATH_PREFIX + portName;
      m_context.m_impl->ioService.post([&, candidate] {
        auto callback = *m_callback;
        if (callback) {
          callback(DeviceDetectionEvent::added, candidate);
        }
      });
    }
  }

  HeapFree(GetProcessHeap(), 0, pspDevInfoData);

  if (hDevInfo) {
    DllSetupDiDestroyDeviceInfoList(hDevInfo);
  }
}

string DeviceDetection::Impl::findPortName(HDEVINFO hDevInfo, SP_DEVINFO_DATA* pspDevInfoData) {
  HKEY hDeviceRegistryKey;
  hDeviceRegistryKey = DllSetupDiOpenDevRegKey(hDevInfo, pspDevInfoData, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);
  if (hDeviceRegistryKey == INVALID_HANDLE_VALUE) {
    throw runtime_error("Unable to open registry");
  }
  // Read in the name of the port
  char portName[20];
  DWORD dwSize = sizeof(portName);
  DWORD dwType = 0;

  if ((RegQueryValueEx(hDeviceRegistryKey, "PortName", NULL, &dwType, (LPBYTE)portName, &dwSize) == ERROR_SUCCESS) &&
      (dwType == REG_SZ)) {
    // Check if it really is a com port
    if (_tcsnicmp(portName, _T("COM"), 3) == 0) {
      return string(portName);
    }
  }

  RegCloseKey(hDeviceRegistryKey);
  return "";
}

void DeviceDetection::Impl::checkDetectedDeviceEvent(PDEV_BROADCAST_DEVICEINTERFACE pDevInf, WPARAM wParam) {
  // dbcc_name:
  // \\?\USB#Vid_04e8&Pid_503b#0002F9A9828E0F06#{a5dcbf10-6530-11d2-901f-00c04fb951ed}
  // convert to
  // USB\Vid_04e8&Pid_503b\0002F9A9828E0F06
  lastDetectedCandidate.pid = 0;

  if (!pDevInf) {
    return;
  }

  lastDetectedAction = (DBT_DEVICEARRIVAL == wParam) ? DeviceDetectionEvent::added : DeviceDetectionEvent::removed;

  tstring szDevId = pDevInf->dbcc_name + 4;
  auto idx = szDevId.rfind(_T('#'));

  if (idx != tstring::npos)
    szDevId.resize(idx);
  std::replace(begin(szDevId), end(szDevId), _T('#'), _T('\\'));
  auto to_upper = [](TCHAR ch) { return std::use_facet<std::ctype<TCHAR>>(std::locale()).toupper(ch); };
  transform(begin(szDevId), end(szDevId), begin(szDevId), to_upper);

  tstring szClass;
  idx = szDevId.find(_T('\\'));
  if (idx != tstring::npos) {
    szClass = szDevId.substr(0, idx);
  }
  // if we are adding device, we only need present devices
  // otherwise, we need all devices
  DWORD dwFlag = DBT_DEVICEARRIVAL != wParam ? DIGCF_ALLCLASSES : (DIGCF_ALLCLASSES | DIGCF_PRESENT);
  HDEVINFO hDevInfo = DllSetupDiGetClassDevs(NULL, szClass.c_str(), NULL, dwFlag);
  if (INVALID_HANDLE_VALUE == hDevInfo) {
    return;
  }

  SP_DEVINFO_DATA* pspDevInfoData = (SP_DEVINFO_DATA*)HeapAlloc(GetProcessHeap(), 0, sizeof(SP_DEVINFO_DATA));
  if (pspDevInfoData) {
    pspDevInfoData->cbSize = sizeof(SP_DEVINFO_DATA);
    for (int i = 0; DllSetupDiEnumDeviceInfo(hDevInfo, i, pspDevInfoData); i++) {
      DWORD nSize = 0;
      TCHAR buf[MAX_PATH];

      if (!DllSetupDiGetDeviceInstanceId(hDevInfo, pspDevInfoData, buf, sizeof(buf), &nSize)) {
        break;
      }
      normalizeSlashes(buf);

      if (szDevId == buf) {
        DWORD DataT;
        DWORD nSize;
        DllSetupDiGetDeviceRegistryProperty(hDevInfo, pspDevInfoData, SPDRP_LOCATION_INFORMATION, &DataT, (PBYTE)buf,
                                            MAX_PATH, &nSize);
        DllSetupDiGetDeviceRegistryProperty(hDevInfo, pspDevInfoData, SPDRP_HARDWAREID, &DataT,
                                            (PBYTE)(buf + nSize - 1), MAX_PATH - nSize, &nSize);
        auto vidPid = extractVidPid(hDevInfo, pspDevInfoData, buf, MAX_PATH);

        if (!DeviceDetection::isValidVidPid(vidPid)) {
          continue;
        }

        string portName = findPortName(hDevInfo, pspDevInfoData);
        if (portName.length()) {
          lastDetectedCandidate.path = COM_PATH_PREFIX + portName;
        }
        lastDetectedCandidate.vid = vidPid.vid;
        lastDetectedCandidate.pid = vidPid.pid;
        break;
      }
    }

    HeapFree(GetProcessHeap(), 0, pspDevInfoData);
  }

  if (hDevInfo) {
    DllSetupDiDestroyDeviceInfoList(hDevInfo);
  }
}

LRESULT CALLBACK DeviceDetection::Impl::monitorDetectCallback(HWND m_hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  if (msg == WM_DEVICECHANGE) {
    if (DBT_DEVICEARRIVAL == wParam || DBT_DEVICEREMOVECOMPLETE == wParam) {
      PDEV_BROADCAST_HDR pHdr = (PDEV_BROADCAST_HDR)lParam;
      PDEV_BROADCAST_DEVICEINTERFACE pDevInf;
      if (pHdr->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE) {
        pDevInf = (PDEV_BROADCAST_DEVICEINTERFACE)pHdr;
        checkDetectedDeviceEvent(pDevInf, wParam);
        if (m_hwnd && !PostMessageA(m_hwnd, APP_DEVICE_EVENT_MSG, 0, 0)) {
          throw runtime_error("Unable to PostMessageA");
        }
      }
    }
    // Return TRUE to grant the request.
    return 1;
  }

  return DefWindowProc(m_hwnd, msg, wParam, lParam);
}

VidPid DeviceDetection::Impl::extractVidPid(HDEVINFO hDevInfo, SP_DEVINFO_DATA* pspDevInfoData, TCHAR* buf, DWORD buffSize) {
  DWORD DataT = 0;
  DWORD nSize = 0;
  VidPid vidPid = {0, 0};

  if (DllSetupDiGetDeviceRegistryProperty(hDevInfo, pspDevInfoData, SPDRP_HARDWAREID, &DataT, (PBYTE)buf, buffSize, &nSize) && buf) {
    // buf - "USB\\VID_1C4F&PID_0026&REV_0110"
    charsToUpper(buf);
    char* temp = nullptr;
    char *pidStr = nullptr, *vidStr = nullptr;

    char* string = new char[strlen(buf) + 1];
    memcpy(string, buf, strlen(buf) + 1);

    vidStr = strstr(string, VID_TAG);
    pidStr = strstr(string, PID_TAG);

    if (vidStr != NULL) {
      temp = (char*)(vidStr + strlen(VID_TAG));
      temp[4] = '\0';
      vidPid.vid = strtol(temp, NULL, 16);
    }

    if (pidStr != NULL) {
      temp = (char*)(pidStr + strlen(PID_TAG));
      temp[4] = '\0';
      vidPid.pid = strtol(temp, NULL, 16);
    }
    delete string;
  }

  return vidPid;
}

void DeviceDetection::Impl::loadFunctions() {
  m_hInstLib = LoadLibrary(LIBRARY_NAME);
  string err = "Could not load library functions from dll: " LIBRARY_NAME;

  if (!m_hInstLib) {
    throw runtime_error(err);
  }

  DllSetupDiEnumDeviceInfo = (_SetupDiEnumDeviceInfo)GetProcAddress(m_hInstLib, "SetupDiEnumDeviceInfo");
  DllSetupDiGetClassDevs = (_SetupDiGetClassDevs)GetProcAddress(m_hInstLib, "SetupDiGetClassDevsA");
  DllSetupDiDestroyDeviceInfoList =
    (_SetupDiDestroyDeviceInfoList)GetProcAddress(m_hInstLib, "SetupDiDestroyDeviceInfoList");
  DllSetupDiGetDeviceInstanceId =
    (_SetupDiGetDeviceInstanceId)GetProcAddress(m_hInstLib, "SetupDiGetDeviceInstanceIdA");
  DllSetupDiGetDeviceRegistryProperty =
    (_SetupDiGetDeviceRegistryProperty)GetProcAddress(m_hInstLib, "SetupDiGetDeviceRegistryPropertyA");
  DllSetupDiOpenDevRegKey = (_SetupDiOpenDevRegKey)GetProcAddress(m_hInstLib, "SetupDiOpenDevRegKey");

  if (!(DllSetupDiEnumDeviceInfo != nullptr && DllSetupDiGetClassDevs != nullptr && DllSetupDiDestroyDeviceInfoList != nullptr &&
        DllSetupDiGetDeviceInstanceId != nullptr && DllSetupDiGetDeviceRegistryProperty != nullptr)) {
    throw runtime_error(err);
  }
}

// DeviceDetection

DeviceDetection::DeviceDetection(IOContext context) : m_impl(new Impl(&this->callback, context)) {
}

DeviceDetection::~DeviceDetection() {
}

void DeviceDetection::start() {
  m_impl->detectConnected();
  m_impl->startMonitoring();
}

void DeviceDetection::stop() {
  m_impl->stopMonitoring();
}

// Utils

static void charsToUpper(char* buf) {
  char* c = buf;
  while (*c != '\0') {
    *c = toupper((unsigned char)*c);
    c++;
  }
}

static void normalizeSlashes(char* buf) {
  char* c = buf;
  while (*c != '\0') {
    if (*c == '/')
      *c = '\\';
    c++;
  }
}

string UTF8Encode(const string& str) {
  if (str.empty()) {
    return std::string();
  }

  // System default code page to wide character
  int wstr_size = MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);
  std::wstring wstr_tmp(wstr_size, 0);
  MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, &wstr_tmp[0], wstr_size);

  // Wide character to Utf8
  int str_size = WideCharToMultiByte(CP_UTF8, 0, &wstr_tmp[0], (int)wstr_tmp.size(), NULL, 0, NULL, NULL);
  std::string str_utf8(str_size, 0);
  WideCharToMultiByte(CP_UTF8, 0, &wstr_tmp[0], (int)wstr_tmp.size(), &str_utf8[0], str_size, NULL, NULL);

  return str_utf8;
}

#endif
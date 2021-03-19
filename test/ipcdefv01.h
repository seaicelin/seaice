#ifndef SEAICE_IPCDEFV01_H
#define SEAICE_IPCDEFV01_H

#include <stdint.h>

typedef uint16_t _WORD;
typedef uint8_t  _byte;
typedef uint8_t  _BYTE;
typedef uint32_t _DWORD;
typedef uint64_t _UINT64;


enum ResultType
{
	LOG_SUCCESS = 100,
	LOG_FAILED = 99
};

enum CmdType
{
	LOG_IN = 0,
	LOG_OUT = 1,
	LOG_IN_RESULT = 2,
	LOG_OUT_RESULT = 3
};

typedef struct
{
	_WORD len;
	_byte cmd;
} ipc_hdr_type;


typedef struct LogIn_
{
	ipc_hdr_type hdr;
	char username[100];
	char password[100];

	LogIn_()
	{
		hdr.len = sizeof(LogIn_);
		hdr.cmd = LOG_IN;
	}
}LogIn;


typedef struct LogOut_
{
	ipc_hdr_type hdr;
	char username[100];

	LogOut_()
	{
		hdr.len = sizeof(LogOut_);
		hdr.cmd = LOG_OUT;
	}
}LogOut;

typedef struct LogInResult_
{
	ipc_hdr_type hdr;
	_byte result;
	LogInResult_()
	{
		hdr.len = sizeof(LogInResult_);
		hdr.cmd = LOG_IN_RESULT;
	}
}LogInResult;

typedef struct LogOutResult_
{
	ipc_hdr_type hdr;
	_byte result;
	LogOutResult_()
	{
		hdr.len = sizeof(LogOutResult_);
		hdr.cmd = LOG_OUT_RESULT;
	}
}LogOutResult;


#endif
//  Copyright (c) 2006-2009 Jonas Persson
//
//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying _pFile LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt
//
//  See http://www.boost.org
//  for more information

// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#include "Malterlib_Contract.h"

namespace NMib::NContract
{
#if defined DMibContract_AnyEnabled
	DMibImpErrorClassImplement(CContractException);
	DMibImpErrorClassImplement(CContractException_Require);
	DMibImpErrorClassImplement(CContractException_Ensure);
	DMibImpErrorClassImplement(CContractException_Check);
	DMibImpErrorClassImplement(CContractException_Invariant);
	DMibImpErrorClassImplement(CContractException_NeverGetHere);
#endif
}

void fg_LogViolation
(
	NMib::NStr::CStrNonTracked &_String,
	const ch8 *_pType, 
	const ch8 *_pFile, 
	int32 _Line, 
	const ch8 *_pFunction, 
	const ch8 *_pVariables, 
	const ch8 *_pStatement,
	bool _bCallstack,
	const ch8 *_pClassName = 0
)
{
	using namespace NMib::NStr;
	_String += CStrNonTracked::CFormat("{} violation:" DMibNewLine) << _pType;
	if (_pStatement) 
		_String += CStrNonTracked::CFormat("    Statement:{\n}        {}" DMibNewLine) << _pStatement;
	if (*_pVariables) 
		_String += CStrNonTracked::CFormat("    Where:{\n}        {}" DMibNewLine) << _pVariables;
	if (_pClassName) 
		_String += CStrNonTracked::CFormat("    Class:{\n}        {}" DMibNewLine) << _pClassName;

	_String += CStrNonTracked::CFormat("    Location:{\n}        Function: {}" DMibNewLine) << _pFunction;
	_String += CStrNonTracked::CFormat("        File: {}" DMibNewLine) << _pFile;
	_String += CStrNonTracked::CFormat("        Line: {}" DMibNewLine) << _Line;
}

#ifdef DMibContract_AnyEnabled

namespace NMib::NContract
{
	NStr::CStrNonTracked fg_GetLastContractMessageLockHeld();
}

constinit NMib::NThread::CLowLevelLockAggregate g_LastContractLock = {DAggregateInit};
int32 g_LastContractLine = 0;
ch8 g_LastContractFile[1024] = {0};
ch8 g_LastContractType[1024] = {0};
ch8 g_LastContractStatement[1024] = {0};
ch8 g_LastContractDetailed[1024] = {0};
bool g_LastContractValid = false;

inline_never bool fg_FailContract(const ch8 *_pFile, int32 _Line, const ch8 *_pContractType, const ch8 *_pStatement, NMib::NStr::CStrNonTracked const *_pDetailed)
{
	NMib::NStr::CStrNonTracked Message;
	if (_pStatement)
		Message = NMib::NStr::CStrNonTracked::CFormat("Contract violation: {} ({})") << _pContractType << _pStatement;
	else
		Message = NMib::NStr::CStrNonTracked::CFormat("Contract violation: {}") << _pContractType;

	NMib::NStr::CStrNonTracked LastContractMessage;
	{
		DMibLock(g_LastContractLock);
		g_LastContractLine = _Line;
		if (_pFile)
			NMib::NStr::fg_StrCopy(g_LastContractFile, _pFile, 1024);
		else
			g_LastContractFile[0] = 0;
		if (_pContractType)
			NMib::NStr::fg_StrCopy(g_LastContractType, _pContractType, 1024);
		else
			g_LastContractType[0] = 0;
		if (_pStatement)
			NMib::NStr::fg_StrCopy(g_LastContractStatement, _pStatement, 1024);
		else
			g_LastContractStatement[0] = 0;

		if (_pDetailed)
			NMib::NStr::fg_StrCopy(g_LastContractDetailed, _pDetailed->f_GetStr(), 1024);
		else
			g_LastContractDetailed[0] = 0;
		g_LastContractValid = true;
		LastContractMessage = NMib::NContract::fg_GetLastContractMessageLockHeld();
		g_LastContractValid = false;
	}
	switch (NMib::NSys::fg_Debug_ReportContractFailure(_pFile, _Line, NMib::NMisc::fg_FunctionPtrToVoidPtr(&fg_FailContract), Message))
	{
	case NMib::EDebugContractFailureAction_NotHandled:
	case NMib::EDebugContractFailureAction_Break:
		{
			if (_pStatement)
				DMibTrace(DMibPFileLineFormat " Contract violation: {} ({})" DMibNewLine, _pFile << _Line << _pContractType << _pStatement);
			else
				DMibTrace(DMibPFileLineFormat " Contract violation: {}" DMibNewLine, _pFile << _Line << _pContractType);
			NMib::NSys::fg_System_ReportContractViolation(LastContractMessage);
			{
				DMibLock(g_LastContractLock);
				g_LastContractValid = true;
			}
			return true;
		}
		break;
	case NMib::EDebugContractFailureAction_Abort:
		{
			NMib::NSys::fg_System_ExitProcess(211);
		}
		break;
	case NMib::EDebugContractFailureAction_Continue:
		{
		}
		break;
	}
	return false;
}
#endif

namespace NMib::NContract
{
#ifdef DMibContract_AnyEnabled
	CContractBreak g_ContractBreak = {};

	NStr::CStrNonTracked fg_GetLastContractMessage()
	{
		DMibLock(g_LastContractLock);
		return fg_GetLastContractMessageLockHeld();
	}

	NStr::CStrNonTracked fg_GetLastContractMessageLockHeld()
	{
		if (g_LastContractValid)
		{
			if (*g_LastContractStatement)
				return NStr::CStrNonTracked::CFormat("{}({}): Contract violation: {} ({})" DMibNewLine "{}") << g_LastContractFile << g_LastContractLine << g_LastContractType << g_LastContractStatement << g_LastContractDetailed;
			else
				return NStr::CStrNonTracked::CFormat("{}({}): Contract violation: {}" DMibNewLine "{}") << g_LastContractFile << g_LastContractLine << g_LastContractType << g_LastContractDetailed;
		}
		return NStr::CStrNonTracked();
	}
#endif

	namespace NPrivate
	{
		constinit NStorage::TCAggregate<NThread::TCThreadLocal<TCAutoClearInt<bool,true>>, 64> g_Enabled = {DAggregateInit};
		bool g_bGlobalEnabled = true;

		void fg_EnableContractsInThisThread(bool enabled)
		{
			*(*g_Enabled) = enabled;
		}
		bool fg_ContractsEnabledInThisThread()
		{
			if (!g_bGlobalEnabled)
				return false;

			if (g_Enabled.f_WasDestructed())
				return true;
			return *(*g_Enabled);
		}
	}

	CContractViolation::CContractViolation
	(
		const ch8 * _pType,
		const ch8 * _pFile,
		int32 _Line,
		const ch8 * _pFunction,
		const ch8 * _pVariables,
		const ch8 * _pStatement,
		bool _bCallstack,
		const ch8 * _pClassName
	)
		: m_pType(_pType)
		, m_pFile(_pFile)
		, m_Line(_Line)
		, m_pFunction(_pFunction)
		, m_pVariables(_pVariables)
		, m_pStatement(_pStatement)
		//, callstack_(boost::none)
		, m_pClassName(_pClassName)
	{
		//if (_bCallstack) callstack_ = CallStack(5);
	}

	NStr::CStrNonTracked CContractViolation::f_ComposeLogMessage() const
	{
		NStr::CStrNonTracked Return;
		fg_LogViolation(Return, m_pType, m_pFile, m_Line, m_pFunction, m_pVariables, m_pStatement, false, m_pClassName);

		Return += NStr::CStrNonTracked::CFormat("Bug Id:{\n}    {}{\n}") << f_BugId();

		return Return;
	}

	uint32 CContractViolation::f_BugId() const { return (NMib::NStr::CStrNonTracked(m_pStatement) + NMib::NStr::CStrNonTracked(m_pFunction)).f_Hash(); }

	namespace
	{
		struct CContractLoggerThreadLocal
		{
			CContractLoggerThreadLocal() = default;
			CContractLoggerThreadLocal(CContractLoggerThreadLocal const &_Other)
			{
				DMibLock(_Other.m_Lock);
				m_fOnContractViolation = _Other.m_fOnContractViolation;
			}

			mutable NThread::CMutual m_Lock;
			NFunction::TCFunction<void(CContractViolation const&)> m_fOnContractViolation;
		};

		constinit NStorage::TCAggregate
			<
				NMib::NThread::TCThreadLocal<CContractLoggerThreadLocal, NMib::NMemory::CAllocator_Heap, NMib::NThread::EThreadLocalFlag_Inherit>
			>
			g_ContratLogger = {DAggregateInit}
		;
	}
	NFunction::TCFunction<void(CContractViolation const&)> fg_SetContractLogger(NFunction::TCFunction<void(CContractViolation const&)> const &_ContractLogger)
	{
		auto &TreadLocal = **g_ContratLogger;
		auto OldLogger = TreadLocal.m_fOnContractViolation;

		DMibLock(TreadLocal.m_Lock);
		TreadLocal.m_fOnContractViolation = _ContractLogger;
		return OldLogger;
	}

#if defined DMibContract_AnyEnabled
	NFunction::TCFunction<void(CContractViolation const&)> fg_SetContractLoggerThrowExceptions()
	{
		using namespace NMib::NStr;
		return fg_SetContractLogger
			(
				[] (CContractViolation const &_ContractViolation)
				{
					using namespace NMib::NStr;
					CStrNonTracked Message;
					if (*_ContractViolation.f_GetClassName())
					{
						if (*_ContractViolation.f_GetVariables())
							Message = CStrNonTracked::CFormat("{} in {} where:{}") << _ContractViolation.f_GetStatement() << _ContractViolation.f_GetClassName() << _ContractViolation.f_GetVariables();
						else
							Message = CStrNonTracked::CFormat("{} in {}") << _ContractViolation.f_GetStatement() << _ContractViolation.f_GetClassName();
					}
					else
					{
						if (*_ContractViolation.f_GetVariables())
							Message = CStrNonTracked::CFormat("{} where:{}") << _ContractViolation.f_GetStatement() << _ContractViolation.f_GetVariables();
						else
							Message = CStrNonTracked::CFormat("{}") << _ContractViolation.f_GetStatement();
					}
					if (fg_StrCmp(_ContractViolation.f_GetType(), "Require") == 0)
						throw CContractException_Require("CContractException_Require", _ContractViolation.f_GetFile(), _ContractViolation.f_GetLine(), _ContractViolation.f_GetFunction(), Message, true);
					else if (fg_StrCmp(_ContractViolation.f_GetType(), "Ensure") == 0)
						throw CContractException_Ensure("CContractException_Ensure", _ContractViolation.f_GetFile(), _ContractViolation.f_GetLine(), _ContractViolation.f_GetFunction(), Message, true);
					else if (fg_StrCmp(_ContractViolation.f_GetType(), "Check") == 0)
						throw CContractException_Check("CContractException_Check", _ContractViolation.f_GetFile(), _ContractViolation.f_GetLine(), _ContractViolation.f_GetFunction(), Message, true);
					else if (fg_StrCmp(_ContractViolation.f_GetType(), "Invariant") == 0)
						throw CContractException_Invariant("CContractException_Invariant", _ContractViolation.f_GetFile(), _ContractViolation.f_GetLine(), _ContractViolation.f_GetFunction(), Message, true);
					else if (fg_StrCmp(_ContractViolation.f_GetType(), "NeverGetHere") == 0)
						throw CContractException_NeverGetHere("CContractException_NeverGetHere", _ContractViolation.f_GetFile(), _ContractViolation.f_GetLine(), _ContractViolation.f_GetFunction(), Message, true);
					else
						throw CContractException("CContractException", _ContractViolation.f_GetFile(), _ContractViolation.f_GetLine(), _ContractViolation.f_GetFunction(), Message, true);
				}
			)
		;
	}
#endif

	namespace NPrivate
	{
#if defined DMibContract_AnyEnabled
		bool fg_HandleRequireViolation(const ch8 *_pFile, int32 _Line, const ch8 *_pFunction, const ch8 *_pVariables, const ch8 *_pStatement,bool _bCallstack)
		{
			if (!g_ContratLogger.f_WasDestructed() && !!(**g_ContratLogger).m_fOnContractViolation)
			{
				(**g_ContratLogger).m_fOnContractViolation(CContractViolation("Require",_pFile,_Line,_pFunction,_pVariables,_pStatement,false,""));
				return false;
			}
			NStr::CStrNonTracked String = DMibNewLine;
			fg_LogViolation(String,"Require",_pFile,_Line,_pFunction,_pVariables,_pStatement,_bCallstack);
			DMibTrace("{}" DMibNewLine, String);
			return fg_FailContract(_pFile, _Line, "DMibRequire", _pStatement, &String);
		}
#endif
#ifdef DMibContractConfigure_EnsureEnabled
		bool fg_HandleEnsureViolation(const ch8 *_pFile, int32 _Line, const ch8 *_pFunction, const ch8 *_pVariables, const ch8 *_pStatement,bool _bCallstack)
		{
			if (!g_ContratLogger.f_WasDestructed() && !!(**g_ContratLogger).m_fOnContractViolation)
			{
				(**g_ContratLogger).m_fOnContractViolation(CContractViolation("Ensure",_pFile,_Line,_pFunction,_pVariables,_pStatement,false,""));
				return false;
			}
			NStr::CStrNonTracked String = DMibNewLine;
			fg_LogViolation(String,"Ensure",_pFile,_Line,_pFunction,_pVariables,_pStatement,_bCallstack);
			DMibTrace("{}" DMibNewLine, String);
			return fg_FailContract(_pFile, _Line, "DMibEnsure", _pStatement, &String);
		}
#endif
#ifdef DMibContractConfigure_CheckEnabled
		bool fg_HandleCheckViolation(const ch8 *_pFile, int32 _Line, const ch8 *_pFunction, const ch8 *_pVariables, const ch8 *_pStatement,bool _bCallstack)
		{
			if (!g_ContratLogger.f_WasDestructed() && !!(**g_ContratLogger).m_fOnContractViolation)
			{
				(**g_ContratLogger).m_fOnContractViolation(CContractViolation("Check",_pFile,_Line,_pFunction,_pVariables,_pStatement,false,""));
				return false;
			}
			NStr::CStrNonTracked String = DMibNewLine;
			fg_LogViolation(String,"Check",_pFile,_Line,_pFunction,_pVariables,_pStatement,_bCallstack);
			DMibTrace("{}" DMibNewLine, String);
			return fg_FailContract(_pFile, _Line, "DMibCheck", _pStatement, &String);
		}
#endif
#ifdef DMibContractConfigure_InvariantEnabled
		bool fg_HandleInvariantViolation(const ch8 *_pFile, int32 _Line, const ch8 *_pFunction, const ch8 *_pVariables, const ch8 *_pStatement,const ch8 *_pClassName,bool _bCallstack)
		{
			if (!g_ContratLogger.f_WasDestructed() && !!(**g_ContratLogger).m_fOnContractViolation)
			{
				(**g_ContratLogger).m_fOnContractViolation(CContractViolation("Invariant",_pFile,_Line,_pFunction,_pVariables,_pStatement,false,_pClassName));
				return false;
			}
			NStr::CStrNonTracked String = DMibNewLine;
			fg_LogViolation(String,"Invariant",_pFile,_Line,_pFunction,_pVariables,_pStatement,_bCallstack,_pClassName);
			DMibTrace("{}" DMibNewLine, String);
			return fg_FailContract(_pFile, _Line, "DMibInvariant", _pStatement, &String);
		}
#endif
#ifdef DMibContractConfigure_NeverGetHereEnabled
		bool fg_HandleNeverGetHereViolation(const ch8 *_pFile, int32 _Line, const ch8 *_pFunction, const ch8 *_pVariables,bool _bCallstack)
		{
			if (!g_ContratLogger.f_WasDestructed() && !!(**g_ContratLogger).m_fOnContractViolation)
			{
				(**g_ContratLogger).m_fOnContractViolation(CContractViolation("NeverGetHere",_pFile,_Line,_pFunction,_pVariables,"",false,""));
				return false;
			}
			NStr::CStrNonTracked String = DMibNewLine;
			fg_LogViolation(String,"NeverGetHere",_pFile,_Line,_pFunction,_pVariables,0,_bCallstack);
			DMibTrace("{}" DMibNewLine, String);
			return fg_FailContract(_pFile, _Line, "DMibNeverGetHere", 0, &String);
		}
#endif
	}
}

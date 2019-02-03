//  Copyright (c) 2006-2009 Jonas Persson
//
//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt
//
//  See http://www.boost.org
//  for more information

// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

/* ========================================================================= *\

USAGE:
======

=== Contract asserts: === 
DRequire( condition );
Precondition.  The part of the contract the _Client must promise before calling a _pFunction.
Usage:
void foo(int32 _Expression) {
    DRequire( _Expression < 14 );
}

DEnsure( condition );
postcondition: The part the _pFunction promises if recondition is fullfilled and nu runtime failure(an exception) occured
Usage:
int32 foo(int32 _Expression) {
    int32 ret = ...;

    DEnsure( ret < 14 );
}

DCheck( condition );
Internal consistency fg_Check. Just a hotted assert.

DNeverGetHere;
Reachability guard. A note in the code stating that it should not be possible to reach that _Line.
Usage:
void foo(int32 _Expression) {
    switch(_Expression) {
        case 1: ... break;
        case 2: ... break;
        default  DNeverGetHere;
    }
}

DHandlingToDo( condition );
Error code fg_Check. Just a placeholder to show that there are unhandled error cases
Usage:
void foo() {
    BOOL ret = ::SystemCall();
    DHandlingToDo(ret);
}
DInvariant( condition );
See below.

=== Extra data === 
All contract asserts can be decorated with extra information
Add logging of data in case of violation:
DEnsure( ret != OK) (ret) (::GetLastError());
Will add "ret = 15; ::GetLasError() = 3014" to the log.
the typed to logged must have a stream operator implemented.
DEnsure( ret != OK) ("Ooops");
Will add "Note: Ooops" to the log.
Any number of extra data can be added for logging.

=== Verifying modifications === 
DOld (variable)
Accessing the stored value of a variable.
DOldDef (type,variable)
Storing the value of a variable.
DOldExpr (type, variable, expression)
Storing the value of an expression in variable.
Usage:
void foo(int32 _Expression)
{
    DOldDef(int32,_Expression);
    _Expression = ...;
    DEnsure( Old(_Expression) < _Expression );
}

=== Invariants === 
DValidateInvariant();
Called at entry in each non-private _pFunction to validate invariants

DValidateContruction();
Called at end of constructor to validate that the object is correctly initialized

DValidateDestruction();
Called at entry in the destructor to verify that the object is safe to destroy

DClassInvariants( class )
Declaration of the calss invariants

DBaseInvariants ( CContractBase )
CContractBase class validataion. Declared in DClassInvariants to propagate validation to base classes

DInvariant( condition )
CContractInvariant. Condition that must always be true during the lifetime of an object.

Usage:
class B {
    int32 _Expression;
    DClassInvariants(B) {
        DInvariant(_Expression < 14);
    }
  public:
    ...
};
class C {
    int32 y;
    DClassInvariants(C) {
        DBaseInvariants(B);
        DInvariant(y > 15);
    }
  public:
    ~C() {
        DValidateDestruction();
        ...;
    }
    C() {
        ...;
        DValidateContruction();
    }
    void foo() {
        DValidateInvariant();
    }
};

=== Configuration === 
DMibContractConfigure_Callstack
set to 1 if a callstack should be added to the contract log

DMibContractConfigure_RequireEnabled
Define if preconditions should be checked

DMibContractConfigure_EnsureEnabled
Define if postconditions should be checked

DMibContractConfigure_InvariantEnabled
Define if invariants should be checked

DMibContractConfigure_InvariantEnabled_Strong
Define if invariants should be checked on _pFunction entry as well as on exit

DMibContractConfigure_CheckEnabled
Define if internal asserts should be checked

DMibContractConfigure_NeverGetHereEnabled
Define if reachability should be checked

DMibContractConfigure_AllEnabled
Define to enable all contract checking

void fg_SetContractLogger(boost::_pFunction<void(CContractViolation const&)>);
void fg_SetContractLogger(void(*)(CContractViolation const&));
Set a customized handler for contract violations

\* ========================================================================= */


//#define DMibContractConfigure_AllEnabled
//#define DMibContractConfigure_InvariantEnabled_Strong

#ifndef DMibContractConfigure_Callstack // DMibContractConfigure_Callstack
# define DMibContractConfigure_Callstack 0
#endif
#if DMibContractConfigure_Callstack != 1 && DMibContractConfigure_Callstack != 0
# error "Incorrect value for DMibContractConfigure_Callstack"
#endif

#include <Mib/Function/Function>

namespace NMib::NContract
{
	//--------------------------------------------------------------------
	// Some helper classes
	//--------------------------------------------------------------------

	DMibImpErrorClassDefine(CContractException, NException::CDebugException);
	DMibImpErrorClassDefine(CContractException_Require, CContractException);
	DMibImpErrorClassDefine(CContractException_Ensure, CContractException);
	DMibImpErrorClassDefine(CContractException_Check, CContractException);
	DMibImpErrorClassDefine(CContractException_Invariant, CContractException);
	DMibImpErrorClassDefine(CContractException_NeverGetHere, CContractException);

	class CContractViolation
	{
		const ch8 * m_pType;
		const ch8 * m_pFile;
		int32 m_Line;
		const ch8 * m_pFunction;
		const ch8 * m_pVariables;
		const ch8 * m_pStatement;
		const ch8 * m_pClassName;
	public:
		CContractViolation(
			const ch8 * _pType,
			const ch8 * _pFile,
			int32 _Line,
			const ch8 * _pFunction,
			const ch8 * _pVariables,
			const ch8 * _pStatement,
			bint _bCallstack,
			const ch8 * _pClassName);

		NStr::CStrNonTracked f_ComposeLogMessage() const;
		//std::wstring composeLogMessageWithCallstack() const;
		uint32 f_BugId() const;
		//boost::optional<CallStack> const& callStack() const { return callstack_; }

		const ch8 * f_GetType() const
		{
			return m_pType;
		}
		const ch8 * f_GetFile() const
		{
			return m_pFile;
		}
		int32 f_GetLine() const
		{
			return m_Line;
		}
		const ch8 * f_GetFunction() const
		{
			return m_pFunction;
		}
		const ch8 * f_GetVariables() const
		{
			return m_pVariables;
		}
		const ch8 * f_GetStatement() const
		{
			return m_pStatement;
		}
		const ch8 * f_GetClassName() const
		{
			return m_pClassName;
		}
	};

	NFunction::TCFunction<void(CContractViolation const&)> fg_SetContractLogger(NFunction::TCFunction<void(CContractViolation const&)> const &_Logger);
	NFunction::TCFunction<void(CContractViolation const&)> fg_SetContractLoggerThrowExceptions();

	class CContractBreakpointEnablerScope
	{
	public:

		CContractBreakpointEnablerScope()
		{
			DMibThreadLocalScopeEnter;
			mp_fOldLogger = fg_SetContractLogger(NFunction::TCFunction<void(CContractViolation const&)>(&CContractBreakpointEnablerScope::fsp_DoDebugBreak));
		}

		~CContractBreakpointEnablerScope()
		{
			fg_SetContractLogger(mp_fOldLogger);
			DMibThreadLocalScopeExit;
		}

	private:

		NFunction::TCFunction<void(CContractViolation const&)> mp_fOldLogger;	// logger we reinstate at destruction
		static void fsp_DoDebugBreak(CContractViolation const&)
		{
			DMibPDebugBreak;
		}
	};

	namespace NPrivate
	{
#if defined DMibContract_AnyEnabled
		// Forward declarations.
		bool fg_HandleRequireViolation      ( const ch8 *_pFile, int32 _Line, const ch8 *_pFunction, const ch8 *_pVariables, const ch8 *_pStatement, bint _pCallstack);
		bool fg_HandleEnsureViolation       ( const ch8 *_pFile, int32 _Line, const ch8 *_pFunction, const ch8 *_pVariables, const ch8 *_pStatement, bint _pCallstack);
		bool fg_HandleCheckViolation        ( const ch8 *_pFile, int32 _Line, const ch8 *_pFunction, const ch8 *_pVariables, const ch8 *_pStatement, bint _pCallstack);
		bool fg_HandleInvariantViolation    ( const ch8 *_pFile, int32 _Line, const ch8 *_pFunction, const ch8 *_pVariables, const ch8 *_pStatement, const ch8 *_pClassName, bint _pCallstack);
		bool fg_HandleNeverGetHereViolation ( const ch8 *_pFile, int32 _Line, const ch8 *_pFunction, const ch8 *_pVariables, bint _pCallstack);

		void fg_EnableContractsInThisThread(bool);
		bool fg_ContractsEnabledInThisThread();

		inline_small bint fg_Check(bint cond)
		{
			return cond;
		}
		inline_small bool fg_Enabled()
		{
			return fg_ContractsEnabledInThisThread();
		}

		class CContractBase
		{
		protected:
			const ch8 *m_pFile;
			int32 m_Line;
			const ch8 *m_pFunction;
			const ch8 *m_pStatement;
			mutable NStr::CStrNonTracked m_Variables;
		public:
			CContractBase(const ch8 *_pFile, int32 _Line, const ch8 *_pFunction, const ch8 *_pStatement)
				: m_pFile(_pFile)
				, m_Line(_Line)
				, m_pFunction(_pFunction)
				, m_pStatement(_pStatement)
				, m_Variables()
			{
			}

			operator bool() const
			{
				return false;
			}

			template <typename tf_CVariable>
			bint f_LogVariable(const ch8 *_Description, tf_CVariable const& _Var) const
			{
				if (_Description[0] == '"')
				{
					if (m_Variables.f_IsEmpty())
						m_Variables += NStr::CStrNonTracked::CFormat("note: '{}'") << _Var;
					else
						m_Variables += NStr::CStrNonTracked::CFormat("{\n}        note: '{}'") << _Var;
				}
				else
				{
					if (m_Variables.f_IsEmpty())
						m_Variables += NStr::CStrNonTracked::CFormat("{} = {}") << _Description << _Var;
					else
						m_Variables += NStr::CStrNonTracked::CFormat("{\n}        {} = {}") << _Description << _Var;
				}
				return false;
			}
		}; // CContractBase -----------------------------------------------------------------

		struct CContractRequire : CContractBase
		{
			CContractRequire(const ch8 *_pFile, int32 _Line, const ch8 *_pFunction, const ch8 *_pStatement)
				: CContractBase(_pFile, _Line, _pFunction, _pStatement)
			{
			}
			bool operator = (bool) const
			{
				if (fg_HandleRequireViolation(m_pFile, m_Line, m_pFunction, m_Variables.f_GetStr(), m_pStatement, DMibContractConfigure_Callstack))
					return true;
				return false;
			}
		}; // DMibRequire --------------------------------------------------------------

		struct CContractEnsure : CContractBase
		{
			CContractEnsure(const ch8 *_pFile, int32 _Line, const ch8 *_pFunction, const ch8 *_pStatement)
				: CContractBase(_pFile, _Line, _pFunction, _pStatement)
			{
			}

			bool operator = (bool) const
			{
				if (fg_HandleEnsureViolation(m_pFile, m_Line, m_pFunction, m_Variables.f_GetStr(), m_pStatement, DMibContractConfigure_Callstack))
					return true;
				return false;
			}
		}; // DMibEnsure ---------------------------------------------------------------

		struct CContractCheck : CContractBase
		{
			CContractCheck(const ch8 *_pFile, int32 _Line, const ch8 *_pFunction, const ch8 *_pStatement)
				: CContractBase(_pFile, _Line, _pFunction, _pStatement)
			{
			}

			bool operator = (bool) const
			{
				if (fg_HandleCheckViolation(m_pFile, m_Line, m_pFunction, m_Variables.f_GetStr(), m_pStatement, DMibContractConfigure_Callstack))
					return true;
				return false;
			}
		}; // DMibCheck ----------------------------------------------------------------

		struct CContractInvariant : CContractBase
		{
			CContractInvariant(const ch8 *_pFile, int32 _Line, const ch8 *_pFunction, const ch8 *_pClassName, const ch8 *_pStatement)
				: CContractBase(_pFile, _Line, _pFunction, _pStatement)
			{
			}

			bool operator = (bool) const
			{
				if (fg_HandleInvariantViolation(m_pFile, m_Line, m_pFunction, m_Variables.f_GetStr(), m_pStatement, m_pClass, DMibContractConfigure_Callstack))
					return true;
				return false;
			}
		private:
			const ch8 *m_pClass;
		}; // CContractInvariant ----------------------------------------------------------------

		struct CContractNeverGetHere : CContractBase
		{
			CContractNeverGetHere(const ch8 *_pFile, int32 _Line, const ch8 *_pFunction)
				: CContractBase(_pFile, _Line, _pFunction, nullptr)
			{
			}

			bool operator = (bool) const
			{
				if (fg_HandleNeverGetHereViolation(m_pFile, m_Line, m_pFunction, m_Variables.f_GetStr(), DMibContractConfigure_Callstack))
					return true;
				return false;
			}
		}; // CContractNeverGetHere ---------------------------------------------------------

#endif
		// Helper for signalling that no return value will be created from DMibNeverGetHere
		struct CDisableReturnValue
		{
			CDisableReturnValue()
			{
			}
#ifdef DCompiler_MSVC
			function_does_not_return
#endif
			~CDisableReturnValue()
			{
			}
			operator bool() const
			{
				return false;
			}
		};
	}
#if defined DMibContractConfigure_InvariantEnabled
	namespace NPrivate
	{
		class CCounter
		{
		public:
			CCounter() : m_Counter(0) { }
			CCounter& operator++() { ++m_Counter; return *this; }
			CCounter& operator--() { --m_Counter; return *this; }
			bint operator==(int32 const& rhs) const { return m_Counter==rhs; }
		private:
			int32 m_Counter;
		};
		//	class InvariantMgrBase {};
		template <class t_CClass>
		class CInvariantManager// : public InvariantMgrBase
		{
		public:
			CInvariantManager(t_CClass const& _Client, bint _bCheckAtEntry, const ch8 *_pFile, int32 _Line, const ch8 *_pFunction)
				: m_Client(_Client)
				, m_File(_pFile)
				, m_Line(_Line)
				, m_pFunction(_pFunction)
			{
				++m_Client.f_GetContractCounterInstance();
				if (m_Client.f_GetContractCounterInstance() == 1 && _bCheckAtEntry)
					m_Client.f_ContractsCheckInvariants(m_File, m_Line, m_pFunction);
			}
			~CInvariantManager()
			{
				if (m_Client.f_GetContractCounterInstance() == 1)
					m_Client.f_ContractsCheckInvariants(m_File, m_Line, m_pFunction);
				--m_Client.f_GetContractCounterInstance();
			}

			typedef t_CClass CClient;
		private:
			CInvariantManager(const CInvariantManager &);
			const CInvariantManager& operator=(const CInvariantManager &);
			CClient const& m_Client;
			const ch8 *m_File;
			int32 m_Line;
			const ch8 *m_pFunction;
		};
	}
#endif
	class CEnableContracts
	{
	public:
		CEnableContracts(bool);
		~CEnableContracts();
	private:
		CEnableContracts(CEnableContracts const&);
		CEnableContracts& operator=(CEnableContracts const&);
		bool m_Old;
	};
#if defined DMibContract_AnyEnabled
	inline CEnableContracts::CEnableContracts(bool enable)
		: m_Old(NPrivate::fg_ContractsEnabledInThisThread())
	{
		NPrivate::fg_EnableContractsInThisThread(enable);
	}
	inline CEnableContracts::~CEnableContracts()
	{
		NPrivate::fg_EnableContractsInThisThread(m_Old);
	}

	NStr::CStrNonTracked fg_GetLastContractMessage();
#else
	inline CEnableContracts::CEnableContracts(bool enable)
		: m_Old(false)
	{
	}
	inline CEnableContracts::~CEnableContracts()
	{
	}
#endif

#ifdef DMibContract_AnyEnabled
	struct CContractBreak
	{
		inline_always void operator = (bool _bBreak)
		{
			if (_bBreak)
				DMibPDebugBreak;
		}
	};
	extern CContractBreak g_ContractBreak;
	// struct CBreak{inline_always CBreak(bool _bBreak){if(_bBreak)DMibPDebugBreak;}} Break
#endif
}

#if defined DMibContract_AnyEnabled
#define DMibContractRecursive
#else
#define DMibContractRecursive
#endif

//--------------------------------------------------------------------
// Definition of Pre-condition
//--------------------------------------------------------------------
#if defined DMibContractConfigure_RequireEnabled

#define DMibContractHelperRequireExpression_B(_Expression) DMibContractHelperRequireExpression_OP(_Expression, A)
#define DMibContractHelperRequireExpression_A(_Expression) DMibContractHelperRequireExpression_OP(_Expression, B)
#define DMibContractHelperRequireExpression_OP(_Expression, _Next) DMibContractHelperRequireExpression_B.f_LogVariable(#_Expression,(_Expression)) || DMibContractHelperRequireExpression_ ## _Next

#define DMibRequire(_Expression) \
	if ( !NMib::NContract::NPrivate::fg_Enabled() || NMib::NContract::NPrivate::fg_Check(!!(_Expression)) ); else \
	if ( NMib::NContract::NPrivate::CContractRequire const DMibContractHelperRequireExpression_B = \
	NMib::NContract::NPrivate::CContractRequire(DMibPFile, DMibPLine, DMibPFunctionSignature, #_Expression)); else \
	if ( NMib::NContract::NPrivate::CContractRequire const& DMibContractHelperRequireExpression_A = DMibContractHelperRequireExpression_B ); else \
	NMib::NContract::g_ContractBreak = DMibContractHelperRequireExpression_A = DMibContractHelperRequireExpression_A || DMibContractHelperRequireExpression_B || DMibContractHelperRequireExpression_A
#else
enum {DMibContractHelperRequireExpression_A, DMibContractHelperRequireExpression_B };
#define DMibContractHelperRequireExpression_B(_Expression) DMibContractHelperRequireExpression_OP(_Expression, A)
#define DMibContractHelperRequireExpression_A(_Expression) DMibContractHelperRequireExpression_OP(_Expression, B)
#define DMibContractHelperRequireExpression_OP(_Expression, _Next) DMibContractHelperRequireExpression_ ## _Next

#define DMibRequire(_Expression) if (false); else (void)DMibContractHelperRequireExpression_A
#endif 


//--------------------------------------------------------------------
// Definition of Post-condition
//--------------------------------------------------------------------
#if defined DMibContractConfigure_EnsureEnabled

#define DMibContractHelperEnsureExpression_B(_Expression) DMibContractHelperEnsureExpression_OP(_Expression, A)
#define DMibContractHelperEnsureExpression_A(_Expression) DMibContractHelperEnsureExpression_OP(_Expression, B)
#define DMibContractHelperEnsureExpression_OP(_Expression, _Next) DMibContractHelperEnsureExpression_B.f_LogVariable(#_Expression,(_Expression)) || DMibContractHelperEnsureExpression_ ## _Next

#define DMibEnsure(_Expression) \
	if (!NMib::NContract::NPrivate::fg_Enabled() || NMib::NContract::NPrivate::fg_Check(!!(_Expression)) ); else \
	if (NMib::NContract::NPrivate::CContractEnsure const DMibContractHelperEnsureExpression_B = NMib::NContract::NPrivate::CContractEnsure(DMibPFile, DMibPLine, DMibPFunctionSignature, #_Expression)); else \
	if (NMib::NContract::NPrivate::CContractEnsure const& DMibContractHelperEnsureExpression_A = DMibContractHelperEnsureExpression_B ); else \
	NMib::NContract::g_ContractBreak = DMibContractHelperEnsureExpression_A = DMibContractHelperEnsureExpression_A || DMibContractHelperEnsureExpression_B || DMibContractHelperEnsureExpression_A
#else
enum {DMibContractHelperEnsureExpression_A, DMibContractHelperEnsureExpression_B };
#define DMibContractHelperEnsureExpression_B(_Expression) DMibContractHelperEnsureExpression_OP(_Expression, A)
#define DMibContractHelperEnsureExpression_A(_Expression) DMibContractHelperEnsureExpression_OP(_Expression, B)
#define DMibContractHelperEnsureExpression_OP(_Expression, _Next) DMibContractHelperEnsureExpression_ ## _Next

#define DMibEnsure(_Expression) if (false); else (void)DMibContractHelperEnsureExpression_A
#endif 


//--------------------------------------------------------------------
// Definition of DMibContractOld
//--------------------------------------------------------------------
#if defined DMibContractConfigure_EnsureEnabled
# define	DMibContractOldExpr( TYPE, VAR, EXPR )	TYPE _old##VAR = EXPR
# define	DMibContractOldDef( TYPE, VAR )	TYPE _old##VAR = VAR
# define DMibContractOld( VAR )			_old##VAR
#else
# define	DMibContractOldExpr( TYPE, VAR, EXPR )
# define	DMibContractOldDef( TYPE, VAR )
# define DMibContractOld( VAR )
#endif 

//--------------------------------------------------------------------
// Definition of DMibCheck
//--------------------------------------------------------------------
#if defined DMibContractConfigure_CheckEnabled

#define DMibContractHelperCheckExpression_B(_Expression) DMibContractHelperCheckExpression_OP(_Expression, A)
#define DMibContractHelperCheckExpression_A(_Expression) DMibContractHelperCheckExpression_OP(_Expression, B)
#define DMibContractHelperCheckExpression_OP(_Expression, _Next) DMibContractHelperCheckExpression_B.f_LogVariable(#_Expression,(_Expression)) || DMibContractHelperCheckExpression_ ## _Next // gcc

#define DMibCheck(_Expression) \
	if (NMib::NContract::NPrivate::fg_Check(!NMib::NContract::NPrivate::fg_Enabled() || !!(_Expression)) ); \
	else if (NMib::NContract::NPrivate::CContractCheck const DMibContractHelperCheckExpression_B = \
	NMib::NContract::NPrivate::CContractCheck(DMibPFile, DMibPLine, DMibPFunctionSignature, #_Expression)); \
	else if (NMib::NContract::NPrivate::CContractCheck const& DMibContractHelperCheckExpression_A = DMibContractHelperCheckExpression_B ); \
	else NMib::NContract::g_ContractBreak = DMibContractHelperCheckExpression_A = DMibContractHelperCheckExpression_A || DMibContractHelperCheckExpression_B || DMibContractHelperCheckExpression_A // gcc
#else
enum {DMibContractHelperCheckExpression_A, DMibContractHelperCheckExpression_B };
#define DMibContractHelperCheckExpression_B(_Expression) DMibContractHelperCheckExpression_OP(_Expression, A)
#define DMibContractHelperCheckExpression_A(_Expression) DMibContractHelperCheckExpression_OP(_Expression, B)
#define DMibContractHelperCheckExpression_OP(_Expression, _Next) DMibContractHelperCheckExpression_ ## _Next

#define DMibCheck(_Expression) if (false); else (void)DMibContractHelperCheckExpression_A
#endif 


//--------------------------------------------------------------------
// Definition of DMibErrorHandlingToDo
//--------------------------------------------------------------------
#if defined DMibContractConfigure_CheckEnabled

#define DMibContractHelperErrorHandlingToDoExpression_B(_Expression) DMibContractHelperErrorHandlingToDoExpression_OP(_Expression, A)
#define DMibContractHelperErrorHandlingToDoExpression_A(_Expression) DMibContractHelperErrorHandlingToDoExpression_OP(_Expression, B)
#define DMibContractHelperErrorHandlingToDoExpression_OP(_Expression, _Next) DMibContractHelperErrorHandlingToDoExpression_B.f_LogVariable(#_Expression,(_Expression)) || DMibContractHelperErrorHandlingToDoExpression_ ## _Next // gcc

#define DMibErrorHandlingToDo(_Expression) \
	if (!NMib::NContract::NPrivate::fg_Enabled() || NMib::NContract::NPrivate::fg_Check(!!(_Expression)) ); \
	else if (NMib::NContract::NPrivate::CContractCheck const DMibContractHelperErrorHandlingToDoExpression_B = \
	NMib::NContract::NPrivate::CContractCheck(DMibPFile, DMibPLine, DMibPFunctionSignature, #_Expression)); \
	else if (NMib::NContract::NPrivate::CContractCheck const& DMibContractHelperErrorHandlingToDoExpression_A = DMibContractHelperErrorHandlingToDoExpression_B ); \
	else NMib::NContract::g_ContractBreak = DMibContractHelperErrorHandlingToDoExpression_A = DMibContractHelperErrorHandlingToDoExpression_A || DMibContractHelperErrorHandlingToDoExpression_B || DMibContractHelperErrorHandlingToDoExpression_A // gcc
#else
enum {DMibContractHelperErrorHandlingToDoExpression_A, DMibContractHelperErrorHandlingToDoExpression_B };
#define DMibContractHelperErrorHandlingToDoExpression_B(_Expression) DMibContractHelperErrorHandlingToDoExpression_OP(_Expression, A)
#define DMibContractHelperErrorHandlingToDoExpression_A(_Expression) DMibContractHelperErrorHandlingToDoExpression_OP(_Expression, B)
#define DMibContractHelperErrorHandlingToDoExpression_OP(_Expression, _Next) DMibContractHelperErrorHandlingToDoExpression_ ## _Next

#define DMibErrorHandlingToDo(_Expression) if (false); else (void)DMibContractHelperErrorHandlingToDoExpression_A
#endif 

//--------------------------------------------------------------------
// Definition of Never-get-here
//--------------------------------------------------------------------
#if defined DMibContractConfigure_NeverGetHereEnabled

#define DMibContractHelperNeverGetHereExpression_B(_Expression) DMibContractHelperNeverGetHereExpression_OP(_Expression, A)
#define DMibContractHelperNeverGetHereExpression_A(_Expression) DMibContractHelperNeverGetHereExpression_OP(_Expression, B)
#define DMibContractHelperNeverGetHereExpression_OP(_Expression, _Next) DMibContractHelperNeverGetHereExpression_B.f_LogVariable(#_Expression,(_Expression)) || DMibContractHelperNeverGetHereExpression_ ## _Next

#define DMibNeverGetHere \
	if (NMib::NContract::NPrivate::CDisableReturnValue const DisableReturnValue = NMib::NContract::NPrivate::CDisableReturnValue()); else \
	if (!NMib::NContract::NPrivate::fg_Enabled() || NMib::NContract::NPrivate::fg_Check(false) ); else \
	if (NMib::NContract::NPrivate::CContractNeverGetHere const DMibContractHelperNeverGetHereExpression_B = NMib::NContract::NPrivate::CContractNeverGetHere(DMibPFile,DMibPLine,DMibPFunctionSignature)); else \
	if (NMib::NContract::NPrivate::CContractNeverGetHere const& DMibContractHelperNeverGetHereExpression_A = DMibContractHelperNeverGetHereExpression_B ); \
	else NMib::NContract::g_ContractBreak = DMibContractHelperNeverGetHereExpression_A = DMibContractHelperNeverGetHereExpression_A || DMibContractHelperNeverGetHereExpression_B || DMibContractHelperNeverGetHereExpression_A // gcc
#else
enum {DMibContractHelperNeverGetHereExpression_A, DMibContractHelperNeverGetHereExpression_B };
#define DMibContractHelperNeverGetHereExpression_B(_Expression) DMibContractHelperNeverGetHereExpression_OP(_Expression, A)
#define DMibContractHelperNeverGetHereExpression_A(_Expression) DMibContractHelperNeverGetHereExpression_OP(_Expression, B)
#define DMibContractHelperNeverGetHereExpression_OP(_Expression, _Next) DMibContractHelperNeverGetHereExpression_ ## _Next

#define DMibNeverGetHere if (NMib::NContract::NPrivate::CDisableReturnValue const DisableReturnValue = NMib::NContract::NPrivate::CDisableReturnValue()); else (void)DMibContractHelperNeverGetHereExpression_A
#endif 

//--------------------------------------------------------------------
// Definitions for invariants
//--------------------------------------------------------------------
#if defined DMibContractConfigure_InvariantEnabled
#define	DMibClassInvariants( Type )		\
	private: \
	typedef NMib::NContract::NPrivate::CInvariantManager<Type> CContractsConfigure_InvariantEnabledChecker; \
	friend class NMib::NContract::NPrivate::CInvariantManager<Type>; \
	mutable NMib::NContract::NPrivate::CCounter m_ContractsCounterInstance; \
	virtual NMib::NContract::NPrivate::CCounter& f_GetContractCounterInstance() const { return m_ContractsCounterInstance; } \
	static char const *_class_name() { return #Type; } \
	protected: \
	virtual void f_ContractsCheckInvariants(const ch8 *_pFile, int32 _Line, const ch8 *_pFunction) const

#define	DMibContractHelperCheckInvariant(AtEntry) \
	CContractsConfigure_InvariantEnabledChecker DMibContractConfigure_InvariantEnabledMgr_Instance(*this, AtEntry, DMibPFile, DMibPLine, DMibPFunctionSignature);

// Invoke f_ContractsCheckInvariants in parent class
#define	DMibBaseInvariant( classType )	classType::f_ContractsCheckInvariants(_pFile, _Line, _pFunction);

// Definitions of the macros used to DMibCheck f_ContractsCheckInvariants conditions.
#define DMibValidateConstruction( ) DMibContractHelperCheckInvariant( 0 )
#define DMibContractValidateDestruction( ) f_ContractsCheckInvariants(m_File, m_Line, m_pFunction);

#ifdef DMibContractConfigure_InvariantEnabled_Strong
#define DMibValidateInvariant( ) DMibContractHelperCheckInvariant( 1 )
#else
#define DMibValidateInvariant( ) DMibContractHelperCheckInvariant( 0 )
#endif
#else 
#define DMibClassInvariants( Type ) \
private: \
	void _not_used_invariant() const
#define	DMibBaseInvariant( classType )
#define	DMibContractHelperCheckInvariant(AtEntry)
#define DMibValidateConstruction( ) 
#define DMibValidateInvariant( )
#endif

#if defined DMibContractConfigure_InvariantEnabled

#define DMibContractHelperInvariantExpression_B(_Expression) DMibContractHelperInvariantExpression_OP(_Expression, A)
#define DMibContractHelperInvariantExpression_A(_Expression) DMibContractHelperInvariantExpression_OP(_Expression, B)
#define DMibContractHelperInvariantExpression_OP(_Expression, _Next) DMibContractHelperInvariantExpression_B.f_LogVariable(#_Expression,(_Expression)) || DMibContractHelperInvariantExpression_ ## _Next

#define DMibInvariant(_Expression) \
	if (!NMib::NContract::NPrivate::fg_Enabled() || NMib::NContract::NPrivate::fg_Check(!!(_Expression)) ); else \
	if (NMib::NContract::NPrivate::CContractInvariant const DMibContractHelperInvariantExpression_B = \
	NMib::NContract::NPrivate::CContractInvariant(_pFile, _Line, _pFunction,_class_name(), #_Expression)); else \
	if (NMib::NContract::NPrivate::CContractInvariant const& DMibContractHelperInvariantExpression_A = DMibContractHelperInvariantExpression_B ); else \
	NMib::NContract::g_ContractBreak = DMibContractHelperInvariantExpression_B = DMibContractHelperInvariantExpression_A || DMibContractHelperInvariantExpression_B || DMibContractHelperInvariantExpression_A
#else
enum {DMibContractHelperInvariantExpression_A, DMibContractHelperInvariantExpression_B };
#define DMibContractHelperInvariantExpression_B(_Expression) DMibContractHelperInvariantExpression_OP(_Expression, A)
#define DMibContractHelperInvariantExpression_A(_Expression) DMibContractHelperInvariantExpression_OP(_Expression, B)
#define DMibContractHelperInvariantExpression_OP(_Expression, _Next) DMibContractHelperInvariantExpression_ ## _Next

#define DMibInvariant(_Expression) if (false); else (void)DMibContractHelperInvariantExpression_A
#endif 


#define DMibContractBreakEnablerScope NMib::NContract::CContractBreakpointEnablerScope ContractsBreakpointEnablerScope



#ifndef DMibPNoShortCuts
	using namespace NMib::NContract;
#	define DValidateInvariant DMibValidateInvariant
#	define DClassInvariants DMibClassInvariants
#	define DBaseInvariants DMibBaseInvariant
#	define DInvariant      DMibInvariant
#	define DCheck          DMibCheck
#	define DEnsure         DMibEnsure
#	define DRequire        DMibRequire
#	define DNeverGetHere   DMibNeverGetHere
#	define DOld            DMibContractOld
#	define DOldDef         DMibContractOldDef
#	define DOldExpr        DMibContractOldExpr
#	define DHandlingToDo   DMibErrorHandlingToDo
#	define DBreakEnablerScope DMibContractBreakEnablerScope
#endif


// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// Copyright (c) 2006-2009 Jonas Persson
// SPDX-License-Identifier: BSL-1.0

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
postcondition: The part the _pFunction promises if recondition is fullfilled and no runtime failure(an exception) occured
Usage:
int32 foo(int32 _Expression) {
    int32 ret = ...;

    DEnsure( ret < 14 );
}

DCheck( condition );
Internal consistency check. Just a hotted assert.

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

=== Configuration ===
DMibContractConfigure_Callstack
set to 1 if a callstack should be added to the contract log

DMibContractConfigure_RequireEnabled
Define if preconditions should be checked

DMibContractConfigure_EnsureEnabled
Define if postconditions should be checked

DMibContractConfigure_CheckEnabled
Define if internal asserts should be checked

DMibContractConfigure_NeverGetHereEnabled
Define if reachability should be checked

DMibContractConfigure_AllEnabled
Define to enable all contract checking

void fg_SetContractLogger(boost::_pFunction<void(CContractViolation const &)>);
void fg_SetContractLogger(void(*)(CContractViolation const &));
Set a customized handler for contract violations

\* ========================================================================= */

//#define DMibContractConfigure_AllEnabled

#ifndef DMibContractConfigure_Callstack // DMibContractConfigure_Callstack
#	define DMibContractConfigure_Callstack 0
#endif
#if DMibContractConfigure_Callstack != 1 && DMibContractConfigure_Callstack != 0
#	error "Incorrect value for DMibContractConfigure_Callstack"
#endif

#include <Mib/Function/Function>

namespace NMib::NContract
{
	//--------------------------------------------------------------------
	// Some helper classes
	//--------------------------------------------------------------------

#if defined DMibContract_AnyEnabled
	DMibImpErrorClassDefine(CContractException, NException::CDebugException);
	DMibImpErrorClassDefine(CContractException_Require, CContractException);
	DMibImpErrorClassDefine(CContractException_Ensure, CContractException);
	DMibImpErrorClassDefine(CContractException_Check, CContractException);
	DMibImpErrorClassDefine(CContractException_NeverGetHere, CContractException);
#endif

	struct CContractViolation
	{
		CContractViolation
			(
				ch8 const *_pType
				, ch8 const *_pFile
				, int32 _Line
				, ch8 const *_pFunction
				, ch8 const *_pVariables
				, ch8 const *_pStatement
				, bool _bCallstack
				, ch8 const *_pClassName
			)
		;

		NStr::CStrNonTracked f_ComposeLogMessage() const;

		uint32 f_BugId() const;

		ch8 const *f_GetType() const
		{
			return mp_pType;
		}

		ch8 const *f_GetFile() const
		{
			return mp_pFile;
		}

		int32 f_GetLine() const
		{
			return mp_Line;
		}

		ch8 const *f_GetFunction() const
		{
			return mp_pFunction;
		}

		ch8 const *f_GetVariables() const
		{
			return mp_pVariables;
		}

		ch8 const *f_GetStatement() const
		{
			return mp_pStatement;
		}

		ch8 const *f_GetClassName() const
		{
			return mp_pClassName;
		}

	private:
		const ch8 * mp_pType;
		const ch8 * mp_pFile;
		const ch8 * mp_pFunction;
		const ch8 * mp_pVariables;
		const ch8 * mp_pStatement;
		const ch8 * mp_pClassName;
		int32 mp_Line;
	};

	NFunction::TCFunction<void(CContractViolation const &)> fg_SetContractLogger(NFunction::TCFunction<void(CContractViolation const &)> const &_Logger);
#if defined DMibContract_AnyEnabled
	NFunction::TCFunction<void(CContractViolation const &)> fg_SetContractLoggerThrowExceptions();
#endif

	struct CContractBreakpointEnablerScope
	{
		CContractBreakpointEnablerScope()
			: mp_fOldLogger(fg_SetContractLogger(NFunction::TCFunction<void(CContractViolation const&)>(&CContractBreakpointEnablerScope::fsp_DoDebugBreak)))
		{

		}

		~CContractBreakpointEnablerScope()
		{
			fg_SetContractLogger(mp_fOldLogger);
		}

	private:
		DMibThreadLocalScopeDebugMember;

		static void fsp_DoDebugBreak(CContractViolation const&)
		{
			DMibPDebugBreak;
		}

		NFunction::TCFunction<void(CContractViolation const &)> mp_fOldLogger;	// logger we reinstate at destruction
	};

	namespace NPrivate
	{
#if defined DMibContract_AnyEnabled
		// Forward declarations.
		bool fg_HandleRequireViolation(ch8 const *_pFile, int32 _Line, ch8 const *_pFunction, ch8 const *_pVariables, ch8 const *_pStatement, bool _pCallstack);
		bool fg_HandleEnsureViolation(ch8 const *_pFile, int32 _Line, ch8 const *_pFunction, ch8 const *_pVariables, ch8 const *_pStatement, bool _pCallstack);
		bool fg_HandleCheckViolation(ch8 const *_pFile, int32 _Line, ch8 const *_pFunction, ch8 const *_pVariables, ch8 const *_pStatement, bool _pCallstack);
		bool fg_HandleNeverGetHereViolation(ch8 const *_pFile, int32 _Line, ch8 const *_pFunction, ch8 const *_pVariables, bool _pCallstack);

		void fg_EnableContractsInThisThread(bool);
		bool fg_ContractsEnabledInThisThread();

		inline_small bool fg_Enabled()
		{
			return fg_ContractsEnabledInThisThread();
		}

		struct CContractBase
		{
			CContractBase(ch8 const *_pFile, int32 _Line, ch8 const *_pFunction, ch8 const *_pStatement)
				: mp_pFile(_pFile)
				, mp_Line(_Line)
				, mp_pFunction(_pFunction)
				, mp_pStatement(_pStatement)
				, mp_Variables()
			{
			}

			operator bool() const
			{
				return false;
			}

			template <typename tf_CVariable>
			bool f_LogVariable(ch8 const *_Description, tf_CVariable const &_Var) const
			{
				if (_Description[0] == '"')
					mp_Variables += NStr::CStrNonTracked::CFormat("{\n}    note: '{}'") << _Var;
				else
					mp_Variables += NStr::CStrNonTracked::CFormat("{\n}    {} = {}") << _Description << _Var;

				return false;
			}

		protected:
			const ch8 *mp_pFile;
			int32 mp_Line;
			const ch8 *mp_pFunction;
			const ch8 *mp_pStatement;
			mutable NStr::CStrNonTracked mp_Variables;
		};

		struct CContractRequire : CContractBase
		{
			CContractRequire(ch8 const *_pFile, int32 _Line, ch8 const *_pFunction, ch8 const *_pStatement) function_does_not_return_analyzer
				: CContractBase(_pFile, _Line, _pFunction, _pStatement)
			{
			}
			bool operator = (bool) const
			{
				if (fg_HandleRequireViolation(mp_pFile, mp_Line, mp_pFunction, mp_Variables.f_GetStr(), mp_pStatement, DMibContractConfigure_Callstack))
					return true;
				return false;
			}
		};

		struct CContractEnsure : CContractBase
		{
			CContractEnsure(ch8 const *_pFile, int32 _Line, ch8 const *_pFunction, ch8 const *_pStatement) function_does_not_return_analyzer
				: CContractBase(_pFile, _Line, _pFunction, _pStatement)
			{
			}

			bool operator = (bool) const
			{
				if (fg_HandleEnsureViolation(mp_pFile, mp_Line, mp_pFunction, mp_Variables.f_GetStr(), mp_pStatement, DMibContractConfigure_Callstack))
					return true;
				return false;
			}
		};

		struct CContractCheck : CContractBase
		{
			CContractCheck(ch8 const *_pFile, int32 _Line, ch8 const *_pFunction, ch8 const *_pStatement) function_does_not_return_analyzer
				: CContractBase(_pFile, _Line, _pFunction, _pStatement)
			{
			}

			bool operator = (bool) const
			{
				if (fg_HandleCheckViolation(mp_pFile, mp_Line, mp_pFunction, mp_Variables.f_GetStr(), mp_pStatement, DMibContractConfigure_Callstack))
					return true;
				return false;
			}
		};

		struct CContractNeverGetHere : CContractBase
		{
			CContractNeverGetHere(ch8 const *_pFile, int32 _Line, ch8 const *_pFunction) function_does_not_return_analyzer
				: CContractBase(_pFile, _Line, _pFunction, nullptr)
			{
			}

			bool operator = (bool) const
			{
				if (fg_HandleNeverGetHereViolation(mp_pFile, mp_Line, mp_pFunction, mp_Variables.f_GetStr(), DMibContractConfigure_Callstack))
					return true;
				return false;
			}
		};

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

	struct CEnableContracts
	{
		CEnableContracts(bool);
		~CEnableContracts();

	private:
		CEnableContracts(CEnableContracts const &);
		CEnableContracts &operator = (CEnableContracts const &);

		bool mp_bOldEnabled;
	};

#if defined DMibContract_AnyEnabled
	inline CEnableContracts::CEnableContracts(bool _bEnable)
		: mp_bOldEnabled(NPrivate::fg_ContractsEnabledInThisThread())
	{
		NPrivate::fg_EnableContractsInThisThread(_bEnable);
	}
	inline CEnableContracts::~CEnableContracts()
	{
		NPrivate::fg_EnableContractsInThisThread(mp_bOldEnabled);
	}

	NStr::CStrNonTracked fg_GetLastContractMessage();
#else
	inline CEnableContracts::CEnableContracts(bool enable)
		: mp_bOldEnabled(false)
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
	if ( !NMib::NContract::NPrivate::fg_Enabled() || expect_analyzer(!!(_Expression)) ); else \
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
	if (!NMib::NContract::NPrivate::fg_Enabled() || expect_analyzer(!!(_Expression)) ); else \
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
	if (!NMib::NContract::NPrivate::fg_Enabled() || expect_analyzer(!!(_Expression)) ); \
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
// Definition of Never-get-here
//--------------------------------------------------------------------
#if defined DMibContractConfigure_NeverGetHereEnabled

#define DMibContractHelperNeverGetHereExpression_B(_Expression) DMibContractHelperNeverGetHereExpression_OP(_Expression, A)
#define DMibContractHelperNeverGetHereExpression_A(_Expression) DMibContractHelperNeverGetHereExpression_OP(_Expression, B)
#define DMibContractHelperNeverGetHereExpression_OP(_Expression, _Next) DMibContractHelperNeverGetHereExpression_B.f_LogVariable(#_Expression,(_Expression)) || DMibContractHelperNeverGetHereExpression_ ## _Next

#define DMibNeverGetHere \
	if (NMib::NContract::NPrivate::CDisableReturnValue const DisableReturnValue = NMib::NContract::NPrivate::CDisableReturnValue()); else \
	if (!NMib::NContract::NPrivate::fg_Enabled()); else \
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

#define DMibContractBreakEnablerScope NMib::NContract::CContractBreakpointEnablerScope ContractsBreakpointEnablerScope

#ifndef DMibPNoShortCuts
	using namespace NMib::NContract;
#	define DCheck          DMibCheck
#	define DEnsure         DMibEnsure
#	define DRequire        DMibRequire
#	define DNeverGetHere   DMibNeverGetHere
#	define DOld            DMibContractOld
#	define DOldDef         DMibContractOldDef
#	define DOldExpr        DMibContractOldExpr
#	define DBreakEnablerScope DMibContractBreakEnablerScope
#endif


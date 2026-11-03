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

		ch8 const *f_GetType() const;
		ch8 const *f_GetFile() const;
		int32 f_GetLine() const;
		ch8 const *f_GetFunction() const;
		ch8 const *f_GetVariables() const;
		ch8 const *f_GetStatement() const;
		ch8 const *f_GetClassName() const;

	private:
		ch8 const *mp_pType;
		ch8 const *mp_pFile;
		ch8 const *mp_pFunction;
		ch8 const *mp_pVariables;
		ch8 const *mp_pStatement;
		ch8 const *mp_pClassName;
		int32 mp_Line;
	};

	auto fg_SetContractLogger(NFunction::TCFunction<void (CContractViolation const &_ContractViolation)> const &_fLogger)
		-> NFunction::TCFunction<void (CContractViolation const &_ContractViolation)>
	;
#if defined DMibContract_AnyEnabled
	NFunction::TCFunction<void (CContractViolation const &_ContractViolation)> fg_SetContractLoggerThrowExceptions();
#endif

	struct CContractBreakpointEnablerScope
	{
		CContractBreakpointEnablerScope();
		~CContractBreakpointEnablerScope();

	private:
		DMibThreadLocalScopeDebugMember;

		static void fsp_DoDebugBreak(CContractViolation const &_ContractViolation);

		NFunction::TCFunction<void (CContractViolation const &_ContractViolation)> mp_fOldLogger; // logger we reinstate at destruction
	};

	namespace NPrivate
	{
#if defined DMibContract_AnyEnabled
		// Forward declarations.
		bool fg_HandleRequireViolation(ch8 const *_pFile, int32 _Line, ch8 const *_pFunction, ch8 const *_pVariables, ch8 const *_pStatement, bool _bCallstack);
		bool fg_HandleEnsureViolation(ch8 const *_pFile, int32 _Line, ch8 const *_pFunction, ch8 const *_pVariables, ch8 const *_pStatement, bool _bCallstack);
		bool fg_HandleCheckViolation(ch8 const *_pFile, int32 _Line, ch8 const *_pFunction, ch8 const *_pVariables, ch8 const *_pStatement, bool _bCallstack);
		bool fg_HandleNeverGetHereViolation(ch8 const *_pFile, int32 _Line, ch8 const *_pFunction, ch8 const *_pVariables, bool _bCallstack);

		void fg_EnableContractsInThisThread(bool _bEnabled);
		bool fg_ContractsEnabledInThisThread();
		inline_small bool fg_Enabled();

		struct CContractBase
		{
			CContractBase(ch8 const *_pFile, int32 _Line, ch8 const *_pFunction, ch8 const *_pStatement);
			operator bool () const;

			template <typename tf_CVariable>
			bool f_LogVariable(ch8 const *_pDescription, tf_CVariable const &_Var) const;

		protected:
			ch8 const *mp_pFile;
			int32 mp_Line;
			ch8 const *mp_pFunction;
			ch8 const *mp_pStatement;
			mutable NStr::CStrNonTracked mp_Variables;
		};

		struct CContractRequire : CContractBase
		{
			CContractRequire(ch8 const *_pFile, int32 _Line, ch8 const *_pFunction, ch8 const *_pStatement) function_does_not_return_analyzer;
			bool operator = (bool _bVariablesLogged) const;
		};

		struct CContractEnsure : CContractBase
		{
			CContractEnsure(ch8 const *_pFile, int32 _Line, ch8 const *_pFunction, ch8 const *_pStatement) function_does_not_return_analyzer;
			bool operator = (bool _bVariablesLogged) const;
		};

		struct CContractCheck : CContractBase
		{
			CContractCheck(ch8 const *_pFile, int32 _Line, ch8 const *_pFunction, ch8 const *_pStatement) function_does_not_return_analyzer;
			bool operator = (bool _bVariablesLogged) const;
		};

		struct CContractNeverGetHere : CContractBase
		{
			CContractNeverGetHere(ch8 const *_pFile, int32 _Line, ch8 const *_pFunction) function_does_not_return_analyzer;
			bool operator = (bool _bVariablesLogged) const;
		};

#endif
		// Helper for signalling that no return value will be created from DMibNeverGetHere
		struct CDisableReturnValue
		{
			CDisableReturnValue();

#ifdef DCompiler_MSVC
			function_does_not_return
#endif
			~CDisableReturnValue();
			operator bool () const;
		};
	}

	struct CEnableContracts
	{
		CEnableContracts(bool _bEnable);
		~CEnableContracts();

	private:
		CEnableContracts(CEnableContracts const &_Other);
		CEnableContracts &operator = (CEnableContracts const &_Other);

		bool mp_bOldEnabled;
	};

#if defined DMibContract_AnyEnabled
	NStr::CStrNonTracked fg_GetLastContractMessage();
#endif

#ifdef DMibContract_AnyEnabled
	struct CContractBreak
	{
		inline_always void operator = (bool _bBreak);
	};

	extern CContractBreak g_ContractBreak;
	// struct CBreak{inline_always CBreak(bool _bBreak){if(_bBreak)DMibPDebugBreak;}} Break
#endif
}

#if defined DMibContract_AnyEnabled
#	define DMibContractRecursive
#else
#	define DMibContractRecursive
#endif

// The contract macros end in an expression that more variables to log are chained onto,
// 'DMibCheck(x)(a)(b)', so they are built of 'if'/'else' rather than a block. The switch
// makes each one statement of its own, so one guarded by a clause without braces leaves
// no 'else' for the clause's 'if' to take.
#define DMibContractStatement switch (0) case 0: default:

//--------------------------------------------------------------------
// Definition of Pre-condition
//--------------------------------------------------------------------
#if defined DMibContractConfigure_RequireEnabled

#	define DMibContractHelperRequireExpression_B(d_Expression) DMibContractHelperRequireExpression_OP(d_Expression, A)
#	define DMibContractHelperRequireExpression_A(d_Expression) DMibContractHelperRequireExpression_OP(d_Expression, B)
#	define DMibContractHelperRequireExpression_OP(d_Expression, d_Next) \
		DMibContractHelperRequireExpression_B.f_LogVariable(#d_Expression,(d_Expression)) || DMibContractHelperRequireExpression_ ## d_Next

#	define DMibRequire(d_Expression) \
		DMibContractStatement \
		if ( !NMib::NContract::NPrivate::fg_Enabled() || expect_analyzer(!!(d_Expression)) ); else \
		if ( NMib::NContract::NPrivate::CContractRequire const DMibContractHelperRequireExpression_B = \
		NMib::NContract::NPrivate::CContractRequire(DMibPFile, DMibPLine, DMibPFunctionSignature, #d_Expression)); else \
		if ( NMib::NContract::NPrivate::CContractRequire const& DMibContractHelperRequireExpression_A = DMibContractHelperRequireExpression_B ); else \
		NMib::NContract::g_ContractBreak = DMibContractHelperRequireExpression_A \
			= DMibContractHelperRequireExpression_A || DMibContractHelperRequireExpression_B || DMibContractHelperRequireExpression_A
#else
enum
{
	DMibContractHelperRequireExpression_A
	, DMibContractHelperRequireExpression_B
};

#	define DMibContractHelperRequireExpression_B(d_Expression) DMibContractHelperRequireExpression_OP(d_Expression, A)
#	define DMibContractHelperRequireExpression_A(d_Expression) DMibContractHelperRequireExpression_OP(d_Expression, B)
#	define DMibContractHelperRequireExpression_OP(d_Expression, d_Next) DMibContractHelperRequireExpression_ ## d_Next

#	define DMibRequire(d_Expression) DMibContractStatement if (false); else (void)DMibContractHelperRequireExpression_A
#endif

//--------------------------------------------------------------------
// Definition of Post-condition
//--------------------------------------------------------------------
#if defined DMibContractConfigure_EnsureEnabled

#	define DMibContractHelperEnsureExpression_B(d_Expression) DMibContractHelperEnsureExpression_OP(d_Expression, A)
#	define DMibContractHelperEnsureExpression_A(d_Expression) DMibContractHelperEnsureExpression_OP(d_Expression, B)
#	define DMibContractHelperEnsureExpression_OP(d_Expression, d_Next) \
		DMibContractHelperEnsureExpression_B.f_LogVariable(#d_Expression,(d_Expression)) || DMibContractHelperEnsureExpression_ ## d_Next

#	define DMibEnsure(d_Expression) \
		DMibContractStatement \
		if (!NMib::NContract::NPrivate::fg_Enabled() || expect_analyzer(!!(d_Expression)) ); else \
		if (NMib::NContract::NPrivate::CContractEnsure const DMibContractHelperEnsureExpression_B = \
		NMib::NContract::NPrivate::CContractEnsure(DMibPFile, DMibPLine, DMibPFunctionSignature, #d_Expression)); else \
		if (NMib::NContract::NPrivate::CContractEnsure const& DMibContractHelperEnsureExpression_A = DMibContractHelperEnsureExpression_B ); else \
		NMib::NContract::g_ContractBreak = DMibContractHelperEnsureExpression_A \
			= DMibContractHelperEnsureExpression_A || DMibContractHelperEnsureExpression_B || DMibContractHelperEnsureExpression_A
#else
enum
{
	DMibContractHelperEnsureExpression_A
	, DMibContractHelperEnsureExpression_B
};

#	define DMibContractHelperEnsureExpression_B(d_Expression) DMibContractHelperEnsureExpression_OP(d_Expression, A)
#	define DMibContractHelperEnsureExpression_A(d_Expression) DMibContractHelperEnsureExpression_OP(d_Expression, B)
#	define DMibContractHelperEnsureExpression_OP(d_Expression, d_Next) DMibContractHelperEnsureExpression_ ## d_Next

#	define DMibEnsure(d_Expression) DMibContractStatement if (false); else (void)DMibContractHelperEnsureExpression_A
#endif

//--------------------------------------------------------------------
// Definition of DMibContractOld
//--------------------------------------------------------------------
#if defined DMibContractConfigure_EnsureEnabled
#	define	DMibContractOldExpr( d_Type, d_Var, d_Expr )	d_Type _old##d_Var = d_Expr
#	define	DMibContractOldDef( d_Type, d_Var )	d_Type _old##d_Var = d_Var
#	define DMibContractOld( d_Var )			_old##d_Var
#else
#	define	DMibContractOldExpr( d_Type, d_Var, d_Expr )
#	define	DMibContractOldDef( d_Type, d_Var )
#	define DMibContractOld( d_Var )
#endif

//--------------------------------------------------------------------
// Definition of DMibCheck
//--------------------------------------------------------------------
#if defined DMibContractConfigure_CheckEnabled

#	define DMibContractHelperCheckExpression_B(d_Expression) DMibContractHelperCheckExpression_OP(d_Expression, A)
#	define DMibContractHelperCheckExpression_A(d_Expression) DMibContractHelperCheckExpression_OP(d_Expression, B)
#	define DMibContractHelperCheckExpression_OP(d_Expression, d_Next) \
		DMibContractHelperCheckExpression_B.f_LogVariable(#d_Expression,(d_Expression)) || DMibContractHelperCheckExpression_ ## d_Next // gcc

#	define DMibCheck(d_Expression) \
		DMibContractStatement \
		if (!NMib::NContract::NPrivate::fg_Enabled() || expect_analyzer(!!(d_Expression)) ); \
		else if (NMib::NContract::NPrivate::CContractCheck const DMibContractHelperCheckExpression_B = \
		NMib::NContract::NPrivate::CContractCheck(DMibPFile, DMibPLine, DMibPFunctionSignature, #d_Expression)); \
		else if (NMib::NContract::NPrivate::CContractCheck const& DMibContractHelperCheckExpression_A = DMibContractHelperCheckExpression_B ); \
		else NMib::NContract::g_ContractBreak = DMibContractHelperCheckExpression_A \
			= DMibContractHelperCheckExpression_A || DMibContractHelperCheckExpression_B || DMibContractHelperCheckExpression_A // gcc
#else
enum
{
	DMibContractHelperCheckExpression_A
	, DMibContractHelperCheckExpression_B
};

#	define DMibContractHelperCheckExpression_B(d_Expression) DMibContractHelperCheckExpression_OP(d_Expression, A)
#	define DMibContractHelperCheckExpression_A(d_Expression) DMibContractHelperCheckExpression_OP(d_Expression, B)
#	define DMibContractHelperCheckExpression_OP(d_Expression, d_Next) DMibContractHelperCheckExpression_ ## d_Next

#	define DMibCheck(d_Expression) DMibContractStatement if (false); else (void)DMibContractHelperCheckExpression_A
#endif

//--------------------------------------------------------------------
// Definition of Never-get-here
//--------------------------------------------------------------------
#if defined DMibContractConfigure_NeverGetHereEnabled

#	define DMibContractHelperNeverGetHereExpression_B(d_Expression) DMibContractHelperNeverGetHereExpression_OP(d_Expression, A)
#	define DMibContractHelperNeverGetHereExpression_A(d_Expression) DMibContractHelperNeverGetHereExpression_OP(d_Expression, B)
#	define DMibContractHelperNeverGetHereExpression_OP(d_Expression, d_Next) \
		DMibContractHelperNeverGetHereExpression_B.f_LogVariable(#d_Expression,(d_Expression)) || DMibContractHelperNeverGetHereExpression_ ## d_Next

#	define DMibNeverGetHere \
		DMibContractStatement \
		if (NMib::NContract::NPrivate::CDisableReturnValue const DisableReturnValue = NMib::NContract::NPrivate::CDisableReturnValue()); else \
		if (!NMib::NContract::NPrivate::fg_Enabled()); else \
		if (NMib::NContract::NPrivate::CContractNeverGetHere const DMibContractHelperNeverGetHereExpression_B = \
		NMib::NContract::NPrivate::CContractNeverGetHere(DMibPFile, DMibPLine, DMibPFunctionSignature)); else \
		if (NMib::NContract::NPrivate::CContractNeverGetHere const& DMibContractHelperNeverGetHereExpression_A = DMibContractHelperNeverGetHereExpression_B ); \
		else NMib::NContract::g_ContractBreak = DMibContractHelperNeverGetHereExpression_A \
			= DMibContractHelperNeverGetHereExpression_A || DMibContractHelperNeverGetHereExpression_B || DMibContractHelperNeverGetHereExpression_A // gcc
#else
enum
{
	DMibContractHelperNeverGetHereExpression_A
	, DMibContractHelperNeverGetHereExpression_B
};

#	define DMibContractHelperNeverGetHereExpression_B(d_Expression) DMibContractHelperNeverGetHereExpression_OP(d_Expression, A)
#	define DMibContractHelperNeverGetHereExpression_A(d_Expression) DMibContractHelperNeverGetHereExpression_OP(d_Expression, B)
#	define DMibContractHelperNeverGetHereExpression_OP(d_Expression, d_Next) DMibContractHelperNeverGetHereExpression_ ## d_Next

#	define DMibNeverGetHere \
		DMibContractStatement \
		if (NMib::NContract::NPrivate::CDisableReturnValue const DisableReturnValue = NMib::NContract::NPrivate::CDisableReturnValue()); else \
		(void)DMibContractHelperNeverGetHereExpression_A
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

#include "Malterlib_Contract.hpp"

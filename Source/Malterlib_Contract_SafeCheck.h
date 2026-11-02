// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once

namespace NMib::NContract
{
	// If dtrace enable has not been specifically set, enable it incase we are in debug mode

#ifndef DMibEnableSafeCheck
#	if defined(DMibDebug) || defined(DConfig_ReleaseTesting)
#		define DMibEnableSafeCheck 1
#	else
#		define DMibEnableSafeCheck 0
#	endif
#endif

	namespace NPrivate
	{
		struct CNoReturn
		{
			constexpr CNoReturn() function_does_not_return_analyzer
			{
			}

			constexpr ~CNoReturn()
			{
			}

			constexpr operator bool () const
			{
				return false;
			}
		};
	}

#if DMibEnableSafeCheck > 0
	static constexpr bool gc_bSafeCheckNoExcept = false;
#	define DMibIfEnableSafeCheck(...) __VA_ARGS__
#	define DMibIfNotEnableSafeCheck(...)
	// A check is one statement that takes the terminator of the call it looks like, so it
	// stands on its own behind a clause without braces, 'if (b) DMibSafeCheck(x, ""); else'.
#	define DMibSafeCheck(d_Expression, d_Message) \
		do {if (!expect_analyzer(!!(d_Expression))) {if (NMib::NContract::fg_MibSafeCheck(DMibPFile, DMibPLine, #d_Expression, d_Message)) DMibPDebugBreak;}} while (false)
#	define DMibFastCheck(d_Expression) \
		do {if (!expect_analyzer(!!(d_Expression))) {DMibPDebugBreak; ::NMib::NContract::NPrivate::CNoReturn NoReturn; [[maybe_unused]] int x = 0; x = 1; ++x;}} while (false)
	bool fg_MibSafeCheck(ch8 const *_pFile, int32 _Line, ch8 const *_pExpression, ch8 const *_pMessage) function_does_not_return_analyzer;
	bool fg_MibSafeCheckSetThrowsException(bool _bThrowsException);
#else
	static constexpr bool gc_bSafeCheckNoExcept = true;
#	define DMibIfEnableSafeCheck(...)
#	define DMibIfNotEnableSafeCheck(...) __VA_ARGS__
#	define DMibSafeCheck(d_Expression, d_Message) (void)0
#	define DMibFastCheck(d_Expression) (void)0
#endif

	// A fact the malterlib-* clang-tidy checks take as true where it is written, like that a value isn't zero, also in the
	// configurations that compile DMibFastCheck out. The expression is never evaluated, so it changes neither the code nor
	// what the optimizer assumes, and a wrong one only misleads the checks. It must compile in every configuration.
#define DMibAnalyzerAssume(d_Expression) static_cast<void>(sizeof(!!(d_Expression)))

#ifndef DMibPNoShortCuts
#	define DSafeCheck DMibSafeCheck
#	define DFastCheck DMibFastCheck
#endif
}

#ifndef DMibPNoShortCuts
using namespace NMib::NContract;
#endif

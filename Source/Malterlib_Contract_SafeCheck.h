// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

namespace NMib::NContract
{
	// If dtrace enable has not been specifically set, enable it incase we are in debug mode

#	ifndef DMibEnableSafeCheck
#		if defined(DMibDebug) || defined(DConfig_ReleaseTesting)
#			define DMibEnableSafeCheck 1
#		else
#			define DMibEnableSafeCheck 0
#		endif
#	endif

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

			constexpr operator bool() const
			{
				return false;
			}
		};
	}

#	if DMibEnableSafeCheck > 0
#		define DMibIfEnableSafeCheck(...) __VA_ARGS__
	#	define DMibIfNotEnableSafeCheck(...)
#		define DMibSafeCheck(d_Expression, d_Message) {if (!expect_analyzer(!!(d_Expression))) {if (NMib::NContract::fg_MibSafeCheck(DMibPFile, DMibPLine, #d_Expression, d_Message)) DMibPDebugBreak;}}
#		define DMibFastCheck(d_Expression) {if (!expect_analyzer(!!(d_Expression))) {DMibPDebugBreak; ::NMib::NContract::NPrivate::CNoReturn NoReturn; [[maybe_unused]] int x = 0; x = 1; ++x;}}
		bool fg_MibSafeCheck(const ch8 *_pFile, int32 _Line, const ch8 *_pExpression, const ch8 *_pMessage) function_does_not_return_analyzer;
		bool fg_MibSafeCheckSetThrowsException(bool _bThrowsException);
#	else
#	define DMibIfEnableSafeCheck(...)
#	define DMibIfNotEnableSafeCheck(...) __VA_ARGS__
#		define DMibSafeCheck(d_Expression, d_Message) (void)0
#		define DMibFastCheck(d_Expression) (void)0
#	endif

#	ifndef DMibPNoShortCuts
#		define DSafeCheck DMibSafeCheck
#		define DFastCheck DMibFastCheck
#	endif

}

#ifndef DMibPNoShortCuts
	using namespace NMib::NContract;
#endif

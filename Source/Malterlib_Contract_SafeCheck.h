// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

namespace NMib
{
	namespace NDebug
	{
		// If dtrace enable has not been specifically set, enable it incase we are in debug mode

#		ifndef DMibEnableSafeCheck
#			if defined(DMibDebug) || defined(DConfig_ReleaseTesting)
#				define DMibEnableSafeCheck 1
#			else
#				define DMibEnableSafeCheck 0
#			endif
#		endif
				
#		if DMibEnableSafeCheck > 0
#			define DMibSafeCheck(d_Expression, d_Message) {if (!(d_Expression)) {if (NMib::NDebug::fg_MibSafeCheck(DMibPFile, DMibPLine, #d_Expression, d_Message)) DMibPDebugBreak;}}
#			define DMibFastCheck(d_Expression) {if (!(d_Expression)) {DMibPDebugBreak;}}
			bint fg_MibSafeCheck(const ch8 *_pFile, int32 _Line, const ch8 *_pExpression, const ch8 *_pMessage);
			bint fg_MibSafeCheckSetThrowsException(bint _bThrowsException);
#		else
#			define DMibSafeCheck(d_Expression, d_Message) (void)0
#			define DMibFastCheck(d_Expression) (void)0
#		endif

#		ifndef DMibPNoShortCuts
#			define DSafeCheck DMibSafeCheck
#			define DFastCheck DMibFastCheck
#		endif
	}
}

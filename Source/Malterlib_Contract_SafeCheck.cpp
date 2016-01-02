// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#if DMibEnableSafeCheck > 0

NMib::NThread::CSpinLockAggregate g_LastSafeCheckLock = {DAggregateInit};
int32 g_LastSafeCheckLine = 0;
ch8 g_LastSafeCheckFile[1024] = {0};
ch8 g_LastSafeCheckExpression[1024] = {0};
ch8 g_LastSafeCheckMessage[1024] = {0};
bint g_LastSafeCheckValid = false;

#endif
namespace NMib
{
	namespace NDebug
	{
#		if DMibEnableSafeCheck > 0

		NAggregate::TCAggregate<NThread::TCThreadLocal<TCAutoClearInt<bint>, NMem::CAllocator_NonTrackedHeap>, 64> g_SafeCheckThrowsException = {DAggregateInit};
		bint fg_MibSafeCheckSetThrowsException(bint _bThrowsException)
		{
			bint bOld = *(*g_SafeCheckThrowsException);
			*(*g_SafeCheckThrowsException) = _bThrowsException;
			return bOld;
		}

		NStr::CStrNonTracked fg_GetLastSafeCheckMessageLockHeld()
		{
			if (g_LastSafeCheckValid)
			{
				if (*g_LastSafeCheckMessage)
					return NStr::CStrNonTracked::CFormat("{}({}): SafeCheck failed: {} '{}'" DMibNewLine) << g_LastSafeCheckFile << g_LastSafeCheckLine << g_LastSafeCheckExpression << g_LastSafeCheckMessage;
				else
					return NStr::CStrNonTracked::CFormat("{}({}): SafeCheck failed: {}" DMibNewLine) << g_LastSafeCheckFile << g_LastSafeCheckLine << g_LastSafeCheckExpression;
			}
			return NStr::CStrNonTracked();
		}

		inline_never bint fg_MibSafeCheck(const ch8 *_pFile, int32 _Line, const ch8 *_pExpression, const ch8 *_pMessage)
		{
			if (!g_SafeCheckThrowsException.f_WasDestructed() && *(*g_SafeCheckThrowsException))
			{
				NStr::CStrNonTracked Message;
				if (*_pMessage)
					Message = NStr::CStrNonTracked::CFormat("{} '{}'") << _pExpression << _pMessage;
				else
					Message = NStr::CStrNonTracked::CFormat("{}") << _pExpression;

				throw NMib::NException::CExceptionSafeCheck(nullptr, _pFile, _Line, "", Message, true);
				return false;
			}
			else
			{
				NStr::CStrNonTracked Message;
			
				if (*_pMessage)
					Message = NStr::CStrNonTracked::CFormat("SafeCheck failed: {} '{}'") << _pExpression << _pMessage;
				else
					Message = NStr::CStrNonTracked::CFormat("SafeCheck failed: {}") << _pExpression;
				
				NStr::CStrNonTracked LastSafeCheck;
				{
					DMibLock(g_LastSafeCheckLock);
					g_LastSafeCheckLine = _Line;
					NStr::fg_StrCopy(g_LastSafeCheckFile, _pFile, 1024);
					NStr::fg_StrCopy(g_LastSafeCheckExpression, _pExpression, 1024);
					NStr::fg_StrCopy(g_LastSafeCheckMessage, _pMessage, 1024);
					g_LastSafeCheckValid = true;
					LastSafeCheck = fg_GetLastSafeCheckMessageLockHeld();
					g_LastSafeCheckValid = false;
				}

				switch (NSys::fg_Debug_ReportContractFailure(_pFile, _Line, NMisc::fg_FunctionPtrToVoidPtr(&fg_MibSafeCheck), Message))
				{
				case EDebugContractFailureAction_NotHandled:
				case EDebugContractFailureAction_Break:
					{
						NStr::CStrNonTracked Message;
						if (*_pMessage)
							Message = NStr::CStrNonTracked::CFormat(DMibPFileLineFormat " SafeCheck failed: {} '{}'" DMibNewLine) << _pFile << _Line << _pExpression << _pMessage;
						else
							Message = NStr::CStrNonTracked::CFormat(DMibPFileLineFormat " SafeCheck failed: {}" DMibNewLine) << _pFile << _Line << _pExpression;
						NSys::fg_DebugOutput(Message.f_GetStr());
						NSys::fg_System_ReportContractViolation(LastSafeCheck);
						{
							DMibLock(g_LastSafeCheckLock);
							g_LastSafeCheckValid = true;
						}
						return true;
					}
					break;
				case EDebugContractFailureAction_Abort:
					{
						NSys::fg_System_ExitProcess(211);
					}
					break;
				case EDebugContractFailureAction_Continue:
					{
					}
					break;
				}
				return false;
			}
		}

		NStr::CStrNonTracked fg_GetLastSafeCheckMessage()
		{
			DMibLock(g_LastSafeCheckLock);
			return fg_GetLastSafeCheckMessageLockHeld();
		}
#endif
	}
}


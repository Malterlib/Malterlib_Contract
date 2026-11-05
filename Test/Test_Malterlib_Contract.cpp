// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#define DMibContractConfigure_AllEnabled
#include <Mib/Core/Core>
#include <Mib/Test/Test>

#include <Mib/Contract/Contract>

namespace
{
	using namespace NMib::NStr;
	using namespace NMib::NContract;

	struct CContracts_Tests : public NMib::NTest::CTest
	{
	private:
#ifdef DMibContract_AnyEnabled
		struct CViolationRecord
		{
			void f_Reset();

			bool m_bTriggered = false;
			CStrNonTracked m_Type;
			CStrNonTracked m_Statement;
			CStrNonTracked m_Variables;
			CStrNonTracked m_Function;
			CStrNonTracked m_LogMessage;
			int32 m_Line = 0;
			uint32 m_BugId = 0;
		};

		NMib::NFunction::TCFunction<void (CContractViolation const &_ContractViolation)> fp_InstallRecordingLogger();
#endif

		void f_DoTests();

#ifdef DMibContract_AnyEnabled
		CViolationRecord mp_Violation;
#endif
	};

#ifdef DMibContract_AnyEnabled
	void CContracts_Tests::CViolationRecord::f_Reset()
	{
		m_bTriggered = false;
		m_Type = CStrNonTracked();
		m_Statement = CStrNonTracked();
		m_Variables = CStrNonTracked();
		m_Function = CStrNonTracked();
		m_LogMessage = CStrNonTracked();
		m_Line = 0;
		m_BugId = 0;
	}

	NMib::NFunction::TCFunction<void (CContractViolation const &_ContractViolation)> CContracts_Tests::fp_InstallRecordingLogger()
	{
		mp_Violation.f_Reset();
		return fg_SetContractLogger
			(
				[this](CContractViolation const &_Violation)
				{
					mp_Violation.m_bTriggered = true;
					mp_Violation.m_Type = _Violation.f_GetType();
					mp_Violation.m_Statement = _Violation.f_GetStatement();
					mp_Violation.m_Variables = _Violation.f_GetVariables();
					mp_Violation.m_Function = _Violation.f_GetFunction();
					mp_Violation.m_Line = _Violation.f_GetLine();
					mp_Violation.m_LogMessage = _Violation.f_ComposeLogMessage();
					mp_Violation.m_BugId = _Violation.f_BugId();
				}
			)
		;
	}
#endif

	void CContracts_Tests::f_DoTests()
	{
#ifdef DMibContractConfigure_RequireEnabled
		DMibTestSuite("Require")
		{
			DMibTestCategory("Passing condition does not trigger")
			{
				auto fOldLogger = fp_InstallRecordingLogger();
				auto Restore = NMib::g_OnScopeExit / [&]
					{
						fg_SetContractLogger(fOldLogger);
					}
				;

				DMibRequire(true);
				DMibExpectFalse(mp_Violation.m_bTriggered);
			};

			DMibTestCategory("Failing condition triggers violation")
			{
				auto fOldLogger = fp_InstallRecordingLogger();
				auto Restore = NMib::g_OnScopeExit / [&]
					{
						fg_SetContractLogger(fOldLogger);
					}
				;

				DMibRequire(false);
				DMibExpectTrue(mp_Violation.m_bTriggered);
				DMibExpect(mp_Violation.m_Type, ==, CStrNonTracked("Require"));
				DMibExpect(mp_Violation.m_Statement, ==, CStrNonTracked("false"));
			};

			DMibTestCategory("Expression condition captures statement")
			{
				auto fOldLogger = fp_InstallRecordingLogger();
				auto Restore = NMib::g_OnScopeExit / [&]
					{
						fg_SetContractLogger(fOldLogger);
					}
				;

				int32 Value = 5;
				DMibRequire(Value > 10);
				DMibExpectTrue(mp_Violation.m_bTriggered);
				DMibExpect(mp_Violation.m_Statement, ==, CStrNonTracked("Value > 10"));
			};

			DMibTestCategory("Variable logging captures name and value")
			{
				auto fOldLogger = fp_InstallRecordingLogger();
				auto Restore = NMib::g_OnScopeExit / [&]
					{
						fg_SetContractLogger(fOldLogger);
					}
				;

				int32 TestVar = 42;
				DMibRequire(false)(TestVar);
				DMibExpectTrue(mp_Violation.m_bTriggered);
				DMibExpect(mp_Violation.m_Variables, ==, CStrNonTracked::CFormat("{\n}    TestVar = 42").f_GetStr());
			};

			DMibTestCategory("String note logging")
			{
				auto fOldLogger = fp_InstallRecordingLogger();
				auto Restore = NMib::g_OnScopeExit / [&]
					{
						fg_SetContractLogger(fOldLogger);
					}
				;

				DMibRequire(false)("Custom note message");
				DMibExpectTrue(mp_Violation.m_bTriggered);
				DMibExpect(mp_Violation.m_Variables, ==, CStrNonTracked::CFormat("{\n}    note: 'Custom note message'").f_GetStr());
			};

			DMibTestCategory("Multiple variable logging")
			{
				auto fOldLogger = fp_InstallRecordingLogger();
				auto Restore = NMib::g_OnScopeExit / [&]
					{
						fg_SetContractLogger(fOldLogger);
					}
				;

				int32 TestVar = 5;
				int32 TestVar2 = 7;
				DMibRequire(false)(TestVar)(TestVar2);
				DMibExpectTrue(mp_Violation.m_bTriggered);
				DMibExpect(mp_Violation.m_Variables, ==, CStrNonTracked::CFormat("{\n}    TestVar = 5{\n}    TestVar2 = 7").f_GetStr());
			};
		};
#endif

#ifdef DMibContractConfigure_EnsureEnabled
		DMibTestSuite("Ensure")
		{
			DMibTestCategory("Passing condition does not trigger")
			{
				auto fOldLogger = fp_InstallRecordingLogger();
				auto Restore = NMib::g_OnScopeExit / [&]
					{
						fg_SetContractLogger(fOldLogger);
					}
				;

				DMibEnsure(true);
				DMibExpectFalse(mp_Violation.m_bTriggered);
			};

			DMibTestCategory("Failing condition triggers violation")
			{
				auto fOldLogger = fp_InstallRecordingLogger();
				auto Restore = NMib::g_OnScopeExit / [&]
					{
						fg_SetContractLogger(fOldLogger);
					}
				;

				DMibEnsure(false);
				DMibExpectTrue(mp_Violation.m_bTriggered);
				DMibExpect(mp_Violation.m_Type, ==, CStrNonTracked("Ensure"));
				DMibExpect(mp_Violation.m_Statement, ==, CStrNonTracked("false"));
			};

			DMibTestCategory("Variable logging captures name and value")
			{
				auto fOldLogger = fp_InstallRecordingLogger();
				auto Restore = NMib::g_OnScopeExit / [&]
					{
						fg_SetContractLogger(fOldLogger);
					}
				;

				int32 Result = -1;
				DMibEnsure(Result >= 0)(Result);
				DMibExpectTrue(mp_Violation.m_bTriggered);
				DMibExpect(mp_Violation.m_Variables, ==, CStrNonTracked::CFormat("{\n}    Result = -1").f_GetStr());
			};
		};
#endif

#ifdef DMibContractConfigure_CheckEnabled
		DMibTestSuite("Check")
		{
			DMibTestCategory("Passing condition does not trigger")
			{
				auto fOldLogger = fp_InstallRecordingLogger();
				auto Restore = NMib::g_OnScopeExit / [&]
					{
						fg_SetContractLogger(fOldLogger);
					}
				;

				DMibCheck(true);
				DMibExpectFalse(mp_Violation.m_bTriggered);
			};

			DMibTestCategory("Failing condition triggers violation")
			{
				auto fOldLogger = fp_InstallRecordingLogger();
				auto Restore = NMib::g_OnScopeExit / [&]
					{
						fg_SetContractLogger(fOldLogger);
					}
				;

				DMibCheck(false);
				DMibExpectTrue(mp_Violation.m_bTriggered);
				DMibExpect(mp_Violation.m_Type, ==, CStrNonTracked("Check"));
				DMibExpect(mp_Violation.m_Statement, ==, CStrNonTracked("false"));
			};

			DMibTestCategory("Mixed variable and note logging")
			{
				auto fOldLogger = fp_InstallRecordingLogger();
				auto Restore = NMib::g_OnScopeExit / [&]
					{
						fg_SetContractLogger(fOldLogger);
					}
				;

				int32 Value = 99;
				DMibCheck(false)(Value)("Consistency check failed");
				DMibExpectTrue(mp_Violation.m_bTriggered);
				DMibExpect(mp_Violation.m_Variables, ==, CStrNonTracked::CFormat("{\n}    Value = 99{\n}    note: 'Consistency check failed'").f_GetStr());
			};
		};
#endif

#ifdef DMibContractConfigure_NeverGetHereEnabled
		DMibTestSuite("NeverGetHere")
		{
			DMibTestCategory("Triggers violation when reached")
			{
				auto fOldLogger = fp_InstallRecordingLogger();
				auto Restore = NMib::g_OnScopeExit / [&]
					{
						fg_SetContractLogger(fOldLogger);
					}
				;

				DMibNeverGetHere;
				DMibExpectTrue(mp_Violation.m_bTriggered);
				DMibExpect(mp_Violation.m_Type, ==, CStrNonTracked("NeverGetHere"));
			};
		};
#endif

#ifdef DMibContractConfigure_EnsureEnabled
		DMibTestSuite("OldValue")
		{
			DMibTestCategory("OldDef captures initial value")
			{
				auto fOldLogger = fp_InstallRecordingLogger();
				auto Restore = NMib::g_OnScopeExit / [&]
					{
						fg_SetContractLogger(fOldLogger);
					}
				;

				int32 Value = 3;
				DMibContractOldDef(int32, Value);
				DMibExpect(DMibContractOld(Value), ==, 3);
				Value = 10;
				// Old value should still be 3 after modification
				int32 OldValue = DMibContractOld(Value);
				DMibExpect(OldValue, ==, 3);

				DMibEnsure(DMibContractOld(Value) < Value);
				DMibExpectFalse(mp_Violation.m_bTriggered);
			};

			DMibTestCategory("OldDef detects postcondition failure")
			{
				auto fOldLogger = fp_InstallRecordingLogger();
				auto Restore = NMib::g_OnScopeExit / [&]
					{
						fg_SetContractLogger(fOldLogger);
					}
				;

				int32 Value = 10;
				DMibContractOldDef(int32, Value);
				Value = 5;
				DMibEnsure(DMibContractOld(Value) < Value);
				DMibExpectTrue(mp_Violation.m_bTriggered);
				DMibExpect(mp_Violation.m_Type, ==, CStrNonTracked("Ensure"));
			};

			DMibTestCategory("OldExpr captures expression result")
			{
				int32 Value = 5;
				DMibContractOldExpr(int32, DoubleX, Value * 2);
				DMibExpect(DMibContractOld(DoubleX), ==, 10);
				Value = 100;
				// Old expression value should still be 10 after modifying Value
				int32 OldDoubleX = DMibContractOld(DoubleX);
				DMibExpect(OldDoubleX, ==, 10);
			};
		};
#endif

#ifdef DMibContractConfigure_CheckEnabled
		DMibTestSuite("EnableContracts")
		{
			DMibTestCategory("Disable prevents violations")
			{
				auto fOldLogger = fp_InstallRecordingLogger();
				auto Restore = NMib::g_OnScopeExit / [&]
					{
						fg_SetContractLogger(fOldLogger);
					}
				;

				{
					CEnableContracts DisableScope(false);
					DMibCheck(false);
					DMibExpectFalse(mp_Violation.m_bTriggered);
				}
			};

			DMibTestCategory("Re-enable after scope exit")
			{
				auto fOldLogger = fp_InstallRecordingLogger();
				auto Restore = NMib::g_OnScopeExit / [&]
					{
						fg_SetContractLogger(fOldLogger);
					}
				;

				{
					CEnableContracts DisableScope(false);
					DMibCheck(false);
					DMibExpectFalse(mp_Violation.m_bTriggered);
				}

				DMibCheck(false);
				DMibExpectTrue(mp_Violation.m_bTriggered);
			};

			DMibTestCategory("Nested enable and disable")
			{
				auto fOldLogger = fp_InstallRecordingLogger();
				auto Restore = NMib::g_OnScopeExit / [&]
					{
						fg_SetContractLogger(fOldLogger);
					}
				;

				{
					CEnableContracts OuterDisable(false);
					DMibCheck(false);
					bool bTriggeredWhileDisabled = mp_Violation.m_bTriggered;
					DMibExpectFalse(bTriggeredWhileDisabled);

					{
						CEnableContracts InnerEnable(true);
						DMibCheck(false);
						DMibExpectTrue(mp_Violation.m_bTriggered);
					}

					mp_Violation.f_Reset();
					DMibCheck(false);
					bool bTriggeredAfterReDisable = mp_Violation.m_bTriggered;
					DMibExpectFalse(bTriggeredAfterReDisable);
				}
			};
		};
#endif

#ifdef DMibContract_AnyEnabled
		DMibTestSuite("ExceptionLogger")
		{
#	ifdef DMibContractConfigure_RequireEnabled
			DMibTestCategory("Require throws CContractException_Require")
			{
				auto fOldLogger = fg_SetContractLoggerThrowExceptions();
				auto Restore = NMib::g_OnScopeExit / [&]
					{
						fg_SetContractLogger(fOldLogger);
					}
				;

				bool bCaught = false;
				try
				{
					DMibRequire(false);
				}
				catch (CContractException_Require const &)
				{
					bCaught = true;
				}
				DMibExpectTrue(bCaught);
			};
#	endif

#	ifdef DMibContractConfigure_EnsureEnabled
			DMibTestCategory("Ensure throws CContractException_Ensure")
			{
				auto fOldLogger = fg_SetContractLoggerThrowExceptions();
				auto Restore = NMib::g_OnScopeExit / [&]
					{
						fg_SetContractLogger(fOldLogger);
					}
				;

				bool bCaught = false;
				try
				{
					DMibEnsure(false);
				}
				catch (CContractException_Ensure const &)
				{
					bCaught = true;
				}
				DMibExpectTrue(bCaught);
			};
#	endif

#	ifdef DMibContractConfigure_CheckEnabled
			DMibTestCategory("Check throws CContractException_Check")
			{
				auto fOldLogger = fg_SetContractLoggerThrowExceptions();
				auto Restore = NMib::g_OnScopeExit / [&]
					{
						fg_SetContractLogger(fOldLogger);
					}
				;

				bool bCaught = false;
				try
				{
					DMibCheck(false);
				}
				catch (CContractException_Check const &)
				{
					bCaught = true;
				}
				DMibExpectTrue(bCaught);
			};
#	endif

#	ifdef DMibContractConfigure_NeverGetHereEnabled
			DMibTestCategory("NeverGetHere throws CContractException_NeverGetHere")
			{
				auto fOldLogger = fg_SetContractLoggerThrowExceptions();
				auto Restore = NMib::g_OnScopeExit / [&]
					{
						fg_SetContractLogger(fOldLogger);
					}
				;

				bool bCaught = false;
				try
				{
					DMibNeverGetHere;
				}
				catch (CContractException_NeverGetHere const &)
				{
					bCaught = true;
				}
				DMibExpectTrue(bCaught);
			};
#	endif

#	ifdef DMibContractConfigure_RequireEnabled
			DMibTestCategory("All contract exceptions derive from CContractException")
			{
				auto fOldLogger = fg_SetContractLoggerThrowExceptions();
				auto Restore = NMib::g_OnScopeExit / [&]
					{
						fg_SetContractLogger(fOldLogger);
					}
				;

				bool bCaughtBase = false;
				try
				{
					DMibRequire(false);
				}
				catch (CContractException const &)
				{
					bCaughtBase = true;
				}
				DMibExpectTrue(bCaughtBase);
			};
#	endif
		};
#endif

#ifdef DMibContractConfigure_CheckEnabled
		DMibTestSuite("ContractViolation")
		{
			DMibTestCategory("Violation captures source location")
			{
				auto fOldLogger = fp_InstallRecordingLogger();
				auto Restore = NMib::g_OnScopeExit / [&]
					{
						fg_SetContractLogger(fOldLogger);
					}
				;

				DMibCheck(false);
				DMibExpectTrue(mp_Violation.m_bTriggered);
				DMibExpectTrue(mp_Violation.m_Line > 0);
				DMibExpectTrue(mp_Violation.m_Function.f_GetLen() > 0);
			};

			DMibTestCategory("ComposeLogMessage produces output")
			{
				auto fOldLogger = fp_InstallRecordingLogger();
				auto Restore = NMib::g_OnScopeExit / [&]
					{
						fg_SetContractLogger(fOldLogger);
					}
				;

				DMibCheck(false);
				DMibExpectTrue(mp_Violation.m_bTriggered);
				DMibExpectTrue(mp_Violation.m_LogMessage.f_GetLen() > 0);
			};

			DMibTestCategory("BugId is consistent for same statement")
			{
				auto fOldLogger = fp_InstallRecordingLogger();
				auto Restore = NMib::g_OnScopeExit / [&]
					{
						fg_SetContractLogger(fOldLogger);
					}
				;

				DMibCheck(false);
				uint32 FirstBugId = mp_Violation.m_BugId;
				DMibExpectTrue(FirstBugId != 0);

				mp_Violation.f_Reset();
				DMibCheck(false);
				uint32 SecondBugId = mp_Violation.m_BugId;

				DMibExpect(FirstBugId, ==, SecondBugId);
			};

			DMibTestCategory("Different statements produce different BugIds")
			{
				auto fOldLogger = fp_InstallRecordingLogger();
				auto Restore = NMib::g_OnScopeExit / [&]
					{
						fg_SetContractLogger(fOldLogger);
					}
				;

				DMibCheck(false);
				uint32 FirstBugId = mp_Violation.m_BugId;

				mp_Violation.f_Reset();
				DMibCheck(1 == 2);
				uint32 SecondBugId = mp_Violation.m_BugId;

				DMibExpect(FirstBugId, !=, SecondBugId);
			};
		};

		DMibTestSuite("CustomLogger")
		{
			DMibTestCategory("Custom logger receives violations")
			{
				bool bCustomCalled = false;
				auto fOldLogger = fg_SetContractLogger
					(
						[&bCustomCalled](CContractViolation const &_ContractViolation)
						{
							bCustomCalled = true;
						}
					)
				;
				auto Restore = NMib::g_OnScopeExit / [&]
					{
						fg_SetContractLogger(fOldLogger);
					}
				;

				DMibCheck(false);
				DMibExpectTrue(bCustomCalled);
			};

#	if defined(DMibContractConfigure_RequireEnabled) && defined(DMibContractConfigure_EnsureEnabled) && defined(DMibContractConfigure_NeverGetHereEnabled)
			DMibTestCategory("Logger receives correct violation type for each contract")
			{
				CStrNonTracked ReceivedType;
				auto fOldLogger = fg_SetContractLogger
					(
						[&ReceivedType](CContractViolation const &_Violation)
						{
							ReceivedType = _Violation.f_GetType();
						}
					)
				;
				auto Restore = NMib::g_OnScopeExit / [&]
					{
						fg_SetContractLogger(fOldLogger);
					}
				;

				DMibRequire(false);
				DMibExpect(ReceivedType, ==, CStrNonTracked("Require"));

				DMibEnsure(false);
				DMibExpect(ReceivedType, ==, CStrNonTracked("Ensure"));

				DMibCheck(false);
				DMibExpect(ReceivedType, ==, CStrNonTracked("Check"));

				DMibNeverGetHere;
				DMibExpect(ReceivedType, ==, CStrNonTracked("NeverGetHere"));
			};
#	endif

			DMibTestCategory("SetContractLogger returns previous logger")
			{
				bool bFirstCalled = false;
				bool bSecondCalled = false;

				auto fOriginal = fg_SetContractLogger
					(
						[&bFirstCalled](CContractViolation const &_ContractViolation)
						{
							bFirstCalled = true;
						}
					)
				;
				auto RestoreOriginal = NMib::g_OnScopeExit / [&]
					{
						fg_SetContractLogger(fOriginal);
					}
				;

				auto fFirst = fg_SetContractLogger
					(
						[&bSecondCalled](CContractViolation const &_ContractViolation)
						{
							bSecondCalled = true;
						}
					)
				;

				DMibCheck(false);
				DMibExpectFalse(bFirstCalled);
				DMibExpectTrue(bSecondCalled);

				// Restore first logger
				fg_SetContractLogger(fFirst);

				bFirstCalled = false;
				bSecondCalled = false;
				DMibCheck(false);
				DMibExpectTrue(bFirstCalled);
				DMibExpectFalse(bSecondCalled);
			};
		};
#endif
	}

	DMibTestRegister(CContracts_Tests, Malterlib::Contract);
}

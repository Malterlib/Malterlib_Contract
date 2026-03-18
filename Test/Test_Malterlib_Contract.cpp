// Copyright © 2015 Hansoft AB
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#define DMibContractConfigure_AllEnabled
#include <Mib/Core/Core>
#include <Mib/Test/Test>

#include <Mib/Contract/Contract>

namespace
{
	using namespace NMib::NStr;
	using namespace NMib::NContract;

	class CContracts_Tests : public NMib::NTest::CTest
	{
	private:
#ifdef DMibContract_AnyEnabled
		struct CViolationRecord
		{
			bool m_bTriggered = false;
			CStrNonTracked m_Type;
			CStrNonTracked m_Statement;
			CStrNonTracked m_Variables;
			CStrNonTracked m_Function;
			CStrNonTracked m_LogMessage;
			int32 m_Line = 0;
			uint32 m_BugId = 0;

			void f_Reset()
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
		};

		CViolationRecord m_Violation;

		NMib::NFunction::TCFunction<void(CContractViolation const &)> fp_InstallRecordingLogger()
		{
			m_Violation.f_Reset();
			return fg_SetContractLogger
				(
					[this](CContractViolation const &_Violation)
					{
						m_Violation.m_bTriggered = true;
						m_Violation.m_Type = _Violation.f_GetType();
						m_Violation.m_Statement = _Violation.f_GetStatement();
						m_Violation.m_Variables = _Violation.f_GetVariables();
						m_Violation.m_Function = _Violation.f_GetFunction();
						m_Violation.m_Line = _Violation.f_GetLine();
						m_Violation.m_LogMessage = _Violation.f_ComposeLogMessage();
						m_Violation.m_BugId = _Violation.f_BugId();
					}
				)
			;
		}
#endif

		void f_DoTests()
		{
#ifdef DMibContractConfigure_RequireEnabled
			DMibTestCategory("Require")
			{
				DMibTestSuite("Passing condition does not trigger")
				{
					auto fOldLogger = fp_InstallRecordingLogger();
					auto Restore = NMib::g_OnScopeExit / [&] { fg_SetContractLogger(fOldLogger); };

					DMibRequire(true);
					DMibExpectFalse(m_Violation.m_bTriggered);
				};

				DMibTestSuite("Failing condition triggers violation")
				{
					auto fOldLogger = fp_InstallRecordingLogger();
					auto Restore = NMib::g_OnScopeExit / [&] { fg_SetContractLogger(fOldLogger); };

					DMibRequire(false);
					DMibExpectTrue(m_Violation.m_bTriggered);
					DMibExpect(m_Violation.m_Type, ==, CStrNonTracked("Require"));
					DMibExpect(m_Violation.m_Statement, ==, CStrNonTracked("false"));
				};

				DMibTestSuite("Expression condition captures statement")
				{
					auto fOldLogger = fp_InstallRecordingLogger();
					auto Restore = NMib::g_OnScopeExit / [&] { fg_SetContractLogger(fOldLogger); };

					int32 x = 5;
					DMibRequire(x > 10);
					DMibExpectTrue(m_Violation.m_bTriggered);
					DMibExpect(m_Violation.m_Statement, ==, CStrNonTracked("x > 10"));
				};

				DMibTestSuite("Variable logging captures name and value")
				{
					auto fOldLogger = fp_InstallRecordingLogger();
					auto Restore = NMib::g_OnScopeExit / [&] { fg_SetContractLogger(fOldLogger); };

					int32 TestVar = 42;
					DMibRequire(false)(TestVar);
					DMibExpectTrue(m_Violation.m_bTriggered);
					DMibExpect(m_Violation.m_Variables, ==, CStrNonTracked::CFormat("{\n}    TestVar = 42").f_GetStr());
				};

				DMibTestSuite("String note logging")
				{
					auto fOldLogger = fp_InstallRecordingLogger();
					auto Restore = NMib::g_OnScopeExit / [&] { fg_SetContractLogger(fOldLogger); };

					DMibRequire(false)("Custom note message");
					DMibExpectTrue(m_Violation.m_bTriggered);
					DMibExpect(m_Violation.m_Variables, ==, CStrNonTracked::CFormat("{\n}    note: 'Custom note message'").f_GetStr());
				};

				DMibTestSuite("Multiple variable logging")
				{
					auto fOldLogger = fp_InstallRecordingLogger();
					auto Restore = NMib::g_OnScopeExit / [&] { fg_SetContractLogger(fOldLogger); };

					int32 TestVar = 5;
					int32 TestVar2 = 7;
					DMibRequire(false)(TestVar)(TestVar2);
					DMibExpectTrue(m_Violation.m_bTriggered);
					DMibExpect(m_Violation.m_Variables, ==, CStrNonTracked::CFormat("{\n}    TestVar = 5{\n}    TestVar2 = 7").f_GetStr());
				};
			};
#endif

#ifdef DMibContractConfigure_EnsureEnabled
			DMibTestCategory("Ensure")
			{
				DMibTestSuite("Passing condition does not trigger")
				{
					auto fOldLogger = fp_InstallRecordingLogger();
					auto Restore = NMib::g_OnScopeExit / [&] { fg_SetContractLogger(fOldLogger); };

					DMibEnsure(true);
					DMibExpectFalse(m_Violation.m_bTriggered);
				};

				DMibTestSuite("Failing condition triggers violation")
				{
					auto fOldLogger = fp_InstallRecordingLogger();
					auto Restore = NMib::g_OnScopeExit / [&] { fg_SetContractLogger(fOldLogger); };

					DMibEnsure(false);
					DMibExpectTrue(m_Violation.m_bTriggered);
					DMibExpect(m_Violation.m_Type, ==, CStrNonTracked("Ensure"));
					DMibExpect(m_Violation.m_Statement, ==, CStrNonTracked("false"));
				};

				DMibTestSuite("Variable logging captures name and value")
				{
					auto fOldLogger = fp_InstallRecordingLogger();
					auto Restore = NMib::g_OnScopeExit / [&] { fg_SetContractLogger(fOldLogger); };

					int32 Result = -1;
					DMibEnsure(Result >= 0)(Result);
					DMibExpectTrue(m_Violation.m_bTriggered);
					DMibExpect(m_Violation.m_Variables, ==, CStrNonTracked::CFormat("{\n}    Result = -1").f_GetStr());
				};
			};
#endif

#ifdef DMibContractConfigure_CheckEnabled
			DMibTestCategory("Check")
			{
				DMibTestSuite("Passing condition does not trigger")
				{
					auto fOldLogger = fp_InstallRecordingLogger();
					auto Restore = NMib::g_OnScopeExit / [&] { fg_SetContractLogger(fOldLogger); };

					DMibCheck(true);
					DMibExpectFalse(m_Violation.m_bTriggered);
				};

				DMibTestSuite("Failing condition triggers violation")
				{
					auto fOldLogger = fp_InstallRecordingLogger();
					auto Restore = NMib::g_OnScopeExit / [&] { fg_SetContractLogger(fOldLogger); };

					DMibCheck(false);
					DMibExpectTrue(m_Violation.m_bTriggered);
					DMibExpect(m_Violation.m_Type, ==, CStrNonTracked("Check"));
					DMibExpect(m_Violation.m_Statement, ==, CStrNonTracked("false"));
				};

				DMibTestSuite("Mixed variable and note logging")
				{
					auto fOldLogger = fp_InstallRecordingLogger();
					auto Restore = NMib::g_OnScopeExit / [&] { fg_SetContractLogger(fOldLogger); };

					int32 Value = 99;
					DMibCheck(false)(Value)("Consistency check failed");
					DMibExpectTrue(m_Violation.m_bTriggered);
					DMibExpect(m_Violation.m_Variables, ==, CStrNonTracked::CFormat("{\n}    Value = 99{\n}    note: 'Consistency check failed'").f_GetStr());
				};
			};
#endif

#ifdef DMibContractConfigure_NeverGetHereEnabled
			DMibTestCategory("NeverGetHere")
			{
				DMibTestSuite("Triggers violation when reached")
				{
					auto fOldLogger = fp_InstallRecordingLogger();
					auto Restore = NMib::g_OnScopeExit / [&] { fg_SetContractLogger(fOldLogger); };

					DMibNeverGetHere;
					DMibExpectTrue(m_Violation.m_bTriggered);
					DMibExpect(m_Violation.m_Type, ==, CStrNonTracked("NeverGetHere"));
				};
			};
#endif

#ifdef DMibContractConfigure_EnsureEnabled
			DMibTestCategory("OldValue")
			{
				DMibTestSuite("OldDef captures initial value")
				{
					auto fOldLogger = fp_InstallRecordingLogger();
					auto Restore = NMib::g_OnScopeExit / [&] { fg_SetContractLogger(fOldLogger); };

					int32 x = 3;
					DMibContractOldDef(int32, x);
					DMibExpect(DMibContractOld(x), ==, 3);
					x = 10;
					// Old value should still be 3 after modification
					int32 OldValue = DMibContractOld(x);
					DMibExpect(OldValue, ==, 3);

					DMibEnsure(DMibContractOld(x) < x);
					DMibExpectFalse(m_Violation.m_bTriggered);
				};

				DMibTestSuite("OldDef detects postcondition failure")
				{
					auto fOldLogger = fp_InstallRecordingLogger();
					auto Restore = NMib::g_OnScopeExit / [&] { fg_SetContractLogger(fOldLogger); };

					int32 x = 10;
					DMibContractOldDef(int32, x);
					x = 5;
					DMibEnsure(DMibContractOld(x) < x);
					DMibExpectTrue(m_Violation.m_bTriggered);
					DMibExpect(m_Violation.m_Type, ==, CStrNonTracked("Ensure"));
				};

				DMibTestSuite("OldExpr captures expression result")
				{
					int32 x = 5;
					DMibContractOldExpr(int32, DoubleX, x * 2);
					DMibExpect(DMibContractOld(DoubleX), ==, 10);
					x = 100;
					// Old expression value should still be 10 after modifying x
					int32 OldDoubleX = DMibContractOld(DoubleX);
					DMibExpect(OldDoubleX, ==, 10);
				};
			};
#endif

#ifdef DMibContractConfigure_CheckEnabled
			DMibTestCategory("EnableContracts")
			{
				DMibTestSuite("Disable prevents violations")
				{
					auto fOldLogger = fp_InstallRecordingLogger();
					auto Restore = NMib::g_OnScopeExit / [&] { fg_SetContractLogger(fOldLogger); };

					{
						CEnableContracts DisableScope(false);
						DMibCheck(false);
						DMibExpectFalse(m_Violation.m_bTriggered);
					}
				};

				DMibTestSuite("Re-enable after scope exit")
				{
					auto fOldLogger = fp_InstallRecordingLogger();
					auto Restore = NMib::g_OnScopeExit / [&] { fg_SetContractLogger(fOldLogger); };

					{
						CEnableContracts DisableScope(false);
						DMibCheck(false);
						DMibExpectFalse(m_Violation.m_bTriggered);
					}

					DMibCheck(false);
					DMibExpectTrue(m_Violation.m_bTriggered);
				};

				DMibTestSuite("Nested enable and disable")
				{
					auto fOldLogger = fp_InstallRecordingLogger();
					auto Restore = NMib::g_OnScopeExit / [&] { fg_SetContractLogger(fOldLogger); };

					{
						CEnableContracts OuterDisable(false);
						DMibCheck(false);
						bool bTriggeredWhileDisabled = m_Violation.m_bTriggered;
						DMibExpectFalse(bTriggeredWhileDisabled);

						{
							CEnableContracts InnerEnable(true);
							DMibCheck(false);
							DMibExpectTrue(m_Violation.m_bTriggered);
						}

						m_Violation.f_Reset();
						DMibCheck(false);
						bool bTriggeredAfterReDisable = m_Violation.m_bTriggered;
						DMibExpectFalse(bTriggeredAfterReDisable);
					}
				};
			};
#endif

#ifdef DMibContract_AnyEnabled
			DMibTestCategory("ExceptionLogger")
			{
#	ifdef DMibContractConfigure_RequireEnabled
				DMibTestSuite("Require throws CContractException_Require")
				{
					auto fOldLogger = fg_SetContractLoggerThrowExceptions();
					auto Restore = NMib::g_OnScopeExit / [&] { fg_SetContractLogger(fOldLogger); };

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
				DMibTestSuite("Ensure throws CContractException_Ensure")
				{
					auto fOldLogger = fg_SetContractLoggerThrowExceptions();
					auto Restore = NMib::g_OnScopeExit / [&] { fg_SetContractLogger(fOldLogger); };

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
				DMibTestSuite("Check throws CContractException_Check")
				{
					auto fOldLogger = fg_SetContractLoggerThrowExceptions();
					auto Restore = NMib::g_OnScopeExit / [&] { fg_SetContractLogger(fOldLogger); };

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
				DMibTestSuite("NeverGetHere throws CContractException_NeverGetHere")
				{
					auto fOldLogger = fg_SetContractLoggerThrowExceptions();
					auto Restore = NMib::g_OnScopeExit / [&] { fg_SetContractLogger(fOldLogger); };

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
				DMibTestSuite("All contract exceptions derive from CContractException")
				{
					auto fOldLogger = fg_SetContractLoggerThrowExceptions();
					auto Restore = NMib::g_OnScopeExit / [&] { fg_SetContractLogger(fOldLogger); };

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
			DMibTestCategory("ContractViolation")
			{
				DMibTestSuite("Violation captures source location")
				{
					auto fOldLogger = fp_InstallRecordingLogger();
					auto Restore = NMib::g_OnScopeExit / [&] { fg_SetContractLogger(fOldLogger); };

					DMibCheck(false);
					DMibExpectTrue(m_Violation.m_bTriggered);
					DMibExpectTrue(m_Violation.m_Line > 0);
					DMibExpectTrue(m_Violation.m_Function.f_GetLen() > 0);
				};

				DMibTestSuite("ComposeLogMessage produces output")
				{
					auto fOldLogger = fp_InstallRecordingLogger();
					auto Restore = NMib::g_OnScopeExit / [&] { fg_SetContractLogger(fOldLogger); };

					DMibCheck(false);
					DMibExpectTrue(m_Violation.m_bTriggered);
					DMibExpectTrue(m_Violation.m_LogMessage.f_GetLen() > 0);
				};

				DMibTestSuite("BugId is consistent for same statement")
				{
					auto fOldLogger = fp_InstallRecordingLogger();
					auto Restore = NMib::g_OnScopeExit / [&] { fg_SetContractLogger(fOldLogger); };

					DMibCheck(false);
					uint32 FirstBugId = m_Violation.m_BugId;
					DMibExpectTrue(FirstBugId != 0);

					m_Violation.f_Reset();
					DMibCheck(false);
					uint32 SecondBugId = m_Violation.m_BugId;

					DMibExpect(FirstBugId, ==, SecondBugId);
				};

				DMibTestSuite("Different statements produce different BugIds")
				{
					auto fOldLogger = fp_InstallRecordingLogger();
					auto Restore = NMib::g_OnScopeExit / [&] { fg_SetContractLogger(fOldLogger); };

					DMibCheck(false);
					uint32 FirstBugId = m_Violation.m_BugId;

					m_Violation.f_Reset();
					DMibCheck(1 == 2);
					uint32 SecondBugId = m_Violation.m_BugId;

					DMibExpect(FirstBugId, !=, SecondBugId);
				};
			};

			DMibTestCategory("CustomLogger")
			{
				DMibTestSuite("Custom logger receives violations")
				{
					bool bCustomCalled = false;
					auto fOldLogger = fg_SetContractLogger
						(
							[&bCustomCalled](CContractViolation const &)
							{
								bCustomCalled = true;
							}
						)
					;
					auto Restore = NMib::g_OnScopeExit / [&] { fg_SetContractLogger(fOldLogger); };

					DMibCheck(false);
					DMibExpectTrue(bCustomCalled);
				};

#	if defined(DMibContractConfigure_RequireEnabled) && defined(DMibContractConfigure_EnsureEnabled) && defined(DMibContractConfigure_NeverGetHereEnabled)
				DMibTestSuite("Logger receives correct violation type for each contract")
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
					auto Restore = NMib::g_OnScopeExit / [&] { fg_SetContractLogger(fOldLogger); };

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

				DMibTestSuite("SetContractLogger returns previous logger")
				{
					bool bFirstCalled = false;
					bool bSecondCalled = false;

					auto fOriginal = fg_SetContractLogger
						(
							[&bFirstCalled](CContractViolation const &)
							{
								bFirstCalled = true;
							}
						)
					;
					auto RestoreOriginal = NMib::g_OnScopeExit / [&] { fg_SetContractLogger(fOriginal); };

					auto fFirst = fg_SetContractLogger
						(
							[&bSecondCalled](CContractViolation const &)
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
	};

	DMibTestRegister(CContracts_Tests, Malterlib::Contract);
}

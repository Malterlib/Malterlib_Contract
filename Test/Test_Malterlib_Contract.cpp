// Copyright © 2015 Hansoft AB
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#define DMibContractConfigure_AllEnabled
#include <Mib/Core/Core>
#include <Mib/Test/Test>

#include <Mib/Contract/Contract>

namespace
{

	class invariant_base_test
	{
		DMibClassInvariants(invariant_base_test)
		{
			DMibInvariant(iValidBase);
			DMibInvariant(iValidBase) (iValidBase);
			DMibInvariant(iValidBase) (iValidBase) (iValidBase);
		}
	protected:
		bool iValidBase;
	};

	class CContracts_Tests : public NMib::NTest::CTest
	{
	public: // Setup
		CContracts_Tests()
		{
			//setContractLogger(fg_Bind(&contract_test::report_issue,this,g_1));
		}
	public: // Tests

		/*
		void testRequire()
		{
			iMessage.clear();
			DMibRequire(false);
			DTestRequire(!empty(algorithm::ifind_first(iMessage,"require violation")));
			DTestRequire(!empty(algorithm::ifind_first(iMessage,lexical_cast<std::string>(__LINE__-2))));
			DTestRequire(!empty(algorithm::ifind_first(iMessage,__FUNCSIG__)));
			DTestRequire(!empty(algorithm::ifind_first(iMessage,__FILE__)));

			DMibRequire(false)("Hello");
			DTestRequire(!empty(algorithm::find_first(iMessage,"Hello")));

			int x = 7;
			DMibRequire(false)(x);
			DTestRequire(!empty(algorithm::find_first(iMessage,"x = 7")));
		}

		void testOld() {
			int x = 3;
			DMibContractOldDef(int,x);
			DTestRequire(DMibContractOld(x)==3);
			x++;
			DTestRequire(DMibContractOld(x)==3);
		}
		void testInvariantSuccess() {
			iMessage = "";
			iValid = true;
			iValidBase = true;
			DMibValidateInvariant();
			DTestRequire(empty(iMessage));
		}
		void testInvariantFail() {
			iMessage = "";
			iValid = false;
			iValidBase = true;
			{
				DMibValidateInvariant();
			} // Invariant is checked at end of scope
			DTestRequire(!empty(iMessage));
			DTestRequire(!empty(algorithm::ifind_first(iMessage,"contract_test")));
		}
		void testBaseInvariantFail() {
			iMessage.clear();
			iValid = true;
			iValidBase = false;
			{
				DMibValidateInvariant();
			} // Invariant is checked at end of scope
			DTestRequire(!empty(iMessage));
			DTestRequire(!empty(algorithm::ifind_first(iMessage,"invariant_base_test")));
		}
		*/
	private: // invariant
		/*DMibClassInvariants(CTest_Tests)
		{
			DMibBaseInvariant(invariant_base_test);
			DMibInvariant(iValid);
		}*/

		bool iValid;
	private:
		/*void report_issue(ContractViolationLog const& log)
		{
			iMessage = log.composeLogMessage();
		}*/

		//std::string iMessage;

		void f_DoTests()
		{
			/*testRequire();
			testOld();
			testInvariantSuccess();
			testInvariantFail();
			testBaseInvariantFail();*/

			//int TestInt = 0;

			//DMibCheck(false)(str_utf16("Test"))(TestInt);
		}

	};

	DMibTestRegister(CContracts_Tests, Malterlib::Contract);

}


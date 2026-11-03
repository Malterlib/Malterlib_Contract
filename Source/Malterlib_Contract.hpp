// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// Copyright (c) 2006-2009 Jonas Persson
// SPDX-License-Identifier: BSL-1.0

#pragma once

namespace NMib::NContract::NPrivate
{
#if defined DMibContract_AnyEnabled
	inline_small bool fg_Enabled()
	{
		return fg_ContractsEnabledInThisThread();
	}

	template <typename tf_CVariable>
	bool CContractBase::f_LogVariable(ch8 const *_pDescription, tf_CVariable const &_Var) const
	{
		if (_pDescription[0] == '"')
			mp_Variables += NStr::CStrNonTracked::CFormat("{\n}    note: '{}'") << _Var;
		else
			mp_Variables += NStr::CStrNonTracked::CFormat("{\n}    {} = {}") << _pDescription << _Var;

		return false;
	}
#endif
}

namespace NMib::NContract
{
#ifdef DMibContract_AnyEnabled
	inline_always void CContractBreak::operator = (bool _bBreak)
	{
		if (_bBreak)
			DMibPDebugBreak;
	}
#endif
}

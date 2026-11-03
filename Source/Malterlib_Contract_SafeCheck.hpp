// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once

namespace NMib::NContract::NPrivate
{
	constexpr CNoReturn::CNoReturn()
	{
	}

	constexpr CNoReturn::~CNoReturn()
	{
	}

	constexpr CNoReturn::operator bool () const
	{
		return false;
	}
}

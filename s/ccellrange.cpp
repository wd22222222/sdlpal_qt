///* -*- mode: c; tab-width: 4; c-basic-offset: 4; c-file-style: "linux" -*- */
//
// Copyright (c) 2021-2026, Wu Dong.
// 
// All rights reserved.
//
// This file is part of SDLPAL.
//
// SDLPAL is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License, version 3
// as published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include "ccellrange.h"
#include <algorithm>
#undef max
#undef min
CCellRange CCellRange::Intersect(const CCellRange& rhs) const
{
#
	return CCellRange(
		std::max(m_nMinRow, rhs.m_nMinRow),
		std::max(m_nMinCol, rhs.m_nMinCol),
		std::min(m_nMaxRow, rhs.m_nMaxRow),
		std::min(m_nMaxCol, rhs.m_nMaxCol)
	);
}

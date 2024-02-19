// dp83tc813 handling code
// Copyright (C) 2024 Charles-Henri Mousset <ch.mousset@gmail.com>

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#if !defined(DP83TC813_H__)
#define DP83TC813_H__


#if defined(APP_DP83TC813)
void _app_dp83tc813_start();
#define APP_DP83TC813_START _app_dp83tc813_start
#else
#define APP_DP83TC813_START() {}
#endif

#endif // DP83TC813_H__

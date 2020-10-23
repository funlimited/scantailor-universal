/*
    Scan Tailor - Interactive post-processing tool for scanned pages.
    Copyright (C) 2021 Alexander Trufanov <trufanovan@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef ENCODINGPROGRESSINFO_H
#define ENCODINGPROGRESSINFO_H

namespace publish {

enum EncodingProgressState
{
    InProgress,
    Completed,
    Skipped
};

enum EncodingProgressProcess
{
    Export,
    EncodeTxt,
    EncodePic,
    Assemble,
    OCR
};

} // namespace publish

#endif // ENCODINGPROGRESSINFO_H
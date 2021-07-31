// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip DLLs.
 * Copyright (c) 2014-2021  Riccardo Ostani - All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * Bit7z is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with bit7z; if not, see https://www.gnu.org/licenses/.
 */

#include "internal/cstdoutstream.hpp"

#include <iterator>

#ifndef _WIN32
#include <myWindows/StdAfx.h>
#include "bittypes.hpp"  //for error codes
#endif

using namespace bit7z;

CStdOutStream::CStdOutStream( std::ostream& outputStream ) : mOutputStream( outputStream ) {}

COM_DECLSPEC_NOTHROW
STDMETHODIMP CStdOutStream::Write( const void* data, UInt32 size, UInt32* processedSize ) {
    if ( processedSize != nullptr ) {
        *processedSize = 0;
    }

    if ( size == 0 ) {
        return S_OK;
    }

    const auto old_pos = mOutputStream.tellp();

    mOutputStream.write( static_cast< const char* >( data ), size );

    if ( processedSize != nullptr ) {
        *processedSize = static_cast< uint32_t >( mOutputStream.tellp() - old_pos );
    }

    return mOutputStream.bad() ? HRESULT_FROM_WIN32( ERROR_WRITE_FAULT ) : S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP CStdOutStream::Seek( Int64 offset, UInt32 seekOrigin, UInt64* newPosition ) {
    std::ios_base::seekdir way{};
    switch ( seekOrigin ) {
        case STREAM_SEEK_SET:
            way = std::ios_base::beg;
            break;
        case STREAM_SEEK_CUR:
            way = std::ios_base::cur;
            break;
        case STREAM_SEEK_END:
            way = std::ios_base::end;
            break;
        default:
            return STG_E_INVALIDFUNCTION;
    }

    /*if ( offset < 0 ) { //Tar sometimes uses negative offsets
        return HRESULT_WIN32_ERROR_NEGATIVE_SEEK;
    }*/

    mOutputStream.seekp( static_cast< std::ostream::off_type >( offset ), way );

    if ( mOutputStream.bad() ) {
        return HRESULT_FROM_WIN32( ERROR_SEEK );
    }

    if ( newPosition != nullptr ) {
        *newPosition = static_cast< uint64_t >( mOutputStream.tellp() );
    }

    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP CStdOutStream::SetSize( UInt64 newSize ) {
    const auto old_pos = mOutputStream.tellp();
    mOutputStream.seekp( 0, ostream::end );

    const auto diff_pos = newSize - static_cast< uint64_t >( mOutputStream.tellp() );
    if ( diff_pos > 0 ) {
        std::fill_n( std::ostream_iterator< char >( mOutputStream ), diff_pos, '\0' );
    }

    mOutputStream.seekp( old_pos );

    return mOutputStream ? S_OK : E_FAIL;
}


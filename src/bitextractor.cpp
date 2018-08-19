// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "../include/bitextractor.hpp"

#include "7zip/Archive/IArchive.h"
#include "Windows/COM.h"
#include "Windows/PropVariant.h"

#include "../include/bitexception.hpp"
#include "../include/extractcallback.hpp"
#include "../include/memextractcallback.hpp"
#include "../include/opencallback.hpp"

using namespace bit7z;
using namespace NWindows;

using std::wstring;

// NOTE: this function is not a method of BitExtractor because it would dirty the header with extra dependencies
CMyComPtr< IInArchive > openArchive( const Bit7zLibrary& lib, const BitInFormat& format,
                                     const wstring& in_file, const BitArchiveOpener& opener ) {
    CMyComPtr< IInArchive > inArchive;
    const GUID formatGUID = format.guid();
    lib.createArchiveObject( &formatGUID, &::IID_IInArchive, reinterpret_cast< void** >( &inArchive ) );

    auto* fileStreamSpec = new CInFileStream;
    CMyComPtr< IInStream > fileStream = fileStreamSpec;
    if ( !fileStreamSpec->Open( in_file.c_str() ) ) {
        throw BitException( L"Cannot open archive file '" + in_file + L"'" );
    }

    auto* openCallbackSpec = new OpenCallback( opener, in_file );

    CMyComPtr< IArchiveOpenCallback > openCallback( openCallbackSpec );
    if ( inArchive->Open( fileStream, nullptr, openCallback ) != S_OK ) {
        throw BitException( L"Cannot open archive '" + in_file + L"'" );
    }
    return inArchive;
}

BitExtractor::BitExtractor( const Bit7zLibrary& lib, const BitInFormat& format ) : BitArchiveOpener( lib, format ) {}

//TODO: Improve reusing of code (both extract methods do the same operations when opening an archive)

/* Most of this code, though heavily modified, is taken from the main() of Client7z.cpp in the 7z SDK
 * Main changes made:
 *  + Generalized the code to work with any type of format (the original works only with 7z format)
 *  + Use of exceptions instead of error codes */
void BitExtractor::extract( const wstring& in_file, const wstring& out_dir ) const {
    CMyComPtr< IInArchive > inArchive = openArchive( mLibrary, mFormat, in_file, *this );

    auto* extractCallbackSpec = new ExtractCallback( *this, inArchive, in_file, out_dir );

    CMyComPtr< IArchiveExtractCallback > extractCallback( extractCallbackSpec );
    if ( inArchive->Extract( nullptr, static_cast< UInt32 >( -1 ), false, extractCallback ) != S_OK ) {
        throw BitException( extractCallbackSpec->getErrorMessage() );
    }
}

void BitExtractor::extract( const wstring& in_file, vector< byte_t >& out_buffer, unsigned int index ) {
    CMyComPtr< IInArchive > inArchive = openArchive( mLibrary, mFormat, in_file, *this );

    auto* extractCallbackSpec = new MemExtractCallback( *this, inArchive, out_buffer );

    const UInt32 indices[] = { index };

    CMyComPtr< IArchiveExtractCallback > extractCallback( extractCallbackSpec );
    if ( inArchive->Extract( indices, 1, false, extractCallback ) != S_OK ) {
        throw BitException( extractCallbackSpec->getErrorMessage() );
    }
}

void BitExtractor::test( const wstring& in_file ) {
    CMyComPtr< IInArchive > inArchive = openArchive( mLibrary, mFormat, in_file, *this );

    auto* extractCallbackSpec = new ExtractCallback( *this, inArchive, in_file, L"" );

    CMyComPtr< IArchiveExtractCallback > extractCallback( extractCallbackSpec );
    if ( inArchive->Extract( nullptr, static_cast< UInt32 >( -1 ), true, extractCallback ) != S_OK ) {
        throw BitException( extractCallbackSpec->getErrorMessage() );
    }
}

/*
 * Kuumba C++ Core
 *
 * $Id: UUID.cpp 23364 2008-04-30 23:30:47Z tvk $
 */
#include <inc/core/Core.h>

#if defined(KCC_WINDOWS)
#   include "rpc.h"
#elif defined(KCC_LINUX)
#   include "uuid/uuid.h"
#endif

#define KCC_FILE "UUID"

namespace kcc
{
    //
    // UUIDDigest implementation
    //

    // clear: reset digest
    inline void UUIDDigest::clear() { std::memset(m_hash, 0, sizeof(m_hash)); }

    // digest: conversion of digest to hex
    void UUIDDigest::digest(String& hex) const
    {
        hex.clear();
        hex.reserve(sizeof(m_hash)*2+4);
        #if defined(KCC_WINDOWS)
            unsigned char* hash = NULL;
            ::UuidToString((::UUID*)m_hash, &hash);
            hex = (const kcc::Char*)hash;
            ::RpcStringFree(&hash);
        #elif defined(KCC_LINUX)
            char hash[65] = {0};
            ::uuid_unparse(*(::uuid_t*)&m_hash, hash);
            hex = (const kcc::Char*)hash;
        #endif
    }
    String UUIDDigest::digest() const
    {
        String hex;
        digest(hex);
        return hex;
    }

    // parse: parse hex digest into uuid
    UUIDDigest UUIDDigest::parse(const String& hex)
    {
        UUIDDigest d;
        #if defined(KCC_WINDOWS)
            ::UuidFromString((unsigned char*)hex.c_str(), (::UUID*)d.m_hash);
        #elif defined(KCC_LINUX)
            ::uuid_parse((char*)hex.c_str(), *(::uuid_t*)&d.m_hash);
        #endif
        return d;
    }
    
    //
    // UUID implementation
    //

    // generate: generate uuid
    void UUID::generate(UUIDDigest& d, bool clear)
    {
        if (clear) d.clear();
        #if defined(KCC_WINDOWS)
            ::UuidCreate((::UUID*)d.m_hash);
        #elif defined(KCC_LINUX)
            ::uuid_generate(*(::uuid_t*)&d.m_hash);
        #endif
    }
    UUIDDigest UUID::generate()
    {
        UUIDDigest d;
        UUID::generate(d, false);
        return d;
    }
}

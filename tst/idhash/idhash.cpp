/*
 * Copyright (c) 2004-2007 Umbria Inc.  All Rights Reserved.
 *
 * $Id: idhash.cpp 21187 2007-10-24 06:07:49Z tvk $
 */
#include <inc/core/Core.h>

#define KCC_FILE    "idhash"
#define KCC_VERSION "$Id: idhash.cpp 21187 2007-10-24 06:07:49Z tvk $"

// main: entry point into console application
int main(int argc, const char* argv[])
{
    // initialize kcc
    kcc::Properties props;
    props.set("kcc.logName",      KCC_FILE);
    props.set("kcc.logMax",       3L);
    props.set("kcc.logVerbosity", (long) kcc::Log::V_INFO_3);
    if (argc > 1) props.load(argc, argv, false);
    kcc::Core::init(props, KCC_VERSION);

    // run
    kcc::Log::Scope scope(KCC_FILE, "main");
    try
    {
        //kcc::String uuidhash("17d930d3-40d2-4ee6-a5f1-d9e7a92fd2cf");
        kcc::UUIDDigest uuid (kcc::UUID::generate());
        kcc::UUIDDigest uuid2(uuid.digest());
        std::cout 
            << "uuid: generate=" << uuid.digest() 
            << " parse=" << uuid2.digest() 
            << " equal=" << (uuid.digest() == uuid2.digest() ? "YES" : "NO")
            << "\n";
        
        kcc::MD5Digest md5   (kcc::MD5::hash("Hello, World!"));
        kcc::MD5Digest md5two(md5.digest());
        std::cout 
            << "md5: generate=" << md5.digest() 
            << " parse=" << md5two.digest() 
            << " equal=" << (md5.digest() == md5two.digest() ? "YES" : "NO")
            << "\n";
    }
    catch (std::bad_alloc& e)
    {
        kcc::Log::exception(e);
        return 2;
    }
    catch (std::exception& e)
    {
        kcc::Log::exception(e);
        return 1;
    }

    return 0;
}

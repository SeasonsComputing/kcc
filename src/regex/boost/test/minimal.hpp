//  (C) Copyright Gennadiy Rozental 2002.
//  Permission to copy, use, modify, sell and distribute this software
//  is granted provided this copyright notice appears in all copies.
//  This software is provided "as is" without express or implied warranty,
//  and with no claim as to its suitability for any purpose.

//  See http://www.boost.org for most recent version including documentation.
//
//  File        : $RCSfile: minimal.hpp,v $
//
//  Version     : $Id: minimal.hpp 15199 2007-03-09 17:57:17Z tvk $
//
//  Description : simple minimal testing definitions and implementation
// ***************************************************************************

#ifndef BOOST_TEST_MINIMAL_HPP
#define BOOST_TEST_MINIMAL_HPP

#define BOOST_CHECK(exp)       \
  ( (exp)                      \
      ? static_cast<void>(0)   \
      : boost::minimal_test::the_monitor->report_error(#exp,__FILE__,__LINE__, BOOST_CURRENT_FUNCTION) )

#define BOOST_REQUIRE(exp)     \
  ( (exp)                      \
      ? static_cast<void>(0)   \
      : boost::minimal_test::the_monitor->report_critical_error(#exp,__FILE__,__LINE__,BOOST_CURRENT_FUNCTION))

#define BOOST_ERROR( msg_ )    \
        boost::minimal_test::the_monitor->report_error( (msg_),__FILE__,__LINE__, BOOST_CURRENT_FUNCTION, true )
#define BOOST_FAIL( msg_ )     \
        boost::minimal_test::the_monitor->report_critical_error( (msg_),__FILE__,__LINE__, BOOST_CURRENT_FUNCTION, true )

//____________________________________________________________________________//

// deprecated interface

#define BOOST_TEST(predicate_)           BOOST_CHECK(predicate_)
#define BOOST_CRITICAL_TEST(predicate_)  BOOST_REQUIRE(predicate_)
#define BOOST_CRITICAL_ERROR(message_)   BOOST_FAIL(message_)

//____________________________________________________________________________//

// Boost.Test
#include <libs/test/src/execution_monitor.cpp>
#include <boost/test/detail/class_properties.hpp>

// BOOST
#include <boost/cstdlib.hpp>            // for exit codes
#include <boost/current_function.hpp>   // for BOOST_CURRENT_FUNCTION

// STL
#include <iostream>                     // std::cerr, std::endl
#include <string>                       // std::string

//____________________________________________________________________________//

int test_main( int argc, char* argv[] );  // prototype for user's test_main()

namespace boost {
namespace minimal_test {

class monitor : public boost::execution_monitor {
    typedef unit_test_framework::c_string_literal c_string_literal;
public:
    // constructor
    monitor( int argc, char** argv )
    : p_errors_counter( 0 ), m_argc( argc ), m_argv( argv ) {}

    // execution monitor hook implementation
    virtual int function()
    {
        return test_main( m_argc, m_argv );
    }

    void        report_error( const char* msg_, const char* file_, int line_, c_string_literal func_name_, bool is_msg_ = false )
    {
        ++p_errors_counter.value;
        std::cerr << file_ << "(" << line_ << "): ";
        
        if( is_msg_ )
            std::cerr << msg_;
        else
            std::cerr << "test " << msg_ << " failed";

        if( std::string( func_name_ ) != "(unknown)" )
            std::cerr << " in function: '" << func_name_ << "'";
        
        std::cerr << std::endl;
    }

    void        report_critical_error( const char* msg_, const char* file_, int line_, c_string_literal func_name_, bool is_msg_ = false )
    {
        report_error( msg_, file_, line_, func_name_, is_msg_ );
        throw boost::execution_exception( boost::execution_exception::no_error, "" );
    }

    // public properties
    BOOST_READONLY_PROPERTY( int, 1, ( monitor ) ) p_errors_counter;

private:
    // Data members
    int         m_argc;
    char**      m_argv;
}; // monitor

monitor* the_monitor;

} // namespace minimal_test
} // namespace boost

//____________________________________________________________________________//

int main( int argc, char* argv[] )
{
    using boost::minimal_test::the_monitor;

    the_monitor = new boost::minimal_test::monitor( argc, argv );

    try {
        int run_result = boost::minimal_test::the_monitor->execute();

        BOOST_CHECK( run_result == 0 || run_result == boost::exit_success );
    }
    catch( boost::execution_exception const& exex ) {
        if( exex.code() != boost::execution_exception::no_error )
            BOOST_ERROR( (std::string( "exception \"" ) + exex.what() + "\" caught").c_str() );
        std::cerr << "\n**** Testing aborted.";
    }

    if( boost::minimal_test::the_monitor->p_errors_counter != 0 ) {
        std::cerr << "\n**** " << the_monitor->p_errors_counter.get()
                  << " error" << (the_monitor->p_errors_counter > 1 ? "s" : "" ) << " detected\n";

        delete the_monitor;
        return boost::exit_test_failure;
    }

    std::cout << "\n**** no errors detected\n";
    
    delete the_monitor;
    return boost::exit_success;
}

// ***************************************************************************
//  Revision History :
//  
//  $Log: minimal.hpp,v $
//  Revision 1.1  2003/10/16 23:15:02  tedk
//  baseline checkin
//
//  Revision 1.7  2002/12/08 17:41:57  rogeeff
//  switched to use c_string_literal
//
//  Revision 1.6  2002/11/02 19:31:04  rogeeff
//  merged into the main trank
//

// ***************************************************************************


#endif // BOOST_TEST_MINIMAL_HPP

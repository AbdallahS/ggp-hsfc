/***************************************************************************
 * Example of the HSFC in action. Simply pick the first joint move and
 * play to termination.
 ****************************************************************************/

#include <iostream>
#include <vector>
#include <algorithm>
#include <exception>
#include <boost/unordered_map.hpp>
#include <boost/foreach.hpp>
#include <boost/exception/all.hpp>
#include <boost/algorithm/string/join.hpp>
#include <hsfc/hsfc.h>

typedef boost::error_info<struct tag_my_info,std::string> errinfo; 
struct someerror: virtual boost::exception, virtual std::exception { }; 

/***************************************************************************
 * Run the game to termination
 **************************************************************************/

void run(const std::string& gdlfilename, bool verbose=false)
{
    HSFC::Game game(boost::filesystem::path(gdlfilename.c_str()));
    if (verbose) std::cout << "Loaded: " << gdlfilename << std::endl;
    HSFC::State state(game);
    while (!state.isTerminal())
    {
        std::vector<HSFC::JointMove> jms = state.joints();
        if (jms.empty())
            BOOST_THROW_EXCEPTION(someerror() << errinfo("No joint moves in non-terminal state"));
        HSFC::JointMove firstmove = jms[0];
        if (verbose) std::cout << "Playing: " << firstmove << std::endl;
        state.play(firstmove);
    }
    HSFC::JointGoal result = state.goals();
    if (verbose) std::cout << "Result: " << result << std::endl;
}

/***************************************************************************
 * Main
 **************************************************************************/

int main(int argc, char* argv[])
{
    try
    {
        if (argc < 2 || argc > 3)
        {
            std::cout << "usage: <gdlfilename> [--verbose]" << std::endl;
            exit(0);
        }
        std::string gdlfilename(argv[1]);
        if (argc == 2) run(gdlfilename);
        else run(gdlfilename, true);
        return 0;
    }
    catch(someerror& e)
    {
        if( std::string const * mi=boost::get_error_info<errinfo>(e) )
            std::cerr << "Error: " << *mi << std::endl;
        return 1;
    }
    catch(HSFC::HSFCException& e)
    {
        if( std::string const * mi=boost::get_error_info<HSFC::ErrorMsgInfo>(e) )
            std::cerr << "Error: " << *mi << std::endl;
        return 1;
    }
    catch(boost::exception& e)
    {
        
        return 1;
    }
    catch(std::exception& e)
    {
        std::cerr << "std::exception: " << e.what() << std::endl;
        return 1;
    }
    catch(...)
    {
        std::cerr << "Unknown error" << std::endl;
        return 2;
    }
}

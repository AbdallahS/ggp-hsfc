/***************************************************************************
 * Perform a set number of random playouts from the initial state.
 * Prints out some statistics.
 ****************************************************************************/

#include <iostream>
#include <vector>
#include <algorithm>
#include <exception>
#include <time.h>
#include <boost/unordered_map.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <boost/exception/all.hpp>
#include <boost/algorithm/string/join.hpp>
#include <hsfc/hsfc.h>

typedef boost::error_info<struct tag_my_info,std::string> errinfo;
struct someerror: virtual boost::exception, virtual std::exception { };

/***************************************************************************
 * difference in timespec
 **************************************************************************/
timespec timespec_diff(timespec start, timespec end)
{
    timespec temp;
    if ((end.tv_nsec-start.tv_nsec)<0) {
        temp.tv_sec = end.tv_sec-start.tv_sec-1;
        temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
    } else {
        temp.tv_sec = end.tv_sec-start.tv_sec;
        temp.tv_nsec = end.tv_nsec-start.tv_nsec;
    }
    return temp;
}

timespec timespec_add(timespec t1, timespec t2)
{
    timespec temp;
    temp.tv_sec = t1.tv_sec + t2.tv_sec;
    unsigned long i = t1.tv_nsec + t2.tv_nsec;
    temp.tv_nsec = i;
    if (i >= 1000000000) {
        temp.tv_nsec = i%1000000000;
        temp.tv_sec += i/1000000000;
    }
    return temp;
}


std::ostream& operator<<(std::ostream& os, const timespec &t)
{
    return os << t.tv_sec << ":" << t.tv_nsec;
}

/***************************************************************************
 * Run the game to termination
 **************************************************************************/

void run(const std::string& gdlfilename, unsigned int numplayouts, bool verbose=false)
{
    HSFC::Game game(boost::filesystem::path(gdlfilename.c_str()), false);
    if (verbose) std::cout << "Loaded: " << gdlfilename << std::endl;
    HSFC::State base_state(game);
    HSFC::JointGoal goals;
    unsigned int totalmoves = 0;
    timespec totaltime;
    totaltime.tv_sec = 0;
    totaltime.tv_nsec = 0;
    for (unsigned int i = 0; i < numplayouts; ++i)
    {
        HSFC::State state(base_state);
        timespec time1, time2, timediff;
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
        state.playout(std::inserter(goals, goals.begin()));
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
        timediff = timespec_diff(time1, time2);
        totaltime = timespec_add(totaltime, timediff);
        totalmoves += state.internal().Round;
        if (verbose) std::cout << "Finished playout " << i << " with "
                               << state.internal().Round << " steps in "
                               << timediff << " seconds" << std::endl;

    }
    unsigned int totalmsec =  (totaltime.tv_nsec/1000000) + (totaltime.tv_sec*1000);

    if (verbose)
    {
        std::cout << "Total num moves: " << totalmoves << " in "
                  << totaltime << " seconds (" << totalmsec << " msec)" << std::endl;
        std::cout << "Average moves per second: " << (float)totalmoves/((float)totalmsec/1000)
                  << std::endl;
    }
}

/***************************************************************************
 * Main
 **************************************************************************/

int main(int argc, char* argv[])
{
    try
    {
        if (argc < 3 || argc > 4)
        {
            std::cout << "usage: <gdlfilename> <numplayout> [--verbose]" << std::endl;
            exit(0);
        }
        std::string gdlfilename(argv[1]);
        unsigned int numplayouts = boost::lexical_cast<unsigned int>(argv[2]);

        if (argc == 2) run(gdlfilename, numplayouts);
        else run(gdlfilename, numplayouts, true);
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

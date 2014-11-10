/***************************************************************************
 * For a given game (GDL), run random playouts for a fixed period of
 * time.  Then advance the game state using a randomly picked joint
 * move. Repeat the process until the game terminates.
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

void run(const std::string& gdlfilename, unsigned int round_length)
{
    std::cerr << "Loading GDL: " << gdlfilename << std::endl;
    HSFC::Game game(boost::filesystem::path(gdlfilename.c_str()), false);
    std::cerr << "Play clock: " << round_length << " seconds" << std::endl;
    HSFC::State base_state(game);
    HSFC::JointGoal goals;
    unsigned int numjm=0;
    unsigned int roundnum = 1;
    std::cerr << "Running..." << std::endl;
    while (!base_state.isTerminal())
    {
        timespec time1, time2, timediff;
        unsigned int nummoves = 0;
        unsigned int numplayouts = 0;
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
        bool done = false;
        while (!done)
        {
            HSFC::State state(base_state);
            goals = state.playout();
            nummoves += state.internal().Round;
            ++numplayouts;
            clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
            timediff = timespec_diff(time1, time2);
            if (timediff.tv_sec >= round_length) done = true;
        }
        unsigned int msec =  (timediff.tv_nsec/1000000) + (timediff.tv_sec*1000);
        float sec = ((float)msec)/1000.0;
        std::cout << "Round " << roundnum++ << ": Ran " << numplayouts << " playouts ("
                  << nummoves << " actual moves) in " << sec
                  << " seconds. " << " Ave moves per second: " << ((float)nummoves)/sec
                  << std::endl;

        std::vector<HSFC::JointMove> jms = base_state.joints();
        base_state.play(jms[0]);
        ++numjm;
    }
    std::cout << "Number of played joint moves to termination: " << numjm << std::endl;
}

/***************************************************************************
 * Main
 **************************************************************************/

int main(int argc, char* argv[])
{
    try
    {
        if (argc != 3)
        {
            std::cout << "usage: <gdlfilename> <play-clock>" << std::endl << std::endl;
            std::cout << "Run a game with a fixed play clock, successively "<< std::endl
                      << "picking the first joint move until the game terminates." << std::endl
                      << "Note: to provide a more accurate measure the play clock " << std::endl
                      << "is calculated based on CPU time and not on real time." << std::endl;
            exit(0);
        }

        if (argc == 3)
        {
            std::string gdlfilename(argv[1]);
            unsigned int round_length = boost::lexical_cast<unsigned int>(argv[2]);
            run(gdlfilename, round_length);
        }
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

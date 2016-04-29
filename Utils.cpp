#include "Utils.h"

//Init Log stuffs
std::ostream* Log::sOstream = &(std::cout);

boost::mutex Log::Mux::V;
boost::mutex Log::Mux::D;
boost::mutex Log::Mux::E;
boost::mutex Log::Mux::W;

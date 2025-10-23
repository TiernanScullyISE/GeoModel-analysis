/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef GEMODELREAD_GUARDEDPRINT_H
#define GEMODELREAD_GUARDEDPRINT_H
#include <mutex>
#include <thread>
namespace GeoModelIO {
    inline std::mutex& printMutex() {
        static std::mutex mutex{};
        return mutex;
    }
}

#define PRINT_MSG(MSG)                                      \
     {                                                      \
        std::lock_guard guard{GeoModelIO::printMutex()};    \
        std::cout<<MSG<<std::endl;                          \
     }

#define PRINT_LOG_MSG(MSG)                                         \
     {                                                             \
        std::lock_guard guard{GeoModelIO::printMutex()};           \
        std::thread::id this_id = std::this_thread::get_id();      \
        std::cout<<__func__<<"() - "<<__LINE__                     \
                 <<" (thread: "<<this_id<<") :"<<MSG<<std::endl;   \
     }

#define PRINT_ERR_MSG(MSG)                                         \
     {                                                             \
        std::lock_guard guard{GeoModelIO::printMutex()};           \
        std::thread::id this_id = std::this_thread::get_id();      \
        std::cerr<<__func__<<"() - "<<__LINE__                     \
                 <<" (thread: "<<this_id<<") :"<<MSG<<std::endl;   \
     }

#endif